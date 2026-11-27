#include <algorithm>
#include <array>
#include <optional>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <string_view>
#include <vector>
#include "nact.h"
#include "ags.h"
#include "encoding.h"
#include "fileio.h"
#include "mako.h"

#define D01 var[1]
#define D02 var[2]
#define D03 var[3]
#define D04 var[4]

namespace {

class NACT_Patton : public NACT_Sys3 {
public:
	NACT_Patton(const Config& config, const GameId& game_id)
		: NACT_Sys3(config, game_id) {
		text_wait_time = 0;	// Y 13 and Y 14 set this
		box[0] = {0, 16, 8, 367, 295};
		box[1] = {0, 40, 0, 599, 298};
		load_executable_data();
		amap.open("AMAP.DAT");
		composed_map = ags->create_offscreen(VIEW_COLS * TILE_SIZE, VIEW_ROWS * TILE_SIZE);
		map_base.fill(0);
		map_layer_z.fill(CELL_EMPTY);
		map_layer_w.fill(CELL_EMPTY);
	}

private:
	void cmd_g() override {
		int page = cali();

		TRACE("G %d:", page);

		stop_long_pcm();
		load_cg(page, -1);
		map_palette_pending = true;
	}

	// --- Y command dispatch -----------------------------------------------

	void cmd_y() override {
		int cmd = cali();
		int param = cali();

		TRACE("Y %d,%d:", cmd, param);

		switch (cmd) {
		// --- Game specific ---
		case 11:
			draw_status_panel(param);
			break;
		case 13:
			set_text_wait(param == 2 ? 3 : 0);
			break;
		case 14:
			set_text_wait(param == 0 ? 0 : param * 16 / 10);
			break;
		case 25:
			if (0 <= param && param <= 3) {
				menu_style.back_color = text_style.back_color =
					(param == 0) ? 1 : 0;
				text_window = param + 1;
				open_text_window(text_window, false);
			}
			break;
		case 26:
			if (0 <= param && param <= 4)
				menu_window = param + 1;
			menu_style.back_color = (param == 0) ? 1 : 0;
			break;
		case 50:
			map_sprite_cg = load_map_sheet(param);
			break;
		case 51:
		case 52:
		case 53:
			load_map_tile_bank(cmd - 51, param);
			break;
		case 55:
			switch_map_area(param);
			break;
		case 70:
			if (param == 1) {
				cg_flags &= ~CG_EXTRACT_PALETTE;
				ags->copy_screen(SCREEN_FRONT, SCREEN_BACK, 0, 0,
					ags->screen_width - 1, ags->screen_height - 1, 0, 0);
				ags->dest_screen = SCREEN_BACK;
			} else if (param == 0) {
				ags->copy_screen(SCREEN_BACK, SCREEN_FRONT, 0, 0,
					ags->screen_width - 1, ags->screen_height - 1, 0, 0);
				ags->apply_palette(0, 16);
				cg_flags |= CG_EXTRACT_PALETTE;
				ags->dest_screen = SCREEN_FRONT;
			}
			break;
		case 83:
			play_pcm_and_wait(param);
			break;
		case 84:
			play_pcm_slot(param);
			break;
		case 85:
		case 86:
		case 87:
		case 88:
		case 89:
			register_pcm_slot(cmd - 85, param);
			break;
		case 111:
			key_panel_cg_page = param;
			key_panel_cg = CG();
			key_last_mode = 0;
			break;
		case 112:
			map_tile_bank_3_slot = param;
			break;
		case 114:
			if (param > 0) {
				int index = param - 1;
				int left = 16 + index % 11 * 32;
				int top = 8 + index / 11 * 32;
				ags->box_fill(ags->dest_screen, left, top, left + 31, top + 31, 0);
			}
			break;
		case 115:
			load_long_pcm(param);
			break;
		case 116:
			play_long_pcm(param);
			break;
		case 255:
			quit(0);
			break;

		// --- Shared with generic System3 ---
		case 1:    // clear message window
		case 2:    // clear variable group
		case 3:    // wait with timeout
		case 4:    // random number
		case 7:    // clear a registered box
		case 8:    // select string variable slot
		case 40:   // fade in
		case 41:   // fade out
		case 42:   // fade in
		case 43:   // fade out
			exec_y(cmd, param);
			break;

		default:
			TRACE_UNIMPLEMENTED("Y %d,%d:", cmd, param);
			break;
		}
	}

	// --- data read out of ADVT2.COM ---------------------------------------

	static constexpr const char* EXECUTABLE_NAME = "ADVT2.COM";

	void load_executable_data() {
		auto fio = FILEIO::open(EXECUTABLE_NAME, FILEIO_READ_BINARY);
		if (!fio) {
			WARNING("Cannot open %s", EXECUTABLE_NAME);
			return;
		}
		if (fio->seek(DIGIT_FONT_OFFSET, SEEK_SET) ||
			!fio->read(digit_font, sizeof(digit_font))) {
			WARNING("%s: too short", EXECUTABLE_NAME);
		}
		if (fio->seek(ICON_DIGIT_FONT_OFFSET, SEEK_SET) ||
			!fio->read(icon_digit_font, sizeof(icon_digit_font))) {
			WARNING("%s: too short", EXECUTABLE_NAME);
		}
		uint8_t cursor_data[MOUSE_CURSOR_BYTES];
		if (fio->seek(MOUSE_CURSOR_OFFSET, SEEK_SET) ||
			!fio->read(cursor_data, sizeof(cursor_data))) {
			WARNING("%s: too short", EXECUTABLE_NAME);
		} else {
			load_mouse_cursor(cursor_data);
		}
	}

	// --- timing -----------------------------------------------------------

	static int ticks_to_ms(int ticks) { return ticks * 1000 / 60; }

	// --- PCM sound effects ------------------------------------------------

	std::array<int, 5> pcm_page = {};
	int long_pcm_page = 0;

	// Y 83: load a page, play it once, and wait until playback finishes.
	void play_pcm_and_wait(int page) {
		mako->stop_pcm();
		mako->play_pcm(page, 1);
		while (!terminate && mako->check_pcm())
			sys_sleep(16);
	}

	// Y 84 0: stop playback
	// Y 84 slot: play a registered slot once without waiting for it to finish.
	void play_pcm_slot(int slot) {
		mako->stop_pcm();
		if (slot == 0)
			return;
		if (slot < 1 || slot > static_cast<int>(pcm_page.size())) {
			WARNING("Y 84: PCM slot %d is out of range", slot);
			return;
		}
		if (pcm_page[slot - 1])
			mako->play_pcm(pcm_page[slot - 1], 1);
	}

	// Y 85--89: register an AMUS.DAT page in one of 5 slots.  Passing
	// page zero clears all the 5 slots.
	void register_pcm_slot(int slot, int page) {
		if (page == 0)
			pcm_page.fill(0);
		else
			pcm_page[slot] = page;
	}

	// Y 115
	void load_long_pcm(int page) {
		mako->stop_pcm();
		long_pcm_page = page;
	}

	// Y 116
	void play_long_pcm(int count) {
		int loops = count & 0xff;
		mako->play_pcm(long_pcm_page, std::max(1, loops));
	}

	void stop_long_pcm() {
		if (long_pcm_page) {
			long_pcm_page = 0;
			mako->stop_pcm();
		}
	}

	// --- screens and CG loading -------------------------------------------

	// Palette index the color keyed blits treat as transparent.
	static constexpr int TRANSPARENT_COLOR = 7;

	void load_cg_to_work_screen(int page) {
		ScreenId saved = ags->dest_screen;
		ags->dest_screen = SCREEN_BACK;
		load_cg(page, -1);
		ags->dest_screen = saved;
	}

	// --- text -------------------------------------------------------------

	void cmd_p() override {
		int p1 = cali();
		TRACE("P %d:", p1);
		text_style.font_color = p1;
	}

	// The per-character wait of Y 13 and Y 14, in 1/100 s units.
	void set_text_wait(int centiseconds) {
		text_wait_time = centiseconds * 10;
	}

	// --- map cells --------------------------------------------------------

	// The map is 100x100 cells. Cell data lives in three parallel planes: the
	// 16-bit base map loaded from AMAP.DAT, and the two 8-bit overlay layers
	// written by Z and W, where 0xff means "empty".
	static constexpr int MAP_WIDTH = 100;
	static constexpr int MAP_HEIGHT = 100;
	static constexpr int MAP_CELLS = MAP_WIDTH * MAP_HEIGHT;
	static constexpr uint8_t CELL_EMPTY = 0xff;

	Dri amap;
	std::array<uint16_t, MAP_CELLS> map_base;
	std::array<uint8_t, MAP_CELLS> map_layer_z;
	std::array<uint8_t, MAP_CELLS> map_layer_w;

	int map_current_area = 0;

	// Writes the base map cell at (x, y) from D01-D04.
	void cmd_b() override {
		uint16_t x = cali();
		uint16_t y = cali();
		TRACE("B %d,%d:", x, y);

		int index = cell_index(x, y);
		if (index < 0)
			return;
		uint8_t high = ((D01 & 1) << 6) | ((D02 & 1) << 5) | (D04 & 0x1f);
		map_base[index] = (high << 8) | (D03 & 0xff);
	}

	// Reads the map cell at (x, y) into D01-D04, W and Z overlays taking
	// precedence over the base map.
	void cmd_n() override {
		uint16_t x = cali();
		uint16_t y = cali();
		TRACE("N %d,%d:", x, y);

		int index = cell_index(x, y);
		if (index < 0)
			return;
		if (map_layer_w[index] != CELL_EMPTY) {
			D01 = 1;
			D02 = 1;
			D03 = map_layer_w[index];
			D04 = 64;
		} else if (map_layer_z[index] != CELL_EMPTY) {
			D01 = 1;
			D02 = 1;
			D03 = map_layer_z[index];
			D04 = 32;
		} else {
			uint16_t cell = map_base[index];
			D01 = (cell >> 14) & 1;
			D02 = (cell >> 13) & 1;
			D03 = cell & 0xff;
			D04 = cell & 0x1f;
		}
	}

	void cmd_w() override {
		uint16_t x = cali();
		uint16_t y = cali();
		uint16_t value = cali();
		TRACE("W %d,%d,%d:", x, y, value);
		write_map_layer(map_layer_w, x, y, value);
	}

	void cmd_z() override {
		uint16_t x = cali();
		uint16_t y = cali();
		uint16_t value = cali();
		TRACE("Z %d,%d,%d:", x, y, value);
		write_map_layer(map_layer_z, x, y, value);
	}

	int cell_index(uint16_t x, uint16_t y) {
		uint16_t index = static_cast<uint16_t>(y * MAP_WIDTH + x);
		if (index >= MAP_CELLS) {
			WARNING("map cell (%d,%d) is out of range", x, y);
			return -1;
		}
		return index;
	}

	// The overlay layers always block; the base map blocks on bit 13.
	bool cell_is_passable(int x, int y) {
		int index = cell_index(static_cast<uint16_t>(x), static_cast<uint16_t>(y));
		if (index < 0)
			return false;
		if (map_layer_w[index] != CELL_EMPTY || map_layer_z[index] != CELL_EMPTY)
			return false;
		return !((map_base[index] >> 13) & 1);
	}

	void write_map_layer(std::array<uint8_t, MAP_CELLS>& layer, uint16_t x, uint16_t y, uint16_t value) {
		// 254 clears the whole layer
		if (value == 254) {
			layer.fill(CELL_EMPTY);
			return;
		}
		int index = cell_index(x, y);
		if (index < 0)
			return;
		layer[index] = value & 0xff;
	}

	void clear_map_layers() {
		map_layer_z.fill(CELL_EMPTY);
		map_layer_w.fill(CELL_EMPTY);
	}

	void load_amap_page(int page) {
		std::vector<uint8_t> data = amap.load(page);
		if (data.size() < MAP_CELLS * 2)
			ERROR("AMAP page %d is missing or too short (%zu bytes)", page, data.size());
		for (int i = 0; i < MAP_CELLS; i++)
			map_base[i] = data[i * 2] | (data[i * 2 + 1] << 8);
	}

	// Y 55
	void switch_map_area(int map_id) {
		map_current_area = map_id;

		if (map_id == 0) {
			// Fill the direction key panel area with color 1.
			ags->box_fill(SCREEN_FRONT, 16, 304, 103, 391, 1);
		} else if (map_id != 255) {
			// 255 selects the map id without loading a map.
			load_amap_page(map_id);
			clear_map_layers();
			map_out_of_bounds_tile = CELL_EMPTY;
		}
	}

	// --- map view ---------------------------------------------------------

	// J draws 11x9 cells of 32x32 pixels into (16,8)-(367,295).
	static constexpr int VIEW_COLS = 11;
	static constexpr int VIEW_ROWS = 9;
	static constexpr int VIEW_X = 16;
	static constexpr int VIEW_Y = 8;
	static constexpr int TILE_SIZE = 32;

	// A CG loaded by Y 50-53 holds an 8-wide grid of 32x32 tiles.
	static constexpr int SHEET_COLS = 8;
	static constexpr int TILES_PER_BANK = 64;
	static constexpr int NR_TILE_BANKS = 3;
	static constexpr int NR_SPRITE_FRAMES = 8;
	// T ...,255 leaves no sprite to draw.
	static constexpr uint8_t SPRITE_HIDDEN = 0xff;

	static constexpr int ANIMATION_FRAME_MS = 16;

	uint16_t map_view_x = 0;
	uint16_t map_view_y = 0;
	uint16_t map_sprite_x = 0;
	uint16_t map_sprite_y = 0;
	uint8_t map_sprite_frame = SPRITE_HIDDEN;
	uint8_t map_out_of_bounds_tile = CELL_EMPTY;
	// The view and sprite state last drawn, for step detection.
	uint16_t map_view_drawn_x = 0;
	uint16_t map_view_drawn_y = 0;
	uint16_t map_sprite_drawn_x = 0;
	uint16_t map_sprite_drawn_y = 0;
	bool map_sprite_drawn = false;
	bool map_view_drawn = false;
	// The CG page last loaded by Y 51-53. The first J after a G restores its
	// palette.
	int map_palette_page = 0;
	bool map_palette_pending = false;
	// The J draw wait, in 1/60 s ticks.
	int map_animation_ticks = 9;

	CG composed_map;

	// The sheets loaded by Y 50-53, kept whole rather than split into per-tile images.
	CG map_sprite_cg;
	CG map_tile_cg[NR_TILE_BANKS];
	uint16_t map_tile_bank_3_slot = 0;

	void cmd_j() override {
		map_view_x = cali();
		map_view_y = cali();
		TRACE("J %d,%d:", map_view_x, map_view_y);
		draw_map_view();
	}

	// T x,y,direction: puts the sprite on map cell (x,y) and picks its
	// animation frame from the direction; 255 hides it. The next J draws it.
	void cmd_t() override {
		uint16_t x = cali();
		uint16_t y = cali();
		uint16_t direction = cali();  // 1, 2, 4, or 8
		TRACE("T %d,%d,%d:", x, y, direction);

		// Even base frame per direction 1-8. Repeating a direction alternates
		// between the even and odd frame; the parity is shared by all
		// directions and restarts even after 255.
		static const uint8_t direction_frames[8] = {0, 2, 0, 4, 0, 0, 0, 6};

		map_sprite_x = x;
		map_sprite_y = y;
		if (direction == 0 || direction == 255) {
			map_sprite_frame = static_cast<uint8_t>(direction);
		} else if (direction >= 9) {
			map_sprite_frame = 0;
		} else {
			uint8_t frame = direction_frames[direction - 1];
			if ((map_sprite_frame & 1) == 0)
				frame++;
			map_sprite_frame = frame;
		}
	}

	void draw_map_view() {
		if (map_palette_pending && map_palette_page)
			ags->load_cg_surface(map_palette_page, -1, CG_GET_PALETTE | CG_EXTRACT_PALETTE);
		map_palette_pending = false;

		int sprite_step_x = static_cast<int>(map_sprite_x) - map_sprite_drawn_x;
		int sprite_step_y = static_cast<int>(map_sprite_y) - map_sprite_drawn_y;
		if (!can_animate_sprite(sprite_step_x, sprite_step_y))
			sprite_step_x = sprite_step_y = 0;

		draw_map_cells();
		animate_map_view(sprite_step_x, sprite_step_y);

		map_view_drawn_x = map_view_x;
		map_view_drawn_y = map_view_y;
		map_sprite_drawn_x = map_sprite_x;
		map_sprite_drawn_y = map_sprite_y;
		map_sprite_drawn = map_sprite_frame != SPRITE_HIDDEN;
		map_view_drawn = true;
	}

	static bool is_single_step(int step) { return -1 <= step && step <= 1; }

	// The sprite walks only when it stays visible, the view does not move,
	// and it moves at most one cell along each axis.
	bool can_animate_sprite(int sprite_step_x, int sprite_step_y) {
		return map_view_drawn && map_sprite_drawn &&
			   map_sprite_frame != SPRITE_HIDDEN &&
			   map_view_x == map_view_drawn_x && map_view_y == map_view_drawn_y &&
			   is_single_step(sprite_step_x) && is_single_step(sprite_step_y);
	}

	// Composes the 11x9 cells centered on (map_view_x, map_view_y).
	void draw_map_cells() {
		int cell_x0 = map_view_x - VIEW_COLS / 2;
		int cell_y0 = map_view_y - VIEW_ROWS / 2;
		for (int row = 0; row < VIEW_ROWS; row++) {
			for (int col = 0; col < VIEW_COLS; col++) {
				int cell_x = cell_x0 + col;
				int cell_y = cell_y0 + row;
				bool inside = 0 <= cell_x && cell_x < MAP_WIDTH &&
							  0 <= cell_y && cell_y < MAP_HEIGHT;
				int offset = cell_y * MAP_WIDTH + cell_x;

				uint8_t base = inside ? (map_base[offset] & 0xff) : map_out_of_bounds_tile;
				uint8_t layer_z = inside ? map_layer_z[offset] : CELL_EMPTY;
				uint8_t layer_w = inside ? map_layer_w[offset] : CELL_EMPTY;

				int px = col * TILE_SIZE;
				int py = row * TILE_SIZE;
				if (base == CELL_EMPTY) {
					SDL_Rect rect = { px, py, TILE_SIZE, TILE_SIZE };
					sdl::FillSurfaceRect(composed_map.surface(), &rect, 0);
				} else {
					draw_map_tile(base, px, py, false);
				}
				if (layer_z != CELL_EMPTY)
					draw_map_tile(layer_z, px, py, false);
				if (layer_w != CELL_EMPTY)
					draw_map_tile(layer_w, px, py, true);
			}
		}
	}

	// The pixel position of something that moves `step` cells and arrives at
	// `to`, at animation frame `frame` of `frames`.
	static int step_position(int to, int step, int frame, int frames) {
		return to - step * TILE_SIZE * (frames - frame) / frames;
	}

	// Draws the composed view and the sprite on top of it, moving the sprite
	// from where it was drawn last to where it is now.
	void animate_map_view(int sprite_step_x, int sprite_step_y) {
		int duration = ticks_to_ms(map_animation_ticks);
		int frames = std::clamp(duration / ANIMATION_FRAME_MS, 2, TILE_SIZE);
		// Pixel position of the sprite relative to the view.
		int sprite_x = (map_sprite_x - map_view_x + VIEW_COLS / 2) * TILE_SIZE;
		int sprite_y = (map_sprite_y - map_view_y + VIEW_ROWS / 2) * TILE_SIZE;

		uint32_t start = SDL_GetTicks();
		for (int frame = 1; frame <= frames && !terminate; frame++) {
			ags->blit_cg(SCREEN_FRONT, composed_map, nullptr, VIEW_X, VIEW_Y);
			if (map_sprite_frame != SPRITE_HIDDEN) {
				int sx = step_position(sprite_x, sprite_step_x, frame, frames);
				int sy = step_position(sprite_y, sprite_step_y, frame, frames);
				draw_map_sprite(map_sprite_frame, VIEW_X + sx, VIEW_Y + sy);
			}
			uint32_t deadline = start + duration * frame / frames;
			uint32_t now = SDL_GetTicks();
			if (now < deadline)
				sys_sleep(deadline - now);
		}
	}

	CG load_map_sheet(int page) {
		if (page == 0)
			return CG();
		CG cg = ags->load_cg_surface(page, TRANSPARENT_COLOR, cg_flags);
		if (!cg)
			ERROR("map resource error: CG page %d not available", page);
		return cg;
	}

	// Y 51--53 load one of the three tile banks.  The first two always replace
	// all 64 tiles.  For Y 53, Y 112,0 does the same, while Y 112,1--16
	// selects one of the third bank's 4-tile slots.
	void load_map_tile_bank(int bank, int page) {

		CG cg = load_map_sheet(page);
		if (!cg)
			return;
		map_palette_page = page;
		if (bank != 2 || map_tile_bank_3_slot == 0) {
			map_tile_cg[bank] = std::move(cg);
			return;
		}
		if (map_tile_bank_3_slot > 16) {
			WARNING("Y 53: tile slot %d is out of range", map_tile_bank_3_slot);
			return;
		}

		if (!map_tile_cg[bank]) {
			map_tile_cg[bank] = ags->create_offscreen(
				SHEET_COLS * TILE_SIZE, SHEET_COLS * TILE_SIZE);
			sdl::FillSurfaceRect(map_tile_cg[bank].surface(), nullptr, 0);
		}

		int first_tile = (map_tile_bank_3_slot - 1) * 4;
		int dx = first_tile % SHEET_COLS * TILE_SIZE;
		int dy = first_tile / SHEET_COLS * TILE_SIZE;
		blit_sheet(map_tile_cg[bank], cg, 0, 0, 4 * TILE_SIZE, TILE_SIZE,
				   dx, dy, false);
	}

	void draw_map_tile(uint8_t tile, int px, int py, bool transparent) {
		int bank = tile / TILES_PER_BANK;
		if (bank >= NR_TILE_BANKS || !map_tile_cg[bank]) {
			WARNING("map tile %d has no loaded bank", tile);
			return;
		}
		int index = tile % TILES_PER_BANK;
		blit_sheet(composed_map, map_tile_cg[bank],
				   (index % SHEET_COLS) * TILE_SIZE,
				   (index / SHEET_COLS) * TILE_SIZE,
				   TILE_SIZE, TILE_SIZE, px, py, transparent);
	}

	// Draws a sprite into the view of screen 0, clipped at the view border.
	void draw_map_sprite(uint8_t frame, int px, int py) {
		if (frame >= NR_SPRITE_FRAMES || !map_sprite_cg) {
			WARNING("map sprite frame %d has no loaded CG", frame);
			return;
		}
		int left = std::max(px, VIEW_X);
		int top = std::max(py, VIEW_Y);
		int right = std::min(px + TILE_SIZE, VIEW_X + VIEW_COLS * TILE_SIZE);
		int bottom = std::min(py + TILE_SIZE, VIEW_Y + VIEW_ROWS * TILE_SIZE);
		if (left >= right || top >= bottom)
			return;
		blit_sheet(SCREEN_FRONT, map_sprite_cg, frame * TILE_SIZE + (left - px), top - py,
				   right - left, bottom - top, left, top, true);
	}

	// Returns the rect of a sheet cell, and sets the color key the blit needs.
	// Returns nullopt if the cell is outside the sheet.
	std::optional<SDL_Rect> sheet_cell(CG& cg, int sheet_x, int sheet_y,
									   int width, int height, bool transparent) {
		if (sheet_x + width > cg.width() || sheet_y + height > cg.height()) {
			WARNING("map resource error: (%d,%d)+%dx%d outside CG %dx%d",
				  sheet_x, sheet_y, width, height, cg.width(), cg.height());
			return std::nullopt;
		}
		sdl::SetSurfaceColorKey(cg.surface(), transparent, TRANSPARENT_COLOR);
		return SDL_Rect{ sheet_x, sheet_y, width, height };
	}

	void blit_sheet(ScreenId dest, CG& cg, int sheet_x, int sheet_y,
					int width, int height, int dx, int dy, bool transparent) {
		if (auto src = sheet_cell(cg, sheet_x, sheet_y, width, height, transparent))
			ags->blit_cg(dest, cg, &*src, dx, dy);
	}

	void blit_sheet(CG& dest, CG& cg, int sheet_x, int sheet_y,
					int width, int height, int dx, int dy, bool transparent) {
		if (auto src = sheet_cell(cg, sheet_x, sheet_y, width, height, transparent))
			ags->blit_cg(dest, cg, &*src, dx, dy);
	}

	// --- direction key input ----------------------------------------------

	// The direction key panel occupies (16,304)-(103,391) on screen.
	static constexpr int KEY_PANEL_X = 16;
	static constexpr int KEY_PANEL_Y = 304;
	static constexpr int KEY_PANEL_SIZE = 88;
	static constexpr int KEY_PANEL_ALT_SRC_X = 264;

	int key_panel_src_x = 0;
	uint8_t key_panel_direction = 0;
	int key_last_mode = -1;
	int key_panel_cg_page = 296;
	CG key_panel_cg;
	uint8_t key_saved_facing = 0;
	// Highlighted direction: the result of a confirm in every mode.
	uint8_t key_facing = 1;
	bool key_menu_from_click = false;
	// Pointer position at the last hover check.
	int key_hover_x = -1;
	int key_hover_y = -1;

	// Unlike the generic System3 K, the result goes to D01, and the direction
	// keys are latched into a facing that survives across calls.
	void cmd_k() override {
		uint8_t mode = sco.getd();
		TRACE("K %d:", mode);

		if (mode == 0)
			return;
		if (mode < 1 || mode > 3) {
			WARNING("K %d: unsupported mode", mode);
			return;
		}

		initialize_key_panel();
		bool menu = mode != 1;

		// K 2 uses the alternate image set.  K 3 uses the K 1 images but
		// otherwise behaves like the four-way K 2 menu.
		key_panel_src_x = mode == 2 ? KEY_PANEL_ALT_SRC_X : 0;
		if (mode != key_last_mode) {
			key_last_mode = mode;
			key_panel_direction = 0;
			key_facing = 0;
			draw_key_panel();
			// K 1 resumes the facing left by the previous call; K 2 and K 3
			// start facing up.
			resolve_direction(menu ? 1 : key_saved_facing);
		}

		wait_key_release(0xf0);

		if (menu)
			run_direction_menu();
		else
			run_move_input();

		wait_key_release(0xf0);
	}

	void initialize_key_panel() {
		if (key_panel_cg)
			return;

		key_panel_cg = ags->load_cg_surface(key_panel_cg_page, -1, cg_flags);
		if (!key_panel_cg)
			ERROR("K command resource error: CG page %d not available", key_panel_cg_page);
	}

	// K 1.
	void run_move_input() {
		reset_hover();
		while (!terminate) {
			if (uint8_t hover = hover_direction(true))
				resolve_direction(hover);
			uint8_t key = get_key();
			// Direction keys return at once.
			if (key & 0x0f) {
				take_step(key);
				return;
			}
			if (key & 0xc0) {
				D01 = key;
				return;
			}
			if (key & 0x20) {
				D01 = 0x20;
				key_menu_from_click = (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_RMASK) != 0;
				return;
			}
			// A left click inside the map view returns the direction toward
			// the clicked cell. Enter and clicks elsewhere return the highlight.
			if (key & 0x10) {
				int x, y;
				get_cursor(&x, &y);
				uint8_t dir = key_facing;
				if ((SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_LMASK) && in_map_view(x, y))
					dir = map_view_direction(x, y);
				if (dir) {
					take_step(dir);
					return;
				}
			}
			sys_sleep(16);
		}
	}

	void take_step(uint8_t key) {
		D01 = resolve_direction(key) & 0x0f;
		key_saved_facing = key_facing;
	}

	// K 2 and K 3. The pointer or direction keys change the highlighted
	// direction; the left button or Enter confirms it.
	void run_direction_menu() {
		// When the right button opened the menu, the pointer moves onto the
		// panel and stays there, so the K 1 that follows (the direction for
		// SEARCH) can use it too.
		if (key_menu_from_click) {
			key_menu_from_click = false;
			set_cursor(KEY_PANEL_X + KEY_PANEL_SIZE / 2, KEY_PANEL_Y + KEY_PANEL_SIZE / 2);
		}
		reset_hover();
		while (!terminate) {
			if (uint8_t hover = hover_direction(false))
				resolve_direction(hover);
			uint8_t key = resolve_direction(get_key());
			if (key & 0x20) {
				D01 = 0x20;
				break;
			}
			if (key & 0x10) {
				D01 = key_facing;
				break;
			}
			sys_sleep(16);
		}
	}

	bool in_map_view(int x, int y) {
		return VIEW_X <= x && x < VIEW_X + VIEW_COLS * TILE_SIZE &&
			   VIEW_Y <= y && y < VIEW_Y + VIEW_ROWS * TILE_SIZE;
	}

	// Direction from the centre of the player's cell toward screen point
	// (x,y), along the axis with the larger offset; 0 inside the player's
	// own cell.
	uint8_t map_view_direction(int x, int y) {
		int center_x = VIEW_X + (map_sprite_x - map_view_x + VIEW_COLS / 2) * TILE_SIZE + TILE_SIZE / 2;
		int center_y = VIEW_Y + (map_sprite_y - map_view_y + VIEW_ROWS / 2) * TILE_SIZE + TILE_SIZE / 2;
		int dx = x - center_x;
		int dy = y - center_y;
		if (abs(dx) < TILE_SIZE / 2 && abs(dy) < TILE_SIZE / 2)
			return 0;
		if (abs(dx) > abs(dy))
			return dx < 0 ? 4 : 8;
		return dy < 0 ? 1 : 2;
	}

	void reset_hover() {
		get_cursor(&key_hover_x, &key_hover_y);
	}

	// The direction under the pointer if it moved since the last call, else 0.
	// Over the panel this is the panel direction in every mode; over the map
	// view in K 1 it points from the player toward the pointer.
	uint8_t hover_direction(bool map_view) {
		int x, y;
		get_cursor(&x, &y);
		// Only movement changes the highlight, so a resting pointer does not
		// override the direction keys.
		if (x == key_hover_x && y == key_hover_y)
			return 0;
		key_hover_x = x;
		key_hover_y = y;
		if (uint8_t dir = key_panel_direction_at(x, y))
			return dir;
		if (map_view && in_map_view(x, y))
			return map_view_direction(x, y);
		return 0;
	}

	// Latches the facing, updates the panel highlight, and replaces the
	// low nibble of the key with the single direction the move will use.
	uint8_t resolve_direction(uint8_t key) {
		uint8_t dir = key & 0x0f;
		if (!dir)
			return key;
		if (dir == 1 || dir == 2 || dir == 4 || dir == 8) {
			key_facing = dir;
		} else {
			// Several directions at once: prefer the newly pressed one if the
			// cell it leads to can be entered, otherwise keep the old facing.
			uint8_t other = dir ^ key_facing;
			int x = map_sprite_x;
			int y = map_sprite_y;
			switch (other) {
			case 1: y--; break;
			case 2: y++; break;
			case 4: x--; break;
			case 8: x++; break;
			default: other = 0; break;
			}
			dir = (other && cell_is_passable(x, y)) ? other : (dir ^ other);
			key = (key & 0xf0) | dir;
		}
		if (dir != key_panel_direction) {
			key_panel_direction = dir;
			draw_key_panel();
		}
		return key;
	}

	// The panel base and the four highlighted variants sit side by side near
	// the bottom of the selected CG page.
	void draw_key_panel() {
		int x = key_panel_src_x;
		blit_key_panel(x + 104, 304, x + 191, 391, KEY_PANEL_X, KEY_PANEL_Y);
		switch (key_panel_direction) {
		case 1:
			blit_key_panel(x + 192, 304, x + 279, 343, KEY_PANEL_X, KEY_PANEL_Y);
			break;
		case 2:
			blit_key_panel(x + 192, 352, x + 279, 391, KEY_PANEL_X, KEY_PANEL_Y + 48);
			break;
		case 4:
			blit_key_panel(x + 280, 304, x + 319, 391, KEY_PANEL_X, KEY_PANEL_Y);
			break;
		case 8:
			blit_key_panel(x + 328, 304, x + 367, 391, KEY_PANEL_X + 48, KEY_PANEL_Y);
			break;
		default:
			break;
		}
	}

	void blit_key_panel(int sx, int sy, int ex, int ey, int dx, int dy) {
		SDL_Rect src = {
			sx - key_panel_cg.x, sy - key_panel_cg.y,
			ex - sx + 1, ey - sy + 1
		};
		ags->blit_cg(SCREEN_FRONT, key_panel_cg, &src, dx, dy);
	}

	bool in_key_panel(int x, int y) {
		return KEY_PANEL_X <= x && x < KEY_PANEL_X + KEY_PANEL_SIZE &&
			   KEY_PANEL_Y <= y && y < KEY_PANEL_Y + KEY_PANEL_SIZE;
	}

	// The panel direction at screen point (x,y): the four triangles cut by
	// its diagonals. 0 outside the panel.
	uint8_t key_panel_direction_at(int x, int y) {
		if (!in_key_panel(x, y))
			return 0;
		int u = x - KEY_PANEL_X;
		int v = y - KEY_PANEL_Y;
		if (u >= v)
			return (u + v <= KEY_PANEL_SIZE) ? 1 : 8;
		return (u + v > KEY_PANEL_SIZE) ? 2 : 4;
	}

	// --- numbers ----------------------------------------------------------

	// Y 11 draws numbers with an 8x16 font stored in ADVT2.COM.
	static constexpr int DIGIT_WIDTH = 8;
	static constexpr int DIGIT_HEIGHT = 16;
	static constexpr uint8_t DIGIT_BACK_COLOR = 15;
	static constexpr int DIGIT_FONT_OFFSET = 0x7f82;

	uint8_t digit_font[10][DIGIT_HEIGHT] = {};

	void draw_digit(ScreenId screen, int digit, int x, int y) {
		for (int row = 0; row < DIGIT_HEIGHT; row++) {
			uint8_t bits = digit_font[digit][row];
			for (int col = 0; col < DIGIT_WIDTH; col++) {
				if (bits & (0x80 >> col))
					ags->set_pixel(screen, x + col, y + row, 0);
			}
		}
	}

	void draw_number(ScreenId screen, uint16_t value, int x, int y, bool clear) {
		if (clear)
			ags->box_fill(screen, x, y, x + 39, y + 15, DIGIT_BACK_COLOR);
		const uint8_t all[5] = {
			static_cast<uint8_t>(value / 10000),
			static_cast<uint8_t>(value / 1000 % 10),
			static_cast<uint8_t>(value / 100 % 10),
			static_cast<uint8_t>(value / 10 % 10),
			static_cast<uint8_t>(value % 10),
		};
		int i = 0;
		while (i < 5 && all[i] == 0)
			i++;
		if (i == 5)
			i = 4;  // Zero still shows one digit in the last cell.
		for (; i < 5; i++)
			draw_digit(screen, all[i], x + i * DIGIT_WIDTH, y);
	}

	// --- Y 11 status panel ------------------------------------------------

	struct StatusField {
		int var_index;
		int x;
		int y;
	};

	static constexpr std::array<StatusField, 9> status_fields = {{
		{69, 432, 242}, {59, 480, 242}, {70, 432, 260}, {63, 480, 260},
		{66, 528, 278}, {67, 576, 278}, {58, 568, 242}, {93, 432, 278},
		{92, 480, 278},
	}};

	void draw_fields(ScreenId screen, const StatusField* fields, size_t count, bool clear) {
		for (size_t i = 0; i < count; i++) {
			const StatusField& field = fields[i];
			draw_number(screen, var[field.var_index], field.x, field.y, clear);
		}
	}

	void draw_status() {
		draw_fields(SCREEN_BACK, status_fields.data(), status_fields.size(), true);
		draw_fields(SCREEN_FRONT, status_fields.data(), status_fields.size(), true);
	}

	std::u16string decode_status_text(std::string_view string) {
		std::u16string text;
		while (!string.empty())
			text.push_back(encoding->next_codepoint(string));
		return text;
	}

	void draw_equipment_string(ScreenId screen, int slot, int x, int y) {
		std::u16string text = decode_status_text(tvar[slot]);
		x += ((80 - static_cast<int>(text.size()) * 10) / 2) & ~1;
		for (char16_t ch : text) {
			ags->draw_text(screen, x, y, std::u16string_view(&ch, 1), 16, 0);
			x += 10;
		}
	}

	void draw_centered_status_string(ScreenId screen, int slot, int center_x, int y) {
		std::u16string text = decode_status_text(tvar[slot]);
		int x = center_x - static_cast<int>(text.size()) * 8;
		ags->draw_text(screen, x, y, text, 16, 0);
	}

	void draw_detailed_status(ScreenId screen) {
		static constexpr std::array<StatusField, 13> fields = {{
			{69, 168, 80}, {59, 216, 80}, {70, 168, 112}, {63, 216, 112},
			{66, 168, 144}, {67, 216, 144}, {58, 112, 48},
			{96, 216, 176}, {97, 216, 208}, {60, 320, 96},
			{61, 320, 128}, {62, 320, 160}, {68, 304, 192},
		}};
		draw_fields(screen, fields.data(), fields.size(), false);

		draw_centered_status_string(screen, 0, 192, 24);
		draw_equipment_string(screen, 1, 24, 56);
		draw_equipment_string(screen, 2, 24, 112);
		draw_equipment_string(screen, 3, 24, 168);
		draw_equipment_string(screen, 4, 24, 224);

		ags->draw_text(screen, 208, 48, u"　格闘家　　", 16, 0);

		if (screen == SCREEN_FRONT)
			ags->invalidate_screen(0, 0, 400, 240);
	}

	void draw_status_panel(int mode) {
		switch (mode) {
		case 3:
			draw_status();
			break;
		case 4:
			draw_detailed_status(ags->dest_screen);
			break;
		default:
			break;
		}
	}

	// --- mouse cursor ------------------------------------------------------

	// The 16x16 mouse cursor shape read from ADVT2.COM.  It consists of two
	// 32-byte planes, the interior first and the silhouette second.
	static constexpr int MOUSE_CURSOR_OFFSET = 0x30e3;
	static constexpr int MOUSE_CURSOR_SIZE = 16;
	static constexpr int MOUSE_CURSOR_BYTES = MOUSE_CURSOR_SIZE * 2 * 2;

	struct MouseCursor {
		uint16_t body[MOUSE_CURSOR_SIZE];
		uint16_t silhouette[MOUSE_CURSOR_SIZE];  // body plus its outline
	};
	MouseCursor mouse_cursor = {};

	void load_mouse_cursor(const uint8_t* data) {
		for (uint16_t* plane : {mouse_cursor.body, mouse_cursor.silhouette}) {
			for (int row = 0; row < MOUSE_CURSOR_SIZE; row++, data += 2)
				plane[row] = data[0] | data[1] << 8;
		}
	}

	void draw_mouse_cursor(int x, int y) {
		for (int row = 0; row < MOUSE_CURSOR_SIZE; row++) {
			for (int col = 0; col < MOUSE_CURSOR_SIZE; col++) {
				uint16_t bit = 0x8000 >> col;
				if (!(mouse_cursor.silhouette[row] & bit))
					continue;
				ags->set_pixel(SCREEN_FRONT, x + col, y + row,
				               (mouse_cursor.body[row] & bit) ? 15 : 0);
			}
		}
		ags->invalidate_screen(x, y, MOUSE_CURSOR_SIZE, MOUSE_CURSOR_SIZE);
	}

	// --- I grid selection -------------------------------------------------

	// Item state is indexed by logical id, including id 0.
	static constexpr int NR_ICON_ITEMS = 89;
	static constexpr uint16_t GRID_SELECT_CANCELED = 80;

	std::array<uint8_t, NR_ICON_ITEMS> icon_item_state = {};
	std::array<int, 3> icon_cg_pages = {};
	std::array<int, 6> icon_cursor_cell = {};

	// I mode,arg,value
	void cmd_i() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE("I %d,%d,%d:", p1, p2, p3);

		switch (p1) {
		case 1:
			if (p3 == 255) {
				std::fill_n(icon_item_state.begin(), 11, 1);
				std::fill(icon_item_state.begin() + 11, icon_item_state.end(), 0);
			} else if (p3 == 10) {
				// Logical item id 10 is reserved.
			} else if (0 <= p3 && p3 < NR_ICON_ITEMS) {
				if (p2 >> 8)
					icon_item_state[p3] |= 0x80;
				else
					icon_item_state[p3] = p2 & 0xff;
			}
			break;
		case 2:
			RND = run_icon_selection(p2, p3);
			for (int i = 0; i < 52; i++) {
				if (icon_item_state[i] == 3)
					icon_item_state[i] = 1;
			}
			break;
		case 3:
			if (1 <= p2 && p2 <= static_cast<int>(icon_cg_pages.size()))
				icon_cg_pages[p2 - 1] = p3;
			break;
		default:
			break;
		}
	}

	struct IconCell {
		SDL_Rect rect;
		int id;
	};

	// The three layouts used by the scenario.  Screen-cell order is converted
	// to the logical ids used by I 1 and returned in RND.
	std::vector<IconCell> icon_cells(int type) {
		std::vector<IconCell> cells;
		if (type == 1) {
			for (int i = 0; i < 4; i++)
				cells.push_back({{32, 24 + i * 56, 48, 32}, i + 1});
			for (int i = 0; i < 5; i++)
				cells.push_back({{40 + i * 64, 248, 48, 40}, i + 5});
		} else if (type == 3) {
			for (int i = 0; i < 6; i++)
				cells.push_back({{600, 8 + i * 48, 24, 48}, i + 1});
			for (int i = 0; i < 63; i++)
				cells.push_back({{376 + i % 7 * 32, 8 + i / 7 * 32, 32, 32}, i + 11});
		} else if (type == 4) {
			for (int i = 0; i < 18; i++)
				cells.push_back({{376 + i % 2 * 112, 8 + i / 2 * 32, 112, 32}, i + 11});
		}
		return cells;
	}

	SDL_Rect icon_window_rect(int type) const {
		switch (type) {
		case 1:
			return {16, 8, 352, 288};
		case 3:
			// The slot-3 CG rectangle includes the 8-pixel frame around the
			// item area.
			return {368, 8, 264, 288};
		case 4:
			// The cells end at x=599, but the slot-3 CG continues through
			// x=623 for the tabs on their right.
			return {376, 8, 248, 288};
		default:
			return {};
		}
	}

	SDL_Rect icon_cg_region(int type, int source) const {
		// The rectangle each CG page contributes.  source 0 is I 2's third
		// argument, 2 is I 3 slot 2, and 3 is slot 3.
		if (source == 0) {
			switch (type) {
			case 1: return {0, 304, 624, 95};
			case 3: return {0, 0, 216, 96};
			default: return {};
			}
		}
		if (source == 2 && type == 1)
			return {16, 8, 352, 288};
		if (source == 3) {
			switch (type) {
			case 3: return {368, 8, 264, 288};
			case 4: return {376, 8, 248, 288};
			default: return {};
			}
		}
		return {};
	}

	CG load_icon_cg(int page) {
		if (!page)
			return CG();
		return ags->load_cg_surface(page, -1, cg_flags);
	}

	void blit_icon_cg_region(CG& cg, const SDL_Rect& region) {
		if (!cg || region.w <= 0 || region.h <= 0)
			return;
		int x1 = std::max(region.x, cg.x);
		int y1 = std::max(region.y, cg.y);
		int x2 = std::min(region.x + region.w, cg.x + cg.width());
		int y2 = std::min(region.y + region.h, cg.y + cg.height());
		if (x1 >= x2 || y1 >= y2)
			return;
		SDL_Rect src = {x1 - cg.x, y1 - cg.y, x2 - x1, y2 - y1};
		ags->blit_cg(SCREEN_BACK, cg, &src, x1, y1);
	}

	bool icon_cell_enabled(const IconCell& cell) const {
		return icon_item_state[cell.id] != 0;
	}

	// The id the right button returns.
	uint16_t icon_cancel_id(int type) const {
		return type == 1 ? 9 : GRID_SELECT_CANCELED;
	}

	int icon_cell_at(const std::vector<IconCell>& cells, int x, int y) const {
		for (int i = 0; i < static_cast<int>(cells.size()); i++) {
			const SDL_Rect& r = cells[i].rect;
			if (r.x <= x && x < r.x + r.w && r.y <= y && y < r.y + r.h)
				return i + 1;
		}
		return 0;
	}

	int icon_neighbor(int type, int cell, int direction) const {
		// direction: 0 up, 1 down, 2 left, 3 right.
		if (type == 1) {
			if (direction == 0 || direction == 2)
				return cell > 1 ? cell - 1 : 0;
			return cell < 9 ? cell + 1 : 0;
		}
		if (type == 3) {
			if (cell <= 6) {
				static constexpr int left[] = {13, 27, 34, 48, 55, 69};
				switch (direction) {
				case 0: return cell > 1 ? cell - 1 : 0;
				case 1: return cell < 6 ? cell + 1 : 0;
				case 2: return left[cell - 1];
				default: return 0;
				}
			}
			int grid = cell - 7;
			int row = grid / 7;
			int col = grid % 7;
			static constexpr int right[] = {1, 1, 2, 3, 3, 4, 5, 5, 6};
			switch (direction) {
			case 0: return row > 0 ? cell - 7 : 0;
			case 1: return row < 8 ? cell + 7 : 0;
			case 2: return col > 0 ? cell - 1 : 0;
			default: return col < 6 ? cell + 1 : right[row];
			}
		}
		if (type == 4) {
			int row = (cell - 1) / 2;
			int col = (cell - 1) % 2;
			switch (direction) {
			case 0: return row > 0 ? cell - 2 : 0;
			case 1: return row < 8 ? cell + 2 : 0;
			case 2: return col ? cell - 1 : 0;
			default: return col ? 0 : cell + 1;
			}
		}
		return 0;
	}

	int move_icon_cursor(int type, int from, int direction) const {
		if (!from)
			return 1;
		int next = icon_neighbor(type, from, direction);
		return next ? next : from;
	}

	// Item quantities use an 8x8 font stored in ADVT2.COM.
	static constexpr int ICON_DIGIT_SIZE = 8;
	static constexpr int ICON_DIGIT_FONT_OFFSET = 0x7f32;

	uint8_t icon_digit_font[10][ICON_DIGIT_SIZE] = {};

	void draw_icon_quantity(int value, int right, int bottom) {
		// The number ends at the item's right edge, without leading zeroes.
		std::string number = std::to_string(value);
		int x = right - static_cast<int>(number.size()) * ICON_DIGIT_SIZE;
		int y = bottom - 7;
		for (char ch : number) {
			const uint8_t* glyph = icon_digit_font[ch - '0'];
			for (int row = 0; row < ICON_DIGIT_SIZE; row++) {
				for (int col = 0; col < ICON_DIGIT_SIZE; col++) {
					if (glyph[row] & (0x80 >> col))
						ags->set_pixel(SCREEN_BACK, x + col, y + row, 0);
				}
			}
			x += ICON_DIGIT_SIZE;
		}
	}

	void draw_icon_frame(int type, const SDL_Rect& rect) {
		// Low byte is used for the bottom/right half of the bevel and the high
		// byte for the top/left half.
		static constexpr uint16_t colors[] = {
			0, 0x0f03, 0x0f03, 0x0f03, 0x0605, 0x0f03, 0x0f03,
		};
		uint16_t pair = colors[type];
		uint8_t low = pair & 0xff;
		uint8_t high = pair >> 8;
		int x1 = rect.x;
		int y1 = rect.y;
		int x2 = rect.x + rect.w - 1;
		int y2 = rect.y + rect.h - 1;
		for (int n = 0; n < 3; n++)
			ags->box_line(SCREEN_BACK, x1, y1, x2 - n, y2 - n, low);
		for (int n = 0; n < 3; n++)
			ags->box_line(SCREEN_BACK, x1 + n, y1 + n, x2 - 2, y2 - 2, high);
	}

	uint16_t run_icon_selection(int type, int cg_page) {
		std::vector<IconCell> cells = icon_cells(type);
		if (cells.empty()) {
			WARNING("I 2: icon UI type %d is not supported", type);
			return GRID_SELECT_CANCELED;
		}

		// Each page contributes only its own rectangle; drawing whole pages
		// would overwrite the map and unrelated UI regions.
		CG command_cg = load_icon_cg(cg_page);
		CG slot3_cg = load_icon_cg(icon_cg_pages[2]);
		CG slot2_cg = load_icon_cg(icon_cg_pages[1]);
		if (icon_cg_pages[0]) {
			// Slot 1, when set, is a whole base page under the other rectangles.
			load_cg_to_work_screen(icon_cg_pages[0]);
		} else {
			blit_icon_cg_region(command_cg, icon_cg_region(type, 0));
		}
		blit_icon_cg_region(slot3_cg, icon_cg_region(type, 3));
		blit_icon_cg_region(slot2_cg, icon_cg_region(type, 2));
		for (const IconCell& cell : cells) {
			uint8_t state = icon_item_state[cell.id];
			int ex = cell.rect.x + cell.rect.w - 1;
			int ey = cell.rect.y + cell.rect.h - 1;
			if (!state && type != 1) {
				ags->box_fill(SCREEN_BACK, cell.rect.x, cell.rect.y, ex, ey, 0);
			} else if ((state & 0x80) || (2 <= state && state <= 10)) {
				draw_icon_frame(type, cell.rect);
			}
			int quantity_state = state & 0x7f;
			if (quantity_state >= 11 || ((state & 0x80) && quantity_state >= 10))
				draw_icon_quantity(quantity_state - 10, ex, ey);
		}
		SDL_Rect window = icon_window_rect(type);
		ags->copy_screen(SCREEN_BACK, SCREEN_FRONT,
		                 window.x, window.y,
		                 window.x + window.w - 1, window.y + window.h - 1,
		                 window.x, window.y);
		if (type == 1)
			draw_detailed_status(SCREEN_FRONT);

		int& cursor_cell = icon_cursor_cell[type - 1];
		if (cursor_cell < 1 || cursor_cell > static_cast<int>(cells.size()))
			cursor_cell = 1;
		int selected_cell = icon_cell_enabled(cells[cursor_cell - 1]) ? cursor_cell : 0;
		int cursor_x, cursor_y;
		auto position_cursor = [&] {
			const SDL_Rect& target = cells[cursor_cell - 1].rect;
			cursor_x = target.x + (target.w - 1) / 2;
			cursor_y = target.y + (target.h - 1) / 2;
			set_cursor(cursor_x, cursor_y);
		};
		position_cursor();
		int seen_x, seen_y;
		get_cursor(&seen_x, &seen_y);
		bool cursor_drawn = false;
		auto show_cursor = [&] {
			sdl::HideCursor();
			draw_mouse_cursor(cursor_x, cursor_y);
			cursor_drawn = true;
		};
		auto hide_cursor = [&] {
			if (!cursor_drawn)
				return;
			sdl::ShowCursor();
			ags->copy_screen(SCREEN_BACK, SCREEN_FRONT,
			                 cursor_x, cursor_y,
			                 cursor_x + MOUSE_CURSOR_SIZE - 1,
			                 cursor_y + MOUSE_CURSOR_SIZE - 1,
			                 cursor_x, cursor_y);
			cursor_drawn = false;
		};
		show_cursor();

		while (!terminate) {
			int mx, my;
			get_cursor(&mx, &my);
			if (cursor_drawn && (mx != seen_x || my != seen_y) &&
			    (abs(mx - cursor_x) > 1 || abs(my - cursor_y) > 1))
				hide_cursor();
			if (mx != seen_x || my != seen_y) {
				int hovered = icon_cell_at(cells, mx, my);
				if (hovered)
					cursor_cell = hovered;
				selected_cell = hovered && icon_cell_enabled(cells[hovered - 1]) ? hovered : 0;
				seen_x = mx;
				seen_y = my;
			}
			uint8_t key = get_key();
			if (!key) {
				sys_sleep(16);
				continue;
			}
			if (key & 0x0f) {
				hide_cursor();
				if (key & 1) cursor_cell = move_icon_cursor(type, cursor_cell, 0);
				if (key & 2) cursor_cell = move_icon_cursor(type, cursor_cell, 1);
				if (key & 4) cursor_cell = move_icon_cursor(type, cursor_cell, 2);
				if (key & 8) cursor_cell = move_icon_cursor(type, cursor_cell, 3);
				selected_cell = icon_cell_enabled(cells[cursor_cell - 1]) ? cursor_cell : 0;
				position_cursor();
				get_cursor(&seen_x, &seen_y);
				show_cursor();
				wait_key_release(0x0f);
				continue;
			}
			if (key & 0x20) {
				uint16_t cancel_id = icon_cancel_id(type);
				hide_cursor();
				wait_key_release(0x20);
				return cancel_id;
			}
			if ((key & 0x10) && selected_cell) {
				hide_cursor();
				wait_key_release();
				return cells[selected_cell - 1].id;
			}
		}
		hide_cursor();
		return GRID_SELECT_CANCELED;
	}

	// --- save and load ----------------------------------------------------

	// Q/L 1000 use PSLEEP.DAT as a checkpoint.  Its layout is the
	// interpreter state block, the three 100x100 map planes, and the third
	// tile bank in the PC-98 four-plane format.
	static constexpr int SAVE_BLOCK_BYTES = 0x13ce;
	static constexpr int SAVE_MAP_BYTES = MAP_CELLS * 4;
	static constexpr int SAVE_IMAGE_BYTES = TILES_PER_BANK * TILE_SIZE * TILE_SIZE / 2;
	static constexpr int SAVE_TOTAL_BYTES = SAVE_BLOCK_BYTES + SAVE_MAP_BYTES + SAVE_IMAGE_BYTES;
	static constexpr int SAVE_HEADER_BYTES = 112;
	static constexpr int SAVE_OFF_PAGE = 0x070;
	static constexpr int SAVE_OFF_MUSIC = 0x078;
	static constexpr int SAVE_OFF_ADDR = 0x07c;
	static constexpr int SAVE_OFF_VAR = 0x080;
	static constexpr int NR_SAVED_VARS = 750;
	static constexpr int SAVE_OFF_TEXT_STATE = 0x680;
	static constexpr int SAVE_OFF_STRVAR = 0x692;
	static constexpr int STRVAR_BYTES = 42;
	static constexpr int SAVE_OFF_PAGE_FRAMES = 0x9da;
	static constexpr int PAGE_FRAME_BYTES = 8;
	static constexpr int SAVE_OFF_PAGE_DEPTH = 0xbda;
	static constexpr int SAVE_OFF_LABEL_FRAMES = 0xbdb;
	static constexpr int LABEL_FRAME_BYTES = 2;
	static constexpr int SAVE_OFF_LABEL_DEPTH = 0xc1b;
	static constexpr int SAVE_OFF_OOB_TILE = 0x1394;
	static constexpr int SAVE_OFF_VIEW_X = 0x1395;
	static constexpr int SAVE_OFF_VIEW_Y = 0x1397;
	static constexpr int SAVE_OFF_ANIMATION_TICKS = 0x139a;
	static constexpr int SAVE_OFF_SCREEN_WAIT = 0x139c;
	static constexpr int DEFAULT_SCREEN_WAIT = 7;
	static constexpr int SAVE_OFF_MAP_AREA = 0x13b6;
	static constexpr int NR_LABEL_FRAMES = 32;
	static constexpr int NR_PAGE_FRAMES = 64;

	static constexpr char save_header[SAVE_HEADER_BYTES + 1] =
		"This is save data for Toushin2 Pasonal For Pc9801 Onry Set   For"
		" ExNact Sysytems          (C)1995 ALICE-SOFT    ";
	static_assert(sizeof(save_header) == SAVE_HEADER_BYTES + 1);
	static_assert(SAVE_TOTAL_BYTES == 0x1300e);

	static void put_le16(std::vector<uint8_t>& buf, int offset, uint16_t value) {
		buf[offset] = value & 0xff;
		buf[offset + 1] = value >> 8;
	}

	static uint16_t get_le16(const std::vector<uint8_t>& buf, int offset) {
		return buf[offset] | (buf[offset + 1] << 8);
	}

	void cmd_l() override {
		int mode = cali();
		TRACE("L %d:", mode);

		if (mode == 1000) {
			wait_for_save_io();
			RND = load_checkpoint() ? 0xff : 0;
		} else {
			TRACE_UNIMPLEMENTED("L %d:", mode);
		}
	}

	void cmd_q() override {
		int mode = cali();
		TRACE("Q %d:", mode);

		if (mode == 1000) {
			wait_for_save_io();
			if (save_checkpoint())
				RND = 1;
		} else {
			TRACE_UNIMPLEMENTED("Q %d:", mode);
		}
	}

	bool save_checkpoint() {
		std::vector<uint8_t> block(SAVE_BLOCK_BYTES, 0);
		memcpy(&block[0], save_header, SAVE_HEADER_BYTES);
		put_le16(block, SAVE_OFF_PAGE, sco.page() + 1);
		put_le16(block, SAVE_OFF_MUSIC, mako->current_music);
		put_le16(block, SAVE_OFF_ADDR, sco.current_addr());
		for (int i = 0; i < NR_SAVED_VARS; i++)
			put_le16(block, SAVE_OFF_VAR + i * 2, var[i]);
		save_call_stack(block);
		save_text_state(block);
		for (int i = 0; i < MAX_STRVAR; i++) {
			size_t len = std::min(tvar[i].size(), static_cast<size_t>(STRVAR_BYTES - 1));
			memcpy(&block[SAVE_OFF_STRVAR + i * STRVAR_BYTES], tvar[i].data(), len);
		}
		block[SAVE_OFF_OOB_TILE] = map_out_of_bounds_tile;
		put_le16(block, SAVE_OFF_VIEW_X, map_view_x);
		put_le16(block, SAVE_OFF_VIEW_Y, map_view_y);
		put_le16(block, SAVE_OFF_ANIMATION_TICKS, map_animation_ticks);
		put_le16(block, SAVE_OFF_SCREEN_WAIT, DEFAULT_SCREEN_WAIT);
		put_le16(block, SAVE_OFF_MAP_AREA, map_current_area);

		auto fio = FILEIO::open("PSLEEP.DAT", FILEIO_WRITE_BINARY | FILEIO_SAVEDATA);
		if (!fio) {
			WARNING("cannot write PSLEEP.DAT");
			return false;
		}
		if (!fio->write(block.data(), block.size()))
			return false;

		std::vector<uint8_t> map(SAVE_MAP_BYTES);
		for (int i = 0; i < MAP_CELLS; i++)
			put_le16(map, i * 2, map_base[i]);
		memcpy(&map[MAP_CELLS * 2], map_layer_z.data(), MAP_CELLS);
		memcpy(&map[MAP_CELLS * 3], map_layer_w.data(), MAP_CELLS);
		if (!fio->write(map.data(), map.size()))
			return false;

		std::vector<uint8_t> image(SAVE_IMAGE_BYTES, 0);
		encode_tile_bank(image);
		return fio->write(image.data(), image.size());
	}

	bool load_checkpoint() {
		auto fio = FILEIO::open("PSLEEP.DAT", FILEIO_READ_BINARY | FILEIO_SAVEDATA);
		if (!fio)
			return false;
		std::vector<uint8_t> data(SAVE_TOTAL_BYTES);
		if (!fio->read(data.data(), data.size())) {
			WARNING("PSLEEP.DAT is shorter than %d bytes", SAVE_TOTAL_BYTES);
			return false;
		}
		fio.reset();

		for (int i = 0; i < NR_SAVED_VARS; i++)
			var[i] = get_le16(data, SAVE_OFF_VAR + i * 2);
		map_current_area = get_le16(data, SAVE_OFF_MAP_AREA);
		load_text_state(data);
		for (int i = 0; i < MAX_STRVAR; i++) {
			const char* p = reinterpret_cast<const char*>(&data[SAVE_OFF_STRVAR + i * STRVAR_BYTES]);
			tvar[i].assign(p, strnlen(p, STRVAR_BYTES));
		}
		map_out_of_bounds_tile = data[SAVE_OFF_OOB_TILE];
		map_view_x = get_le16(data, SAVE_OFF_VIEW_X);
		map_view_y = get_le16(data, SAVE_OFF_VIEW_Y);
		map_animation_ticks = get_le16(data, SAVE_OFF_ANIMATION_TICKS);

		int map_off = SAVE_BLOCK_BYTES;
		for (int i = 0; i < MAP_CELLS; i++)
			map_base[i] = get_le16(data, map_off + i * 2);
		memcpy(map_layer_z.data(), &data[map_off + MAP_CELLS * 2], MAP_CELLS);
		memcpy(map_layer_w.data(), &data[map_off + MAP_CELLS * 3], MAP_CELLS);
		decode_tile_bank(&data[SAVE_BLOCK_BYTES + SAVE_MAP_BYTES]);

		int page = get_le16(data, SAVE_OFF_PAGE);
		sco.page_jump(page - 1, get_le16(data, SAVE_OFF_ADDR));
		load_call_stack(data);
		mako->play_music(get_le16(data, SAVE_OFF_MUSIC));

		map_view_drawn = false;
		map_sprite_drawn = false;
		return true;
	}

	void wait_for_save_io() {
		while (!terminate && mako->check_pcm())
			sys_sleep(16);
	}

	void save_text_state(std::vector<uint8_t>& block) {
		int off = SAVE_OFF_TEXT_STATE;
		put_le16(block, off, menu_style.font_size);
		put_le16(block, off + 2, text_style.font_size);
		put_le16(block, off + 4, ags->palette_bank == -1 ? 0 : ags->palette_bank);
		put_le16(block, off + 6, text_style.font_color);
		put_le16(block, off + 8, menu_style.font_color);
		put_le16(block, off + 10, menu_style.frame_color);
		put_le16(block, off + 12, menu_style.back_color);
		put_le16(block, off + 14, text_style.frame_color);
		put_le16(block, off + 16, text_style.back_color);
	}

	void load_text_state(const std::vector<uint8_t>& data) {
		int off = SAVE_OFF_TEXT_STATE;
		menu_style.font_size = get_le16(data, off);
		text_style.font_size = get_le16(data, off + 2);
		int bank = get_le16(data, off + 4);
		ags->palette_bank = bank ? bank : -1;
		text_style.font_color = get_le16(data, off + 6);
		menu_style.font_color = get_le16(data, off + 8);
		menu_style.frame_color = get_le16(data, off + 10);
		menu_style.back_color = get_le16(data, off + 12);
		text_style.frame_color = get_le16(data, off + 14);
		text_style.back_color = get_le16(data, off + 16);
	}

	void encode_tile_bank(std::vector<uint8_t>& image) {
		if (!map_tile_cg[2])
			return;
		SDL_Surface* surface = map_tile_cg[2].surface();
		for (int tile = 0; tile < TILES_PER_BANK; tile++) {
			int tx = tile % SHEET_COLS * TILE_SIZE;
			int ty = tile / SHEET_COLS * TILE_SIZE;
			for (int y = 0; y < TILE_SIZE; y++) {
				const uint8_t* pixels = surface_line(surface, ty + y) + tx;
				for (int xb = 0; xb < TILE_SIZE / 8; xb++) {
					for (int plane = 0; plane < 4; plane++) {
						uint8_t bits = 0;
						for (int bit = 0; bit < 8; bit++)
							bits |= ((pixels[xb * 8 + bit] >> plane) & 1) << (7 - bit);
						image[tile * 512 + plane * 128 + y * 4 + xb] = bits;
					}
				}
			}
		}
	}

	void decode_tile_bank(const uint8_t* image) {
		if (!map_tile_cg[2])
			map_tile_cg[2] = ags->create_offscreen(SHEET_COLS * TILE_SIZE, SHEET_COLS * TILE_SIZE);
		SDL_Surface* surface = map_tile_cg[2].surface();
		for (int tile = 0; tile < TILES_PER_BANK; tile++) {
			int tx = tile % SHEET_COLS * TILE_SIZE;
			int ty = tile / SHEET_COLS * TILE_SIZE;
			for (int y = 0; y < TILE_SIZE; y++) {
				uint8_t* pixels = surface_line(surface, ty + y) + tx;
				for (int xb = 0; xb < TILE_SIZE / 8; xb++) {
					for (int bit = 0; bit < 8; bit++) {
						uint8_t color = 0;
						for (int plane = 0; plane < 4; plane++)
							color |= ((image[tile * 512 + plane * 128 + y * 4 + xb]
									  >> (7 - bit)) & 1) << plane;
						pixels[xb * 8 + bit] = color;
					}
				}
			}
		}
	}

	void save_call_stack(std::vector<uint8_t>& block) {
		int labels = 0, pages = 0;
		for (const auto& frame : sco.get_call_stack()) {
			if (frame.is_page_call) {
				if (pages >= NR_PAGE_FRAMES)
					ERROR("page call stack too deep to save");
				int off = SAVE_OFF_PAGE_FRAMES + pages++ * PAGE_FRAME_BYTES;
				put_le16(block, off, frame.page + 1);
				put_le16(block, off + 2, frame.addr);
			} else {
				if (labels >= NR_LABEL_FRAMES)
					ERROR("label call stack too deep to save");
				put_le16(block, SAVE_OFF_LABEL_FRAMES + labels++ * LABEL_FRAME_BYTES,
					  frame.addr);
			}
		}
		block[SAVE_OFF_LABEL_DEPTH] = labels;
		block[SAVE_OFF_PAGE_DEPTH] = pages;
	}

	// Call this after page_jump(), because label frames carry no page number.
	void load_call_stack(const std::vector<uint8_t>& data) {
		std::vector<Scenario::StackFrame> frames;
		int pages = std::min<int>(data[SAVE_OFF_PAGE_DEPTH], NR_PAGE_FRAMES);
		for (int i = 0; i < pages; i++) {
			int off = SAVE_OFF_PAGE_FRAMES + i * PAGE_FRAME_BYTES;
			frames.emplace_back(true, get_le16(data, off) - 1,
								get_le16(data, off + 2));
		}
		int labels = std::min<int>(data[SAVE_OFF_LABEL_DEPTH], NR_LABEL_FRAMES);
		for (int i = 0; i < labels; i++) {
			frames.emplace_back(false, sco.page(),
								get_le16(data, SAVE_OFF_LABEL_FRAMES + i * LABEL_FRAME_BYTES));
		}
		sco.set_call_stack(std::move(frames));
	}
};

} // namespace

NACT_Sys3* create_patton(const Config& config, const GameId& game_id) {
	return new NACT_Patton(config, game_id);
}

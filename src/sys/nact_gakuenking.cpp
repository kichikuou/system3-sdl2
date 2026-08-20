#include <algorithm>
#include <array>
#include <string.h>
#include <string>
#include <string_view>
#include <vector>
#include "nact.h"
#include "ags.h"
#include "fileio.h"
#include "mako.h"

#define D01 var[1]
#define D02 var[2]
#define D03 var[3]
#define D04 var[4]
#define VAR_CHAR_LEVEL var[57]
#define VAR_LOYALTY var[58]
#define VAR_TROOP_COUNT var[59]
#define VAR_TROOP_COUNT_FULL var[60]
#define VAR_TROOP_LIMIT var[61]
#define VAR_UNASSIGNED_TROOPS var[62]
#define VAR_ATTACK_POWER var[63]
#define VAR_DEFENSE_POWER var[64]
#define VAR_SPECIAL_MOVE_GAUGE var[65]
#define VAR_JAPANESE_SCORE var[67]
#define VAR_MATH_SCORE var[68]
#define VAR_ENGLISH_SCORE var[69]
#define VAR_PE_SCORE var[70]
#define VAR_MONEY var[71]
#define VAR_ENEMY_TROOP_COUNT var[72]
#define VAR_ENEMY_TROOP_COUNT_FULL var[73]
#define VAR_ENEMY_ATTACK_POWER var[74]
#define VAR_ENEMY_DEFENSE_POWER var[75]
#define VAR_BATTLE_DRAW_X8 var[76]
#define VAR_BATTLE_DRAW_Y var[77]
#define VAR_BATTLE_DRAW_CG var[78]

namespace {

class NACT_GakuenKing : public NACT_Sys3 {
public:
	NACT_GakuenKing(const Config& config, const GameId& game_id)
		: NACT_Sys3(config, game_id) {
		text_wait_time = 0;	// Y 13 and Y 14 set this
		ags->set_box(1, 0, 16, 8, 367, 295);
		ags->set_box(2, 0, 40, 0, 599, 298);
		load_executable_data();
		amap.open("AMAP.DAT");
		map_base.fill(0);
		map_layer_z.fill(CELL_EMPTY);
		map_layer_w.fill(CELL_EMPTY);
		for (auto& record : map_explored)
			record.fill(0);
	}

private:
	void cmd_a() override {
		NACT::cmd_a();
		map_overview_rebuild_pending = true;
	}

	// --- Y command dispatch -----------------------------------------------

	void cmd_y() override {
		int cmd = cali();
		// Y 82 takes a variable reference, not a value.
		int param = (cmd == 82) ? cali2() : cali();

		TRACE("Y %d,%d:", cmd, param);

		switch (cmd) {
		// --- Gakuen KING specific ---
		case 5:    // CG cache preload - no cache in this engine
		case 6:    // CG cache release - no cache in this engine
			break;
		case 9:
			for (int page = 1; page <= 99; page++)
				mako->set_cd_track(page, param ? page : 0);
			break;
		case 11:
			draw_status_panel(param);
			break;
		case 12:
			run_troop_allocation();
			break;
		case 13:
			set_text_wait(param == 2 ? 3 : 0);
			break;
		case 14:
			set_text_wait(param == 0 ? 0 : param * 16 / 10);
			break;
		case 20:  // audio device check
			exec_y(14, param);
			break;
		case 25:
			fixed_cursor_shape = (param == 1);
			if (0 <= param && param <= 4) {
				ags->menu.back_color = ags->text.back_color =
					(param == 1 || param == 2) ? 0 : 1;
				text_window = param + 1;
				ags->open_text_window(text_window, false);
			}
			break;
		case 26:
			if (0 <= param && param <= 4)
				menu_window = param + 1;
			ags->menu.back_color = (param == 0) ? 1 : 0;
			break;
		case 50:
			map_overview_rebuild_pending = true;
			map_sprite_cg_page = param;
			map_sprite_cg = load_map_sheet(param);
			break;
		case 51:
		case 52:
		case 53:
			{
				int bank = cmd - 51;
				map_overview_rebuild_pending = true;
				map_tile_cg_page[bank] = param;
				map_tile_cg[bank] = load_map_sheet(param);
			}
			break;
		case 55:
			switch_map_area(param);
			break;
		case 56:
			map_base_visible_view = map_base_visible_overview = !!param;
			break;
		case 57:
			map_overview_visible = map_marker_visible = !!param;
			reset_map_overview();
			break;
		case 58:
			map_out_of_bounds_tile = param & 0xff;
			break;
		case 59:
			explored_record().fill(param ? 0xff : 0x00);
			break;
		case 80:
			update_battle_screen(param);
			break;
		case 81:
			run_staff_roll();
			break;
		case 82:
			load_saved_variable(param);
			break;
		case 83:
			sco.label_stack_clear();
			sco.page_stack_clear();
			break;
		case 84:
			pixel_dissolve(param);
			break;
		case 111:
			load_cg_to_work_screen(param);
			break;
		case 112:
			map_overview_rebuild_pending = true;
			break;
		case 118:
			ags->text.pos.x = param;
			break;
		case 119:
			ags->text.pos.y = param;
			break;

		// --- Shared with generic System3 ---
		case 1:    // clear message window
		case 2:    // clear variable group
		case 3:    // wait with timeout
		case 4:    // random number
		case 7:    // clear a registered box
		case 8:    // select string variable slot
		case 16:   // stop BGM
		case 31:   // select drawing screen
		case 40:   // fade in
		case 41:   // fade out
		case 42:   // fade in
		case 43:   // fade out
		case 46:   // CG / palette flags
		case 60:   // screen scroll position
		case 101:  // select PCM slot
		case 102:  // register PCM
		case 103:  // play PCM
		case 239:  // save file timestamp
		case 240:  // halfwidth drawing mode
		case 253:  // page break mark
		case 255:  // quit
			exec_y(cmd, param);
			break;

		default:
			TRACE_UNIMPLEMENTED("Y %d,%d:", cmd, param);
			break;
		}
	}

	// --- data read out of GAKUEN.COM --------------------------------------

	static constexpr const char* EXECUTABLE_NAME = "GAKUEN.COM";

	void load_executable_data() {
		auto fio = FILEIO::open(EXECUTABLE_NAME, FILEIO_READ_BINARY);
		if (!fio) {
			WARNING("Cannot open %s", EXECUTABLE_NAME);
			return;
		}
		std::vector<uint8_t> exe(CREDIT_DATA_END);
		if (!fio->read(exe.data(), exe.size())) {
			WARNING("%s: too short", EXECUTABLE_NAME);
			return;
		}
		load_digit_font(exe);
		load_mouse_cursors(exe);
		load_credit_text(exe);
	}

	// --- timing -----------------------------------------------------------

	static int ticks_to_ms(int ticks) { return ticks * 1000 / 60; }

	void wait_ticks(int ticks) {
		uint32_t deadline = SDL_GetTicks() + ticks_to_ms(ticks);
		while (!terminate && SDL_GetTicks() < deadline)
			sys_sleep(16);
	}

	// --- screens and CG loading -------------------------------------------

	// The second VRAM page for offscreen work.
	static constexpr int WORK_SCREEN = 1;

	// Palette index the color keyed blits treat as transparent.
	static constexpr int TRANSPARENT_COLOR = 7;

	// Y 111
	void load_cg_to_work_screen(int page) {
		int saved = ags->dest_screen;
		ags->dest_screen = WORK_SCREEN;
		ags->load_cg(page, -1);
		ags->dest_screen = saved;
	}

	// --- text -------------------------------------------------------------

	void cmd_p() override {
		int p1 = cali();
		TRACE("P %d:", p1);
		ags->text.font_color = p1;
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

	// The exploration bitmap is one bit per cell, 14 bytes per row, MSB first.
	static constexpr int EXPLORED_ROW_BYTES = 14;
	static constexpr int EXPLORED_RECORD_BYTES = EXPLORED_ROW_BYTES * MAP_HEIGHT;
	// One record per map area, plus an extra record standing in for map ids
	// outside that range (Y 55,255).
	static constexpr int NR_EXPLORED_RECORDS = 60;
	static constexpr int EXPLORED_SCRATCH = NR_EXPLORED_RECORDS;

	Dri amap;
	std::array<uint16_t, MAP_CELLS> map_base;
	std::array<uint8_t, MAP_CELLS> map_layer_z;
	std::array<uint8_t, MAP_CELLS> map_layer_w;
	std::array<std::array<uint8_t, EXPLORED_RECORD_BYTES>, NR_EXPLORED_RECORDS + 1> map_explored;

	int map_current_area = 0;
	int amap_loaded_page = -1;

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
		amap_loaded_page = page;
	}

	std::array<uint8_t, EXPLORED_RECORD_BYTES>& explored_record() {
		int record = map_current_area;
		if (record < 0 || record >= NR_EXPLORED_RECORDS)
			record = EXPLORED_SCRATCH;
		return map_explored[record];
	}

	bool is_explored(int x, int y) {
		return explored_record()[y * EXPLORED_ROW_BYTES + x / 8] & (0x80 >> (x % 8));
	}

	void set_explored(int x, int y) {
		explored_record()[y * EXPLORED_ROW_BYTES + x / 8] |= 0x80 >> (x % 8);
	}

	// Y 55
	void switch_map_area(int map_id) {
		map_overview_rebuild_pending = true;
		map_current_area = map_id;
		path.clear();

		if (map_id == 0) {
			// Fill the direction key panel area with color 1.
			ags->box_fill(0, 16, 304, 103, 391, 1);
			reset_map_overview();
		} else if (map_id == 255) {
			for (auto& record : map_explored)
				record.fill(0);
		} else {
			if (map_id != amap_loaded_page)
				load_amap_page(map_id);
			clear_map_layers();
			map_out_of_bounds_tile = CELL_EMPTY;
			reset_map_overview();
		}
	}

	// --- map view ---------------------------------------------------------

	// J draws 11x9 cells of 32x32 pixels into (16,8)-(367,295).
	static constexpr int VIEW_COLS = 11;
	static constexpr int VIEW_ROWS = 9;
	static constexpr int VIEW_X = 16;
	static constexpr int VIEW_Y = 8;
	static constexpr int TILE_SIZE = 32;

	// A CG loaded by Y 50-53 holds an 8-wide grid of 32x32 tiles,
	// followed by an 8-wide grid of the matching 2x2 overview patterns.
	static constexpr int OVERVIEW_PATTERN_Y = 256;
	static constexpr int SHEET_COLS = 8;
	static constexpr int TILES_PER_BANK = 64;
	static constexpr int NR_TILE_BANKS = 3;
	static constexpr int NR_SPRITE_FRAMES = 8;
	// T ...,255 leaves no sprite to draw.
	static constexpr uint8_t SPRITE_HIDDEN = 0xff;

	uint16_t map_view_x = 0;
	uint16_t map_view_y = 0;
	uint16_t map_sprite_x = 0;
	uint16_t map_sprite_y = 0;
	uint16_t map_sprite_cell = 0;
	uint8_t map_sprite_frame = SPRITE_HIDDEN;
	uint8_t map_out_of_bounds_tile = CELL_EMPTY;
	bool map_base_visible_view = true;
	bool map_overview_rebuild_pending = false;
	// The J draw wait and the marker blink period. GAKUEN.COM sets this to
	// 1-10 with the number keys, but we do not support that.
	int map_animation_ticks = 9;

	// The sheets loaded by Y 50-53, kept whole rather than split into per-tile images.
	CG map_sprite_cg;
	CG map_tile_cg[NR_TILE_BANKS];
	int map_sprite_cg_page = 0;
	int map_tile_cg_page[NR_TILE_BANKS] = {};

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
		map_sprite_cell = static_cast<uint16_t>(y * MAP_WIDTH + x);
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
		if (map_overview_rebuild_pending)
			map_marker_saved_color_stale = true;

		for (int row = 0; row < VIEW_ROWS; row++) {
			for (int col = 0; col < VIEW_COLS; col++) {
				int cell_x = map_view_x - 5 + col;
				int cell_y = map_view_y - 4 + row;
				bool inside = 0 <= cell_x && cell_x < MAP_WIDTH &&
							  0 <= cell_y && cell_y < MAP_HEIGHT;
				int offset = cell_y * MAP_WIDTH + cell_x;

				uint8_t base = inside ? (map_base[offset] & 0xff) : map_out_of_bounds_tile;
				uint8_t layer_z = inside ? map_layer_z[offset] : CELL_EMPTY;
				uint8_t layer_w = inside ? map_layer_w[offset] : CELL_EMPTY;
				uint8_t sprite = (inside && offset == map_sprite_cell)
					? map_sprite_frame : SPRITE_HIDDEN;

				int px = VIEW_X + col * TILE_SIZE;
				int py = VIEW_Y + row * TILE_SIZE;
				if (!map_base_visible_view)
					base = CELL_EMPTY;
				if (base == CELL_EMPTY) {
					ags->box_fill(0, px, py, px + TILE_SIZE - 1, py + TILE_SIZE - 1, 0);
				} else {
					draw_map_tile(base, px, py, false);
				}
				if (layer_z != CELL_EMPTY)
					draw_map_tile(layer_z, px, py, false);
				if (layer_w != CELL_EMPTY)
					draw_map_tile(layer_w, px, py, true);
				if (sprite != SPRITE_HIDDEN)
					draw_map_sprite(sprite, px, py);
			}
		}

		update_map_overview();

		// Movement and animation speed: every map redraw blocks for this long.
		wait_ticks(map_animation_ticks);

		map_overview_rebuild_pending = false;
	}

	CG load_map_sheet(int page) {
		if (page == 0)
			return CG();
		CG cg = ags->load_cg_surface(page, TRANSPARENT_COLOR);
		if (!cg)
			ERROR("map resource error: CG page %d not available", page);
		return cg;
	}

	void draw_map_tile(uint8_t tile, int px, int py, bool transparent) {
		int bank = tile / TILES_PER_BANK;
		if (bank >= NR_TILE_BANKS || !map_tile_cg[bank]) {
			WARNING("map tile %d has no loaded bank", tile);
			return;
		}
		int index = tile % TILES_PER_BANK;
		blit_sheet(map_tile_cg[bank],
				   (index % SHEET_COLS) * TILE_SIZE,
				   (index / SHEET_COLS) * TILE_SIZE,
				   TILE_SIZE, TILE_SIZE, px, py, transparent);
	}

	void draw_map_sprite(uint8_t frame, int px, int py) {
		if (frame >= NR_SPRITE_FRAMES || !map_sprite_cg) {
			WARNING("map sprite frame %d has no loaded CG", frame);
			return;
		}
		blit_sheet(map_sprite_cg, frame * TILE_SIZE, 0,
				   TILE_SIZE, TILE_SIZE, px, py, true);
	}

	void blit_sheet(CG& cg, int sheet_x, int sheet_y,
					int width, int height, int dx, int dy, bool transparent) {
		SDL_Rect src = { sheet_x, sheet_y, width, height };
		if (src.x + width > cg.width() || src.y + height > cg.height()) {
			WARNING("map resource error: (%d,%d)+%dx%d outside CG %dx%d",
				  sheet_x, sheet_y, width, height, cg.width(), cg.height());
			return;
		}
		SDL_SetColorKey(cg.surface(), transparent ? SDL_TRUE : SDL_FALSE,
						TRANSPARENT_COLOR);
		ags->blit_cg(0, cg, &src, dx, dy);
	}

	// --- overview map -----------------------------------------------------

	// The overview map draws the whole map as 2x2 pixels per cell into
	// (392,24)-(591,223).
	static constexpr int OVERVIEW_X = 392;
	static constexpr int OVERVIEW_Y = 24;
	static constexpr int OVERVIEW_CELL = 2;
	static constexpr int OVERVIEW_W = MAP_WIDTH * OVERVIEW_CELL;
	static constexpr int OVERVIEW_H = MAP_HEIGHT * OVERVIEW_CELL;

	bool map_base_visible_overview = true;
	bool map_overview_visible = true;
	bool map_marker_visible = true;

	// Current-position marker
	uint16_t map_marker_x = 0;
	uint16_t map_marker_y = 0;
	uint16_t map_marker_prev_x = 0;
	uint16_t map_marker_prev_y = 0;
	uint8_t map_marker_saved_color = 0;
	// Set when the overview is about to be redrawn under the marker, so the
	// saved color no longer belongs to the screen.
	bool map_marker_saved_color_stale = false;
	uint8_t map_marker_blink_color = 0;
	uint32_t map_marker_blink_deadline = 0;

	void reset_map_overview() {
		map_overview_rebuild_pending = true;
		ags->box_fill(0, OVERVIEW_X, OVERVIEW_Y,
					  OVERVIEW_X + OVERVIEW_W - 1, OVERVIEW_Y + OVERVIEW_H - 1, 0);
	}

	void update_map_overview() {
		if (!map_overview_visible)
			return;
		if (map_overview_rebuild_pending)
			rebuild_map_overview();

		map_marker_x = map_view_x;
		map_marker_y = map_view_y;

		for (int row = 0; row < VIEW_ROWS; row++) {
			uint16_t cell_y = static_cast<uint16_t>(map_view_y - 4 + row);
			if (cell_y >= MAP_HEIGHT)
				continue;
			for (int col = 0; col < VIEW_COLS; col++) {
				uint16_t cell_x = static_cast<uint16_t>(map_view_x - 5 + col);
				if (cell_x >= MAP_WIDTH)
					continue;
				int index = cell_y * MAP_WIDTH + cell_x;
				uint8_t tile = map_layer_w[index];
				if (tile == CELL_EMPTY) {
					// With the base layer hidden, a cell without a W tile is
					// neither drawn nor recorded as explored.
					if (!map_base_visible_overview)
						continue;
					tile = map_layer_z[index];
					if (tile == CELL_EMPTY)
						tile = map_base[index] & 0xff;
				}
				draw_map_overview_cell(cell_x, cell_y, tile);
			}
		}
	}

	void rebuild_map_overview() {
		ags->box_fill(0, OVERVIEW_X, OVERVIEW_Y,
					  OVERVIEW_X + OVERVIEW_W - 1, OVERVIEW_Y + OVERVIEW_H - 1, 0);
		for (int y = 0; y < MAP_HEIGHT; y++) {
			for (int x = 0; x < MAP_WIDTH; x++) {
				if (!is_explored(x, y))
					continue;
				int index = y * MAP_WIDTH + x;
				// The full rebuild always falls back to the base layer,
				// regardless of map_base_visible_overview.
				uint8_t tile = map_layer_w[index];
				if (tile == CELL_EMPTY)
					tile = map_layer_z[index];
				if (tile == CELL_EMPTY)
					tile = map_base[index] & 0xff;
				draw_map_overview_cell(x, y, tile);
			}
		}
	}

	void draw_map_overview_cell(int x, int y, uint8_t tile) {
		set_explored(x, y);
		int bank = tile / TILES_PER_BANK;
		if (bank >= NR_TILE_BANKS || !map_tile_cg[bank])
			return;
		int index = tile % TILES_PER_BANK;
		blit_sheet(map_tile_cg[bank],
				   (index % SHEET_COLS) * OVERVIEW_CELL,
				   OVERVIEW_PATTERN_Y + (index / SHEET_COLS) * OVERVIEW_CELL,
				   OVERVIEW_CELL, OVERVIEW_CELL,
				   OVERVIEW_X + x * OVERVIEW_CELL, OVERVIEW_Y + y * OVERVIEW_CELL, false);
	}

	void update_map_marker() {
		if (!map_marker_visible)
			return;
		if (map_marker_x >= MAP_WIDTH || map_marker_y >= MAP_HEIGHT)
			return;

		int x = OVERVIEW_X + map_marker_x * OVERVIEW_CELL;
		int y = OVERVIEW_Y + map_marker_y * OVERVIEW_CELL;
		if (map_marker_prev_x != map_marker_x || map_marker_prev_y != map_marker_y) {
			if (!map_marker_saved_color_stale) {
				int px = OVERVIEW_X + map_marker_prev_x * OVERVIEW_CELL;
				int py = OVERVIEW_Y + map_marker_prev_y * OVERVIEW_CELL;
				ags->box_fill(0, px, py, px + OVERVIEW_CELL - 1, py + OVERVIEW_CELL - 1,
							  map_marker_saved_color);
			}
			map_marker_prev_x = map_marker_x;
			map_marker_prev_y = map_marker_y;
			map_marker_saved_color = ags->get_pixel(0, x, y);
			map_marker_saved_color_stale = false;
		}

		uint32_t now = SDL_GetTicks();
		if (now >= map_marker_blink_deadline) {
			map_marker_blink_deadline = now + ticks_to_ms(map_animation_ticks + 10);
			map_marker_blink_color ^= 0x0f;
			ags->box_fill(0, x, y, x + OVERVIEW_CELL - 1, y + OVERVIEW_CELL - 1, map_marker_blink_color);
		}
	}

	// --- direction key input ----------------------------------------------

	// The direction key panel occupies (16,304)-(103,391) on screen.
	static constexpr int KEY_PANEL_X = 16;
	static constexpr int KEY_PANEL_Y = 304;
	static constexpr int KEY_PANEL_SIZE = 88;
	static constexpr int KEY_PANEL_ALT_SRC_X = 264;
	static constexpr int PATH_MAX_STEPS = 20;

	struct Neighbor { int dx, dy; uint8_t dir; };
	static constexpr Neighbor neighbor_offsets[4] = {
		{0, -1, 1}, {0, 1, 2}, {-1, 0, 4}, {1, 0, 8},
	};

	int key_panel_src_x = 0;
	uint8_t key_panel_direction = 0;
	int key_last_mode = -1;
	uint8_t key_saved_facing = 0;
	// Last direction key seen by K, used as the result of K 2.
	uint8_t key_facing = 1;
	bool key_menu_from_panel = false;
	bool key_from_mouse = false;
	std::vector<uint16_t> path;

	// Unlike the generic System3 K, the result goes to D01, and the direction
	// keys are latched into a facing that survives across calls.
	void cmd_k() override {
		uint8_t mode = sco.getd();
		TRACE("K %d:", mode);

		if (mode != 1 && mode != 2)
			WARNING("K %d: unsupported mode", mode);
		bool menu = mode != 1;

		// K 2 highlights a second copy of the panel in the work screen.
		key_panel_src_x = menu ? KEY_PANEL_ALT_SRC_X : 0;
		if (mode != key_last_mode) {
			key_last_mode = mode;
			key_panel_direction = 0;
			key_facing = 0;
			draw_key_panel();
			// K 1 resumes the facing left by the previous call; K 2 starts
			// facing up.
			resolve_direction(menu ? 1 : key_saved_facing);
		}
		if (menu)
			path.clear();

		if (!key_from_mouse)
			wait_key_release(0xf0);
		key_from_mouse = false;

		if (menu)
			run_direction_menu();
		else
			run_move_input();

		if (!key_from_mouse)
			wait_key_release(0xf0);
	}

	void run_move_input() {
		if (uint8_t dir = next_path_step()) {
			take_step(dir);
			key_from_mouse = true;
			return;
		}
		while (!terminate) {
			uint8_t key = get_key();
			if (key & 0x0f) {
				take_step(key);
				return;
			}
			if (key & 0xc0) {
				D01 = key;
				return;
			}
			if (key & 0x30) {
				int x, y;
				get_cursor(&x, &y);  // pumps the events SDL_GetMouseState needs
				uint32_t buttons = SDL_GetMouseState(NULL, NULL);
				if (!buttons)
					D01 = (key & 0x10) ? 0x10 : 0x20;  // Enter or the cancel key
				else
					move_input_click(buttons & SDL_BUTTON_LMASK, x, y);
				return;
			}
			update_map_marker();
			sys_sleep(16);
		}
	}

	// Maps a click during K 1 to a result in D01. The original captures the
	// pointer and steps in the facing direction. This port cannot capture it,
	// so a click walks to the cell it lands on, like the System3.5 port.
	void move_input_click(bool left, int x, int y) {
		bool in_view = VIEW_X <= x && x < VIEW_X + VIEW_COLS * TILE_SIZE &&
					   VIEW_Y <= y && y < VIEW_Y + VIEW_ROWS * TILE_SIZE;
		if (in_key_panel(x, y)) {
			D01 = 0x20;
			key_menu_from_panel = true;
		} else if (!in_view) {
			D01 = left ? 0x10 : 0x40;
		} else if (!left) {
			D01 = 0x20;
		} else if (build_path(map_view_x - VIEW_COLS / 2 + (x - VIEW_X) / TILE_SIZE,
							  map_view_y - VIEW_ROWS / 2 + (y - VIEW_Y) / TILE_SIZE)) {
			take_step(next_path_step());
			key_from_mouse = true;
		} else {
			int dx = x - (VIEW_X + VIEW_COLS * TILE_SIZE / 2);
			int dy = y - (VIEW_Y + VIEW_ROWS * TILE_SIZE / 2);
			if (std::abs(dx) > std::abs(dy))
				D01 = dx < 0 ? 4 : 8;
			else
				D01 = dy < 0 ? 1 : 2;
		}
	}

	void take_step(uint8_t key) {
		D01 = resolve_direction(key) & 0x0f;
		key_saved_facing = key_facing;
	}

	// K 2. The pointer picks a direction on the panel by hovering; the
	// left button or Enter confirms it.
	void run_direction_menu() {
		int saved_x = -1, saved_y = -1;
		if (key_menu_from_panel) {
			key_menu_from_panel = false;
			get_cursor(&saved_x, &saved_y);
			set_cursor(KEY_PANEL_X + KEY_PANEL_SIZE / 2, KEY_PANEL_Y + KEY_PANEL_SIZE / 2);
		}
		while (!terminate) {
			if (uint8_t hover = key_panel_hover_direction())
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
			update_map_marker();
			sys_sleep(16);
		}
		if (saved_x >= 0)
			set_cursor(saved_x, saved_y);
	}

	// False when no walk of PATH_MAX_STEPS or fewer reaches cell (x,y).
	bool build_path(int x, int y) {
		path.clear();
		int target = cell_at(x, y);
		int start = cell_at(map_sprite_x, map_sprite_y);
		if (target < 0 || start < 0)
			return false;

		// Distance from the player, biased by one so that 0 means unvisited.
		std::array<uint8_t, MAP_CELLS> dist = {};
		std::vector<uint16_t> queue{static_cast<uint16_t>(start)};
		dist[start] = 1;
		for (size_t i = 0; i < queue.size() && !dist[target]; i++) {
			if (dist[queue[i]] > PATH_MAX_STEPS)
				break;
			for (auto& n : neighbor_offsets) {
				int nx = queue[i] % MAP_WIDTH + n.dx;
				int ny = queue[i] / MAP_WIDTH + n.dy;
				int index = cell_at(nx, ny);
				if (index < 0 || dist[index] || !cell_is_passable(nx, ny))
					continue;
				dist[index] = dist[queue[i]] + 1;
				queue.push_back(static_cast<uint16_t>(index));
			}
		}

		if (dist[target] < 2)
			return false;
		for (int d = dist[target]; d > 1; d--) {
			path.push_back(static_cast<uint16_t>(cell_at(x, y)));
			for (auto& n : neighbor_offsets) {
				int prev = cell_at(x - n.dx, y - n.dy);
				if (prev >= 0 && dist[prev] == d - 1) {
					x -= n.dx;
					y -= n.dy;
					break;
				}
			}
		}
		return true;
	}

	// -1 when the cell is off the map.
	int cell_at(int x, int y) {
		if (x < 0 || x >= MAP_WIDTH || y < 0 || y >= MAP_HEIGHT)
			return -1;
		return y * MAP_WIDTH + x;
	}

	uint8_t next_path_step() {
		if (path.empty())
			return 0;
		int dx = path.back() % MAP_WIDTH - map_sprite_x;
		int dy = path.back() / MAP_WIDTH - map_sprite_y;
		path.pop_back();
		for (auto& n : neighbor_offsets) {
			if (n.dx == dx && n.dy == dy)
				return n.dir;
		}
		path.clear();
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

	// The panel base and the four highlighted variants sit side by side in
	// the work screen.
	void draw_key_panel() {
		int x = key_panel_src_x;
		ags->copy_screen(WORK_SCREEN, 0, x + 104, 304, x + 191, 391, KEY_PANEL_X, KEY_PANEL_Y);
		switch (key_panel_direction) {
		case 1:
			ags->copy_screen(WORK_SCREEN, 0, x + 192, 304, x + 279, 343, KEY_PANEL_X, KEY_PANEL_Y);
			break;
		case 2:
			ags->copy_screen(WORK_SCREEN, 0, x + 192, 352, x + 279, 391, KEY_PANEL_X, KEY_PANEL_Y + 48);
			break;
		case 4:
			ags->copy_screen(WORK_SCREEN, 0, x + 280, 304, x + 319, 391, KEY_PANEL_X, KEY_PANEL_Y);
			break;
		case 8:
			ags->copy_screen(WORK_SCREEN, 0, x + 328, 304, x + 367, 391, KEY_PANEL_X + 48, KEY_PANEL_Y);
			break;
		default:
			break;
		}
	}

	bool in_key_panel(int x, int y) {
		return KEY_PANEL_X <= x && x < KEY_PANEL_X + KEY_PANEL_SIZE &&
			   KEY_PANEL_Y <= y && y < KEY_PANEL_Y + KEY_PANEL_SIZE;
	}

	// The original reads relative mouse movement with the pointer captured at
	// the screen centre, which this port cannot do. The pointer picks a
	// direction from the panel instead: the four triangles cut by its diagonals.
	uint8_t key_panel_hover_direction() {
		int x, y;
		get_cursor(&x, &y);
		if (!in_key_panel(x, y))
			return 0;
		int u = x - KEY_PANEL_X;
		int v = y - KEY_PANEL_Y;
		if (u >= v)
			return (u + v <= KEY_PANEL_SIZE) ? 1 : 8;
		return (u + v > KEY_PANEL_SIZE) ? 2 : 4;
	}

	// --- numbers ----------------------------------------------------------

	// Y 11 / Y 12 draw numbers with an 8x16 font stored in GAKUEN.COM.
	static constexpr int DIGIT_WIDTH = 8;
	static constexpr int DIGIT_HEIGHT = 16;
	static constexpr uint8_t DIGIT_BACK_COLOR = 15;
	static constexpr int DIGIT_FONT_OFFSET = 0x0c98;

	uint8_t digit_font[10][DIGIT_HEIGHT] = {};

	void load_digit_font(const std::vector<uint8_t>& exe) {
		memcpy(digit_font, &exe[DIGIT_FONT_OFFSET], sizeof(digit_font));
	}

	void draw_digit(int digit, int x, int y, uint8_t color) {
		ags->box_fill(0, x, y, x + DIGIT_WIDTH - 1, y + DIGIT_HEIGHT - 1, DIGIT_BACK_COLOR);
		for (int row = 0; row < DIGIT_HEIGHT; row++) {
			uint8_t bits = digit_font[digit][row];
			for (int col = 0; col < DIGIT_WIDTH; col++) {
				if (bits & (0x80 >> col))
					ags->set_pixel(0, x + col, y + row, color);
			}
		}
	}

	void draw_number(uint16_t value, int digits, int x, int y, uint8_t color = 0) {
		if (digits == 3 && value > 999)
			value = 999;
		const uint8_t all[5] = {
			static_cast<uint8_t>(value / 10000),
			static_cast<uint8_t>(value / 1000 % 10),
			static_cast<uint8_t>(value / 100 % 10),
			static_cast<uint8_t>(value / 10 % 10),
			static_cast<uint8_t>(value % 10),
		};
		const uint8_t* d = all + 5 - digits;
		int i = 0;
		while (i < digits && d[i] == 0) {
			// Leading zeros are blanked.
			ags->box_fill(0, x + i * DIGIT_WIDTH, y,
						  x + i * DIGIT_WIDTH + DIGIT_WIDTH - 1, y + DIGIT_HEIGHT - 1,
						  DIGIT_BACK_COLOR);
			i++;
		}
		if (i == digits) {
			// Zero still shows one digit, in the last cell.
			draw_digit(0, x + (digits - 1) * DIGIT_WIDTH, y, color);
			return;
		}
		for (; i < digits; i++)
			draw_digit(d[i], x + i * DIGIT_WIDTH, y, color);
	}

	// --- Y 11 status panel ------------------------------------------------

	void draw_status_panel(int mode) {
		if (mode == 0) {
			draw_number(VAR_CHAR_LEVEL, 3, 432, 248);
			draw_number(VAR_TROOP_COUNT, 3, 544, 248);
			draw_number(VAR_TROOP_COUNT_FULL, 3, 584, 248);
			draw_number(VAR_MONEY, 5, 464, 272);
			draw_number(VAR_SPECIAL_MOVE_GAUGE, 2, 594, 272);
		} else if (mode == 1) {
			draw_number(VAR_CHAR_LEVEL, 3, 72, 120);
			// 1000 marks this field as absent.
			if (VAR_LOYALTY != 1000)
				draw_number(VAR_LOYALTY, 3, 88, 168);
			draw_number(VAR_TROOP_COUNT, 3, 256, 96);
			draw_number(VAR_TROOP_COUNT_FULL, 3, 296, 96);
			draw_number(VAR_TROOP_LIMIT, 3, 304, 128);
			draw_number(VAR_ATTACK_POWER, 3, 304, 152);
			draw_number(VAR_DEFENSE_POWER, 3, 304, 176);
			draw_number(VAR_SPECIAL_MOVE_GAUGE, 2, 312, 200);
			draw_number(VAR_JAPANESE_SCORE, 2, 88, 200);
			draw_number(VAR_MATH_SCORE, 2, 88, 224);
			draw_number(VAR_ENGLISH_SCORE, 2, 88, 248);
			draw_number(VAR_PE_SCORE, 2, 88, 272);
		} else {
			// Redraw the direction key panel from the work screen.
			ags->copy_screen(WORK_SCREEN, 0, 104, 304, 191, 391, KEY_PANEL_X, KEY_PANEL_Y);
		}
	}

	// --- Y 12 troop allocation --------------------------------------------

	static constexpr int TROOP_BUTTON_WIDTH = 24;
	static constexpr int TROOP_BUTTON_HEIGHT = 20;
	// Button images on the work screen, by state.
	static constexpr int TROOP_BUTTON_NORMAL_X = 480;
	static constexpr int TROOP_BUTTON_HOVER_X = 504;
	static constexpr int TROOP_BUTTON_PRESSED_X = 528;

	struct TroopButton {
		int x, y;
		int step;

		// The hit rectangle is one pixel inside the image at top and bottom.
		bool contains(int px, int py) const {
			return x <= px && px < x + TROOP_BUTTON_WIDTH &&
			       y < py && py < y + TROOP_BUTTON_HEIGHT - 1;
		}
	};
	static constexpr TroopButton troop_buttons[6] = {
		{24, 212, 100}, {56, 212, 10}, {88, 212, 1},
		{24, 264, -100}, {56, 264, -10}, {88, 264, -1},
	};

	void run_troop_allocation() {
		draw_status_panel(1);

		load_cg_to_work_screen(230);
		// Save the part of the display CG 208 overwrites.
		ags->copy_screen(0, WORK_SCREEN, 16, 192, 367, 294, 0, 48);
		ags->load_cg(208, -1);

		VAR_TROOP_COUNT_FULL = VAR_TROOP_COUNT;
		draw_troop_counts();

		set_cursor(99, 223);
		const TroopButton* hover = nullptr;

		while (!terminate) {
			int mx, my;
			get_cursor(&mx, &my);
			uint8_t key = get_key();

			const TroopButton* under_cursor = troop_button_at(mx, my);
			if (under_cursor != hover) {
				if (hover)
					draw_troop_button(TROOP_BUTTON_NORMAL_X, *hover);
				if (under_cursor)
					draw_troop_button(TROOP_BUTTON_HOVER_X, *under_cursor);
				hover = under_cursor;
			}

			if (!key) {
				sys_sleep(16);
				continue;
			}
			if (key & 0x20) {
				wait_key_release(0x20);
				break;
			}
			if (!(key & 0x10)) {
				// Direction keys jump the pointer between the six buttons.
				if (key & 1) my -= 48;
				if (key & 2) my += 48;
				if (key & 4) mx -= 32;
				if (key & 8) mx += 32;
				set_cursor(std::clamp(mx, 35, 99), std::clamp(my, 223, 271));
				wait_key_release(0x0f);
				continue;
			}

			if (!hover)
				continue;
			draw_troop_button(TROOP_BUTTON_PRESSED_X, *hover);
			apply_troop_change(*hover);
			draw_troop_counts();
			wait_ticks(10);
			draw_troop_button(TROOP_BUTTON_HOVER_X, *hover);
			wait_key_release(0x10);
		}

		// Restore the saved area.
		ags->copy_screen(WORK_SCREEN, 0, 0, 48, 351, 150, 16, 192);
	}

	const TroopButton* troop_button_at(int x, int y) {
		for (const TroopButton& b : troop_buttons) {
			if (b.contains(x, y))
				return &b;
		}
		return nullptr;
	}

	void draw_troop_button(int image_x, const TroopButton& b) {
		// The subtract buttons use the lower row of images.
		int image_y = b.step < 0 ? 24 : 4;
		ags->copy_screen(WORK_SCREEN, 0, image_x, image_y,
		                 image_x + TROOP_BUTTON_WIDTH - 1,
		                 image_y + TROOP_BUTTON_HEIGHT - 1, b.x, b.y);
	}

	void draw_troop_counts() {
		draw_number(VAR_TROOP_COUNT, 3, 256, 96);
		draw_number(VAR_TROOP_COUNT_FULL, 3, 296, 96);
		draw_troop_pool();
	}

	// The unassigned count uses 48x48 digits from the top of the work screen,
	// with no leading-zero suppression.
	void draw_troop_pool() {
		uint16_t value = VAR_UNASSIGNED_TROOPS;
		uint16_t divisor = 10000;
		for (int i = 0; i < 5; i++) {
			int digit = value / divisor;
			value %= divisor;
			divisor /= 10;
			ags->copy_screen(WORK_SCREEN, 0, digit * 48, 0, digit * 48 + 47, 47,
			                 120 + i * 48, 232);
		}
	}

	void apply_troop_change(const TroopButton& b) {
		uint16_t step = b.step > 0 ? b.step : -b.step;
		if (b.step > 0) {
			if (VAR_UNASSIGNED_TROOPS < step)
				step = VAR_UNASSIGNED_TROOPS;
			if (static_cast<uint16_t>(VAR_TROOP_COUNT_FULL + step) >= VAR_TROOP_LIMIT)
				step = VAR_TROOP_LIMIT - VAR_TROOP_COUNT_FULL;
			VAR_TROOP_COUNT_FULL += step;
			VAR_UNASSIGNED_TROOPS -= step;
		} else {
			if (VAR_TROOP_COUNT_FULL < step)
				step = VAR_TROOP_COUNT_FULL;
			VAR_TROOP_COUNT_FULL -= step;
			VAR_UNASSIGNED_TROOPS += step;
		}
		// A unit never drops below one student.
		if (VAR_TROOP_COUNT_FULL == 0) {
			VAR_TROOP_COUNT_FULL = 1;
			VAR_UNASSIGNED_TROOPS--;
		}
		VAR_TROOP_COUNT = VAR_TROOP_COUNT_FULL;
	}

	// --- mouse cursor ------------------------------------------------------

	// The shapes GAKUEN.COM hands to the mouse driver: six 16x16 patterns of
	// two 32-byte planes each, the interior first and the silhouette second.
	// Pattern 0 is blank and unused.
	static constexpr int MOUSE_CURSOR_OFFSET = 0x2a92;
	static constexpr int NR_MOUSE_CURSORS = 6;
	static constexpr int MOUSE_CURSOR_SIZE = 16;

	struct MouseCursor {
		uint16_t body[MOUSE_CURSOR_SIZE];
		uint16_t silhouette[MOUSE_CURSOR_SIZE];  // body plus its outline
	};
	MouseCursor mouse_cursor[NR_MOUSE_CURSORS] = {};

	bool fixed_cursor_shape = false;  // set by Y 25,1

	void load_mouse_cursors(const std::vector<uint8_t>& exe) {
		const uint8_t* p = &exe[MOUSE_CURSOR_OFFSET];
		for (MouseCursor& cursor : mouse_cursor) {
			for (uint16_t* plane : {cursor.body, cursor.silhouette}) {
				for (int row = 0; row < MOUSE_CURSOR_SIZE; row++, p += 2)
					plane[row] = p[0] | p[1] << 8;
			}
		}
	}

	int pick_cursor_shape() {
		if (fixed_cursor_shape)
			return 5;
		int r = random(100);
		return r < 5 ? r : 1;
	}

	void draw_mouse_cursor(int shape, int x, int y) {
		const MouseCursor& cursor = mouse_cursor[shape];
		for (int row = 0; row < MOUSE_CURSOR_SIZE; row++) {
			for (int col = 0; col < MOUSE_CURSOR_SIZE; col++) {
				uint16_t bit = 0x8000 >> col;
				if (!(cursor.silhouette[row] & bit))
					continue;
				ags->set_pixel(0, x + col, y + row, (cursor.body[row] & bit) ? 15 : 0);
			}
		}
		ags->draw_screen(x, y, MOUSE_CURSOR_SIZE, MOUSE_CURSOR_SIZE);
	}

	// --- I grid selection -------------------------------------------------

	// I 2 composes its overlays over the background CG on the work screen and
	// then copies (368,0)-(631,303) to the display.
	static constexpr int GRID_X = 376;
	static constexpr int GRID_COPY_X1 = 368;
	static constexpr int GRID_COPY_X2 = 631;
	static constexpr int GRID_COPY_Y2 = 303;
	static constexpr int NR_GRID_ITEMS = 63;
	static constexpr uint16_t GRID_SELECT_CANCELED = 80;

	uint8_t grid_item_state[NR_GRID_ITEMS] = {};
	// The two layouts remember their cursor separately.
	int grid_cursor_x[2] = {392, 432};
	int grid_cursor_y[2] = {23, 22};

	// I mode,arg,value
	void cmd_i() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE("I %d,%d,%d:", p1, p2, p3);

		switch (p1) {
		case 1:
			if (p3 == 255) {
				for (int i = 0; i < NR_GRID_ITEMS; i++)
					grid_item_state[i] = p2 & 0xff;
			} else if (1 <= p3 && p3 <= NR_GRID_ITEMS) {
				grid_item_state[p3 - 1] = p2 & 0xff;
			}
			break;
		case 2:
			RND = run_grid_selection(p2, p3);
			break;
		default:
			break;
		}
	}

	uint16_t run_grid_selection(int layout, int cg_page) {
		const bool wide = (layout == 2);
		const int cols = wide ? 2 : 7;
		const int rows = 9;
		const int cell_w = wide ? 112 : 32;
		const int nr_items = wide ? 18 : NR_GRID_ITEMS;

		load_cg_to_work_screen(cg_page);
		for (int i = 0; i < nr_items; i++) {
			uint8_t state = grid_item_state[i];
			if (state & 0xfd)
				continue;  // only states 0 and 2 draw anything
			int sx = GRID_X + (i % cols) * cell_w;
			int sy = (wide ? 8 : 7) + (i / cols) * 32;
			int ex = sx + cell_w - 1;
			int ey = sy + (wide ? 31 : 32);
			if (state == 0) {
				ags->box_fill(WORK_SCREEN, sx, sy, ex, ey, 0);
			} else {
				for (int n = 0; n < 3; n++)
					ags->box_line(WORK_SCREEN, sx + n, sy + n, ex - n, ey - n, 15);
			}
		}
		ags->copy_screen(WORK_SCREEN, 0, GRID_COPY_X1, 0, GRID_COPY_X2, GRID_COPY_Y2,
		                 GRID_COPY_X1, 0);

		int& cx = grid_cursor_x[wide ? 1 : 0];
		int& cy = grid_cursor_y[wide ? 1 : 0];

		// The original relies on the mouse pointer movement for visual feedback
		// on player input. In system3-sdl2 the pointer may be absent or
		// unmovable, so we draw the cursor shape onto the grid instead. It
		// disappears once the player moves the pointer, and comes back on the
		// next direction key.
		const int shape = pick_cursor_shape();
		bool cursor_drawn = false;
		int seen_x, seen_y;
		get_cursor(&seen_x, &seen_y);
		// Hides the system pointer and draws the cursor at (cx, cy) instead.
		auto show_cursor = [&] {
			SDL_ShowCursor(SDL_DISABLE);
			draw_mouse_cursor(shape, cx, cy);
			cursor_drawn = true;
		};
		// Erases the drawn cursor and restores the system pointer. Must be
		// called before (cx, cy) moves, which is where the cursor is drawn.
		auto hide_cursor = [&] {
			if (!cursor_drawn)
				return;
			SDL_ShowCursor(SDL_ENABLE);
			ags->copy_screen(WORK_SCREEN, 0, cx, cy, cx + MOUSE_CURSOR_SIZE - 1,
			                 cy + MOUSE_CURSOR_SIZE - 1, cx, cy);
			cursor_drawn = false;
		};
		set_cursor(cx, cy);
		show_cursor();

		while (!terminate) {
			uint8_t key = get_key();
			int mx, my;
			get_cursor(&mx, &my);
			// A pointer position that is neither where it was nor where
			// set_cursor() put it is a move by the player. The pointer comes
			// back through a scaling that rounds, so allow it one pixel.
			if ((mx != seen_x || my != seen_y) &&
			    (std::abs(mx - cx) > 1 || std::abs(my - cy) > 1))
				hide_cursor();
			seen_x = mx;
			seen_y = my;
			if (!key) {
				sys_sleep(16);
				continue;
			}
			if (!(key & 0xb0)) {
				hide_cursor();
				if (key & 1 && cy >= (wide ? 54 : 55)) cy -= 32;
				if (key & 2 && cy < (wide ? 247 : 248)) cy += 32;
				if (key & 4 && cx >= (wide ? 544 : 424)) cx -= cell_w;
				if (key & 8 && cx < (wide ? 433 : 553)) cx += cell_w;
				set_cursor(cx, cy);
				show_cursor();
				wait_key_release(0x0f);
				continue;
			}
			if (key & 0xa0) {
				hide_cursor();
				wait_key_release();
				return GRID_SELECT_CANCELED;
			}
			int col, row;
			if (cursor_drawn) {
				col = (cx - (wide ? 432 : 392)) / cell_w;
				row = (cy - (wide ? 22 : 23)) / 32;
			} else {
				if (mx < GRID_X || mx >= GRID_X + cols * cell_w ||
				    my < 7 || my >= 7 + rows * 32) {
					// outside the grid.
					wait_key_release();
					continue;
				}
				col = (mx - GRID_X) / cell_w;
				row = (my - 7 - (wide ? 1 : 0)) / 32;
				cx = col * cell_w + (wide ? 432 : 392);
				cy = row * 32 + (wide ? 22 : 23);
				set_cursor(cx, cy);
			}
			hide_cursor();
			wait_key_release();
			// The state table is not consulted: state 0 returns its number
			// too, and the scenario rejects it.
			return static_cast<uint16_t>(row * cols + col + 1);
		}
		hide_cursor();
		return GRID_SELECT_CANCELED;
	}

	// --- Y 80 battle screen -----------------------------------------------

	// Y 80 composes the battle screen on the work screen: four 160x240 frames side
	// by side at the top, the sheet of 16x16 parts at y=240, and the left and right
	// templates at y=280. Y 80,3-6 send one frame to the display.
	static constexpr int BATTLE_FRAME_W = 160;
	static constexpr int BATTLE_FRAME_H = 240;
	static constexpr int BATTLE_PART_SIZE = 16;
	static constexpr int BATTLE_PART_SHEET_Y = 240;
	static constexpr int BATTLE_PARTS_PER_ROW = 40;
	static constexpr int BATTLE_TEMPLATE_Y = 280;
	static constexpr int BATTLE_TEMPLATE_H = 120;
	static constexpr int BATTLE_DEST_Y = 32;
	static constexpr int BATTLE_LEFT_DEST_X = 152;
	static constexpr int BATTLE_RIGHT_DEST_X = 328;
	static constexpr int BATTLE_BACK_COLOR = 1;
	static constexpr uint8_t CASUALTY_DIGIT_COLOR = 5;

	void update_battle_screen(int mode) {
		// Make the next K redraw its mode specific panel.
		key_last_mode = 0;
		if (mode >= 3)
			draw_battle_status();

		int part_x = VAR_BATTLE_DRAW_X8 * 8;
		int part_y = VAR_BATTLE_DRAW_Y;
		switch (mode) {
		case 0:
			ags->cg_dest = SDL_Point{part_x, part_y};
			load_cg_to_work_screen(VAR_BATTLE_DRAW_CG);
			break;
		case 1:
			ags->box_fill(WORK_SCREEN, 0, 0, 4 * BATTLE_FRAME_W - 1, BATTLE_FRAME_H - 1,
						  BATTLE_BACK_COLOR);
			break;
		case 2:
			{
				// The sheet holds parts 0-39 in one row and 40 and up in the
				// next. A part number past the sheet is not range checked.
				int part = VAR_BATTLE_DRAW_CG;
				int row = (part >= BATTLE_PARTS_PER_ROW) ? 1 : 0;
				if (row)
					part -= BATTLE_PARTS_PER_ROW;
				int sx = part * BATTLE_PART_SIZE;
				int sy = BATTLE_PART_SHEET_Y + row * BATTLE_PART_SIZE;
				ags->copy_screen(WORK_SCREEN, WORK_SCREEN,
				                 sx, sy, sx + BATTLE_PART_SIZE - 1, sy + BATTLE_PART_SIZE - 1,
				                 part_x, part_y, TRANSPARENT_COLOR);
			}
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			{
				int frame = mode - 3;
				int sx = frame * BATTLE_FRAME_W;
				int dx = (frame < 2) ? BATTLE_LEFT_DEST_X : BATTLE_RIGHT_DEST_X;
				ags->copy_screen(WORK_SCREEN, 0, sx, 0, sx + BATTLE_FRAME_W - 1,
				                 BATTLE_FRAME_H - 1, dx, BATTLE_DEST_Y);
			}
			break;
		case 7:
		case 8:
			build_battle_frames(mode == 7 ? 0 : 2 * BATTLE_FRAME_W);
			break;
		default:
			break;
		}
	}

	// The template is stored as two 160x120 halves next to each other; joining
	// them gives one 160x240 frame, which is written to both of the pair's
	// slots so the two animation frames start out identical.
	void build_battle_frames(int base_x) {
		for (int half = 0; half < 2; half++) {
			int sx = base_x + half * BATTLE_FRAME_W;
			for (int frame = 0; frame < 2; frame++) {
				ags->copy_screen(WORK_SCREEN, WORK_SCREEN,
				                 sx, BATTLE_TEMPLATE_Y,
				                 sx + BATTLE_FRAME_W - 1, BATTLE_TEMPLATE_Y + BATTLE_TEMPLATE_H - 1,
				                 base_x + frame * BATTLE_FRAME_W, half * BATTLE_TEMPLATE_H);
			}
		}
	}

	void draw_battle_status() {
		draw_number(VAR_CHAR_LEVEL, 3, 80, 144);
		draw_number(VAR_TROOP_COUNT, 3, 24, 184);
		draw_number(VAR_TROOP_COUNT_FULL - VAR_TROOP_COUNT, 3, 80, 184, CASUALTY_DIGIT_COLOR);
		draw_number(VAR_ATTACK_POWER, 3, 96, 208);
		draw_number(VAR_DEFENSE_POWER, 3, 96, 232);
		draw_number(VAR_SPECIAL_MOVE_GAUGE, 2, 64, 272);
		draw_number(VAR_ENEMY_TROOP_COUNT, 3, 520, 184);
		draw_number(VAR_ENEMY_TROOP_COUNT_FULL - VAR_ENEMY_TROOP_COUNT, 3, 576, 184, CASUALTY_DIGIT_COLOR);
		draw_number(VAR_ENEMY_ATTACK_POWER, 3, 592, 208);
		draw_number(VAR_ENEMY_DEFENSE_POWER, 3, 592, 232);
	}

	// --- Y 84 pixel dissolve ----------------------------------------------

	// Y 84 takes its order from a table of 6606 positions in CG archive page 197,
	// packed as x + y * 402 behind a 16-byte header.
	static constexpr int DISSOLVE_TABLE_PAGE = 197;
	static constexpr int DISSOLVE_TABLE_OFFSET = 16;
	static constexpr int NR_DISSOLVE_POINTS = 6606;
	static constexpr int DISSOLVE_STRIDE = 402;
	static constexpr int DISSOLVE_X = 124;
	static constexpr int DISSOLVE_Y = 254;

	// The CG is loaded out of sight and then revealed one pixel at a time in
	// the order the table gives.
	void pixel_dissolve(int page) {
		load_cg_to_work_screen(page);

		std::vector<uint8_t> table = ags->load_cg_data(DISSOLVE_TABLE_PAGE);
		if (static_cast<int>(table.size()) < DISSOLVE_TABLE_OFFSET + NR_DISSOLVE_POINTS * 2) {
			WARNING("Y 84: dissolve table page %d is missing or too short (%zu bytes)",
				DISSOLVE_TABLE_PAGE, table.size());
			return;
		}

		for (int i = 0; i < NR_DISSOLVE_POINTS && !terminate; i++) {
			int offset = DISSOLVE_TABLE_OFFSET + i * 2;
			int value = table[offset] | (table[offset + 1] << 8);
			int x = DISSOLVE_X + value % DISSOLVE_STRIDE;
			int y = DISSOLVE_Y + value / DISSOLVE_STRIDE;
			ags->copy_screen(WORK_SCREEN, 0, x, y, x, y, x, y);
			if ((i & 0x2f) == 0x2f)
				wait_ticks(1);
		}
	}

	// --- Y 81 staff roll --------------------------------------------------

	// Y 81 builds the credits in a 312-pixel wide column on the work screen and
	// scrolls a 74-pixel window of it through the band at y=314 on the display.
	static constexpr int CREDIT_X1 = 160;
	static constexpr int CREDIT_X2 = 471;
	static constexpr int CREDIT_BAND_Y = 314;
	static constexpr int CREDIT_BAND_H = 74;
	static constexpr int CREDIT_DATA_OFFSET = 0x4896;
	static constexpr int CREDIT_DATA_END = 0x4b95;

	// In the credit stream, a control byte of 2 or more starts a line at
	// x = control * 8, 1 inserts a 2-pixel gap, and 0 leaves a blank line.
	struct CreditEntry {
		uint8_t control;
		std::string text;
	};
	// Each group fills the column, then scrolls it past the band.
	struct CreditGroup {
		int nr_entries;
		int scroll_steps;
	};

	static constexpr CreditGroup credit_groups[] = {
		{15, 292}, {13, 256}, {15, 309}, {13, 256},
		{15, 292}, {13, 256}, {14, 274}, {1, 45},
	};

	std::string credit_title;
	std::string credit_subtitle;
	std::vector<CreditEntry> credit_entries;

	void load_credit_text(const std::vector<uint8_t>& exe) {
		size_t pos = CREDIT_DATA_OFFSET;
		bool truncated = false;
		auto read_string = [&]() -> std::string {
			size_t start = pos;
			while (pos < exe.size() && exe[pos])
				pos++;
			if (pos == exe.size()) {
				truncated = true;
				return {};
			}
			return std::string(reinterpret_cast<const char*>(&exe[start]), pos++ - start);
		};
		std::string title = read_string();
		std::string subtitle = read_string();
		std::vector<CreditEntry> entries;
		for (const CreditGroup& group : credit_groups) {
			for (int i = 0; i < group.nr_entries && !truncated; i++) {
				if (pos == exe.size()) {
					truncated = true;
					break;
				}
				uint8_t control = exe[pos++];
				entries.push_back({control, control >= 2 ? read_string() : std::string()});
			}
		}
		// The data is intact only if the last entry ends exactly at
		// CREDIT_DATA_END.
		if (truncated || pos != exe.size()) {
			WARNING("%s: unexpected staff roll data", EXECUTABLE_NAME);
			return;
		}
		credit_title = std::move(title);
		credit_subtitle = std::move(subtitle);
		credit_entries = std::move(entries);
	}

	void run_staff_roll() {
		constexpr int TOP_Y = 72;
		constexpr int LINE_HEIGHT = 18;
		constexpr int TITLE_STEPS = 127;

		if (credit_entries.empty())
			return;
		int saved_size = ags->text.font_size;
		uint8_t saved_color = ags->text.font_color;
		ags->text.font_size = 16;
		ags->text.font_color = 15;

		clear_credit_column();
		draw_credit_text(168, TOP_Y, credit_title, false);
		draw_credit_text(256, TOP_Y + 36, credit_subtitle, true);
		scroll_credits(TITLE_STEPS);

		size_t index = 0;
		for (const CreditGroup& group : credit_groups) {
			clear_credit_column();
			int y = TOP_Y;
			for (int i = 0; i < group.nr_entries; i++) {
				const CreditEntry& entry = credit_entries[index++];
				if (entry.control == 1) {
					y += 2;
					continue;
				}
				if (entry.control >= 2)
					draw_credit_text(entry.control * 8, y, entry.text, false);
				y += LINE_HEIGHT;
			}
			scroll_credits(group.scroll_steps);
			if (terminate)
				break;
		}

		ags->text.font_size = saved_size;
		ags->text.font_color = saved_color;
	}

	void clear_credit_column() {
		ags->box_fill(WORK_SCREEN, CREDIT_X1, 0, CREDIT_X2, 399, 0);
	}

	void draw_credit_text(int x, int y, std::string_view text, bool hankaku) {
		int saved_screen = ags->dest_screen;
		bool saved_hankaku = ags->draw_hankaku;
		ags->dest_screen = WORK_SCREEN;
		ags->draw_hankaku = hankaku;
		ags->text.pos.x = x;
		ags->text.pos.y = y;
		ags->draw_text(text);
		ags->draw_hankaku = saved_hankaku;
		ags->dest_screen = saved_screen;
	}

	void scroll_credits(int steps) {
		constexpr int band_rows[] = {315, 317, 384, 386};

		for (int step = 0; step < steps && !terminate; step++) {
			ags->copy_screen(WORK_SCREEN, 0, CREDIT_X1, step, CREDIT_X2,
			                 step + CREDIT_BAND_H - 1, CREDIT_X1, CREDIT_BAND_Y);
			for (int row : band_rows)
				ags->box_fill(0, CREDIT_X1, row, CREDIT_X2, row, 0);
			wait_ticks(get_key() ? 1 : 4);
		}
	}

	// --- save and load ----------------------------------------------------

	// A save file is a 5005-byte resident block followed by the exploration
	// bitmaps of all 60 areas, 1400 bytes each. Everything not listed below is
	// written as zero.
	static constexpr int SAVE_BLOCK_BYTES = 0x138d;
	static constexpr int SAVE_TOTAL_BYTES =
		SAVE_BLOCK_BYTES + NR_EXPLORED_RECORDS * EXPLORED_RECORD_BYTES;
	static constexpr int SAVE_HEADER_BYTES = 112;
	static constexpr int SAVE_OFF_PAGE = 0x070;          // 1-based scenario page
	static constexpr int SAVE_OFF_MUSIC = 0x078;         // page read by play_music
	static constexpr int SAVE_OFF_ADDR = 0x07c;          // offset within the page
	static constexpr int SAVE_OFF_MAP_AREA = 0x080;
	static constexpr int SAVE_OFF_SPRITE_CG = 0x084;     // Y 50
	static constexpr int SAVE_OFF_TILE_CG = 0x088;       // Y 51-53, 4 apart
	static constexpr int SAVE_OFF_VAR = 0x094;           // RND is index 0
	static constexpr int NR_SAVED_VARS = 768;            // the text state follows
	static constexpr int SAVE_OFF_TEXT_STATE = 0x694;
	static constexpr int SAVE_OFF_LABEL_DEPTH = 0x6a6;
	static constexpr int SAVE_OFF_LABEL_NEXT = 0x6a8;    // address of the next push
	static constexpr int SAVE_OFF_LABEL_FRAMES = 0x6ac;
	static constexpr int LABEL_FRAME_BYTES = 4;
	static constexpr int NR_LABEL_FRAMES = 32;
	static constexpr int LABEL_STACK_ADDR = 0x6096;
	static constexpr int SAVE_OFF_PAGE_DEPTH = 0x72c;
	static constexpr int SAVE_OFF_PAGE_NEXT = 0x72e;
	static constexpr int SAVE_OFF_PAGE_FRAMES = 0x732;
	static constexpr int PAGE_FRAME_BYTES = 16;
	static constexpr int NR_PAGE_FRAMES = 64;
	static constexpr int PAGE_STACK_ADDR = 0x611c;
	// Frame layout of the page stack: page, return offset, scenario_body_offset,
	// scenario_header_word, one word each 4 bytes apart.
	static constexpr int PAGE_FRAME_OFF_ADDR = 4;
	static constexpr int PAGE_FRAME_OFF_BODY = 8;
	static constexpr int SCENARIO_BODY_OFFSET = 2;
	static constexpr int SAVE_OFF_VIEW_X = 0xb32;
	static constexpr int SAVE_OFF_VIEW_Y = 0xb34;
	static constexpr int SAVE_OFF_ANIMATION_TICKS = 0xb36;
	static constexpr int SAVE_OFF_EXPLORED = 0xb38;      // the current area's record
	static constexpr int SAVE_OFF_OOB_TILE = 0x12b0;
	static constexpr int SAVE_OFF_STRVAR = 0x12b1;       // 10 slots, X and M
	static constexpr int STRVAR_BYTES = 22;

	static constexpr char save_header[SAVE_HEADER_BYTES + 1] =
		"This is save data for GAKUEN.  [PC-9801:FM-TOWNS:DOS/V:WINDOWS]"
		" on NACT/ADV system (C)1995 ALICE-SOFT         \r\n";

	static void put_le16(std::vector<uint8_t>& buf, int offset, uint16_t value) {
		buf[offset] = value & 0xff;
		buf[offset + 1] = value >> 8;
	}

	static uint16_t get_le16(const std::vector<uint8_t>& buf, int offset) {
		return buf[offset] | (buf[offset + 1] << 8);
	}

	void cmd_l() override {
		int index = cali();
		TRACE("L %d:", index);

		if (index < 1 || index > 26 || !load_game(index))
			RND = 255;
	}

	void cmd_q() override {
		int index = cali();
		TRACE("Q %d:", index);

		RND = (index >= 1 && index <= 26 && save_game(index)) ? 1 : 255;
	}

	bool save_game(int slot) {
		std::vector<uint8_t> block(SAVE_BLOCK_BYTES, 0);
		memcpy(&block[0], save_header, SAVE_HEADER_BYTES);
		put_le16(block, SAVE_OFF_PAGE, sco.page() + 1);
		put_le16(block, SAVE_OFF_MUSIC, mako->current_music);
		put_le16(block, SAVE_OFF_ADDR, sco.current_addr());
		put_le16(block, SAVE_OFF_MAP_AREA, map_current_area);
		put_le16(block, SAVE_OFF_SPRITE_CG, map_sprite_cg_page);
		for (int i = 0; i < NR_TILE_BANKS; i++)
			put_le16(block, SAVE_OFF_TILE_CG + i * 4, map_tile_cg_page[i]);
		for (int i = 0; i < NR_SAVED_VARS; i++)
			put_le16(block, SAVE_OFF_VAR + i * 2, var[i]);
		save_call_stack(block);
		put_le16(block, SAVE_OFF_VIEW_X, map_view_x);
		put_le16(block, SAVE_OFF_VIEW_Y, map_view_y);
		put_le16(block, SAVE_OFF_ANIMATION_TICKS, map_animation_ticks);
		// The current area's record goes into the block as well as into its
		// slot at the end of the file.
		memcpy(&block[SAVE_OFF_EXPLORED], explored_record().data(), EXPLORED_RECORD_BYTES);
		block[SAVE_OFF_OOB_TILE] = map_out_of_bounds_tile;
		save_text_state(block);
		for (int i = 0; i < MAX_STRVAR; i++) {
			size_t len = std::min(tvar[i].size(), (size_t)STRVAR_BYTES - 1);
			memcpy(&block[SAVE_OFF_STRVAR + i * STRVAR_BYTES], tvar[i].data(), len);
		}

		auto fio = FILEIO::open_save(slot, FILEIO_WRITE_BINARY);
		if (!fio) {
			WARNING("cannot write save slot %d", slot);
			return false;
		}
		if (!fio->write(block.data(), block.size()))
			return false;
		for (int i = 0; i < NR_EXPLORED_RECORDS; i++) {
			if (!fio->write(map_explored[i].data(), EXPLORED_RECORD_BYTES))
				return false;
		}
		return true;
	}

	bool load_game(int slot) {
		auto fio = FILEIO::open_save(slot, FILEIO_READ_BINARY);
		if (!fio)
			return false;
		std::vector<uint8_t> data(SAVE_TOTAL_BYTES);
		if (!fio->read(data.data(), data.size())) {
			WARNING("save slot %d is shorter than %d bytes", slot, SAVE_TOTAL_BYTES);
			return false;
		}
		fio.reset();

		for (int i = 0; i < NR_SAVED_VARS; i++)
			var[i] = get_le16(data, SAVE_OFF_VAR + i * 2);
		map_current_area = get_le16(data, SAVE_OFF_MAP_AREA);
		map_sprite_cg_page = get_le16(data, SAVE_OFF_SPRITE_CG);
		for (int i = 0; i < NR_TILE_BANKS; i++)
			map_tile_cg_page[i] = get_le16(data, SAVE_OFF_TILE_CG + i * 4);
		map_view_x = get_le16(data, SAVE_OFF_VIEW_X);
		map_view_y = get_le16(data, SAVE_OFF_VIEW_Y);
		map_animation_ticks = get_le16(data, SAVE_OFF_ANIMATION_TICKS);
		for (int i = 0; i < NR_EXPLORED_RECORDS; i++) {
			memcpy(map_explored[i].data(),
				   &data[SAVE_BLOCK_BYTES + i * EXPLORED_RECORD_BYTES],
				   EXPLORED_RECORD_BYTES);
		}
		// In a save written by GAKUEN.COM the slot at the end of the file is
		// only as new as entering the area, so the block copy wins.
		memcpy(explored_record().data(), &data[SAVE_OFF_EXPLORED], EXPLORED_RECORD_BYTES);
		load_text_state(data);
		for (int i = 0; i < MAX_STRVAR; i++) {
			const char* p = (const char*)&data[SAVE_OFF_STRVAR + i * STRVAR_BYTES];
			tvar[i].assign(p, strnlen(p, STRVAR_BYTES - 1));
		}

		int page = get_le16(data, SAVE_OFF_PAGE);
		sco.page_jump(page - 1, get_le16(data, SAVE_OFF_ADDR));
		load_call_stack(data);

		map_sprite_cg = load_map_sheet(map_sprite_cg_page);
		for (int i = 0; i < NR_TILE_BANKS; i++)
			map_tile_cg[i] = load_map_sheet(map_tile_cg_page[i]);
		if (map_current_area >= 1 && map_current_area != amap_loaded_page)
			load_amap_page(map_current_area);
		clear_map_layers();
		map_out_of_bounds_tile = CELL_EMPTY;
		reset_map_overview();

		mako->play_music(get_le16(data, SAVE_OFF_MUSIC));

		RND = 0;
		map_overview_rebuild_pending = true;
		return true;
	}

	void save_text_state(std::vector<uint8_t>& block) {
		int off = SAVE_OFF_TEXT_STATE;
		put_le16(block, off, ags->menu.font_size);
		put_le16(block, off + 2, ags->text.font_size);
		put_le16(block, off + 4, ags->palette_bank == -1 ? 0 : ags->palette_bank);
		put_le16(block, off + 6, ags->text.font_color);
		put_le16(block, off + 8, ags->menu.font_color);
		put_le16(block, off + 10, ags->menu.frame_color);
		put_le16(block, off + 12, ags->menu.back_color);
		put_le16(block, off + 14, ags->text.frame_color);
		put_le16(block, off + 16, ags->text.back_color);
	}

	void load_text_state(const std::vector<uint8_t>& data) {
		int off = SAVE_OFF_TEXT_STATE;
		ags->menu.font_size = get_le16(data, off);
		ags->text.font_size = get_le16(data, off + 2);
		int bank = get_le16(data, off + 4);
		ags->palette_bank = bank ? bank : -1;
		ags->text.font_color = get_le16(data, off + 6);
		ags->menu.font_color = get_le16(data, off + 8);
		ags->menu.frame_color = get_le16(data, off + 10);
		ags->menu.back_color = get_le16(data, off + 12);
		ags->text.frame_color = get_le16(data, off + 14);
		ags->text.back_color = get_le16(data, off + 16);
	}

	void save_call_stack(std::vector<uint8_t>& block) {
		int labels = 0, pages = 0;
		for (const auto& frame : sco.get_call_stack()) {
			if (frame.is_page_call) {
				if (pages >= NR_PAGE_FRAMES)
					ERROR("page call stack too deep to save");
				int off = SAVE_OFF_PAGE_FRAMES + pages++ * PAGE_FRAME_BYTES;
				put_le16(block, off, frame.page + 1);
				put_le16(block, off + PAGE_FRAME_OFF_ADDR, frame.addr);
				put_le16(block, off + PAGE_FRAME_OFF_BODY, SCENARIO_BODY_OFFSET);
			} else {
				if (labels >= NR_LABEL_FRAMES)
					ERROR("label call stack too deep to save");
				put_le16(block, SAVE_OFF_LABEL_FRAMES + labels++ * LABEL_FRAME_BYTES,
					  frame.addr);
			}
		}
		put_le16(block, SAVE_OFF_LABEL_DEPTH, labels);
		put_le16(block, SAVE_OFF_LABEL_NEXT, LABEL_STACK_ADDR + labels * LABEL_FRAME_BYTES);
		put_le16(block, SAVE_OFF_PAGE_DEPTH, pages);
		put_le16(block, SAVE_OFF_PAGE_NEXT, PAGE_STACK_ADDR + pages * PAGE_FRAME_BYTES);
	}

	// Call this after page_jump(), because label frames carry no page number.
	void load_call_stack(const std::vector<uint8_t>& data) {
		std::vector<Scenario::StackFrame> frames;
		int pages = std::min<int>(get_le16(data, SAVE_OFF_PAGE_DEPTH), NR_PAGE_FRAMES);
		for (int i = 0; i < pages; i++) {
			int off = SAVE_OFF_PAGE_FRAMES + i * PAGE_FRAME_BYTES;
			frames.emplace_back(true, get_le16(data, off) - 1,
								get_le16(data, off + PAGE_FRAME_OFF_ADDR));
		}
		int labels = std::min<int>(get_le16(data, SAVE_OFF_LABEL_DEPTH), NR_LABEL_FRAMES);
		for (int i = 0; i < labels; i++) {
			frames.emplace_back(false, sco.page(),
								get_le16(data, SAVE_OFF_LABEL_FRAMES + i * LABEL_FRAME_BYTES));
		}
		sco.set_call_stack(std::move(frames));
	}

	// Y 82,var. RND selects the save file; a variable that cannot be read from
	// it becomes 0 only when the file itself is missing.
	void load_saved_variable(int index) {
		if (index < 0 || index >= NR_SAVED_VARS) {
			WARNING("Y 82: variable %d is outside the saved area", index);
			return;
		}
		auto fio = (RND >= 1 && RND <= 26)
			? FILEIO::open_save(RND, FILEIO_READ_BINARY) : nullptr;
		if (!fio) {
			var[index] = 0;
			return;
		}
		uint8_t buf[2];
		if (fio->seek(SAVE_OFF_VAR + index * 2, SEEK_SET) == 0 && fio->read(buf, 2))
			var[index] = buf[0] | (buf[1] << 8);
	}
};

} // namespace

NACT_Sys3* create_gakuen_king(const Config& config, const GameId& game_id) {
	return new NACT_GakuenKing(config, game_id);
}

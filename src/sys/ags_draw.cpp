/*
	ALICE SOFT SYSTEM 3 for Win32

	[ AGS - draw ]
*/

#include "ags.h"
#include <tuple>
#include <utility>
#include <vector>
#include <algorithm>
#include <string.h>
#include "dri.h"
#include "game_id.h"
#include "debugger/debugger.h"

namespace {

const int MOSAIC_SIZE = 16;

void mosaic(SDL_Surface* sf) {
	SDL_Surface *tmp = SDL_CreateRGBSurfaceWithFormat(
		0, (sf->w + MOSAIC_SIZE - 1) / MOSAIC_SIZE, (sf->h + MOSAIC_SIZE - 1) / MOSAIC_SIZE, 8, SDL_PIXELFORMAT_INDEX8);
	if (sf->format->palette)
		SDL_SetSurfacePalette(tmp, sf->format->palette);
	// NOTE: SDL_BlitScaled() does not support 8-bit surfaces.
	SDL_SoftStretch(sf, NULL, tmp, NULL);
	SDL_SoftStretch(tmp, NULL, sf, NULL);
	SDL_FreeSurface(tmp);
}

}  // namespace

void AGS::load_cg(int page, int transparent)
{
	if (bmp_prefix) {
		WARNING("not implemented");
		return;
	}
	std::vector<uint8_t> data = acg.load(page);
	if (data.empty())
		return;

	bool set_palette = extract_palette && !ignore_palette.count(page);
	CG cg;
	switch (game_id.sys_ver) {
	case 1:
		switch (game_id.game) {
		case GameId::BUNKASAI:
		case GameId::GAKUEN:
			cg = load_vsp(data, set_palette, transparent);
			break;
		case GameId::INTRUDER:
			// cg = load_gm3(data, transparent);
			cg = load_vsp(data, set_palette, transparent);  // 暫定
			break;
		case GameId::LITTLE_VAMPIRE:
			cg = load_vsp2l(data, transparent);
			break;
		default:
			cg = load_gl3(data, set_palette, transparent);
			break;
		}
		break;
	case 2:
		if (game_id.is(GameId::AYUMI_PROTO)) {
			// あゆみちゃん物語 PROTO
			cg = load_gl3(data, set_palette, transparent);
		} else if (game_id.is(GameId::AYUMI_FD) || game_id.is(GameId::AYUMI_HINT)) {
			// あゆみちゃん物語
			cg = load_vsp(data, set_palette, transparent);
		} else if (game_id.is_sdps()) {
			// Super D.P.S
			cg = load_pms(page, data, set_palette, transparent);
		} else {
			if(data[0x8] == 0) {
				cg = load_vsp(data, set_palette, transparent);
			} else {
				cg = load_pms(page, data, set_palette, transparent);
			}
		}
		break;
	case 3:
		if(data[0x8] == 0) {
			if (game_id.is(GameId::FUNNYBEE_FD) || game_id.is(GameId::FUNNYBEE_CD))
				set_palette = !ignore_palette.count(page);
			cg = load_vsp(data, set_palette, transparent);
		} else {
			set_palette = set_palette || game_id.is(GameId::FUNNYBEE_CD);
			cg = load_pms(page, data, set_palette, transparent);
		}
		break;
	}

	// J command
	if (cg_dest.has_value()) {
		cg.x = cg_dest->x;
		cg.y = cg_dest->y;
		cg_dest = std::nullopt;
	}

	if (extract_cg && cg.surface()) {
		SDL_SetSurfacePalette(cg.surface(), screen_palette);
		if (censor_list.count(page)) {
			mosaic(cg.surface());
		}
		SDL_Rect dstrect = { cg.x, cg.y, cg.width(), cg.height() };
		SDL_BlitSurface(cg.surface(), NULL, hBmpScreen[dest_screen], &dstrect);
		if (dest_screen == 0) {
			draw_screen(dstrect.x, dstrect.y, dstrect.w, dstrect.h);
		}
	}
	if (set_palette) {
		dirty_rect = {0, 0, screen_width, screen_height};
#ifdef ENABLE_DEBUGGER
		if (g_debugger)
			g_debugger->on_palette_change();
#endif
	}
}

void AGS::copy(int sx, int sy, int ex, int ey, int dx, int dy)
{
	int width = ex - sx + 1;
	int height = ey - sy + 1;
	SDL_Rect srcrect = {sx, sy, width, height};
	SDL_Rect destrect = {dx, dy, width, height};

	SDL_BlitSurface(hBmpScreen[src_screen], &srcrect, hBmpScreen[dest_screen], &destrect);

	if(dest_screen == 0) {
		draw_screen(dx, dy, ex - sx + 1, ey - sy + 1);
	}
}

void AGS::gcopy(int gsc, int gde, int glx, int gly, int gsw)
{
	// N88-BASIC時代のコピーコマンド
	int src = (gsw == 0 || gsw == 2) ? 0 : 1;
	int dest = (gsw == 0 || gsw == 3) ? 0 : 1;
	int sx = (gsc % 80) * 8;
	int sy = gsc / 80;
	int dx = (gde % 80) * 8;
	int dy = gde / 80;
	SDL_Rect srcrect = {sx, sy, glx * 8, gly};
	SDL_Rect destrect = {dx, dy, glx * 8, gly};

	SDL_BlitSurface(hBmpScreen[src], &srcrect, hBmpScreen[dest], &destrect);

	if(dest == 0) {
		draw_screen(dx, dy, glx * 8, gly);
	}
}

void AGS::paint(int x, int y, uint8 color)
{
	uint8_t old_color = vram[0][y][x];
	if (old_color == color)
		return;

	int minx = x, maxx = x, miny = y, maxy = y;
	std::vector<std::pair<int, int>> stack;
	stack.push_back({x, y});

	while (!stack.empty()) {
		std::tie(x, y) = stack.back();
		stack.pop_back();
		while (x >= 0 && vram[0][y][x] == old_color) x--;
		x++;
		minx = std::min(x, minx);
		bool span_above = false, span_below = false;
		for (; x < 640 && vram[0][y][x] == old_color; x++) {
			vram[0][y][x] = color;
			if (y > 0) {
				if (!span_above && vram[0][y - 1][x] == old_color) {
					stack.push_back({x, y - 1});
					span_above = true;
				} else if (span_above && vram[0][y - 1][x] != old_color) {
					span_above = false;
				}
			}
			if (y < screen_height - 1) {
				if (!span_below && vram[0][y + 1][x] == old_color) {
					stack.push_back({x, y + 1});
					span_below = true;
				} else if (span_below && vram[0][y + 1][x] != old_color) {
					span_below = false;
				}
			}
		}
		maxx = std::max(x - 1, maxx);
		miny = std::min(y, miny);
		maxy = std::max(y, maxy);
	}
	draw_screen(minx, miny, maxx - minx + 1, maxy - miny + 1);
}

void AGS::draw_box(int index)
{
	if(index == 0) {
		// 全画面消去
		box_fill(dest_screen, 0, 0, 639, 479, 0);
		return;
	}

	Box& b = box[index - 1];
	if (1 <= index && index <= 10) {
		box_fill(dest_screen, b.sx, b.sy, b.ex, b.ey, b.color);
	} else if (11 <= index && index <= 20) {
		box_line(dest_screen, b.sx, b.sy, b.ex, b.ey, b.color);
	}
}

void AGS::draw_mesh(int sx, int sy, int width, int height)
{
	// super d.p.s
	for(int y = sy, h = 0; h < height && y < 480; y += 2, h += 2) {
		for(int x = sx, w = 0; w < width && x < 640; x += 2, w += 2) {
			vram[0][y][x] = 255;
		}
		for(int x = sx + 1, w = 1; w < width && x < 640; x += 2, w += 2) {
			vram[0][y + 1][x] = 255;
		}
	}
	draw_screen(sx, sy, width, height);
}

void AGS::box_fill(int dest, int sx, int sy, int ex, int ey, uint8 color)
{
	SDL_Rect rect = {sx, sy, ex - sx + 1, ey - sy + 1};
	SDL_FillRect(hBmpScreen[dest], &rect, color);
	if(dest == 0) {
		draw_screen(sx, sy, ex - sx + 1, ey - sy + 1);
	}
}

void AGS::box_line(int dest, int sx, int sy, int ex, int ey, uint8 color)
{
	SDL_Rect top    = {sx, sy, ex - sx + 1, 1};
	SDL_Rect bottom = {sx, ey, ex - sx + 1, 1};
	SDL_Rect left   = {sx, sy, 1, ey - sy + 1};
	SDL_Rect right  = {ex, sy, 1, ey - sy + 1};

	SDL_FillRect(hBmpScreen[dest], &top, color);
	SDL_FillRect(hBmpScreen[dest], &bottom, color);
	SDL_FillRect(hBmpScreen[dest], &left, color);
	SDL_FillRect(hBmpScreen[dest], &right, color);
	if(dest == 0) {
		draw_screen(sx, sy, ex - sx + 1, ey - sy + 1);
	}
}


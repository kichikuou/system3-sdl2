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

CG AGS::load_cg_surface(int page, int transparent, uint8_t flags)
{
	if (bmp_prefix) {
		WARNING("not implemented");
		return CG();
	}
	std::vector<uint8_t> data = acg.load(page);
	if (data.empty())
		return CG();

	bool set_palette = (flags & CG_EXTRACT_PALETTE) && !ignore_palette.count(page);
	CG cg;
	switch (game_id.sys_ver) {
	case 1:
		switch (game_id.game) {
		case GameId::BUNKASAI:
		case GameId::GAKUEN_SENKI:
			cg = load_vsp(data, set_palette, transparent, flags);
			break;
		case GameId::INTRUDER:
			// cg = load_gm3(data, transparent, flags);
			cg = load_vsp(data, set_palette, transparent, flags);  // 暫定
			break;
		case GameId::LITTLE_VAMPIRE:
			cg = load_vsp2l(data, transparent, flags);
			break;
		default:
			cg = load_gl3(data, set_palette, transparent, flags);
			break;
		}
		break;
	case 2:
		if (game_id.is(GameId::AYUMI_PROTO)) {
			// あゆみちゃん物語 PROTO
			cg = load_gl3(data, set_palette, transparent, flags);
		} else if (game_id.is(GameId::AYUMI_FD) || game_id.is(GameId::AYUMI_HINT)) {
			// あゆみちゃん物語
			cg = load_vsp(data, set_palette, transparent, flags);
		} else if (game_id.is_sdps()) {
			// Super D.P.S
			cg = load_pms(page, data, set_palette, transparent, flags);
		} else {
			if(data[0x8] == 0) {
				cg = load_vsp(data, set_palette, transparent, flags);
			} else {
				cg = load_pms(page, data, set_palette, transparent, flags);
			}
		}
		break;
	case 3:
		if(data[0x8] == 0) {
			if (game_id.is(GameId::FUNNYBEE_FD) || game_id.is(GameId::FUNNYBEE_CD))
				set_palette = !ignore_palette.count(page);
			cg = load_vsp(data, set_palette, transparent, flags);
		} else {
			set_palette = set_palette || game_id.is(GameId::FUNNYBEE_CD);
			cg = load_pms(page, data, set_palette, transparent, flags);
		}
		break;
	}

	if (cg.surface()) {
		SDL_SetSurfacePalette(cg.surface(), screen_palette);
		if (censor_list.count(page))
			mosaic(cg.surface());
	}

	if (set_palette) {
		dirty_rect = {0, 0, screen_width, screen_height};
#ifdef ENABLE_DEBUGGER
		if (g_debugger)
			g_debugger->on_palette_change();
#endif
	}
	return cg;
}

void AGS::blit_cg(ScreenId dest, CG& cg, const SDL_Rect* src, int dx, int dy)
{
	if (!cg.surface())
		return;
	SDL_Rect dstrect = { dx, dy, src ? src->w : cg.width(), src ? src->h : cg.height() };
	SDL_BlitSurface(cg.surface(), src, hBmpScreen[dest], &dstrect);
	if (dest == SCREEN_FRONT) {
		invalidate_screen(dstrect.x, dstrect.y, dstrect.w, dstrect.h);
	}
}

void AGS::blit_cg(CG& dest, CG& cg, const SDL_Rect* src, int dx, int dy)
{
	if (!cg.surface())
		return;
	SDL_Rect dstrect = { dx, dy, src ? src->w : cg.width(), src ? src->h : cg.height() };
	SDL_BlitSurface(cg.surface(), src, dest.surface(), &dstrect);
}

CG AGS::create_offscreen(int width, int height)
{
	CG cg(0, 0, width, height);
	SDL_SetSurfacePalette(cg.surface(), screen_palette);
	return cg;
}

void AGS::load_cg(int page, int transparent, uint8_t flags)
{
	CG cg = load_cg_surface(page, transparent, flags);
	if (!cg)
		return;

	// J command
	if (cg_dest.has_value()) {
		cg.x = cg_dest->x;
		cg.y = cg_dest->y;
		cg_dest = std::nullopt;
	}

	if (flags & CG_EXTRACT_CG)
		blit_cg(dest_screen, cg, NULL, cg.x, cg.y);
}

void AGS::copy_screen(ScreenId src, ScreenId dest, int sx, int sy, int ex, int ey, int dx, int dy, int transparent_color)
{
	int width = ex - sx + 1;
	int height = ey - sy + 1;
	SDL_Rect srcrect = {sx, sy, width, height};
	SDL_Rect destrect = {dx, dy, width, height};

	SDL_Surface* src_surface = hBmpScreen[src];
	if (transparent_color >= 0)
		SDL_SetColorKey(src_surface, SDL_TRUE, transparent_color);
	SDL_BlitSurface(src_surface, &srcrect, hBmpScreen[dest], &destrect);
	if (transparent_color >= 0)
		SDL_SetColorKey(src_surface, SDL_FALSE, 0);

	if (dest == SCREEN_FRONT)
		invalidate_screen(dx, dy, width, height);
}

void AGS::gcopy(int gsc, int gde, int glx, int gly, int gsw)
{
	// N88-BASIC時代のコピーコマンド
	ScreenId src = (gsw == 0 || gsw == 2) ? SCREEN_FRONT : SCREEN_BACK;
	ScreenId dest = (gsw == 0 || gsw == 3) ? SCREEN_FRONT : SCREEN_BACK;
	int sx = (gsc % 80) * 8;
	int sy = gsc / 80;
	int dx = (gde % 80) * 8;
	int dy = gde / 80;
	SDL_Rect srcrect = {sx, sy, glx * 8, gly};
	SDL_Rect destrect = {dx, dy, glx * 8, gly};

	SDL_BlitSurface(hBmpScreen[src], &srcrect, hBmpScreen[dest], &destrect);

	if(dest == SCREEN_FRONT) {
		invalidate_screen(dx, dy, glx * 8, gly);
	}
}

void AGS::paint(int x, int y, uint8 color)
{
	uint8_t old_color = vram[SCREEN_FRONT][y][x];
	if (old_color == color)
		return;

	int minx = x, maxx = x, miny = y, maxy = y;
	std::vector<std::pair<int, int>> stack;
	stack.push_back({x, y});

	while (!stack.empty()) {
		std::tie(x, y) = stack.back();
		stack.pop_back();
		while (x >= 0 && vram[SCREEN_FRONT][y][x] == old_color) x--;
		x++;
		minx = std::min(x, minx);
		bool span_above = false, span_below = false;
		for (; x < 640 && vram[SCREEN_FRONT][y][x] == old_color; x++) {
			vram[SCREEN_FRONT][y][x] = color;
			if (y > 0) {
				if (!span_above && vram[SCREEN_FRONT][y - 1][x] == old_color) {
					stack.push_back({x, y - 1});
					span_above = true;
				} else if (span_above && vram[SCREEN_FRONT][y - 1][x] != old_color) {
					span_above = false;
				}
			}
			if (y < screen_height - 1) {
				if (!span_below && vram[SCREEN_FRONT][y + 1][x] == old_color) {
					stack.push_back({x, y + 1});
					span_below = true;
				} else if (span_below && vram[SCREEN_FRONT][y + 1][x] != old_color) {
					span_below = false;
				}
			}
		}
		maxx = std::max(x - 1, maxx);
		miny = std::min(y, miny);
		maxy = std::max(y, maxy);
	}
	invalidate_screen(minx, miny, maxx - minx + 1, maxy - miny + 1);
}

void AGS::draw_mesh(int sx, int sy, int width, int height)
{
	// super d.p.s
	for(int y = sy, h = 0; h < height && y < 480; y += 2, h += 2) {
		for(int x = sx, w = 0; w < width && x < 640; x += 2, w += 2) {
			vram[SCREEN_FRONT][y][x] = 255;
		}
		for(int x = sx + 1, w = 1; w < width && x < 640; x += 2, w += 2) {
			vram[SCREEN_FRONT][y + 1][x] = 255;
		}
	}
	invalidate_screen(sx, sy, width, height);
}

void AGS::box_fill(ScreenId dest, int sx, int sy, int ex, int ey, uint8 color)
{
	SDL_Rect rect = {sx, sy, ex - sx + 1, ey - sy + 1};
	SDL_FillRect(hBmpScreen[dest], &rect, color);
	if(dest == SCREEN_FRONT) {
		invalidate_screen(sx, sy, ex - sx + 1, ey - sy + 1);
	}
}

void AGS::box_line(ScreenId dest, int sx, int sy, int ex, int ey, uint8 color)
{
	SDL_Rect top    = {sx, sy, ex - sx + 1, 1};
	SDL_Rect bottom = {sx, ey, ex - sx + 1, 1};
	SDL_Rect left   = {sx, sy, 1, ey - sy + 1};
	SDL_Rect right  = {ex, sy, 1, ey - sy + 1};

	SDL_FillRect(hBmpScreen[dest], &top, color);
	SDL_FillRect(hBmpScreen[dest], &bottom, color);
	SDL_FillRect(hBmpScreen[dest], &left, color);
	SDL_FillRect(hBmpScreen[dest], &right, color);
	if(dest == SCREEN_FRONT) {
		invalidate_screen(sx, sy, ex - sx + 1, ey - sy + 1);
	}
}

void AGS::draw_window(int sx, int sy, int ex, int ey, bool frame, uint8 frame_color, uint8 back_color)
{
	SDL_Rect rect = {sx, sy, ex - sx + 1, ey - sy + 1};
	SDL_FillRect(hBmpScreen[SCREEN_FRONT], &rect, back_color);

	if (frame) {
		SDL_Rect rects[] = {
			{sx + 1, sy + 1, ex - sx - 1, 2},
			{sx + 1, ey - 2, ex - sx - 1, 2},
			{sx + 1, sy + 1, 2, ey - sy - 1},
			{ex - 2, sy + 1, 2, ey - sy - 1},
		};
		SDL_FillRects(hBmpScreen[SCREEN_FRONT], rects, 4, frame_color);
		box_line(SCREEN_FRONT, sx + 4, sy + 4, ex - 4, ey - 4, frame_color);
	}
	invalidate_screen(sx, sy, ex - sx + 1, ey - sy + 1);
}

CG AGS::save_rect(int x, int y, int width, int height)
{
	CG cg(x, y, width, height);
	SDL_SetPaletteColors(cg.palette(), screen_palette->colors, 0, 256);

	SDL_Rect rect = {x, y, width, height};
	SDL_BlitSurface(hBmpScreen[SCREEN_FRONT], &rect, cg.surface(), NULL);
	return cg;
}

void AGS::restore_rect(const CG& cg)
{
	SDL_SetSurfacePalette(hBmpScreen[SCREEN_FRONT], cg.palette());
	SDL_Rect rect = {cg.x, cg.y, cg.width(), cg.height()};
	SDL_BlitSurface(cg.surface(), NULL, hBmpScreen[SCREEN_FRONT], &rect);
	invalidate_screen(cg.x, cg.y, cg.width(), cg.height());
	SDL_SetSurfacePalette(hBmpScreen[SCREEN_FRONT], screen_palette);
}

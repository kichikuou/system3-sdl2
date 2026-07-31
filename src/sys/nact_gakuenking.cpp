#include <algorithm>
#include "nact.h"
#include "ags.h"
#include "mako.h"

namespace {

class NACT_GakuenKing : public NACT_Sys3 {
public:
	NACT_GakuenKing(const Config& config, const GameId& game_id)
		: NACT_Sys3(config, game_id) {}

	void cmd_b() override {
		int p1 = cali();
		int p2 = cali();
		TRACE_UNIMPLEMENTED("B %d,%d:", p1, p2);
	}

	void cmd_i() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();

		switch (p1) {
		case 1:
			if (p3 == 255) {
				for (int i = 0; i < 63; i++) {
					grid_select_data[i] = p2;
				}
			} else if (1 <= p3 && p3 <= 63) {
				grid_select_data[p3 - 1] = p2;
			}
			break;
		case 2:
			switch (p2) {
			case 1:
				RND = grid_select(376, 8, 32, 32, 7, 9, grid_prev_selection);
				if (RND != GRID_SELECT_CANCELED)
					grid_prev_selection = RND;
				wait_key_release();
				break;
			default:
				TRACE_UNIMPLEMENTED("I %d,%d,%d:", p1, p2, p3);
				return;
			}
			break;
		default:
			TRACE_UNIMPLEMENTED("I %d,%d,%d:", p1, p2, p3);
			return;
		}
		TRACE("I %d,%d,%d:", p1, p2, p3);
	}

	void cmd_n() override {
		int p1 = cali();
		int p2 = cali();
		TRACE_UNIMPLEMENTED("N %d,%d:", p1, p2);
	}

	void cmd_p() override {
		int p1 = cali();
		TRACE("P %d:", p1);
		ags->text.font_color = p1;
	}

	void cmd_t() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE_UNIMPLEMENTED("T %d,%d,%d:", p1, p2, p3);
	}

	void cmd_w() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE_UNIMPLEMENTED("W %d,%d,%d:", p1, p2, p3);
	}

	void cmd_y() override {
		int cmd = cali();
		int param = cali();

		TRACE("Y %d,%d:", cmd, param);

		switch (cmd) {
		case 14:
		case 26:
		case 80:
		case 81:
		case 82:
			TRACE_UNIMPLEMENTED("Y %d,%d:", cmd, param);
			break;
		case 9:
			for (int page = 1; page <= 99; page++)
				mako->set_cd_track(page, param ? page : 0);
			break;
		case 20:  // audio device check
			exec_y(14, param);
			break;
		case 25:
			if (0 <= param && param <= 4) {
				ags->menu.back_color = ags->text.back_color =
					(param == 1 || param == 2) ? 0 : 1;
				text_window = param + 1;
				ags->open_text_window(text_window, false);
			}
			break;
		case 83:
			sco.label_stack_clear();
			sco.page_stack_clear();
			break;
		case 118:
			ags->text.pos.x = param;
			break;
		case 119:
			ags->text.pos.y = param;
			break;
		default:
			exec_y(cmd, param);
			break;
		}
	}

	void cmd_z() override {
		int p1 = cali();
		int p2 = cali();
		int p3 = cali();
		TRACE_UNIMPLEMENTED("Z %d,%d,%d:", p1, p2, p3);
	}

private:
	const uint16_t GRID_SELECT_CANCELED = 80;
	uint16_t grid_select_data[63] = {};
	uint16_t grid_prev_selection = 0;

	// I command
	uint16_t grid_select(int left_x, int top_y, int xsize, int ysize, int nr_cols, int nr_rows, int initial_index) {
		int x = initial_index % nr_cols;
		int y = initial_index / nr_cols;
		set_cursor(x * xsize + xsize / 2 + left_x, y * ysize + ysize / 2 + top_y);
		while (!terminate) {
			int mx, my;
			get_cursor(&mx, &my);
			x = (mx - left_x) / xsize;
			y = (my - top_y) / ysize;
			int key = get_key();
			if (key & 0xf) {
				if (key & 1) y--;  // up
				if (key & 2) y++;  // down
				if (key & 4) x--;  // left
				if (key & 8) x++;  // right
				x = std::clamp(x, 0, nr_cols - 1);
				y = std::clamp(y, 0, nr_rows - 1);
				set_cursor(x * xsize + xsize / 2 + left_x, y * ysize + ysize / 2 + top_y);
				wait_key_release();
			} else if (key == 16) {  // enter
				wait_key_release();
				if (0 <= x && x < nr_cols && 0 <= y && y < nr_rows) {
					int sel = nr_cols * y + x;
					if (grid_select_data[sel])
						return sel + 1;
				}
			} else if (key == 32) {  // space
				wait_key_release();
				return GRID_SELECT_CANCELED;
			} else {
				sys_sleep(16);
			}
		}
		return GRID_SELECT_CANCELED;
	}
};

} // namespace

NACT_Sys3* create_gakuen_king(const Config& config, const GameId& game_id) {
	return new NACT_GakuenKing(config, game_id);
}

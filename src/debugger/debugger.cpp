#include "debugger/debugger.h"
#include <algorithm>
#include <map>
#include "common.h"
#include "nact.h"
#include "debugger/frontend.h"

namespace debugger {

namespace {

const int INTERNAL_BREAKPOINT_NO = -1;

struct Breakpoint {
	int page;
	int addr;
	int no;
	uint8_t restore_op;
};

std::map<std::pair<int, int>, Breakpoint> breakpoints;
int next_breakpoint_no = 1;

// Used in State::STOPPED_STEP and State::STOPPED_NEXT
struct {
	int page;
	int line;
} step_exec_state;

bool should_continue_step(const DebugInfo& symbols) {
	if (step_exec_state.line < 0)
		return false;  // line info was not available
	return step_exec_state.page == g_nact->sco.page()
		&& step_exec_state.line == symbols.addr2line(step_exec_state.page, g_nact->sco.cmd_addr());
}

int get_retaddr_if_funcall() {
	int orig_addr = g_nact->sco.current_addr();
	int retaddr = -1;

	g_nact->sco.jump_to(g_nact->sco.cmd_addr());
	int c0 = g_nact->sco.getd();
	if (c0 == BREAKPOINT_INSTRUCTION) {
		auto bp = breakpoints.find({g_nact->sco.page(), g_nact->sco.cmd_addr()});
		if (bp == breakpoints.end())
			sys_error("Illegal BREAKPOINT instruction");
		c0 = bp->second.restore_op;
	}
	switch (c0) {
	case '%':
		if (g_nact->cali() != 0)
			retaddr = g_nact->sco.current_addr();
		break;
	case '\\':
		if (g_nact->sco.getw() != 0)
			retaddr = g_nact->sco.current_addr();
		break;
	}
	g_nact->sco.jump_to(orig_addr);
	return retaddr;
}

} // namespace

Debugger::Debugger(const char *symbols_path, DebuggerMode mode) {
	symbols.load(symbols_path);
	if (mode == DebuggerMode::DAP) {
		// frontend = std::make_unique<DapFrontend>(this);
	} else {
		frontend = std::unique_ptr<Frontend>(Frontend::create_cli(this, symbols));
	}
}

Debugger::~Debugger() = default;

void Debugger::repl(int bp_no) {
	delete_breakpoint(INTERNAL_BREAKPOINT_NO);

	switch (state) {
	case State::STOPPED_STEP:
		if (should_continue_step(symbols))
			return;
		break;
	case State::STOPPED_NEXT:
		if (should_continue_step(symbols)) {
			do_next();
			return;
		}
		break;
	default:
		break;
	}
	frontend->repl(bp_no);
}

void Debugger::on_sleep() {
	frontend->on_sleep();
}

void Debugger::on_palette_change() {
	frontend->on_palette_change();
}

uint8_t Debugger::handle_breakpoint(int page, int addr) {
	auto bp = breakpoints.find({page, addr});
	if (bp == breakpoints.end())
		sys_error("Illegal BREAKPOINT instruction");

	uint8_t restore_op = bp->second.restore_op;

	state = bp->second.no == INTERNAL_BREAKPOINT_NO ? State::STOPPED_NEXT : State::STOPPED_BREAKPOINT;

	repl(bp->second.no);  // this may destroy bp
	return restore_op;
}

bool Debugger::console_vprintf(int lv, const char *format, va_list ap) {
	return false;
}
void Debugger::post_command(void *data) {
}

int Debugger::set_breakpoint(int page, int addr, bool is_internal)
{
	int restore_op = g_nact->sco.write_instruction(page, addr, BREAKPOINT_INSTRUCTION);
	if (restore_op == -1)
		return -1;
	if (restore_op == BREAKPOINT_INSTRUCTION) {
		auto bp = breakpoints.find({page, addr});
		if (bp != breakpoints.end()) {
			sys_error("Illegal BREAKPOINT instruction");
		} else {
			return bp->second.no;
		}
	}
	int no = is_internal ? INTERNAL_BREAKPOINT_NO : next_breakpoint_no++;
	breakpoints[{page, addr}] = {page, addr, no, static_cast<uint8_t>(restore_op)};
	return no;
}

bool Debugger::delete_breakpoint(int no)
{
	for (auto it = breakpoints.begin(); it != breakpoints.end(); ++it) {
		if (it->second.no == no) {
			g_nact->sco.write_instruction(it->second.page, it->second.addr, it->second.restore_op);
			breakpoints.erase(it);
			return true;
		}
	}
	return false;
}

void Debugger::delete_breakpoints_in_page(int page)
{
	auto lower = breakpoints.lower_bound({page, 0});
	auto upper = breakpoints.upper_bound({page, 0xffff});
	for (auto it = lower; it != upper; ++it) {
		g_nact->sco.write_instruction(it->second.page, it->second.addr, it->second.restore_op);
	}
	breakpoints.erase(lower, upper);
}

void Debugger::stepin()
{
	step_exec_state.page = g_nact->sco.page();
	step_exec_state.line = symbols.addr2line(step_exec_state.page, g_nact->sco.cmd_addr());
	state = State::STOPPED_STEP;
}

void Debugger::stepout()
{
	// Set an internal breakpoint at the return address of current frame.
	auto& call_stack = g_nact->sco.get_call_stack();
	if (call_stack.empty()) {
		return;
	}
	const auto& frame = call_stack.back();
	set_breakpoint(frame.page, frame.addr, true);
}

void Debugger::next()
{
	step_exec_state.page = g_nact->sco.page();
	step_exec_state.line = symbols.addr2line(step_exec_state.page, g_nact->sco.cmd_addr());
	state = State::STOPPED_NEXT;
	do_next();
}

void Debugger::do_next()
{
	assert(state == State::STOPPED_NEXT);

	int retaddr = get_retaddr_if_funcall();
	if (retaddr >= 0) {
		set_breakpoint(step_exec_state.page, retaddr, true);
		state = State::RUNNING;
	}
}

std::vector<StackFrame> Debugger::stack_trace()
{
	std::vector<StackFrame> frames;
	const char *src = symbols.page2src(g_nact->sco.page());
	int line = symbols.addr2line(g_nact->sco.page(), g_nact->sco.cmd_addr());
	frames.emplace_back(g_nact->sco.page(), g_nact->sco.cmd_addr(), src, line);

	auto& call_stack = g_nact->sco.get_call_stack();
	std::for_each(call_stack.rbegin(), call_stack.rend(), [&](const auto& frame) {
		const char* src = symbols.page2src(frame.page);
		// -1 because the return address is the address of the next command
		int line = symbols.addr2line(frame.page, frame.addr - 1);
		frames.emplace_back(frame.page, frame.addr, src, line);
	});
	return frames;
}

} // namespace debugger

std::unique_ptr<debugger::Debugger> g_debugger;

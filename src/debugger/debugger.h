#ifndef _DEBUGGER_H_
#define _DEBUGGER_H_

#ifdef ENABLE_DEBUGGER

#include <memory>
#include <stdarg.h>
#include "config.h"
#include "debugger/debug_info.h"

namespace debugger {

const uint8_t BREAKPOINT_INSTRUCTION = 0x0f;

class Frontend;

enum class State {
	RUNNING,
	STOPPED_ENTRY,
	STOPPED_STEP,
	STOPPED_NEXT,
	STOPPED_BREAKPOINT,
	STOPPED_INTERRUPT,
	STOPPED_EXCEPTION,
};

struct StackFrame {
	int page;
	int addr;
	const char *src;
	int line;

	StackFrame(int page, int addr, const char *src, int line)
		: page(page), addr(addr), src(src), line(line) {}
};

class Debugger {
public:
	Debugger(const char *symbols_path, DebuggerMode mode);
	~Debugger();
	void repl(int bp_no);

	// API for VM
	bool trapped() const { return state != State::RUNNING; }
	void on_sleep();
	void on_palette_change();
	uint8_t handle_breakpoint(int page, int addr);
	bool console_vprintf(int lv, const char *format, va_list ap);
	void post_command(void *data);

	// API for frontend
	State get_state() const { return state; }
	void set_state(State s) { state = s; }
	int set_breakpoint(int page, int addr, bool is_internal);
	bool delete_breakpoint(int no);
	void delete_breakpoints_in_page(int page);
	void stepin();
	void stepout();
	void next();
	std::vector<StackFrame> stack_trace();

private:
	State state = State::RUNNING;
	DebugInfo symbols;
	std::unique_ptr<Frontend> frontend;

	void do_next();
};

} // namespace debugger

extern std::unique_ptr<debugger::Debugger> g_debugger;

#endif // ENABLE_DEBUGGER

#endif /* _DEBUGGER_H_ */

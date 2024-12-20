#ifndef _DEBUGGER_FRONTEND_H_
#define _DEBUGGER_FRONTEND_H_

#include <stdarg.h>

namespace debugger {

class Debugger;
class DebugInfo;

class Frontend {
public:
	static Frontend* create_cli(Debugger* backend, const DebugInfo& symbols);
	static Frontend* create_dap(Debugger* backend, const DebugInfo& symbols);

	Frontend(Debugger* backend, const DebugInfo& symbols) : backend(backend), symbols(symbols) {}
	virtual ~Frontend() = default;
	virtual void init() = 0;
	virtual void repl(int bp_no) = 0;
	virtual void on_command(void* data) = 0;
	virtual void on_sleep() = 0;
	virtual void on_palette_change() = 0;
	virtual bool console_output(const char* format, va_list ap) = 0;
protected:
	Debugger* backend;
	const DebugInfo& symbols;
};

} // namespace debugger

#endif /* _DEBUGGER_FRONTEND_H_ */

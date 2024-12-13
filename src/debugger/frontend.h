#ifndef _DEBUGGER_FRONTEND_H_
#define _DEBUGGER_FRONTEND_H_

namespace debugger {

class Debugger;
class DebugInfo;

class Frontend {
public:
	static Frontend* create_cli(Debugger* backend, const DebugInfo& symbols);

	Frontend(Debugger* backend, const DebugInfo& symbols) : backend(backend), symbols(symbols) {}
	virtual ~Frontend() = default;
	virtual void repl(int bp_no) = 0;
	virtual void on_sleep() = 0;
	virtual void on_palette_change() = 0;
	virtual void console_output(int lv, const char *output) = 0;
protected:
	Debugger* backend;
	const DebugInfo& symbols;
};

} // namespace debugger

#endif /* _DEBUGGER_FRONTEND_H_ */

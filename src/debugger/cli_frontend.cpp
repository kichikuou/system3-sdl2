#include "debugger/frontend.h"
#include <stdio.h>
#include "debugger/debug_info.h"
#include "debugger/debugger.h"
#include "encoding.h"
#include "nact.h"
#ifndef _WIN32
#include <signal.h>
#endif

namespace debugger {

namespace {

const char whitespaces[] = " \t\r\n";
const size_t CMD_BUFFER_SIZE = 256;

#ifndef _WIN32
void handle_sigint(int) {
	g_debugger->set_state(State::STOPPED_INTERRUPT);
}

void set_sigint_handler() {
	struct sigaction sa;
	sa.sa_handler = handle_sigint;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sigaction(SIGINT, &sa, nullptr);
}
#endif

} // namespace

class CliFrontend : public Frontend {
public:
	CliFrontend(Debugger* backend, const DebugInfo& symbols) : Frontend(backend, symbols) {
		backend->set_state(State::STOPPED_ENTRY);
#ifndef _WIN32
		set_sigint_handler();
#endif
	}

	~CliFrontend() override = default;

	void init() override {}

	void repl(int bp_no) override {
		if (backend->get_state() == State::STOPPED_BREAKPOINT && bp_no)
			printf("Breakpoint %d\n", bp_no);
		printf("Stopped at %s\n", format_address(g_nact->sco.page(), g_nact->sco.cmd_addr()).c_str());
		backend->set_state(State::RUNNING);

		char buf[CMD_BUFFER_SIZE];
		while (printf("dbg> "), fflush(stdout), fgets(buf, CMD_BUFFER_SIZE, stdin)) {
			// If input line is empty, repeat the last command.
			if (buf[0] == '\n')
				strcpy(buf, default_command);
			else
				strcpy(default_command, buf);

			char *token = strtok(buf, whitespaces);
			if (!token)
				continue;

			const Command *cmd = find_command(token);
			if (cmd) {
				if ((this->*(cmd->func))() == EXIT_REPL)
					return;
			}
		}
	}

	void on_command(void* data) override {}

	void on_sleep() override  {
		if (backend->get_state() == State::STOPPED_INTERRUPT)
			backend->repl(0);
	}

	void on_palette_change() override {
	}

	bool console_output(const char* format, va_list ap) override {
		return false;
	}

private:
	enum CommandResult { CONTINUE_REPL, EXIT_REPL };

	char default_command[CMD_BUFFER_SIZE];

	CommandResult cmd_backtrace();
	CommandResult cmd_break();
	CommandResult cmd_continue();
	CommandResult cmd_delete();
	CommandResult cmd_help();
	CommandResult cmd_list();
	CommandResult cmd_step();
	CommandResult cmd_finish();
	CommandResult cmd_next();
	CommandResult cmd_print();
	CommandResult cmd_quit();
	CommandResult cmd_string();

	struct Command {
		const char *name;
		const char *alias;
		const char *description;
		const char *help;
		CommandResult (CliFrontend::* func)();
	};

	static const Command commands[];

	const Command *find_command(const char *str);

	void cmd_print_help(const Command& cmd) {
		if (cmd.alias)
			printf("%s, %s -- %s\n", cmd.name, cmd.alias, cmd.description);
		else
			printf("%s -- %s\n", cmd.name, cmd.description);
	}

	std::string format_address(int page, int addr) {
		char buf[200];
		const char *src = symbols.page2src(page);
		int line = symbols.addr2line(page, addr);
		if (src && line > 0)
			snprintf(buf, sizeof(buf), "%s:%d", src, line);
		else
			snprintf(buf, sizeof(buf), "%d:0x%x", page, addr);
		return buf;
	}


	bool parse_address(const char *str, int *page, int *addr) {
		// <page>:<address>
		if (sscanf(str, "%i:%i", page, addr) == 2)
			return true;

		// <filename>:<linenum>
		char filename[100];
		int line_no;
		if (sscanf(str, "%[^:]:%i", filename, &line_no) == 2) {
			*page = symbols.src2page(filename);
			if (*page < 0) {
				printf("No source file named %s.\n", filename);
				return false;
			}
			*addr = symbols.line2addr(*page, line_no);
			if (*addr < 0) {
				printf("No line %d in file %s.\n", line_no, filename);
				return false;
			}
			return true;
		}

		*page = g_nact->sco.page();

		// <linenum>
		char *endptr;
		line_no = strtol(str, &endptr, 0);
		if (!*endptr) {
			*addr = symbols.line2addr(g_nact->sco.page(), line_no);
			if (*addr < 0) {
				printf("No line %d in current file.\n", line_no);
				return false;
			}
			return true;
		}

		return false;
	}
};

static const char desc_backtrace[] = "Print backtrace of stack frames.";
static const char * const help_backtrace = nullptr;
CliFrontend::CommandResult CliFrontend::cmd_backtrace() {
	for (const StackFrame &frame : backend->stack_trace()) {
		if (frame.src && frame.line > 0) {
			printf("\t%s:%d\n", frame.src, frame.line);
		} else {
			printf("\t%d:%04x\n", frame.page, frame.addr);
		}
	}
	return CONTINUE_REPL;
}

static const char desc_break[] = "Set breakpoint at specified location.";
static const char help_break[] =
	"Syntax: break <filename>:<linenum> [if <condition>]\n"
	"        break <linenum> [if <condition>]\n"
	"        break <page>:<address> [if <condition>]";

CliFrontend::CommandResult CliFrontend::cmd_break() {
	char *token = strtok(nullptr, whitespaces);
	int page, addr;
	if (!token || !parse_address(token, &page, &addr)) {
		puts(help_break);
		return CONTINUE_REPL;
	}
	int bp_no = backend->set_breakpoint(page, addr, false);
	if (bp_no < 0) {
		printf("Failed to set breakpoint at %d:%04x: invalid address\n", page, addr);
		return CONTINUE_REPL;
	}

	printf("Breakpoint %d at %s\n", bp_no, format_address(page, addr).c_str());
	return CONTINUE_REPL;
}

static const char desc_continue[] = "Continue program execution.";
static const char * const help_continue = nullptr;
CliFrontend::CommandResult CliFrontend::cmd_continue() {
	return EXIT_REPL;
}

static const char desc_delete[] = "Delete breakpoints.";
static const char help_delete[] =
	"Syntax: delete <breakpoint_no>...";
CliFrontend::CommandResult CliFrontend::cmd_delete() {
	char *arg = strtok(nullptr, whitespaces);
	if (!arg) {
		puts(help_delete);
		return CONTINUE_REPL;
	}

	while (arg) {
		char *endptr;
		int bp_no = strtol(arg, &endptr, 0);
		if (*endptr)
			printf("Bad breakpoint number %s\n", arg);
		else {
			if (!backend->delete_breakpoint(bp_no))
				printf("No breakpoint number %d.\n", bp_no);
		}
		arg = strtok(nullptr, whitespaces);
	}
	return CONTINUE_REPL;
}

static const char desc_list[] = "List specified line.";
static const char help_list[] =
	"Syntax: list\n"
	"        list <linenum>\n"
	"        list <filename>:<linenum>\n"
	"\n"
	"With no argument, lists ten lines around current location.";

CliFrontend::CommandResult CliFrontend::cmd_list() {
	char *arg = strtok(NULL, whitespaces);
	int page, addr;
	if (arg) {
		if (!parse_address(arg, &page, &addr)) {
			puts(help_list);
			return CONTINUE_REPL;
		}
	} else {
		page = g_nact->sco.page();
		addr = g_nact->sco.cmd_addr();
	}

	int line_no = symbols.addr2line(page, addr);
	if (line_no <= 0) {
		printf("Cannot determine source location for %x:%x\n", page, addr);
		return CONTINUE_REPL;
	}

	if ((line_no -= 5) <= 0)
		line_no = 1;

	for (int i = 0; i < 10; i++) {
		const char *line = symbols.source_line(page, line_no + i);
		if (!line) {
			if (i == 0)
				printf("No source text for %s:%d\n", symbols.page2src(page), line_no);
			break;
		}
		printf("%d\t%s\n", line_no + i, line);
	}

	// Empty REPL input will print the next 10 lines.
	snprintf(default_command, CMD_BUFFER_SIZE, "l %s:%d", symbols.page2src(page), line_no + 15);

	return CONTINUE_REPL;
}

static const char desc_step[] = "Step program until it reaches a different source line.";
static const char * const help_step = nullptr;
CliFrontend::CommandResult CliFrontend::cmd_step() {
	backend->stepin();
	return EXIT_REPL;
}

static const char desc_finish[] = "Execute until current function returns.";
static const char * const help_finish = nullptr;
CliFrontend::CommandResult CliFrontend::cmd_finish() {
	backend->stepout();
	return EXIT_REPL;
}

static const char desc_next[] = "Step program, proceeding through subroutine calls.";
static const char * const help_next = nullptr;
CliFrontend::CommandResult CliFrontend::cmd_next() {
	backend->next();
	return EXIT_REPL;
}

static const char desc_print[] = "Print value of variable.";
static const char help_print[] =
	"Syntax: print <variable>";

CliFrontend::CommandResult CliFrontend::cmd_print() {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		puts(help_print);
		return CONTINUE_REPL;
	}
	int var = symbols.lookup_variable(arg);
	if (var < 0) {
		printf("Unrecognized variable name \"%s\".\n", arg);
		return CONTINUE_REPL;
	}
	printf("%s = %d\n", arg, g_nact->get_var(var));
	return CONTINUE_REPL;
}

static const char desc_quit[] = "Exit system3-sdl2.";
static const char * const help_quit = nullptr;
CliFrontend::CommandResult CliFrontend::cmd_quit() {
	g_nact->quit(0);
	return EXIT_REPL;
}

static const char desc_string[] = "Print value of string variable.";
static const char help_string[] =
	"Syntax: string <index>";

CliFrontend::CommandResult CliFrontend::cmd_string() {
	char *arg = strtok(NULL, whitespaces);
	if (!arg) {
		puts(help_string);
		return CONTINUE_REPL;
	}

	while (arg) {
		int no = atoi(arg);
		if (no < 1 || no > MAX_STRVAR) {
			printf("Bad string index %s\n", arg);
		} else {
			char *utf = g_nact->encoding->toUtf8(g_nact->get_string(no - 1));
			printf("string[%d] = \"%s\"\n", no, utf);  // TODO: escaping
			free(utf);
		}
		arg = strtok(NULL, whitespaces);
	}
	return CONTINUE_REPL;
}

static const char desc_help[] = "Print list of commands.";
static const char * const help_help = nullptr;

const CliFrontend::Command CliFrontend::commands[] = {
	{"backtrace", "bt",    desc_backtrace, help_backtrace, &CliFrontend::cmd_backtrace},
	{"break",     "b",     desc_break,     help_break,     &CliFrontend::cmd_break},
	{"continue",  "c",     desc_continue,  help_continue,  &CliFrontend::cmd_continue},
	{"delete",    "d",     desc_delete,    help_delete,    &CliFrontend::cmd_delete},
	{"help",      "h",     desc_help,      help_help,      &CliFrontend::cmd_help},
	{"list",      "l",     desc_list,      help_list,      &CliFrontend::cmd_list},
	{"step",      "s",     desc_step,      help_step,      &CliFrontend::cmd_step},
	{"finish",    nullptr, desc_finish,    help_finish,    &CliFrontend::cmd_finish},
	{"next",      "n",     desc_next,      help_next,      &CliFrontend::cmd_next},
	{"print",     "p",     desc_print,     help_print,     &CliFrontend::cmd_print},
	{"quit",      "q",     desc_quit,      help_quit,      &CliFrontend::cmd_quit},
	{"string",    "str",   desc_string,    help_string,    &CliFrontend::cmd_string},
};

const CliFrontend::Command *CliFrontend::find_command(const char *str) {
	for (const Command &cmd : commands) {
		if (!strcmp(str, cmd.name) || (cmd.alias && !strcmp(str, cmd.alias)))
			return &cmd;
	}
	printf("Unknown command \"%s\". Try \"help\".\n", str);
	return nullptr;
}

CliFrontend::CommandResult CliFrontend::cmd_help() {
	char *token = strtok(nullptr, whitespaces);
	if (!token) {
		puts("List of commands:");
		puts("");
		for (auto& cmd : commands)
			cmd_print_help(cmd);
		puts("");
		puts("Type \"help\" followed by command name for full documentation.");
		return CONTINUE_REPL;
	}
	const Command *cmd = find_command(token);
	if (cmd) {
		cmd_print_help(*cmd);
		if (cmd->help) {
			puts("");
			puts(cmd->help);
		}
	}
	return CONTINUE_REPL;
}

// static
Frontend* Frontend::create_cli(Debugger* backend, const DebugInfo& symbols) {
	return new CliFrontend(backend, symbols);
}

} // namespace debugger

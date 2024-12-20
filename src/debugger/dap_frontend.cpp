#define JSON_USE_IMPLICIT_CONVERSIONS 0

#include "debugger/frontend.h"
#include <algorithm>
#include <queue>
#include <SDL.h>
#include "nlohmann/json.hpp"
#include "common.h"
#include "encoding.h"
#include "nact.h"
#include "debugger/debugger.h"
using Json = nlohmann::json;

namespace debugger {

namespace {

enum VariablesReference {
	VREF_GLOBALS = 1,
	VREF_STRINGS,
};

void post_debugger_command(void *data) {
	SDL_Event event = {};
	event.user.type = sdl_custom_event_type;
	event.user.code = DEBUGGER_COMMAND;
	event.user.data1 = data;
	SDL_PushEvent(&event);
}

int read_command_thread(void*) {
	int content_length = -1;
	char header[512];
	while (fgets(header, sizeof(header), stdin)) {
		if (sscanf(header, "Content-Length: %d", &content_length) == 1) {
			continue;
		} else if ((header[0] == '\r' && header[1] == '\n') || header[0] == '\n') {
			if (content_length < 0) {
				fprintf(stderr, "Debug Adapter Protocol error: no Content-Length header\n");
				continue;
			}
			char* buf = (char*)malloc(content_length + 1);
			fread(buf, content_length, 1, stdin);
			buf[content_length] = '\0';
			post_debugger_command(buf);
			content_length = -1;
		} else {
			fprintf(stderr, "Unknown Debug Adapter Protocol header: %s", header);
		}
	}
	post_debugger_command(NULL);  // end of messages
	return 0;
}

} // namespace

class DapFrontend : public Frontend {
public:
	DapFrontend(Debugger* backend, const DebugInfo& symbols) : Frontend(backend, symbols) {
		SDL_CreateThread(read_command_thread, "Debugger", NULL);
	}

	void init() override {
		while (!initialized && !g_nact->is_terminating()) {
			g_nact->process_next_event();
			while (!queue.empty()) {
				char* msg = queue.front();
				queue.pop();
				if (!msg)
					return;
				handle_message(msg);
			}
		}
	}

	void repl(int bp_no) override {
		emit_stopped_event();
		backend->set_state(State::RUNNING);

		bool continue_repl = true;
		while (continue_repl && !g_nact->is_terminating()) {
			g_nact->process_next_event();
			while (!queue.empty()) {
				char* msg = queue.front();
				queue.pop();
				if (!msg)
					return;
				continue_repl = handle_message(msg);
			}
		}
	}

	void on_command(void* data) override {
		queue.push(static_cast<char*>(data));
	}

	void on_sleep() override  {
		while (!queue.empty()) {
			char* msg = queue.front();
			queue.pop();
			if (!msg)
				return;
			handle_message(msg);
		}
		if (backend->get_state() == State::STOPPED_INTERRUPT || backend->get_state() == State::STOPPED_EXCEPTION)
			backend->repl(0);
	}

	void on_palette_change() override {
	}

	bool console_output(const char* format, va_list ap) override {
		return true;
	}

private:
	Json create_source(const char *name) {
		Json source;
		source["name"] = name;
		if (!src_dir.empty())
			source["path"] = src_dir + "/" + name;
		source["sourceReference"] = 0;
		return source;
	}

	std::string format_string_value(const char* str) {
		char *utf = g_nact->encoding->toUtf8(str);
		std::string value = "\"";
		value += utf;
		value += "\"";
		free(utf);
		return value;
	}

	void send_json(Json& json) {
		json["seq"] = ++seq_;
		std::string str = json.dump();
		printf("Content-Length: %zu\r\n\r\n%s", str.size(), str.c_str());
		fflush(stdout);
	}

	void emit_initialized_event() {
		Json json = {
			{"type", "event"},
			{"event", "initialized"},
		};
		send_json(json);
	}

	void emit_stopped_event() {
		std::string reason;
		switch (backend->get_state()) {
		case State::STOPPED_ENTRY: reason = "entry"; break;
		case State::STOPPED_STEP: reason = "step"; break;
		case State::STOPPED_NEXT: reason = "step"; break;
		case State::STOPPED_BREAKPOINT: reason = "breakpoint"; break;
		case State::STOPPED_INTERRUPT: reason = "pause"; break;
		case State::STOPPED_EXCEPTION: reason = "exception"; break;
		default: reason = "unknown"; break;
		}
		Json json = {
			{"type", "event"},
			{"event", "stopped"},
			{"body", {
				{"reason", reason},
				{"allThreadsStopped", true},
			}}
		};
		send_json(json);
	}

	bool handle_message(char* msg) {
		Json json = Json::parse(msg);
		auto type = json["type"].get<std::string>();
		bool continue_repl = true;
		if (type == "request")
			continue_repl = handle_request(json);
		free(msg);
		return continue_repl;
	}

	bool handle_request(Json& request) {
		Json resp;
		resp["type"] = "response";
		resp["request_seq"] = request["seq"];
		resp["command"] = request["command"];
		auto command = request["command"].get<std::string>();
		Json& args = request["arguments"];

		bool continue_repl = true;
		if (command == "initialize") {
			cmd_initialize(args, resp);
		} else if (command == "disconnect") {
			cmd_disconnect(args, resp);
			continue_repl = false;
		} else if (command == "launch") {
			cmd_launch(args, resp);
		} else if (command == "configurationDone") {
			cmd_configurationDone(args, resp);
		} else if (command == "threads") {
			cmd_threads(args, resp);
		} else if (command == "scopes") {
			cmd_scopes(args, resp);
		} else if (command == "variables") {
			cmd_variables(args, resp);
		} else if (command == "setVariable") {
			cmd_setVariable(args, resp);
		} else if (command == "stackTrace") {
			cmd_stackTrace(args, resp);
		} else if (command == "evaluate") {
			cmd_evaluate(args, resp);
		} else if (command == "setBreakpoints") {
			cmd_setBreakpoints(args, resp);
		} else if (command == "continue") {
			cmd_continue(args, resp);
			continue_repl = false;
		} else if (command == "pause") {
			cmd_pause(args, resp);
		} else if (command == "stepIn") {
			cmd_stepIn(args, resp);
			continue_repl = false;
		} else if (command == "stepOut") {
			cmd_stepOut(args, resp);
			continue_repl = false;
		} else if (command == "next") {
			cmd_next(args, resp);
			continue_repl = false;
		} else {
			fprintf(stderr, "Unknown command '%s'\n", command.c_str());
			resp["success"] = false;
			resp["message"] = "Unknown command " + command;
		}
		send_json(resp);
		return continue_repl;
	}

	void cmd_initialize(Json& args, Json& resp) {
		resp["success"] = true;
		resp["body"] = {
			{"supportsConfigurationDoneRequest", true},
			{"supportsEvaluateForHovers", true},
			{"supportsSetVariable", true},
		};
	}

	void cmd_disconnect(Json& args, Json& resp) {
		resp["success"] = true;
		g_nact->quit(0);
	}

	void cmd_launch(Json& args, Json& resp) {
		if (args["noDebug"].is_boolean() && args["noDebug"].get<bool>()) {
			resp["success"] = true;
			initialized = true;
			return;
		}

		if (!symbols.loaded()) {
			resp["success"] = false;
			resp["message"] = "system3: Cannot load debug symbols";
			return;
		}

		if (args["srcDir"].is_string())
			src_dir = args["srcDir"].get<std::string>();
		if (args["stopOnEntry"].is_boolean() && args["stopOnEntry"].get<bool>())
			backend->set_state(State::STOPPED_ENTRY);
		resp["success"] = true;

		emit_initialized_event();
	}

	void cmd_configurationDone(Json& args, Json& resp) {
		initialized = true;
		resp["success"] = true;
	}

	void cmd_threads(Json& args, Json& resp) {
		resp["success"] = true;
		resp["body"] = {
			{"threads", {
				{{"id", 1}, {"name", "main thread"}},
			}}
		};
	}

	void cmd_scopes(Json& args, Json& resp) {
		resp["success"] = true;
		resp["body"] = {
			{"scopes", {
				{
					{"name", "All Variables"},
					{"variablesReference", VREF_GLOBALS},
					{"namedVariables", symbols.num_variables()},
					{"expensive", false}
				},
				{
					{"name", "Strings"},
					{"variablesReference", VREF_STRINGS},
					{"indexedVariables", MAX_STRVAR},
					{"expensive", false}
				},
			}}
		};
	}

	void cmd_variables(Json& args, Json& resp) {
		const Json& start_ = args["start"];
		const Json& count_ = args["count"];
		int start = start_.is_number() ? start_.get<int>() : 0;
		int count = count_.is_number() ? count_.get<int>() : MAX_VAR;

		int var_ref = args["variablesReference"].get<int>();
		if (var_ref == VREF_GLOBALS) {
			resp["success"] = true;
			Json& variables = resp["body"]["variables"];
			int end = std::min(start + count, static_cast<int>(symbols.num_variables()));
			for (int i = start; i < end; i++) {
				Json item = {
					{"name", symbols.variable_name(i)},
					{"value", std::to_string(g_nact->get_var(i))},
					{"variablesReference", 0},
				};
				variables.push_back(std::move(item));
			}
		} else if (var_ref == VREF_STRINGS) {
			resp["success"] = true;
			Json& variables = resp["body"]["variables"];
			int end = std::min(start + count, MAX_STRVAR);
			for (int i = start; i < end; i++) {
				char name[10];
				snprintf(name, sizeof(name), "[%d]", i + 1);
				Json item = {
					{"name", name},
					{"value", format_string_value(g_nact->get_string(i))},
					{"variablesReference", 0},
				};
				variables.push_back(std::move(item));
			}
		} else {
			resp["success"] = false;
			resp["message"] = "Invalid variables reference";
		}
	}

	void cmd_setVariable(Json& args, Json& resp) {
		switch (args["variablesReference"].get<int>()) {
		case VREF_GLOBALS:
			{
				int var = symbols.lookup_variable(args["name"].get<std::string>().c_str());
				if (var < 0) {
					resp["success"] = false;
					resp["message"] = "Invalid variable name";
					return;
				}
				int parsed_value;
				if (sscanf(args["value"].get<std::string>().c_str(), "%i", &parsed_value) != 1) {
					resp["success"] = false;
					resp["message"] = "Syntax error";
					return;
				}
				g_nact->set_var(var, parsed_value);
				resp["success"] = true;
				resp["body"] = {
					{"value", std::to_string(g_nact->get_var(var))},
				};
			}
			break;
		case VREF_STRINGS:
			{
				int index;
				if (sscanf(args["name"].get<std::string>().c_str(), "[%d]", &index) != 1 || index <= 0 || index > MAX_STRVAR) {
					resp["success"] = false;
					resp["message"] = "Invalid string index";
					return;
				}
				index--;  // 1-based to 0-based

				std::string value = args["value"].get<std::string>();
				if (value.size() < 2 || value[0] != '"' || value.back() != '"') {
					resp["success"] = false;
					resp["message"] = "Syntax error";
					return;
				}
				value = value.substr(1, value.size() - 2);

				char* encoded = g_nact->encoding->fromUtf8(value.c_str());
				g_nact->set_string(index, encoded);
				free(encoded);

				resp["success"] = true;
				resp["body"] = {
					{"value", format_string_value(g_nact->get_string(index))},
				};
			}
			break;
		default:
			resp["success"] = false;
			resp["message"] = "Invalid variables reference";
			break;
		}
	}

	void cmd_stackTrace(Json& args, Json& resp) {
		resp["success"] = true;
		Json& body = resp["body"];
		Json& stackFrames = body["stackFrames"];
		int i = 0;
		for (const StackFrame &frame : backend->stack_trace()) {
			Json& item = stackFrames[i++];
			item["id"] = i;
			item["name"] = frame.src;
			item["source"] = create_source(frame.src);
			item["line"] = frame.line;
			item["column"] = 0;
		}
		body["totalFrames"] = i;
	}

	void cmd_evaluate(Json& args, Json& resp) {
		int var = symbols.lookup_variable(args["expression"].get<std::string>().c_str());
		if (var < 0) {
			resp["success"] = false;
			resp["message"] = "Invalid expression";
			return;
		}
		resp["success"] = true;
		resp["body"] = {
			{"result", std::to_string(g_nact->get_var(var))},
			{"variablesReference", 0},
		};
	}

	void cmd_setBreakpoints(Json& args, Json& resp) {
		const auto filename = args["source"]["name"].get<std::string>();
		int page = symbols.src2page(filename.c_str());

		backend->delete_breakpoints_in_page(page);

		resp["success"] = true;
		Json& body = resp["body"];
		Json& out_bps = body["breakpoints"];

		for (const Json& srcbp : args["breakpoints"]) {
			Json& item = out_bps.emplace_back();

			int line = srcbp["line"].get<int>();
			int addr = symbols.line2addr(page, line);
			if (page < 0) {
				item["verified"] = false;
				item["message"] = "no source file named " + filename;
				continue;
			}
			if (addr < 0) {
				item["verified"] = false;
				item["message"] = "no line " + std::to_string(line) + " in file " + filename;
				continue;
			}
			int bp_no = backend->set_breakpoint(page, addr, false);
			if (bp_no < 0) {
				char message[256];
				snprintf(message, sizeof(message), "failed to set breakpoint at %d:0x%x", page, addr);
				item["verified"] = false;
				item["message"] = message;
				continue;
			}

			line = symbols.addr2line(page, addr);
			item["id"] = bp_no;
			item["verified"] = true;
			item["source"] = create_source(filename.c_str());
			item["line"] = line;
		}
	}

	void cmd_continue(Json& args, Json& resp) {
		// TODO: sdl_raiseWindow();
		resp["success"] = true;
	}

	void cmd_pause(Json& args, Json& resp) {
		backend->set_state(State::STOPPED_INTERRUPT);
		resp["success"] = true;
	}

	void cmd_stepIn(Json& args, Json& resp) {
		backend->stepin();
		resp["success"] = true;
	}

	void cmd_stepOut(Json& args, Json& resp) {
		backend->stepout();
		resp["success"] = true;
	}

	void cmd_next(Json& args, Json& resp) {
		backend->next();
		resp["success"] = true;
	}

	std::queue<char*> queue;
	bool initialized = false;
	int seq_ = 0;
	std::string src_dir;
};

// static
Frontend* Frontend::create_dap(Debugger* backend, const DebugInfo& symbols) {
	return new DapFrontend(backend, symbols);
}

} // namespace debugger

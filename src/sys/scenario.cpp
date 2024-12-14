#include "scenario.h"
#include <algorithm>
#include <string.h>
#include "common.h"
#include "encoding.h"

uint8_t Scenario::fetch_command()
{
	if (addr_ < 2 || static_cast<size_t>(addr_) >= data_.size())
		sys_error("Scenario error: invalid address %d:%04x", page_, addr_);

	// Skip SysEng's "new style" marker
	if (page_ == 0 && addr_ == 2 && data_[2] == 'R' && data_[3] == 'E' && data_[4] == 'V')
		addr_ = 5;

	cmd_addr_ = addr_;
	return getd();
}

void Scenario::skip_syseng_string(Encoding *enc, uint8_t terminator)
{
	for (uint8_t c = getd(); c != terminator; c = getd()) {
		if (c == '\\')
			c = getd();
		skip(enc->mblen(c) - 1);
	}
}

void Scenario::get_syseng_string(char* buf, int size, Encoding *enc, uint8_t terminator)
{
	int i = 0;
	for (uint8_t c = getd(); c != terminator; c = getd()) {
		if (c == '\\')
			c = getd();
		int len = enc->mblen(c);
		if (i + len >= size)
			sys_error("String buffer overrun at %d:%04x", page_, cmd_addr_);
		buf[i++] = c;
		for (int j = 1; j < len; ++j)
			buf[i++] = getd();
	}
	buf[i] = '\0';
}

void Scenario::label_call(int label)
{
	if (label == 0) {
		// Return
		auto it = std::find_if(call_stack.rbegin(), call_stack.rend(),
			[](const StackFrame& frame) { return !frame.is_page_call; });
		if (it == call_stack.rend()) {
			WARNING("Label stack underflow");
			return;
		}
		if (it != call_stack.rbegin()) {
			WARNING("Return from non-top label call");
		}
		if (it->page != page_) {
			sys_error("Illegal label return at %d:%04x", page_, cmd_addr_);
		}
		jump_to(it->addr);
		call_stack.erase(std::next(it).base());
	} else {
		call_stack.emplace_back(false, page_, addr_);
		jump_to(label);
	}
}

size_t Scenario::label_stack_size() const
{
	return std::count_if(call_stack.begin(), call_stack.end(),
		[](const StackFrame& frame) { return !frame.is_page_call; });
}

void Scenario::label_stack_pop()
{
	auto it = std::find_if(call_stack.rbegin(), call_stack.rend(),
		[](const StackFrame& frame) { return !frame.is_page_call; });
	if (it == call_stack.rend())
		return;
	call_stack.erase(std::next(it).base());
}

void Scenario::label_stack_clear()
{
	call_stack.erase(std::remove_if(call_stack.begin(), call_stack.end(),
		[](const StackFrame& frame) { return !frame.is_page_call; }), call_stack.end());
}

void Scenario::page_call(int target_page)
{
	if (target_page == 0) {
		// Return
		auto it = std::find_if(call_stack.rbegin(), call_stack.rend(),
			[](const StackFrame& frame) { return frame.is_page_call; });
		if (it == call_stack.rend()) {
			WARNING("Page stack underflow");
			return;
		}
		if (it != call_stack.rbegin()) {
			WARNING("Return from non-top page call");
		}
		page_jump(it->page, it->addr);
		call_stack.erase(std::next(it).base());
	} else {
		call_stack.emplace_back(true, page_, addr_);
		page_jump(target_page, 2);
	}
}

size_t Scenario::page_stack_size() const
{
	return std::count_if(call_stack.begin(), call_stack.end(),
		[](const StackFrame& frame) { return frame.is_page_call; });
}

void Scenario::page_stack_pop()
{
	auto it = std::find_if(call_stack.rbegin(), call_stack.rend(),
		[](const StackFrame& frame) { return frame.is_page_call; });
	if (it == call_stack.rend())
		return;
	call_stack.erase(std::next(it).base());
}

void Scenario::page_stack_clear()
{
	call_stack.erase(std::remove_if(call_stack.begin(), call_stack.end(),
		[](const StackFrame& frame) { return frame.is_page_call; }), call_stack.end());
}

[[noreturn]] void Scenario::unknown_command(uint8_t cmd)
{
	if (cmd >= 0x20 && cmd < 0x7f) {
		sys_error("Unknown Command: '%c' at %d:%04x", cmd, page_, cmd_addr_);
	} else {
		sys_error("Unknown Command: %02x at %d:%04x", cmd, page_, cmd_addr_);
	}
}
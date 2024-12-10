#include "scenario.h"
#include <string.h>
#include "common.h"
#include "encoding.h"

void Scenario::skip_syseng_string(Encoding *enc, uint8_t terminator)
{
	for (uint8_t c = getd(); c != terminator; c = getd()) {
		if (c != '\\')
			ungetd();
		skip(enc->mblen(ptr()));
	}
}

void Scenario::get_syseng_string(char* buf, int size, Encoding *enc, uint8_t terminator)
{
	int start_addr = addr_;

	int i = 0;
	for (uint8_t c = getd(); c != terminator; c = getd()) {
		if (c != '\\')
			ungetd();
		int len = enc->mblen(ptr());
		if (i + len >= size)
			sys_error("String buffer overrun at %d:0x%x", page_, start_addr);
		memcpy(&buf[i], ptr(), len);
		i += len;
		skip(len);
	}
	buf[i] = '\0';
}

void Scenario::label_call(int label)
{
	if (label == 0) {
		if (label_stack.empty()) {
			WARNING("Label stack underflow");
			return;
		}
		jump_to(label_stack.back());
		label_stack.pop_back();
	} else {
		label_stack.push_back(addr_);
		jump_to(label);
	}
}

void Scenario::page_call(int target_page)
{
	if (target_page == 0) {
		if (page_stack.empty()) {
			WARNING("Page stack underflow");
			return;
		}
		auto& pair = page_stack.back();
		page_jump(pair.first, pair.second);
		page_stack.pop_back();
	} else {
		page_stack.emplace_back(page_, addr_);
		page_jump(target_page, 2);
	}
}

[[noreturn]] void Scenario::unknown_command(uint8_t cmd) {
	if (cmd >= 0x20 && cmd < 0x7f) {
		sys_error("Unknown Command: '%c' at %d:0x%d", cmd, page_, cmd_start_addr);
	} else {
		sys_error("Unknown Command: %02x at %d:0x%d", cmd, page_, cmd_start_addr);
	}
}
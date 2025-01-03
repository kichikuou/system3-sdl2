#ifndef _SCENARIO_H_
#define _SCENARIO_H_

#include <utility>
#include <vector>
#include <stdint.h>
#include "dri.h"

class Encoding;

class Scenario {
public:
	void open(const char* file_name) { adisk.open(file_name); }
	bool loaded() const { return adisk.loaded(); }
	void page_jump(int page, int addr);
	int default_addr() { return data_[0] | data_[1] << 8; }

	int page() const { return page_; }
	// start address of the current command
	int cmd_addr() const { return cmd_addr_; }
	// current address, possibly in the middle of a command
	int current_addr() const { return addr_; }

	void jump_to(int addr) { cmd_addr_ = addr_ = addr; }
	void skip(int n) { addr_ += n; }

	uint8_t fetch_command();
	void update_cmd_addr() { cmd_addr_ = addr_; }

	uint8_t getd() { return data_[addr_++]; }
	uint16_t getw() {
		uint16_t w = data_[addr_++];
		w |= data_[addr_++] << 8;
		return w;
	}
	void ungetd() { addr_--; }
	void skip_syseng_string(Encoding *enc, uint8_t terminator);
	void get_syseng_string(char* buf, int size, Encoding *enc, uint8_t terminator);

	void label_call(int label);
	size_t label_stack_size() const;
	void label_stack_pop();
	void label_stack_clear();

	void page_call(int page);
	size_t page_stack_size() const;
	void page_stack_pop();
	void page_stack_clear();

	[[noreturn]] void unknown_command(uint8_t cmd);

	struct StackFrame {
		bool is_page_call;
		uint8_t page;
		uint16_t addr;

		StackFrame(bool is_page_call, uint8_t page, uint16_t addr)
			: is_page_call(is_page_call), page(page), addr(addr) {}
	};
	const std::vector<StackFrame>& get_call_stack() const { return call_stack; }
	int write_instruction(int page, int addr, uint8_t op);

private:
	std::vector<uint8_t>* load_page(int page);

	Dri adisk;
	std::vector<std::vector<uint8_t>> pages_;
	const uint8_t* data_;
	size_t data_size_;
	int page_;
	int addr_;
	int cmd_addr_;
	std::vector<StackFrame> call_stack;
};

#endif // _SCENARIO_H_

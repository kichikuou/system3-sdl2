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
	void page_jump(int page, int addr) {
		data_ = adisk.load(page + 1);
		page_ = page;
		cmd_addr_ = addr_ = addr;
	}
	size_t size() const { return data_.size(); }
	uint8_t& operator[](int i) { return data_[i]; }
	const uint8_t* ptr() const { return &data_[addr_]; }
	int default_addr() { return data_[0] | data_[1] << 8; }

	int page() const { return page_; }
	// start address of the current command
	int cmd_addr() const { return cmd_addr_; }
	// current address, possibly in the middle of a command
	int current_addr() const { return addr_; }

	void jump_to(int addr) { cmd_addr_ = addr_ = addr; }
	void skip(int n) { addr_ += n; }

	uint8_t fetch_command();

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
	size_t label_stack_size() const { return label_stack.size(); }
	void label_stack_pop() { label_stack.pop_back(); }
	void label_stack_clear() { label_stack.clear(); }

	void page_call(int page);
	size_t page_stack_size() const { return page_stack.size(); }
	void page_stack_pop() { page_stack.pop_back(); }
	void page_stack_clear() { page_stack.clear(); }

	[[noreturn]] void unknown_command(uint8_t cmd);

private:
	Dri adisk;
	std::vector<uint8_t> data_;
	int page_;
	int addr_;
	int cmd_addr_;
	std::vector<int> label_stack;
	std::vector<std::pair<int, int>> page_stack;
};

#endif // _SCENARIO_H_

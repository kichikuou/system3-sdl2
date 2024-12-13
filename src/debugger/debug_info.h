#ifndef _DEBUG_INFO_H_
#define _DEBUG_INFO_H_

#include <string>
#include <vector>

namespace debugger {

struct SrcInfo {
	struct Mapping {
		int line;
		int addr;
	};
	std::string filename;
	std::vector<std::string> lines;
	std::vector<Mapping> mappings;

	int line2addr(int line) const;
	int addr2line(int addr) const;
};

class DebugInfo {
public:
	void load(const char *filename);
	bool loaded() const { return loaded_; }
	int src2page(const char *fname) const;
	const char *page2src(int page) const;
	int line2addr(int page, int line) const;
	int addr2line(int page, int addr) const;
	const char *source_line(int page, int line) const;
	size_t num_variables() const { return variables.size(); }
	const char *variable_name(int i) const;
	int lookup_variable(const char *name) const;

private:
	bool loaded_ = false;
	std::vector<SrcInfo> srcs;
	std::vector<std::string> variables;
};

} // namespace debugger

#endif // _DEBUG_INFO_H_

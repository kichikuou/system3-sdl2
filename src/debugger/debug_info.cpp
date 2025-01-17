#include "debugger/debug_info.h"
#include <algorithm>
#include <string.h>
#include "fileio.h"

namespace debugger {

namespace {

inline uint32_t LittleEndian_getDW(uint8_t* buf, int ofs) {
	return buf[ofs] | (buf[ofs + 1] << 8) | (buf[ofs + 2] << 16) | (buf[ofs + 3] << 24);
}

std::string fget4cc(FILEIO& fio) {
	char buf[5] = {0};
	if (!fio.read(buf, 4))
		buf[0] = '\0';
	return buf;
}

bool ensure_srcs(std::vector<SrcInfo>& srcs, uint32_t nr_srcs) {
	if (srcs.empty()) {
		srcs.resize(nr_srcs);
	} else if (nr_srcs != srcs.size()) {
		WARNING("malformed debug information");
		return false;
	}
	return true;
}

bool load_srcs(std::vector<SrcInfo>& srcs, std::vector<uint8_t>& buf) {
	uint32_t nr_srcs = LittleEndian_getDW(buf.data(), 0);
	if (!ensure_srcs(srcs, nr_srcs))
		return false;

	char *p = reinterpret_cast<char*>(buf.data()) + 4;
	for (uint32_t i = 0; i < nr_srcs; i++) {
		srcs[i].filename = p;
		p += strlen(p) + 1;
	}
	return true;
}

void store_src_lines(SrcInfo& info, char *src) {
	while (src) {
		char *eol = strchr(src, '\n');
		if (eol) {
			if (src < eol && eol[-1] == '\r')
				eol[-1] = '\0';
			else
				eol[0] = '\0';
			eol++;
		}
		info.lines.push_back(src);
		src = eol;
	}
}

bool load_scnt(std::vector<SrcInfo>& srcs, std::vector<uint8_t>& buf) {
	uint32_t nr_srcs = LittleEndian_getDW(buf.data(), 0);
	if (!ensure_srcs(srcs, nr_srcs))
		return false;

	char *p = reinterpret_cast<char*>(buf.data()) + 4;
	for (uint32_t i = 0; i < nr_srcs; i++) {
		size_t len = strlen(p);
		store_src_lines(srcs[i], p);
		p += len + 1;
	}
	return true;
}

bool load_line(std::vector<SrcInfo>& srcs, std::vector<uint8_t>& buf) {
	uint32_t nr_srcs = LittleEndian_getDW(buf.data(), 0);
	if (!ensure_srcs(srcs, nr_srcs))
		return false;

	int ofs = 4;
	for (uint32_t i = 0; i < nr_srcs; i++) {
		SrcInfo& info = srcs[i];
		uint32_t nr_mappings = LittleEndian_getDW(buf.data(), ofs);
		ofs += 4;
		info.mappings.resize(nr_mappings);
		for (uint32_t i = 0; i < nr_mappings; i++) {
			info.mappings[i].line = LittleEndian_getDW(buf.data(), ofs);
			info.mappings[i].addr = LittleEndian_getDW(buf.data(), ofs + 4);
			ofs += 8;
		}
	}
	return true;
}

bool load_vari(std::vector<std::string>& variables, std::vector<uint8_t>& buf) {
	uint32_t nr_vars = LittleEndian_getDW(buf.data(), 0);
	char *p = reinterpret_cast<char*>(buf.data()) + 4;
	for (uint32_t i = 0; i < nr_vars; i++) {
		variables.push_back(p);
		p += strlen(p) + 1;
	}
	return true;
}

} // namespace

int SrcInfo::line2addr(int line) const {
	// Find the first mapping with line >= `line`.
	auto it = std::lower_bound(mappings.begin(), mappings.end(), line,
        [](const Mapping& mapping, int line) {
            return mapping.line < line;
        });
	return it != mappings.end() ? it->addr : -1;
}

int SrcInfo::addr2line(int addr) const {
	// Find the last mapping with addr <= `addr`.
	auto it = std::upper_bound(mappings.begin(), mappings.end(), addr,
		[](int addr, const Mapping& mapping) {
			return addr < mapping.addr;
		});
	return it != mappings.begin() ? (it - 1)->line : -1;
}

void DebugInfo::load(const char *path) {
	auto fio = FILEIO::open(path, FILEIO_READ_BINARY);
	if (!fio) {
		WARNING("Cannot open %s: %s", path, strerror(errno));
		return;
	}
	if (fget4cc(*fio) != "DSYM") {
		WARNING("%s: wrong signature", path);
		return;
	}
	int version = fio->getdw();
	if (version != 0) {
		WARNING("%s: unsupported debug info version", path);
		return;
	}

	uint32_t nr_sections = fio->getdw();
	for (uint32_t i = 0; i < nr_sections; i++) {
		std::string tag = fget4cc(*fio);
		uint32_t section_size = fio->getdw();
		std::vector<uint8_t> section_content(section_size - 8);
		if (!fio->read(section_content.data(), section_content.size())) {
			WARNING("%s: %s", path, strerror(errno));
			return;
		}

		if (tag == "SRCS") {
			if (!load_srcs(srcs, section_content)) return;
		} else if (tag == "SCNT") {
			if (!load_scnt(srcs, section_content)) return;
		} else if (tag == "LINE") {
			if (!load_line(srcs, section_content)) return;
		} else if (tag == "VARI") {
			if (!load_vari(variables, section_content)) return;
		} else {
			WARNING("%s: unrecognized section %s", path, tag.c_str());
		}
	}
	if (fio->getc() != EOF) {
		WARNING("%s: broken debug information structure", path);
		return;
	}
	loaded_ = !srcs.empty() && !variables.empty();
}

int DebugInfo::src2page(const char *fname) const {
	for (size_t i = 0; i < srcs.size(); i++) {
		if (!strcasecmp(srcs[i].filename.c_str(), fname))
			return static_cast<int>(i);
	}
	return -1;
}

const char *DebugInfo::page2src(int page) const {
	if (static_cast<size_t>(page) >= srcs.size())
		return NULL;
	return srcs[page].filename.c_str();
}

int DebugInfo::line2addr(int page, int line) const {
	if (static_cast<size_t>(page) >= srcs.size())
		return -1;
	return srcs[page].line2addr(line);
}

int DebugInfo::addr2line(int page, int addr) const {
	if (static_cast<size_t>(page) >= srcs.size())
		return -1;
	return srcs[page].addr2line(addr);
}

const char *DebugInfo::source_line(int page, int line) const {
	if (static_cast<size_t>(page) >= srcs.size())
		return NULL;
	const SrcInfo& info = srcs[page];

	line--; // 1-based to 0-based index
	if (static_cast<size_t>(line) >= info.lines.size())
		return NULL;
	return info.lines[line].c_str();
}

const char *DebugInfo::variable_name(int i) const {
	if (static_cast<size_t>(i) >= variables.size())
		return NULL;
	return variables[i].c_str();
}

int DebugInfo::lookup_variable(const char *name) const {
	for (size_t i = 0; i < variables.size(); i++) {
		if (variables[i] == name)
			return static_cast<int>(i);
	}
	return -1;
}

} // namespace debugger

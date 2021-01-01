#include "msgskip.h"
#include <string.h>
#include "nact.h"
#include "fileio.h"

namespace {

// Parameters for Bloom filter that can contain 47,543 items with 0.5% false
// positive rate.
const int BLOOM_FILTER_SIZE = 1 << 19;	// 64KiB
const int BLOOM_FILTER_HASHES = 8;

const char MSGSKIP_FILENAME[] = "MSGSKIP.DAT";

// Knuth's multiplicative hash.
inline uint32 hash(uint32 x, int s) {
	return (x * 2654435761ULL) >> s;
}

} // namespace

MsgSkip::MsgSkip(NACT* nact)
	: nact(nact),
	  bloom(new uint8[BLOOM_FILTER_SIZE / 8]),
	  dirty(false),
	  flags(MSGSKIP_STOP_ON_UNSEEN | MSGSKIP_STOP_ON_MENU | MSGSKIP_STOP_ON_CLICK),
	  activated(false),
	  enabled(true)
{
	memset(bloom, 0, BLOOM_FILTER_SIZE / 8);
}

MsgSkip::~MsgSkip()
{
	write_to_file();
	delete[] bloom;
}

void MsgSkip::load_from_file()
{
	FILEIO fio;
	if (fio.Fopen(MSGSKIP_FILENAME, FILEIO_READ_BINARY | FILEIO_SAVEDATA)) {
		uint32 size = fio.Fgetw();
		size |= fio.Fgetw() << 16;
		if (size == BLOOM_FILTER_SIZE)
			fio.Fread(bloom, BLOOM_FILTER_SIZE / 8, 1);
		fio.Fclose();
	}
}

bool MsgSkip::write_to_file()
{
	if (!dirty)
		return false;
	FILEIO fio;
	if (fio.Fopen(MSGSKIP_FILENAME, FILEIO_WRITE_BINARY | FILEIO_SAVEDATA)) {
		fio.Fputw(BLOOM_FILTER_SIZE & 0xffff);
		fio.Fputw(BLOOM_FILTER_SIZE >> 16);
		fio.Fwrite(bloom, BLOOM_FILTER_SIZE / 8, 1);
		fio.Fclose();
		dirty = false;
	}
	return true;
}

void MsgSkip::activate(bool enable)
{
	if (activated != enable) {
		activated = enable;
		nact->set_skip_menu_state(enabled, activated);
	}
}

void MsgSkip::on_message(int page, int addr)
{
	if (!bloom)
		return;
	uint32 h1 = hash(page, 5);
	uint32 h2 = hash(addr, 11);
	uint8 unseen = 0;
	for (int i = 0; i < BLOOM_FILTER_HASHES; i++) {
		uint32 h = (h1 + i * h2) % BLOOM_FILTER_SIZE;
		uint8 bit = 1 << (h & 7);
		unseen |= (bloom[h >> 3] & bit) ^ bit;
		bloom[h >> 3] |= bit;
	}
	if (unseen)
		dirty = true;

	bool old_enabled = enabled;
	bool old_activated = activated;
	if (unseen && !(flags & MSGSKIP_SKIP_UNSEEN)) {
		if (flags & MSGSKIP_STOP_ON_UNSEEN)
			activated = false;
		enabled = false;
	} else {
		enabled = true;
	}
	if (enabled != old_enabled || activated != old_activated)
		nact->set_skip_menu_state(enabled, activated);
}

void MsgSkip::set_flags(unsigned val, unsigned mask) {
	flags &= ~mask;
	flags |= (val & mask);
}

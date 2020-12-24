#include "msgskip.h"
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
	  skip_enabled(false),
	  menu_enabled(true)
{
	memset(bloom, 0, BLOOM_FILTER_SIZE / 8);
	FILEIO fio;
	if (fio.Fopen(MSGSKIP_FILENAME, FILEIO_READ_BINARY | FILEIO_SAVEDATA)) {
		uint32 size = fio.Fgetw();
		size |= fio.Fgetw() << 16;
		if (size == BLOOM_FILTER_SIZE)
			fio.Fread(bloom, BLOOM_FILTER_SIZE / 8, 1);
		fio.Fclose();
	}
}

MsgSkip::~MsgSkip()
{
	FILEIO fio;
	if (fio.Fopen(MSGSKIP_FILENAME, FILEIO_WRITE_BINARY | FILEIO_SAVEDATA)) {
		fio.Fputw(BLOOM_FILTER_SIZE & 0xffff);
		fio.Fputw(BLOOM_FILTER_SIZE >> 16);
		fio.Fwrite(bloom, BLOOM_FILTER_SIZE / 8, 1);
		fio.Fclose();
	}
	delete[] bloom;
}

void MsgSkip::enable_skip(bool enable) {
	if (skip_enabled != enable) {
		skip_enabled = enable;
		nact->set_skip_menu_state(menu_enabled, skip_enabled);
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
	if (unseen) {
		if (skip_enabled || menu_enabled) {
			skip_enabled = menu_enabled = false;
			nact->set_skip_menu_state(menu_enabled, skip_enabled);
		}
	} else {
		if (!menu_enabled) {
			menu_enabled = true;
			nact->set_skip_menu_state(menu_enabled, skip_enabled);
		}
	}
}

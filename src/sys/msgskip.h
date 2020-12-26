#ifndef _MSGSKIP_H_
#define _MSGSKIP_H_

#include "common.h"

class NACT;

enum MsgSkipFlags {
	MSGSKIP_SKIP_UNSEEN = 1,
	MSGSKIP_STOP_ON_UNSEEN = 2,
	MSGSKIP_STOP_ON_MENU = 4,
	MSGSKIP_STOP_ON_CLICK = 8,
};

class MsgSkip {
 public:
	MsgSkip(NACT* nact);
	~MsgSkip();

	void activate(bool enable);
	bool is_activated() const { return activated; }
	bool skipping() const { return enabled && activated; }
	void on_message(int page, int addr);
	void load_from_file();
	bool write_to_file();
	unsigned get_flags() const { return flags; }
	void set_flags(unsigned flags, unsigned mask);

 private:
	NACT* nact;
	uint8* bloom;
	bool dirty;
	unsigned flags;
	bool activated;
	bool enabled;
};

#endif

#ifndef _MSGSKIP_H_
#define _MSGSKIP_H_

#include "common.h"

class NACT;

class MsgSkip {
 public:
	MsgSkip(NACT* nact);
	~MsgSkip();

	void enable_skip(bool enable);
	bool is_skip_enabled() const { return skip_enabled; }
	void on_message(int page, int addr);

 private:
	NACT* nact;
	uint8* bloom;
	bool skip_enabled;
	bool menu_enabled;
};

#endif

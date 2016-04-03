#include "mako.h"
#include <fcntl.h>
#include <unistd.h>

static int midifd = -1;

void MAKO::initialize_midi()
{
	midifd = open("/dev/midi1", O_RDWR);
}

void MAKO::release_midi()
{
	if (midifd >= 0)
		close(midifd);
}

void MAKO::reset_midi()
{
	if (midifd < 0)
		return;
	const uint8 gs_reset[11] = {0xf0, 0x41, 0x20, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7};
	write(midifd, gs_reset, 11);
}

void MAKO::send_2bytes(uint8 d1, uint8 d2)
{
	if (midifd < 0)
		return;
	uint8 data[2];
	data[0] = d1;
	data[1] = d2;
	write(midifd, data, 2);
}

void MAKO::send_3bytes(uint8 d1, uint8 d2, uint8 d3)
{
	if (midifd < 0)
		return;
	uint8 data[3];
	data[0] = d1;
	data[1] = d2;
	data[2] = d3;
	write(midifd, data, 3);
}

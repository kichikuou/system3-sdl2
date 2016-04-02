#include <windows.h>
#include <mmsystem.h>
#include "mako.h"

// MIDI音源
union UNION_MIDI_DATA {
	DWORD msg;
	BYTE data[4];
};
static HMIDIOUT hMidi;

void MAKO::initialize_midi()
{
	// MIDI音源の初期化
	midiOutOpen(&hMidi, MIDI_MAPPER, NULL, NULL, CALLBACK_NULL);
}

void MAKO::release_midi()
{
	// MIDI音源の開放
	midiOutClose(hMidi);
}

void MAKO::reset_midi()
{
	// MIDI音源のGS-RESET
	BYTE gs_reset[11] = {0xf0, 0x41, 0x20, 0x42, 0x12, 0x40, 0x00, 0x7f, 0x00, 0x41, 0xf7};
	MIDIHDR mhMidi;
	ZeroMemory(&mhMidi, sizeof(mhMidi));
	mhMidi.lpData = (LPSTR)gs_reset;
	mhMidi.dwBufferLength = 11;
	mhMidi.dwBytesRecorded = 11;

	midiOutPrepareHeader(hMidi, &mhMidi, sizeof(mhMidi));
	midiOutLongMsg(hMidi, &mhMidi, sizeof(mhMidi));
	while(!(mhMidi.dwFlags & MHDR_DONE)) {
		SDL_Delay(10);
	}
	midiOutUnprepareHeader(hMidi, &mhMidi, sizeof(mhMidi));
}

void MAKO::send_2bytes(uint8 d1, uint8 d2)
{
	UNION_MIDI_DATA midi;
	midi.data[0] = (BYTE)d1;
	midi.data[1] = (BYTE)d2;
	midiOutShortMsg(hMidi, midi.msg);
}

void MAKO::send_3bytes(uint8 d1, uint8 d2, uint8 d3)
{
	UNION_MIDI_DATA midi;
	midi.data[0] = (BYTE)d1;
	midi.data[1] = (BYTE)d2;
	midi.data[2] = (BYTE)d3;
	midiOutShortMsg(hMidi, midi.msg);
}

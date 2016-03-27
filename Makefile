CXXFLAGS = -Isrc -Isrc/sys -D_SYSTEM3 -D_DEBUG_CONSOLE

LIBS = -lgdi32 -lgdiplus -lwinmm

OBJS = src/fileio.o \
	src/winmain.o \
	src/sys/ags.o \
	src/sys/ags_bmp.o \
	src/sys/ags_cursor.o \
	src/sys/ags_draw.o \
	src/sys/ags_pms.o \
	src/sys/ags_text.o \
	src/sys/ags_vsp.o \
	src/sys/ags_window.o \
	src/sys/dri.o \
	src/sys/mako.o \
	src/sys/mako_midi.o \
	src/sys/nact.o \
	src/sys/nact_crc32.o \
	src/sys/nact_input.o \
	src/sys/nact_sys3.o \
	src/res3/Script1.o

system3.exe: $(OBJS)
	$(CXX) $(OBJS) $(LIBS) -mwindows -static -o $@

src/res3/Script1.o: src/res3/Script1.rc
	windres $< $@

clean:
	rm -f src/*.o src/sys/*.o src/res3/*.o system3.exe

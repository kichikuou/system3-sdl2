SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LFLAGS := -lSDL2_ttf $(shell sdl2-config --libs)

CXXFLAGS = -Isrc -Isrc/sys -D_SYSTEM3 -D_DEBUG_CONSOLE -MMD $(SDL_CFLAGS) -g

LIBS = -lfreetype -lharfbuzz -lglib-2.0 -lpng -lbz2 -lz -lintl -liconv \
	-luuid -limm32 -lole32 -loleaut32 -lversion -lgdi32 -lgdiplus -lwinmm -lws2_32

OBJS = src/fileio.o \
	src/sdlmain.o \
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
	src/win/nact_win.o \
	src/win/unicode.o \
	src/res3/Script1.o

DEPS := $(OBJS:%.o=%.d)

system3.exe: $(OBJS)
	$(CXX) $(OBJS) $(SDL_LFLAGS) $(LIBS) -static -o $@

src/res3/Script1.o: src/res3/Script1.rc
	windres $< $@

clean:
	rm -f src/*.o src/sys/*.o src/win/*.o src/res3/*.o system3.exe $(DEPS)

-include $(DEPS)

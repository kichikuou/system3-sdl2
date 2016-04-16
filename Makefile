include Makefile.common

ifdef EMSCRIPTEN
  include Makefile.emscripten
else
  UNAME := $(shell uname -s)
  ifeq ($(UNAME),Linux)
    include Makefile.linux
  else
    include Makefile.win
  endif
endif

-include $(DEPS)

XBE_TITLE = fallout1-ce-xbox
GEN_XISO  = $(XBE_TITLE).iso

SRCS = \
  src/audio_engine.cpp \
  src/fps_limiter.cpp \
  src/movie_lib.cpp \
  src/platform_compat.cpp \
  src/pointer_registry.cpp \
  $(wildcard src/game/*.cpp) \
  $(wildcard src/int/*.cpp) \
  $(wildcard src/int/support/*.cpp) \
  $(wildcard src/plib/**/*.cpp) \
  $(wildcard third_party/adecode/adecode/src/*.c)

NXDK_DIR = /home/milenko/nxdk
NXDK_SDL = y
NXDK_CXX = y
NXDK_NET = y

CFLAGS   += -Isrc -Ithird_party -g
CXXFLAGS += -Isrc -Ithird_party -std=c++17 -g

include $(NXDK_DIR)/Makefile

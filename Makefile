export PATH := /usr/bin:/bin:$(PATH)

# Use all CPU cores
MAKEFLAGS += -j$(shell nproc)

CXX       = g++
OPT      ?= -O2
CXXFLAGS  = -Wall -Wextra $(OPT) -std=c++20 -MMD -MP

# Include paths for subfolders
CXXFLAGS += -I. -Inpc -Iplayer -Iskills -Iworld
LDFUSE    = -B/usr/bin

# Pick fastest linker
ifneq ($(shell command -v mold 2>/dev/null),)
    LDFUSE += -fuse-ld=mold
else ifneq ($(shell command -v ld.lld 2>/dev/null),)
    LDFUSE += -fuse-ld=lld
endif

TARGET = ./game

# Update this if your raylib is elsewhere
RAYLIB_ROOT = $(HOME)/raylib

RAYLIB_LIBFILE := $(firstword $(shell find $(RAYLIB_ROOT) \( -name 'libraylib.so' -o -name 'libraylib.so.*' -o -name 'libraylib.a' \) 2>/dev/null | sort))

ifeq ($(RAYLIB_LIBFILE),)
$(error No libraylib found under $(RAYLIB_ROOT))
endif

RAYLIB_LIBDIR := $(dir $(RAYLIB_LIBFILE))
RAYLIB_DIR    := $(patsubst %/,%,$(dir $(patsubst %/,%,$(RAYLIB_LIBDIR))))

INCLUDES = -isystem $(RAYLIB_DIR)/include

LDFLAGS = \
    -Wl,-rpath,$(RAYLIB_LIBDIR)

SYS_LIBS = \
    -lpthread \
    -ldl \
    -lX11 \
    -lGL \
    -lm \
    -lrt \
    -lXrandr \
    -lXinerama \
    -lXcursor \
    -lXrender \
    -lXext \
    -lXi

LIBS = $(RAYLIB_LIBFILE) $(SYS_LIBS)

# Find all .cpp files in root + subfolders
SRCS = $(sort $(filter-out pch.cpp,$(wildcard *.cpp) $(wildcard */*.cpp)))
OBJS = $(SRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)

PCH = pch.h.gch

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET) $(LDFUSE) $(LDFLAGS) $(LIBS)

$(PCH): pch.h
	$(CXX) $(CXXFLAGS) $(INCLUDES) -x c++-header pch.h -o $(PCH)

%.o: %.cpp | $(PCH)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -include pch.h -c $< -o $@

clean:
	rm -f *.o *.d */*.o */*.d $(PCH) $(TARGET)

run: $(TARGET)
	$(TARGET)

which-ld:
	@which -a ld
	@$(CXX) $(LDFUSE) -print-prog-name=ld

-include $(DEPS)

.PHONY: all clean run which-ld
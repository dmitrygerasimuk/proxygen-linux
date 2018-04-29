#
# Cross Platform Makefile
# Compatible with MSYS2/MINGW, Ubuntu 14.04.1 and Mac OS X
#
# You will need SDL2 (http://www.libsdl.org):
# Linux:
#   apt-get install libsdl2-dev
# Mac OS X:
#   brew install sdl2
# MSYS2:
#   pacman -S mingw-w64-i686-SDL
#

#CXX = g++
#CXX = clang++

EXE = ../build/main 
SOURCES = main.cpp TextEditor.cpp imgui_impl_sdl_gl3.cpp
SOURCES += ../libs/imgui/imgui.cpp ../libs/imgui/imgui_demo.cpp ..libs/imgui/imgui_draw.cpp
SOURCES += ../imgui/libs/gl3w/GL/gl3w.c
OBJS = $(addsuffix .o, $(basename $(notdir $(SOURCES))))

UNAME_S := $(shell uname -s)


ifeq ($(UNAME_S), Linux) #LINUX
	ECHO_MESSAGE = "Linux"
	LIBS = ./libnfd.a -lGL -ldl -lpthread `sdl2-config --libs` `pkg-config --libs gtk+-3.0` 

	CXXFLAGS = -I. -I../libs/imgui -I../libs/imgui/libs/gl3w `sdl2-config --cflags` `pkg-config --cflags gtk+-3.0`
	CXXFLAGS += -Wall -Wformat
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(UNAME_S), Darwin) #APPLE
	ECHO_MESSAGE = "Mac OS X"
	LIBS = -framework OpenGL -framework Cocoa -framework IOKit -framework CoreVideo `sdl2-config --libs` ./libnfd.a

	CXXFLAGS = -I./ -I../libs/imgui/ -I../libs/imgui/libs/gl3w -I/usr/local/include `sdl2-config --cflags`
	CXXFLAGS += -Wall -Wformat
	CFLAGS = $(CXXFLAGS)
endif

ifeq ($(findstring MINGW,$(UNAME_S)),MINGW)
   ECHO_MESSAGE = "Windows"
   LIBS = -lgdi32 -lopengl32 -limm32 `pkg-config --static --libs sdl2`

   CXXFLAGS = -I../../ -I../libs/gl3w `pkg-config --cflags sdl2`
   CXXFLAGS += -Wall -Wformat
   CFLAGS = $(CXXFLAGS)
endif


%.o:%.cpp
	$(CXX) $(CXXFLAGS) -c -std=c++14 -o $@ $<
#%.o:ImGuiColorTextEdit/%.cpp
	$(CXX) $(CXXFLAGS) -std=c++14 -c -o $@ $<

%.o:../libs/imgui/%.cpp
	$(CXX) $(CXXFLAGS) -std=c++14 -c -o $@ $<

%.o:../libs/imgui/libs/gl3w/GL/%.c
	$(CC) $(CFLAGS) -std=c++14 -c -o $@ $<

all: $(EXE)
	@echo Build complete for $(ECHO_MESSAGE)

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

clean:
	rm -f $(EXE) $(OBJS)

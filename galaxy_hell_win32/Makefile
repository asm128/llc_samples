IncDir			:= ../../llc/llc ../../llc/llc_engine ../galaxy_hell
Libs			:= galaxy_hell.a llc.a
GccLibs			:= xcb xcb-image xkbcommon xkbcommon-x11 pthread
Platform		:= Linux.x64
ProjectName		:= galaxy_hell
Configuration	:= Debug
ProjectDir		?= .
SolutionDir		?= $(ProjectDir)/..

C_DEFS			:= -DNOMINMAX -DLLC_XCB -DDEBUG
OPT				:= 0

ifeq ($(DEBUG), 0)
	Configuration	:= Release
	OPT				:= 3
	C_DEFS			:= -DNOMINMAX -DLLC_XCB
endif

OutDir		:= $(SolutionDir)/../$(Platform).$(Configuration)
IntDir		:= $(SolutionDir)/../obj/$(Platform).$(Configuration)/$(ProjectName)
EngineIntDir	:= $(IntDir)/llc_engine

LIBS		:= $(patsubst %, $(OutDir)/%, $(Libs))
GCCLIBS		:= $(patsubst %, -l%, $(GccLibs))
INCDIR		:= $(patsubst %,-I%,$(IncDir))

CFLAGS		:= -std=c++20 -Wall -fpermissive -Wno-changes-meaning -O$(OPT) $(C_DEFS) $(INCDIR)
LLCFLAGS	:= -std=c++20 -Wall -fpermissive -Wno-changes-meaning -Wno-invalid-constexpr -O$(OPT) -I../../zlib $(C_DEFS) -include stdio.h -include stdlib.h
CC			:= g++

OBJ			:= $(IntDir)/galaxy_hell_win32.o
EngineSrc	:= llc_celestial_body.cpp llc_engine_orbit.cpp llc_engine_planetary_system.cpp llc_engine_renderbuffer.cpp llc_rigid_body.cpp llc_sun.cpp llc_engine.cpp llc_engine_collision.cpp llc_engine_explorer.cpp llc_engine_scene.cpp llc_engine_shader.cpp llc_planetary_system.cpp
EngineObj	:= $(patsubst %.cpp,$(EngineIntDir)/%.o,$(EngineSrc))

$(IntDir)/%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(EngineIntDir)/%.o: ../../llc/llc_engine/%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(OutDir)/llc.a:
	$(MAKE) -C ../../llc/llc DEBUG=$(DEBUG) CFLAGS="$(LLCFLAGS)" build

$(OutDir)/galaxy_hell.a:
	$(MAKE) -C ../galaxy_hell DEBUG=$(DEBUG) build

build: $(IntDir) $(EngineIntDir) $(OBJ) $(EngineObj) $(LIBS)
	mkdir -p $(OutDir)
	$(CC) $(OBJ) $(EngineObj) $(CFLAGS) -o $(OutDir)/$(ProjectName).bin $(LIBS) $(GCCLIBS)

$(IntDir) $(EngineIntDir):
	mkdir -p $@

.PHONY: build clean

clean:
	rm -rf $(IntDir)

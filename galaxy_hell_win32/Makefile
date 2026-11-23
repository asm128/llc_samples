FRAMEBUFFER		?= 0
ExtraIncDir		?=
ExtraLibDir		?=
LLCExcludeSrc	?=
IncDir			:= ../../llc/llc ../galaxy_hell $(ExtraIncDir)
Libs			:= galaxy_hell.a llc.a
GccLibs			:= xcb xcb-image xkbcommon xkbcommon-x11 pthread
Platform		:= Linux.x64
ProjectName		:= galaxy_hell
Configuration	:= Release
ProjectDir		?= .
SolutionDir		?= $(ProjectDir)/..

C_DEFS			:= -DNOMINMAX -DLLC_XCB
OPT				:= 3

ifeq ($(FRAMEBUFFER), 1)
	Platform		= LinuxFB.x64
	GccLibs			= :libxkbcommon.so.0 pthread
	C_DEFS			= -DNOMINMAX -DLLC_FRAMEBUFFER
	LLCExcludeSrc	= llc_process_broker.cpp llc_test_core.cpp
endif

ifeq ($(DEBUG), 1)
	Configuration	= Debug
	OPT				= 0
	C_DEFS			+= -DDEBUG
endif

OutDir		:= $(SolutionDir)/../$(Platform).$(Configuration)
IntDir		:= $(SolutionDir)/../obj/$(Platform).$(Configuration)/$(ProjectName)

LIBS		:= $(patsubst %, $(OutDir)/%, $(Libs))
GCCLIBS		:= $(patsubst %, -l%, $(GccLibs))
INCDIR		:= $(patsubst %,-I%,$(IncDir))
LIBDIR		:= $(patsubst %,-L%,$(ExtraLibDir))

CFLAGS		:= -std=c++23 -Wall -fpermissive -Wno-changes-meaning -O$(OPT) $(C_DEFS) $(INCDIR)
LLCFLAGS	:= -std=c++23 -Wall -O$(OPT) -I../../zlib $(patsubst %,-I%,$(ExtraIncDir)) $(C_DEFS) -include stdio.h -include stdlib.h
CC			:= g++

OBJ			:= $(IntDir)/galaxy_hell_win32.o

$(IntDir)/%.o: %.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

$(OutDir)/llc.a:
	$(MAKE) -C ../../llc/llc DEBUG=$(DEBUG) FRAMEBUFFER=$(FRAMEBUFFER) ExtraIncDir="$(ExtraIncDir)" ExcludeSrc="$(LLCExcludeSrc)" CFLAGS="$(LLCFLAGS)"

$(OutDir)/galaxy_hell.a:
	$(MAKE) -C ../galaxy_hell DEBUG=$(DEBUG) FRAMEBUFFER=$(FRAMEBUFFER) ExtraIncDir="$(ExtraIncDir)"

build: $(IntDir) $(OBJ) $(LIBS)
	mkdir -p $(OutDir)
	$(CC) $(OBJ) $(CFLAGS) -o $(OutDir)/$(ProjectName).bin $(LIBS) $(LIBDIR) $(GCCLIBS)

$(IntDir):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(IntDir)

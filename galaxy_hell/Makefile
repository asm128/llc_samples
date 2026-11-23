FRAMEBUFFER		?= 0
ExtraIncDir		?=
IncDir			:= ../../llc/llc $(ExtraIncDir)
Platform		:= Linux.x64
ProjectName		:= galaxy_hell
Configuration	:= Release
ProjectDir		?= .
SolutionDir		?= $(ProjectDir)/..

C_DEFS			:= -DNOMINMAX -DLLC_XCB
OPT				:= 3

ifeq ($(FRAMEBUFFER), 1)
	Platform		= LinuxFB.x64
	C_DEFS			= -DNOMINMAX -DLLC_FRAMEBUFFER
endif

ifeq ($(DEBUG), 1)
	Configuration	= Debug
	OPT				= 0
	C_DEFS			+= -DDEBUG
endif

OutDir	= $(SolutionDir)/../$(Platform).$(Configuration)
IntDir	= $(SolutionDir)/../obj/$(Platform).$(Configuration)/$(ProjectName)

OBJDIRS	:= $(patsubst %, $(IntDir), $(ProjectName))
INCDIR	:= $(patsubst %,-I%,$(IncDir))

CFLAGS	:= -std=c++23 -Wall -fpermissive -Wno-changes-meaning -O$(OPT) $(INCDIR)
CC		:= g++

_OBJ	:= $(patsubst %.cpp,%.o,$(wildcard *.cpp))
OBJ		:= $(patsubst %,$(IntDir)/%,$(_OBJ))

$(IntDir)/%.o: %.cpp
	$(CC) $(CFLAGS) $(C_DEFS) -c -o $@ $<

build: $(OBJDIRS) $(OBJ)
	mkdir -p $(OutDir)
	ar rcs $(OutDir)/$(ProjectName).a $(OBJ)

$(OBJDIRS):
	mkdir -p $@

.PHONY: clean

clean:
	rm -rf $(IntDir)

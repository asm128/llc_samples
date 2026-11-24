IncDir			:= ../../llc/llc ../../llc/llc_engine
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

OutDir	:= $(SolutionDir)/../$(Platform).$(Configuration)
IntDir	:= $(SolutionDir)/../obj/$(Platform).$(Configuration)/$(ProjectName)

OBJDIRS	:= $(IntDir)
INCDIR	:= $(patsubst %,-I%,$(IncDir))

CFLAGS	:= -std=c++20 -Wall -fpermissive -Wno-changes-meaning -O$(OPT) $(INCDIR)
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

.PHONY: build clean

clean:
	rm -rf $(IntDir)

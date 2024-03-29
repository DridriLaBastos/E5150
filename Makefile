CXXSRC = $(wildcard src/*.cpp) main.cpp
CXXOBJ = $(CXXSRC:.cpp=.o)

CPUSRC = $(wildcard src/cpu/*.cpp)
FLOPPYSRC = $(wildcard src/floppy/*.cpp)

PCHSRC = include/pch.hpp
PCHOBJ = $(PCHSRC).pch
ASMSRC = $(wildcard test/*.s)
ASMOBJ = $(ASMSRC:.s=.bin)

CXX := $(CXX) --std=c++17 -O1

OBJ = $(CXXOBJ)
SPDLOG_INCLUDE = third-party/spdlog/include
CATCH2_INCLUDE = third-party/catch2/include

DEBUG = 1

ifeq ($(DEBUG),1)
	CPPFLAGS := $(CPPFLAGS) -DDEBUG_BUILD
endif

CPPPCH_FLAGS := $(CPPFLAGS) -I. -Iinclude -I$(SPDLOG_INCLUDE)
CPPFLAGS := $(CPPFLAGS) -Iinclude -I$(SPDLOG_INCLUDE) -include $(PCHSRC)
CXXFLAGS := $(CXXFLAGS) -Wall -Wextra -Wno-switch

PRODUCT = epc.out
TESTING_PRODUCT = test.out

.PHONY: clean mrproper cleanpch asm run testing all

all: $(PRODUCT)
asm: $(ASMOBJ)
testing: $(TESTING_PRODUCT)

$(PRODUCT): $(OBJ)
	$(CXX) $(LDFLAGS) $^ -lxed -lsfml-system -o $@

$(TESTING_PRODUCT): $(wildcard testing/*.cpp) $(CXXOBJ)
	$(CXX) $(CPPFLAGS) -I$(CATCH2_INCLUDE) $(CXXFLAGS) $^ -o $@

$(PCHOBJ): $(PCHSRC) include/util.hpp include/config.hpp
	$(CXX) $(CPPPCH_FLAGS) $(CXXFLAGS) -x c++-header $(PCHSRC) -o $@

%.o:%.cpp $(PCHOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

%.bin: %.s test/util.inc
	nasm -f bin $< -o $@

src/cpu.o: src/cpu.cpp $(CPUSRC) $(PCHOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

src/floppy.o: src/floppy.cpp $(FLOPPYSRC) $(PCHOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

run:
	@./$(PRODUCT)

clean:
	rm $(CXXOBJ) $(ASMOBJ)

mrproper:
	rm $(PRODUCT)

cleanpch:
	rm $(PCHOBJ)
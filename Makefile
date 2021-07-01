DEPFLAGS = -MM -MF

CXXSRC = main.cpp $(wildcard src/*.cpp)
CXXOBJ = $(CXXSRC:.cpp=.o)
DEPOBJ = $(CXXSRC:.cpp=.d)

#CPUSRC = $(wildcard src/cpu/*.cpp)
#CPUOBJ = $(CPUSRC: .cpp=.o)

#FLOPPYSRC = $(wildcard src/floppy/*.cpp)
#FLOPPYOBJ = $(FLOPPYSRC: .cpp=.o)

PCHSRC = include/pch.hpp
PCHOBJ = $(PCHSRC).pch

ASMSRC = $(wildcard test/*.s)
ASMOBJ = $(ASMSRC:.s=.bin)

CXX := $(CXX) --std=c++17 -O3 -flto

SPDLOG_INCLUDE	= third-party/spdlog/include
CATCH2_INCLUDE	= third-party/catch2/include
THIRD_PARTY_INCLUDE	= third-party/include
THIRD_PARTY_INCLUDE_FLAGS = -I $(SPDLOG_INCLUDE) -I $(THIRD_PARTY_INCLUDE)

THIRD_PARTY_LIB = third-party/lib/libxed.a
DEBUG = 0

ifeq ($(DEBUG),1)
	CPPFLAGS := $(CPPFLAGS) -DDEBUG_BUILD
endif

COMMON_CPPFLAGS := $(CPPFLAGS) $(THIRD_PARTY_INCLUDE_FLAGS) -Iinclude
CPPPCH_FLAGS := $(COMMON_CPPFLAGS) -I.
CPPFLAGS := $(COMMON_CPPFLAGS) -include-pch $(PCHOBJ)
CXXFLAGS := $(CXXFLAGS) -Wall -Wextra -Wno-switch
LDFLAGS := $(LDFLAGS) -Lthird-party/lib

PRODUCT = epc.out
TESTING_PRODUCT = test.out

.PHONY: clean mrproper cleanpch asm run testing third-party dep

$(PRODUCT): $(CXXOBJ)
	$(CXX) $(LDFLAGS) $(CXXOBJ) -lxed -lsfml-system -o $@
	install_name_tool -change @rpath/libsfml-system.2.5.dylib $(DEV)/lib/libsfml-system.2.5.dylib $@

$(CXXOBJ): %.o: %.d

dep: $(DEPOBJ)
asm: $(ASMOBJ)
testing: $(TESTING_PRODUCT)
third-party: $(THIRD_PARTY_LIB)

$(TESTING_PRODUCT): $(wildcard testing/*.cpp) $(CXXOBJ)
	$(CXX) $(CPPFLAGS) -I$(CATCH2_INCLUDE) $(CXXFLAGS) $^ -o $@

$(THIRD_PARTY_LIB):
	$(MAKE) -C third-party

$(PCHOBJ): $(PCHSRC)# $(THIRD_PARTY_LIB)
	$(CXX) $(CPPPCH_FLAGS) $(CXXFLAGS) -x c++-header $(PCHSRC) -o $@

%.d: %.cpp $(PCHOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) $@ $<

%.bin: %.s test/util.inc
	nasm -f bin $< -o $@

run:
	@./$(PRODUCT)

clean:
	rm -f $(CXXOBJ)

cleanpch:
	rm -f $(PCHOBJ)

mrproper:
	rm -f $(PRODUCT)

distclean: clean cleanpch
	rm -f main.d src/*.d

include $(DEPOBJ)

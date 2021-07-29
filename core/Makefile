PRODUCT = libcore.a

CXXSRC = $(wildcard src/*.cpp)
CXXOBJ = $(CXXSRC:.cpp=.o)
DEPOBJ = $(CXXSRC:.cpp=.d)

PCHSRC = include/pch.hpp
PCHOBJ = $(PCHSRC).pch

COMMON_CPPFLAGS := $(CPPFLAGS) -Iinclude
CPPPCH_FLAGS := $(COMMON_CPPFLAGS)
CPPFLAGS := $(COMMON_CPPFLAGS) -include $(PCHSRC)

CXXFLAGS := $(CXXFLAGS) -Wno-switch

BUILD_XED = $(shell sh xed-is-missing.sh)
THIRD_PARTY_LIB = 

ifeq ($(BUILD_XED),1)
	THIRD_PARTY_LIB += third-party/lib/libxed.a
endif

.PHONY: clean mrproper

$(PRODUCT): $(CXXOBJ)
	$(AR) -rcv $@ $^

$(CXXOBJ): %.o: %.d

$(THIRD_PARTY_LIB):
	$(MAKE) -C third-party

$(PCHOBJ): $(PCHSRC) $(THIRD_PARTY_LIB)
	$(CXX) $(CPPPCH_FLAGS) $(CXXFLAGS) -x c++-header $(PCHSRC) -o $@

%.d: %.cpp $(PCHOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEPFLAGS) $@ $<

clean:
	rm -f $(CXXOBJ)

mrproper:
	rm -f $(PRODUCT)

include $(DEPOBJ)
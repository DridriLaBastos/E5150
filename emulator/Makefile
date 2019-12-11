CXXSRC = $(wildcard src/*.cpp)
CXXOBJ = $(CXXSRC:.cpp=.o)
CPUSRC = $(wildcard src/cpu/*.cpp)
PCHSRC = pch.hpp
PCHOBJ = $(PCHSRC).pch
CPPPCH_FLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)/emulator
CPPFLAGS += -I$(PROJECT_DIR)/emulator -include $(PCHSRC) -DBUILD -DSTOP_AT_END -DSEE_CURRENT_INST -DSEE_REGS -DSEE_FLAGS
CFLAGS = -Wno-switch
CXXFLAGS = $(CFLAGS)

PRODUCT = libibmpc.dylib

.PHONY: clean mrproper cleanpch

$(PRODUCT): $(PCHOBJ) $(CXXOBJ)
	$(LD) $(LDFLAGS) -dylib -install_name @rpath/$(PRODUCT) -current_version 1.0.0 -lxed -lc++ -lSystem $(CXXOBJ) -o $@

$(PCHOBJ): $(PCHSRC)
	$(CXX) $(CPPPCH_FLAGS) $(CXXFLAGS) -x c++-header $^ -o $@

src/cpu.o: src/cpu.cpp $(CPUSRC)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(CXXOBJ)

mrproper:
	rm $(PRODUCT)

cleanpch:
	rm $(PCHOBJ)
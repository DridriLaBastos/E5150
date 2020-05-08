CXXSRC = main.cpp
ASSRC  = $(wildcard *.s)
ASOBJ  = $(ASSRC:.s=.bin)
PRODUCT = test.out
CPPFLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)/emulator
LDFLAGS  := $(LDFLAGS) -L../emulator
DYLD_LIBRARY_PATH := $(DYLD_LIBRARY_PATH):../emulator

.PHONY: mrproper clean run

$(PRODUCT): $(CXXSRC) $(ASOBJ)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -libmpc $(CXXSRC) -o $@

%.bin: %.s
	nasm -f bin $^ -o $@

run:
	@./$(PRODUCT)

clean:
	rm $(ASOBJ) $(CXXOBJ)

mrproper:
	rm $(PRODUCT)
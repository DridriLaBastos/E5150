CXXSRC = main.cpp
ASSRC  = $(wildcard *.s)
ASOBJ  = $(ASSRC:.s=.bin)
PRODUCT = test.out
CPPFLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)/emulator

all: asm $(PRODUCT)

$(PRODUCT): $(CXXSRC)
	$(CXX) $(CPPFLAGS) $(LDFLAGS) -libmpc $^ -o $@

asm: $(ASOBJ)

%.bin: %.s
	nasm -f bin $^ -o $@

.PHONY: mrproper clean

clean:
	rm $(ASOBJ) $(CXXOBJ)

mrproper:
	rm $(PRODUCT)
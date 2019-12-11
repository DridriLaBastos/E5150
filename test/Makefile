CXXSRC = main.cpp
ASSRC  = $(wildcard *.s)
ASOBJ  = $(ASSRC:.s=.bin)
PRODUCT = test.out

all: asm $(PRODUCT)

$(PRODUCT): $(CXXSRC)
	$(CXX) $(CPPFLAGS) -I$(PROJECT_DIR)/emulator -L$(PROJECT_DIR)/emulator -rpath $(PROJECT_DIR)/emulator -libmpc $^ -o $@

asm: $(ASOBJ)

%.bin: %.s
	nasm -f bin $^ -o $@

.PHONY: mrproper clean

clean:
	rm $(ASOBJ) $(CXXOBJ)

mrproper:
	rm $(PRODUCT)
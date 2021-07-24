PRODUCT = epc.out

ASSRC = $(wildcard test/*.s)
ASOBJ = $(ASSRC:.s=.bin)

CXXOBJ = main.o

CPPFLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)
LDFLAGS := $(LDFLAGS) -L$(PROJECT_DIR)/core

.PHONY: clean mrproper run

$(PRODUCT): $(CXXOBJ) $(ASOBJ) $(PROJECT_DIR)/core/libcore.a
	$(CXX) $(LDFLAGS) -lxed -lsfml-system -lcore -o $@ $(CXXOBJ)

%.bin: %.s test/util.inc
	nasm -f bin $< -o $@

run:
	./$(PRODUCT)

clean:
	rm -f *.o

mrproper:
	rm -f $(PRODUCT)
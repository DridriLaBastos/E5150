PRODUCT = epc.out

ASSRC = $(wildcard test/*.s)
ASOBJ = $(ASSRC:.s=.bin)

CXXOBJ = main.o $(CORE_OBJ)

CPPFLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)
LDFLAGS := $(LDFLAGS) -L$(PROJECT_DIR)/core

.PHONY: clean mrproper run

$(PRODUCT): $(CXXOBJ) $(ASOBJ)
	$(CXX) $(LDFLAGS) -lxed -lsfml-system -o $@ $(CXXOBJ)

%.bin: %.s test/util.inc
	nasm -f bin $< -o $@

run:
	./$(PRODUCT)

clean:
	rm -f *.o

mrproper:
	rm -f $(PRODUCT)
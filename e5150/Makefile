PRODUCT = epc.out

ASSRC = $(wildcard test/*.s)
ASOBJ = $(ASSRC:.s=.bin)

CXXOBJ = main.o

CPPFLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)
LDFLAGS := $(LDFLAGS) -L$(PROJECT_DIR)/core -L$(PROJECT_DIR)/core/third-party/lib

.PHONY: clean mrproper run

all: $(PRODUCT) $(ASOBJ)

$(PRODUCT): $(CXXOBJ) $(PROJECT_DIR)/core/libcore.a
	$(CXX) $(LDFLAGS) -Xlinker -rpath -Xlinker $(DEV)/lib -o $@ $(CXXOBJ) -lcore -lsfml-system -lxed 

%.bin: %.s test/util.inc
	nasm -f bin $< -o $@

run:
	./$(PRODUCT)

clean:
	rm -f *.o

mrproper:
	rm -f $(PRODUCT)
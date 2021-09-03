PRODUCT = epc.out

ASSRC = $(wildcard test/*.s)
ASOBJ = $(ASSRC:.s=.bin) test/ibm_bios.bin

CXXOBJ = main.o

CPPFLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)
LDFLAGS := $(LDFLAGS) -L$(PROJECT_DIR)/core -L$(PROJECT_DIR)/core/third-party/lib
PLATFORM = $(shell uname -s)
DOWNLOAD_TOOL = curl

ifeq ($(PLATFORM),Linux)
	DOWNLOAD_TOOL = wget
endif

.PHONY: clean mrproper run

all: $(PRODUCT) $(ASOBJ)

$(PRODUCT): $(CXXOBJ) $(PROJECT_DIR)/core/libcore.a
	$(CXX) $(LDFLAGS) -Xlinker -rpath -Xlinker $(DEV)/lib -o $@ $(CXXOBJ) -lcore -lsfml-system -lxed 

%.bin: %.s test/util.inc
	nasm -f bin $< -o $@

test/ibm_bios.bin:
	$(DOWNLOAD_TOOL) https://ibm.retropc.se/bios/BIOS_5150_24APR81_U33.BIN -o $@

run:
	./$(PRODUCT)

clean:
	rm -f *.o

mrproper:
	rm -f $(PRODUCT)
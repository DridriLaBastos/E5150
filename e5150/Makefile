PRODUCT = epc.out

ASSRC = $(wildcard test/*.s)
ASOBJ = $(ASSRC:.s=.bin) test/ibm_bios.bin

CXXSRC = main.cpp

CPPFLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)
LDFLAGS := $(LDFLAGS) -L$(PROJECT_DIR)/core -L$(PROJECT_DIR)/core/third-party/lib
PLATFORM = $(shell uname -s)
DOWNLOAD_TOOL = curl

ifeq ($(PLATFORM),Linux)
	DOWNLOAD_TOOL = wget
endif

.PHONY: clean mrproper run

$(PRODUCT): $(CXXSRC) $(PROJECT_DIR)/core/libcore.a | $(ASOBJ)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $(CXXSRC) -lcore

%.bin: %.s test/util.inc
	nasm -f bin $< -o $@

test/ibm_bios.bin:
	$(DOWNLOAD_TOOL) https://ibm.retropc.se/bios/BIOS_5150_24APR81_U33.BIN -o $@

run:
	./$(PRODUCT)

clean:
	rm -f $(CXXOBJ) $(ASOBJ)

mrproper:
	rm -f $(PRODUCT)
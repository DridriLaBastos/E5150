export MACOSX_VERSION_MIN = 10.10
export PROJECT_DIR = $(PWD)
export CXX = clang++ -O3 -std=c++11 -mmacosx-version-min=$(MACOSX_VERSION_MIN)
export LD  = ld -macosx_version_min $(MACOSX_VERSION_MIN)

.PHONY: clean cleantest mrproper test emulator run

emulator:
	$(MAKE) -C $@

test:
	$(MAKE) -C $@

run:
	@cd test; ./test.out

clean:
	$(MAKE) -C emulator $@

cleantest:
	$(MAKE) -C test clean
	$(MAKE) -C test mrproper

mrproper:
	$(MAKE) -C emulator $@
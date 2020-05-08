export MACOSX_VERSION_MIN = 10.10
export PROJECT_DIR = $(PWD)
export CXX = clang++ -std=c++11 -mmacosx-version-min=$(MACOSX_VERSION_MIN)
export LD  = ld -macosx_version_min $(MACOSX_VERSION_MIN)
export DEBUG = 0

ifeq ($(DEBUG), 1)
	CPPFLAGS += -DDEBUG
endif

.PHONY: clean cleantest mrproper test emulator run

emulator:
	$(MAKE) -C $@

test:
	$(MAKE) -C $@

run:
	$(MAKE) -C test $@

clean:
	$(MAKE) -C emulator $@

cleantest:
	$(MAKE) -C test clean
	$(MAKE) -C test mrproper

mrproper:
	$(MAKE) -C emulator $@
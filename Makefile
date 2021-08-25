export PROJECT_DIR = $(PWD)

SPDLOG_INCLUDE	= $(PROJECT_DIR)/third-party/spdlog/include
CORE_THIRDPARTY_INCLUDE = $(PROJECT_DIR)/core/third-party/include

THIRD_PARTY_INCLUDE_FLAGS = -I$(SPDLOG_INCLUDE) -I$(CORE_THIRDPARTY_INCLUDE)
DEBUG = 1
RELEASE = 0

ifneq ($(RELEASE),0)
	CXX := $(CXX) --std=c++17 -O3 -flto
	DEBUG = 0
else
	CXX := $(CXX) --std=c++17 -O0 -g
endif

ifneq ($(DEBUG),0)
	CPPFLAGS := $(CPPFLAGS) -DDEBUG_BUILD
endif

export CXX
export DEPFLAGS = -MM -MF
export CPPFLAGS := $(CPPFLAGS) $(THIRD_PARTY_INCLUDE_FLAGS)
export CXXFLAGS := $(CXXFLAGS) -Wall -Wextra
export CORE_OBJ = $(wildcard $(PROJECT_DIR)/core/src/*.o)

SRC_DIR = core e5150

.PHONY: all $(SRC_DIR) clean mrproper

all: e5150

e5150 tests: core

e5150 tests core:
	$(MAKE) -C $@

run:
	$(MAKE) -C e5150 $@

clean mrproper:
	for d in $(SRC_DIR); do $(MAKE) -C $$d $@; done;

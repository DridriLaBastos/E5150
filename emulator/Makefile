CXXSRC = $(wildcard src/*.cpp)
CXXOBJ = $(CXXSRC:.cpp=.o)
CPUSRC = $(wildcard src/cpu/*.cpp)
PCHSRC = pch.hpp
PCHOBJ = $(PCHSRC).pch

SPDLOG_INCLUDE = third-party/spdlog/include

CPPPCH_FLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)/emulator -I$(SPDLOG_INCLUDE)
CPPFLAGS := $(CPPFLAGS) -I. -I$(SPDLOG_INCLUDE) -include $(PCHSRC)
CFLAGS = -Wno-switch
CXXFLAGS = $(CFLAGS)

ifneq ($(DEBUG),1)
	CFLAGS += -Ofast
endif

PRODUCT = libibmpc.dylib

.PHONY: clean mrproper cleanpch

$(PRODUCT): $(PCHOBJ) $(CXXOBJ)
	$(CXX) $(LDFLAGS) -dynamiclib -install_name $(PRODUCT) -current_version 1.0.0 -lxed -lsfml-system $(CXXOBJ) -o $@

$(PCHOBJ): $(PCHSRC) component.hpp
	$(CXX) $(CPPPCH_FLAGS) $(CXXFLAGS) -x c++-header $(PCHSRC) -arch x86_64 -o $@

%.o:%.cpp util.hpp config.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

src/cpu.o: src/cpu.cpp $(CPUSRC) util.hpp config.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(CXXOBJ)

mrproper:
	rm $(PRODUCT)

cleanpch:
	rm $(PCHOBJ)
CXXSRC = $(wildcard src/*.cpp)
CXXOBJ = $(CXXSRC:.cpp=.o)
CPUSRC = $(wildcard src/cpu/*.cpp)
PCHSRC = pch.hpp
PCHOBJ = $(PCHSRC).pch
CPPFLAGS += -DBUILD
CPPPCH_FLAGS := $(CPPFLAGS) -I$(PROJECT_DIR)/emulator
CPPFLAGS += -I$(PROJECT_DIR)/emulator -include $(PCHSRC)
CFLAGS = -Wno-switch
CXXFLAGS = $(CFLAGS)

ifneq ($(DEBUG),1)
	CFLAGS += -Ofast
endif

PRODUCT = libibmpc.dylib

.PHONY: clean mrproper cleanpch

$(PRODUCT): $(PCHOBJ) $(CXXOBJ)
	$(CXX) $(LDFLAGS) -dynamiclib -install_name @rpath/$(PRODUCT) -rpath $(DEV)/lib -current_version 1.0.0 -lxed -lc++ -lSystem -lsfml-system $(CXXOBJ) -o $@

$(PCHOBJ): $(PCHSRC)
	$(CXX) $(CPPPCH_FLAGS) $(CXXFLAGS) -x c++-header $^ -o $@

%.o:%.cpp config.hpp util.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

src/cpu.o: src/cpu.cpp $(CPUSRC) config.hpp util.hpp
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(CXXOBJ)

mrproper:
	rm $(PRODUCT)

cleanpch:
	rm $(PCHOBJ)
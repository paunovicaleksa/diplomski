# (c) 2010 The Board of Trustees of the University of Illinois.

# set this to directory where kokkos was cloned 
KOKKOS_PATH = ${HOME}/mups/kokkos-4.4.00
KOKKOS_DEVICES = "OpenMP"

SRCS=arithmetic.cpp
BIN=arithmetic
APP_LDFLAGS=-lm -lstdc++
BUILDDIR=build
OBJS=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(SRCS))
OBJS+=$(BUILDDIR)/parboil.o

ifneq (,$(findstring Cuda,$(KOKKOS_DEVICES)))
CXX = ${KOKKOS_PATH}/bin/nvcc_wrapper
BIN = ${BIN}.cuda
KOKKOS_CUDA_OPTIONS = "enable_lambda"
else
CXX = g++
KOKKOS_ARCH = "BDW"
endif
  
PLATFORM_CXXFLAGS = -I../../../include
  
LINKER = g++
PLATFORM_LDFLAGS = -lm -lpthread

# (c) 2007 The Board of Trustees of the University of Illinois.

# Default language wide options

LANG_CFLAGS=
LANG_CXXFLAGS=$(LANG_CFLAGS)
LANG_LDFLAGS=

CFLAGS=$(LANG_CFLAGS) $(PLATFORM_CFLAGS) $(APP_CFLAGS)
CXXFLAGS=$(LANG_CXXFLAGS) $(PLATFORM_CXXFLAGS) $(APP_CXXFLAGS)
LDFLAGS=$(LANG_LDFLAGS) $(PLATFORM_LDFLAGS) $(APP_LDFLAGS)

.PHONY: run

default: $(BUILDDIR) $(BIN)

include $(KOKKOS_PATH)/Makefile.kokkos
########################################
# Derived variables
########################################


########################################
# Rules
########################################


clean : kokkos-clean
	rm -f $(BUILDDIR)/*
	if [ -f $(BIN) ]; then rm $(BIN); fi
	if [ -d $(BUILDDIR) ]; then rmdir $(BUILDDIR); fi

$(BIN) : $(OBJS) $(KOKKOS_LINK_DEPENDS) $(KOKKOS_LIBS)
	$(LINKER) $^ -o $@ $(LDFLAGS) $(KOKKOS_LDFLAGS) $(EXTRA_PATH)

$(BUILDDIR) :
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/parboil.o : ../../../src/parboil.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o : %.cpp $(KOKKOS_CPP_DEPENDS)
	$(CXX) $(KOKKOS_CPPFLAGS) $(KOKKOS_CXXFLAGS) $(CXXFLAGS) $(EXTRA_INC) -c $< -o $@


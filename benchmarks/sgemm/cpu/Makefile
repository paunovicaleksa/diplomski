# (c) 2010 The Board of Trustees of the University of Illinois.

SRCS=main.cpp io.cpp sgemm_kernel.cpp
BIN=sgemm
APP_LDFLAGS=-lm -lstdc++
BUILDDIR=build
OBJS=$(patsubst %.cpp, $(BUILDDIR)/%.o, $(SRCS))
OBJS+=$(BUILDDIR)/parboil.o

  
CXX = g++
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

########################################
# Derived variables
########################################


########################################
# Rules
########################################

default: $(BUILDDIR) $(BIN)

clean :
	rm -f $(BUILDDIR)/*
	if [ -f $(BIN) ]; then rm $(BIN); fi
	if [ -d $(BUILDDIR) ]; then rmdir $(BUILDDIR); fi

$(BIN) : $(OBJS) 
	$(CXX) $^ -o $@ $(LDFLAGS)

$(BUILDDIR) :
	mkdir -p $(BUILDDIR)

$(BUILDDIR)/parboil.o : ../../../src/parboil.c
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILDDIR)/%.o : %.cpp 
	$(CXX) $(CXXFLAGS) -c $< -o $@


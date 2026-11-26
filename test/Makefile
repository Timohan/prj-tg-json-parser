#/*!
#* \file Makefile
#* \brief Makefile for compiling and running the tests
#*
#* Builds against the library in ../lib (not the installed one).
#*
#* Copyright of Timo hannukkala, Inc. All rights reserved.
#*
#* \author Timo Hannukkala <timohannukkala@hotmail.com>
#*/
TARGET:=prj-tg-json-parser-test

# set current make dir
CURRENT_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))
LIB_DIR:=$(abspath $(CURRENT_DIR)../lib)
LIB_TARGET:=$(LIB_DIR)/libprj-tg-json-parser-lib.so
# set TG_GLOBAL_LOG_LEVEL=1 to see debug logs
TG_GLOBAL_LOG_LEVEL?=2

CXXFLAGS+=-g -Wall -pedantic -pipe -std=gnu++17 -W -D_REENTRANT -fPIC
CXXFLAGS+=-I$(CURRENT_DIR)src -I$(LIB_DIR)/src
CXXFLAGS+=-Wno-unused-parameter -Wuninitialized -Wconversion -Wshadow -Wpointer-arith \
	 -Wswitch-default -Wswitch-enum -Wcast-align \
	 -Winline -Wundef -Wcast-qual -Wunreachable-code -Wlogical-op -Wfloat-equal \
	 -Wredundant-decls -Werror \
	 -Wno-unused-const-variable
CXXFLAGS+=-MMD -MP
CXXFLAGS+=-DTG_GLOBAL_LOG_LEVEL=$(TG_GLOBAL_LOG_LEVEL)
LDFLAGS+=-L$(LIB_DIR) -lprj-tg-json-parser-lib -Wl,--rpath=$(LIB_DIR)

src_SRCDIR:=$(CURRENT_DIR)src
src_SRCS:=$(wildcard $(src_SRCDIR)/*.cpp)
src_OBJS:=$(src_SRCS:.cpp=.o)
DEPS:=$(src_OBJS:.o=.d)

ORDERS_FOLDER=$(CURRENT_DIR)orders
CXXFLAGS+=-DORDERS_FOLDER=\"$(ORDERS_FOLDER)\"

.PHONY: all default lib run clean

all: default

default: $(CURRENT_DIR)$(TARGET)

lib:
	$(MAKE) -C $(LIB_DIR)

$(LIB_TARGET): lib

$(CURRENT_DIR)$(TARGET): $(src_OBJS) $(LIB_TARGET)
	$(CXX) $(src_OBJS) $(LDFLAGS) -o $@

$(src_OBJS):%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

run: default
	$(CURRENT_DIR)$(TARGET)

clean:
	rm -f $(CURRENT_DIR)$(TARGET)
	rm -f $(src_OBJS) $(DEPS)

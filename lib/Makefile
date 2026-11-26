#/*!
#* \file Makefile
#* \brief Makefile for compiling prj-tg-json-parser-lib library
#* and generating pkgconfig file
#* and making install & uninstall
#*
#* Build as normal user first, then install:
#*   make && sudo make install
#*
#* Copyright of Timo Hannukkala. All rights reserved.
#*
#* \author Timo Hannukkala <timohannukkala@hotmail.com>
#*/
# VERSION_MAJOR must be increased on every ABI incompatible change
VERSION_MAJOR:=1
VERSION_MINOR:=0
VERSION_PATCH:=0
VERSION:=$(VERSION_MAJOR).$(VERSION_MINOR).$(VERSION_PATCH)

LIBNAME:=libprj-tg-json-parser-lib.so
SONAME:=$(LIBNAME).$(VERSION_MAJOR)
REALNAME:=$(LIBNAME).$(VERSION)

PREFIX:=/usr
LIBDIR:= $(PREFIX)/lib
INSTALL_LIBDIR:=$(LIBDIR)/prj-tg-json-parser-lib
INSTALL_INCLUDEDIR:=$(PREFIX)/include/prj-tg-json-parser-lib
PKGCONFIGDIR:=$(LIBDIR)/pkgconfig
PKGCONFIGFILE:=prj-tg-json-parser-lib
CXXFLAGS+=-Wuninitialized -Wconversion -Wshadow -Wpointer-arith \
	-Wswitch-default -Wswitch-enum -Wcast-align \
	-Wundef -Wcast-qual -Wunreachable-code -Wfloat-equal \
	-Wredundant-decls -Wextra -Wall
CXXFLAGS+=-Werror
CXXFLAGS+=-O2
CXXFLAGS+=-g -fPIC -std=c++17
# only classes marked TG_JSON_PARSER_LIBRARY_EXPORT are exported
CXXFLAGS+=-fvisibility=hidden -fvisibility-inlines-hidden
CXXFLAGS+=-MMD -MP
LDFLAGS+=-shared -Wl,-soname,$(SONAME)

CURRENT_DIR:=$(dir $(abspath $(lastword $(MAKEFILE_LIST))))

src_SRCDIR:=$(CURRENT_DIR)src
src_SRCS:=$(wildcard $(src_SRCDIR)/*.cpp)
src_OBJS:=$(src_SRCS:.cpp=.o)

src_private_SRCDIR:=$(CURRENT_DIR)src/private
src_private_SRCS:=$(wildcard $(src_private_SRCDIR)/*.cpp)
src_private_OBJS:=$(src_private_SRCS:.cpp=.o)

OBJS:=$(src_OBJS) $(src_private_OBJS)
DEPS:=$(OBJS:.o=.d)

.PHONY: all default install uninstall clean pkgconfig check-prefix check-built

all: default

# real file + soname and development symlinks
default: $(CURRENT_DIR)$(REALNAME)
	ln -sf $(REALNAME) $(CURRENT_DIR)$(SONAME)
	ln -sf $(SONAME) $(CURRENT_DIR)$(LIBNAME)

$(CURRENT_DIR)$(REALNAME): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $@

$(OBJS):%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

-include $(DEPS)

check-prefix:
	@if [ -z "$(strip $(PREFIX))" ] || [ "$(strip $(PREFIX))" = "/" ]; then \
		echo "PREFIX must not be empty or /"; exit 1; fi

# install doesn't build, so "sudo make install" never creates root owned build files
check-built:
	@if [ ! -f "$(CURRENT_DIR)$(REALNAME)" ]; then \
		echo "$(REALNAME) is not built, run make first (as normal user)"; exit 1; fi

# "install" replaces the file instead of rewriting it in place (like cp does),
# so programs that have the old library loaded keep running during an upgrade
install: check-prefix check-built pkgconfig
	install -d $(INSTALL_LIBDIR)
	install -m 0755 $(CURRENT_DIR)$(REALNAME) $(INSTALL_LIBDIR)/$(REALNAME)
	ln -sf $(REALNAME) $(INSTALL_LIBDIR)/$(SONAME)
	ln -sf $(SONAME) $(INSTALL_LIBDIR)/$(LIBNAME)
	install -d $(INSTALL_INCLUDEDIR)
	install -m 0644 $(src_SRCDIR)/prj_tg_json_parser_lib.h $(INSTALL_INCLUDEDIR)
	install -m 0644 $(src_SRCDIR)/prj_tg_json_parser_object.h $(INSTALL_INCLUDEDIR)

clean:
	rm -f $(OBJS) $(DEPS)
	rm -f $(CURRENT_DIR)$(LIBNAME) $(CURRENT_DIR)$(LIBNAME).*

# regenerated on every install so VERSION and PREFIX are always current
pkgconfig: check-prefix
	@echo "installing $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc"
	@install -d $(PKGCONFIGDIR)
	@rm -f $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "prefix=$(PREFIX)" > $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "exec_prefix=\$${prefix}" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "libdir=\$${prefix}/lib/prj-tg-json-parser-lib" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "includedir=\$${prefix}/include/prj-tg-json-parser-lib" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "Name: prj-tg-json-parser-lib" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "Description: prj-tg-json-parser-lib lib is a strict json parser" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "Version: $(VERSION)" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "Cflags: -I\$${includedir}" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc
	@echo "Libs: -L\$${libdir} -lprj-tg-json-parser-lib -Wl,--rpath=\$${libdir}" >> $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc

uninstall: check-prefix
	rm -rf $(INSTALL_LIBDIR)
	rm -rf $(INSTALL_INCLUDEDIR)
	rm -f $(PKGCONFIGDIR)/$(PKGCONFIGFILE).pc

# Copyright (C) 2002 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

AUTOMAKE_OPTIONS = foreign

lib_LTLIBRARIES = libguile-gtk-foo.la

INCLUDES = $(GUILEGTK_CFLAGS)

libguile_gtk_foo_la_SOURCES = foo.c foo.h foo-glue.c
libguile_gtk_foo_la_LDFLAGS = -version-info 0:0:0 -export-dynamic
libguile_gtk_foo_la_LIBADD  = $(GUILEGTK_LIBLIBS)

BUILT_SOURCES = foo-glue.c

foo-glue.c: foo.defs
	build-guile-gtk-2.0 glue $< >tmp && mv tmp $@

scmgtkdir=$(datadir)/guile/gtk
scmgtk_DATA = foo.scm

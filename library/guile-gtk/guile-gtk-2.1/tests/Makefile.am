# -*- Makefile -*-
#
# Copyright (C) 2006 Free Software Foundation, Inc.
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

TESTS_ENVIRONMENT = $(GUILE) -s
TESTS = gdk.scm gtk.scm

if BUILD_GDK_PIXBUF
TESTS += gdk-pixbuf.scm
endif

if BUILD_GLADE
TESTS += glade.scm
endif

if BUILD_GTK_GL
TESTS += gdk-gl.scm gtk-gl-area.scm
endif

EXTRA_DIST = $(TESTS) tests.scm

INCLUDES = -I $(top_srcdir) -I $(top_builddir) $(GTK_CFLAGS) $(GUILE_CFLAGS)

# Need -rpath in LDFLAGS here or libtool only makes a convenience library,
# which cannot be dlopened by tests.scm.
#
noinst_LTLIBRARIES = tests.la
tests_la_SOURCES = tests.c
tests_la_LDFLAGS = -module -rpath `pwd`
nodist_tests_la_SOURCES = tests.x
BUILT_SOURCES = tests.x
CLEANFILES = tests.x

SUFFIXES = .x
.c.x:
	guile-snarf -o $@ $< $(DEFS) $(INCLUDES) $(CPPFLAGS) $(CFLAGS)

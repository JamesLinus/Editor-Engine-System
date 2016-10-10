/* Guile Gtk C code for tests.

   Copyright (C) 2006, 2007 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 3 of the License, or (at your
   option) any later version.

   This program is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
   Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include "config.h"

#if HAVE_MALLOC_H
#include <malloc.h>
#endif

#include <libguile.h>

#include "guile-gtk-compat.h"


SCM_DEFINE (tests_mallinfo_uordblks, "mallinfo-uordblks", 0, 0, 0,
            (),
            "")
#define FUNC_NAME s_tests_output_port_width
{
#if HAVE_MALLINFO
  return scm_from_int (mallinfo().uordblks);
#else
  return scm_from_int (0);
#endif
}
#undef FUNC_NAME


void
tests_init (void)
{
#ifndef SCM_MAGIC_SNARFER
#include "tests.x"
#endif
}

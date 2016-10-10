#!/bin/sh
#
# Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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
# along with this program.  If not, see http://www.gnu.org/licenses/.

if test -z "$ACLOCAL" ; then
 for each in aclocal aclocal-1.7 aclocal-1.6 aclocal-1.5 ; do
   ACLOCAL=$each
   if test -n "`which $each`" ; then break ; fi
 done
fi

ACDIR=`which $ACLOCAL`
ACDIR=`dirname $ACDIR`
ACDIR=`dirname $ACDIR`/share/aclocal
GUDIR=`guile-config info prefix`/share/aclocal

# We used to look in wherever gtk-config (the gtk 1.2 program) said Gtk is
# installed, but no longer since Gtk 2 uses pkg-config.
#
# GTDIR=`gtk-config --prefix`/share/aclocal

# Back in Gtk 1.2 we had copies of some .m4 macro files in a macros
# directory in the source tree, but no longer.  (aclocal is upset if you
# give it a non-existant directory in a -I, so this has to be disabled.)
#
# AFLAGS="-I macros "

AFLAGS=""
for each in $GUDIR $G2DIR $G1DIR $ACDIR ; do
    if test -d "$each"  ; then 
	AFLAGS="-I $each $AFLAGS"
	break
    fi
done

echo $ACLOCAL $AFLAGS $@
$ACLOCAL $AFLAGS $@

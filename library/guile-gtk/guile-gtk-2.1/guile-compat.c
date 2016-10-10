/*
 * Copyright (C) 2006, 2007 Free Software Foundation, Inc.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"

#include <string.h>
#include <libguile.h>

#include "guile-gtk-compat.h"
#include "guile-gtk.h"


#if SCM_MAJOR_VERSION == 1 && SCM_MINOR_VERSION == 6

/* Guile 1.6 implementation of current funcs.

   The test here is on the version number 1.6 explicitly, to make it clear
   what's being targeted.  */


/* same as in Guile 1.8 */
SCM
sgtk_cell (scm_t_bits word0, scm_t_bits word1)
{
  SCM z;
  SCM_NEWCELL (z);
  SCM_SET_CELL_WORD_0 (z, word0);
  SCM_SET_CELL_WORD_1 (z, word1);
  return z;
}

/* Same as Guile 1.8 scm_to_int8, but return type is "char" so we don't have
   to bother figuring how to get int8_t from the compiler. */
char
sgtk_to_int8 (SCM obj)
{
  short n = scm_num2short (obj, SCM_ARG1, "sgtk_to_int8");
  if (n < -0x80 || n > 0x7F)
    scm_out_of_range ("sgtk_to_int8", obj);
  return n;
}

/* Same as Guile 1.8, but return type is "short" so we don't have to bother
   figuring how to get int16_t from the compiler. */
short
sgtk_to_int16 (SCM obj)
{
  short n = scm_num2short (obj, SCM_ARG1, "sgtk_to_int16");
  if (n < -32768 || n > 32767)
    scm_out_of_range ("sgtk_to_int16", obj);
  return n;
}

/* Same as Guile 1.8, but return type is unsigned char so we don't have to
   bother figuring how to get uint8_t from the compiler. */
unsigned char
sgtk_to_uint8 (SCM obj)
{
  unsigned short n = scm_num2ushort (obj, SCM_ARG1, "sgtk_to_uint8");
  if (n > 255)
    scm_out_of_range ("sgtk_to_uint8", obj);
  return n;
}

/* Same as Guile 1.8, but return type is unsigned short so we don't have to
   bother figuring how to get uint16_t from the compiler. */
unsigned short
sgtk_to_uint16 (SCM obj)
{
  unsigned short n = scm_num2ushort (obj, SCM_ARG1, "sgtk_to_uint16");
  if (n > 65535)
    scm_out_of_range ("sgtk_to_uint16", obj);
  return n;
}

/* Same as Guile 1.8, but return type is unsigned short so we don't have to
   bother figuring how to get uint32_t from the compiler. */
unsigned long
sgtk_to_uint32 (SCM obj)
{
  unsigned long n = scm_num2ulong (obj, SCM_ARG1, "sgtk_to_uint32");
  if (n > 4294967295UL)
    scm_out_of_range ("sgtk_to_uint32", obj);
  return n;
}

/* Same as Guile 1.8, but range args are only "long"s so we don't have to
   bother figuring how to get intmax_t from the compiler. */
int
sgtk_is_signed_integer (SCM obj, long min, long max)
{
  if (SCM_INUMP (obj))
    {
      long n = SCM_INUM (obj);
      return (n >= min && n <= max);
    }
  if (SCM_BIGP (obj))
    {
      if (scm_is_true (scm_less_p (obj, scm_long2num (min))))
        return 0;  /* smaller than demanded */
      if (scm_is_true (scm_less_p (scm_long2num (max), obj)))
        return 0;  /* bigger than demanded */
      return 1;
    }
  return 0;
}

/* Same as Guile 1.8, but range args are only "unsigned long"s so we don't
   have to bother figuring how to get uintmax_t from the compiler. */
int
sgtk_is_unsigned_integer (SCM obj, unsigned long min, unsigned long max)
{
  if (SCM_INUMP (obj))
    {
      long n = SCM_INUM (obj);
      unsigned long u;
      if (n < 0)
        return 0; /* negative */
      u = (unsigned long) n;
      return (u >= min && u <= max);
    }
  if (SCM_BIGP (obj))
    {
      if (scm_is_true (scm_negative_p (obj)))
        return 0;
      if (scm_is_true (scm_less_p (obj, scm_ulong2num (min))))
        return 0;  /* smaller than demanded */
      if (scm_is_true (scm_less_p (scm_ulong2num (max), obj)))
        return 0;  /* bigger than demanded */
      return 1;
    }
  return 0;
}

/* same as in Guile 1.8 */
void *
sgtk_malloc (size_t size)
{
  char *p = scm_must_malloc (size, "guile-gtk");
  scm_done_free (size);
  return p;
}

/* same as in Guile 1.8 */
char *
sgtk_to_locale_stringn (SCM str, size_t *lenp)
#define FUNC_NAME "sgtk_to_locale_stringn"
{
  const char *c_str;
  size_t c_len;
  char *ret;

  /* we can cope with both ordinary strings and with shared substrings */
  SCM_ASSERT_TYPE (SCM_STRINGP (str) || SCM_SUBSTRP (str),
                   str, SCM_ARG1, "sgtk_to_locale_stringn", "string");
  c_str = SCM_ROCHARS (str);
  c_len = SCM_STRING_LENGTH (str);

  if (lenp == NULL)
    {
      if (memchr (c_str, '\0', c_len))
        scm_misc_error (FUNC_NAME, "string contains #\\nul character: ~S",
                        scm_list_1 (str));
    }
  else
    *lenp = c_len;

  ret = scm_malloc (c_len + 1);
  memcpy (ret, c_str, c_len);
  scm_remember_upto_here_1 (str);
  ret[c_len] = '\0';
  return ret;
}
#undef FUNC_NAME

/* same as in Guile 1.8 */
char *
sgtk_to_locale_string (SCM obj)
{
  return sgtk_to_locale_stringn (obj, NULL);
}

/* Same as in Guile 1.8.
   Note that the Guile 1.6 scm_take_str() is not equivalent to the new
   scm_take_locale_stringn.  scm_take_str assumes there's already a
   terminating '\0', but scm_take_locale_stringn doesn't need that and will
   realloc to put one.  (We use that feature in
   gdk_property_get_interp().)  */
SCM
sgtk_take_locale_stringn (char *str, size_t len)
{
  str = realloc (str, len + 1);
  if (str == NULL)
    scm_memory_error ("sgtk_take_locale_stringn");
  str[len] = '\0';
  return scm_take_str (str, len);
}

/* Same as Guile 1.8 scm_c_vector_length. */
size_t
sgtk_c_vector_length (SCM vec)
#define FUNC_NAME "sgtk_c_vector_length"
{
  SCM_VALIDATE_VECTOR (SCM_ARG1, vec);
  return SCM_VECTOR_LENGTH (vec);
}
#undef FUNC_NAME

/* Same as Guile 1.8 scm_c_vector_ref. */
SCM
sgtk_c_vector_ref (SCM vec, size_t i)
#define FUNC_NAME "sgtk_c_vector_ref"
{
  SCM_VALIDATE_VECTOR (SCM_ARG1, vec);
  return SCM_VELTS(vec)[i];
}
#undef FUNC_NAME

/* Same as Guile 1.8 scm_c_vector_set_x. */
void
sgtk_c_vector_set_x (SCM vec, size_t i, SCM val)
#define FUNC_NAME "sgtk_c_vector_set_x"
{
  SCM_VALIDATE_VECTOR (SCM_ARG1, vec);
  SCM_VELTS(vec)[i] = val;
}
#undef FUNC_NAME


/* Same as Guile 1.8 scm_c_generalized_vector_length, except only vectors
   and byte vectors.  */
size_t
sgtk_c_generalized_vector_length (SCM vec)
#define FUNC_NAME "sgtk_c_generalized_vector_length"
{
  if (SCM_VECTORP (vec))
    return SCM_VECTOR_LENGTH (vec);

  if (SCM_NIMP (vec) && SCM_TYP7 (vec) == scm_tc7_byvect)
    return SCM_UVECTOR_LENGTH (vec);

  SCM_WRONG_TYPE_ARG (SCM_ARG1, vec);
}
#undef FUNC_NAME

/* Same as Guile 1.8 scm_c_generalized_vector_ref, except only vectors and
   byte vectors.  */
SCM
sgtk_c_generalized_vector_ref (SCM vec, size_t i)
#define FUNC_NAME "sgtk_c_generalized_vector_ref"
{
  if (SCM_VECTORP (vec))
    return SCM_VELTS(vec)[i];

  if (SCM_NIMP (vec) && SCM_TYP7 (vec) == scm_tc7_byvect)
    {
      char *base = (char *) SCM_UVECTOR_BASE (vec);
      return SCM_MAKINUM (base[i]);
    }

  SCM_WRONG_TYPE_ARG (SCM_ARG1, vec);
}
#undef FUNC_NAME

/* Same as in Guile 1.8 */
SCM
sgtk_car (SCM obj)
#define FUNC_NAME "scm_car"
{
  SCM_VALIDATE_CONS (SCM_ARG1, obj);
  return SCM_CAR (obj);
}
#undef FUNC_NAME

/* Same as in Guile 1.8 */
SCM
sgtk_cdr (SCM obj)
#define FUNC_NAME "scm_cdr"
{
  SCM_VALIDATE_CONS (SCM_ARG1, obj);
  return SCM_CDR (obj);
}
#undef FUNC_NAME

#endif /* Guile 1.6 */

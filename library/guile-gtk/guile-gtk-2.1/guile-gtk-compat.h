/* classes: h_files */

#ifndef COMPATH
#define COMPATH
/*	Copyright (C) 2001, 2002, 2004, 2006 Free Software Foundation, Inc.
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


#include <libguile.h>


/*---------------------------------------------------------------------------*/
/* Guile 1.6 implementation of current funcs.

   The test here is on the version number 1.6 explicitly, to make it clear
   what's being targeted.  */

#if SCM_MAJOR_VERSION == 1 && SCM_MINOR_VERSION == 6

/* not in <libguile.h> in Guile 1.6.7 and earlier, wanted here for
   scm_c_register_extension prototype */
#include <libguile/extensions.h>

#undef scm_cell  /* lose the #define to scm_t_cell typedef */
#define scm_cell(word0, word1)  sgtk_cell(word0, word1)
SCM sgtk_cell (scm_t_bits word0, scm_t_bits word1);

#define scm_from_int(n)      scm_int2num (n)
#define scm_from_int8(n)     scm_int2num (n)
#define scm_from_uint(n)     scm_uint2num (n)
#define scm_from_long(n)     scm_long2num (n)
#define scm_from_ulong(n)    scm_ulong2num (n)
#define scm_from_uint32(n)   scm_ulong2num (n)   /* ulong >= 32 bits */
#define scm_from_double(x)   scm_double2num (x)
#define scm_to_int(x)        scm_num2int (x, SCM_ARGn, "scm_to_int")
#define scm_to_uint(x)       scm_num2uint (x, SCM_ARGn, "scm_to_uint")
#define scm_to_double(x)     scm_num2double (x, SCM_ARGn, "scm_to_double")

/* strictly speaking this depends on whether size_t is int or long, but we
   don't need the full range for present uses */
#define scm_to_size_t(x)     scm_num2uint (x, SCM_ARGn, "scm_to_size_t")

#define scm_to_int8(x)       sgtk_to_int8(x)
#define scm_to_int16(x)      sgtk_to_int16(x)
#define scm_to_uint8(x)      sgtk_to_uint8(x)
#define scm_to_uint16(x)     sgtk_to_uint16(x)
#define scm_to_uint32(x)     sgtk_to_uint32(x)
char           sgtk_to_int8 (SCM obj);
short          sgtk_to_int16 (SCM obj);
unsigned char  sgtk_to_uint8 (SCM obj);
unsigned short sgtk_to_uint16 (SCM obj);
unsigned long  sgtk_to_uint32 (SCM obj);

#define scm_is_signed_integer(obj,lo,hi)   sgtk_is_signed_integer(obj,lo,hi)
int sgtk_is_signed_integer (SCM obj, long min, long max);
#define scm_is_unsigned_integer(obj,lo,hi) sgtk_is_unsigned_integer(obj,lo,hi)
int sgtk_is_unsigned_integer (SCM obj, unsigned long min, unsigned long max);

/* any SCM_COMPLEXP has a non-zero imaginary part */
#define scm_is_real(obj)     (SCM_NUMBERP (obj) && ! SCM_COMPLEXP (obj))

#define scm_is_integer(obj)  scm_is_true (scm_integer_p (obj))

#define scm_is_string(obj)   SCM_ROSTRINGP(obj)
#define scm_from_locale_string(s)       scm_makfrom0str(s)
#define scm_from_locale_stringn(s,len)  scm_mem2string(s,len)
#define scm_take_locale_string(s)       scm_take0str(s)
#define scm_take_locale_stringn(s,len)  sgtk_take_locale_stringn(s,len)
SCM sgtk_take_locale_stringn (char *str, size_t len);
#define scm_to_locale_string(obj)   sgtk_to_locale_string(obj)
char *sgtk_to_locale_string (SCM obj);
#define scm_to_locale_stringn(obj,lenp)   sgtk_to_locale_stringn(obj,lenp)
char *sgtk_to_locale_stringn (SCM obj, size_t *lenp);

#define scm_is_symbol(obj)   SCM_SYMBOLP (obj)
#define scm_from_locale_symbol(s)   scm_str2symbol(s)
#define scm_take_locale_symbol(s)   scm_string_to_symbol (scm_take_locale_string (s))

#define scm_is_false(obj)    SCM_FALSEP (obj)
#define scm_is_true(obj)     SCM_NFALSEP (obj)
#define scm_from_bool(n)     SCM_BOOL (n)

#define scm_is_eq(obj1,obj2) SCM_EQ_P (obj1, obj2)
#define scm_is_pair(obj)     SCM_CONSP (obj)
#define scm_is_null(obj)     SCM_NULLP (obj)
#define scm_is_keyword(obj)  SCM_KEYWORDP (obj)

#define scm_is_vector(obj)   SCM_VECTORP (obj)
#define scm_c_make_vector(size,fill) \
  scm_make_vector (scm_from_uint (size), fill)
#define SCM_SIMPLE_VECTOR_SET(vec,idx,val)  (SCM_VELTS(vec)[idx] = (val))

#define scm_c_vector_length(vec)  sgtk_c_vector_length(vec)
size_t sgtk_c_vector_length (SCM vec);

#define scm_c_vector_ref(vec,i)   sgtk_c_vector_ref(vec,i)
SCM sgtk_c_vector_ref (SCM vec, size_t i);

#define scm_c_vector_set_x(vec,i,val)   sgtk_c_vector_set_x(vec,i,val)
void sgtk_c_vector_set_x (SCM vec, size_t i, SCM val);

#define scm_u8vector_p(obj)  0
#define scm_s8vector_p(obj)  (SCM_NIMP(obj) && SCM_TYP7(obj) == scm_tc7_byvect)

#define scm_c_generalized_vector_length(vec)    \
  sgtk_c_generalized_vector_length(vec)
size_t sgtk_c_generalized_vector_length (SCM vec);

#define scm_c_generalized_vector_ref(vec,i)     \
  sgtk_c_generalized_vector_ref(vec,i)
SCM sgtk_c_generalized_vector_ref (SCM vec, size_t i);

#define scm_malloc(size)              sgtk_malloc (size)
void *sgtk_malloc (size_t size);

#define scm_gc_malloc(size, what)     scm_must_malloc (size, what)
#define scm_gc_free(ptr, size, what)    \
  do {                                  \
    scm_must_free (ptr);                \
    scm_done_free (size);               \
  } while (0)

#define scm_gc_register_collectable_memory(ptr,len,name)  scm_done_malloc (len)
#define scm_gc_unregister_collectable_memory(ptr,len,name) scm_done_free (len)

#define SCM_SMOB_DATA_2(x)  SCM_CELL_WORD_2(x)

#define scm_car(obj)  sgtk_car(obj)
#define scm_cdr(obj)  sgtk_cdr(obj)
SCM sgtk_car (SCM obj);
SCM sgtk_cdr (SCM obj);

/* This isn't terribly efficient, but is about all that can be done since
   keywords in 1.6 used "dash symbols".  */
#define scm_keyword_to_symbol(kw)                                       \
  (scm_string_to_symbol                                                 \
   (scm_substring (scm_symbol_to_string (scm_keyword_dash_symbol(kw)),  \
                   SCM_MAKINUM (1), SCM_UNDEFINED)))

#endif /* Guile 1.6 */


#endif /* COMPATH */

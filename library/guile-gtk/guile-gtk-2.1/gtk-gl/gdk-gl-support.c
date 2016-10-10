/*
 * Copyright (C) 2003 Free Software Foundation, Inc.
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

#include <glib.h>
#include <gtkgl/gdkgl.h>

#include <libguile.h>
#include <guile/gh.h>

typedef struct {
  char*		name;
  int		glx_value;
  int		intparam;
} sgtk_glXCC_type;

static sgtk_glXCC_type sgtk_glXCC_params [] = 
{
  {"use-gl",		GDK_GL_USE_GL,			0},
  {"buffer-size",	GDK_GL_BUFFER_SIZE,		1},
  {"level",		GDK_GL_LEVEL,			1},
  {"rgba",		GDK_GL_RGBA,			0},
  {"doublebuffer",	GDK_GL_DOUBLEBUFFER,		0},
  {"stereo",		GDK_GL_STEREO,			0},
  {"aux-buffers",	GDK_GL_AUX_BUFFERS,		1},
  {"red-size",		GDK_GL_RED_SIZE,		1},
  {"green-size",	GDK_GL_GREEN_SIZE,		1},
  {"blue-size",		GDK_GL_BLUE_SIZE,		1},
  {"alpha-size",	GDK_GL_ALPHA_SIZE,		1},
  {"depth-size",	GDK_GL_DEPTH_SIZE,		1},
  {"stencil-size",	GDK_GL_STENCIL_SIZE,		1},
  {"accum-red-size",	GDK_GL_ACCUM_RED_SIZE,		1},
  {"accum-green-size",	GDK_GL_ACCUM_GREEN_SIZE,	1},
  {"accum-blue-size",	GDK_GL_ACCUM_BLUE_SIZE,		1},
  {"accum-alpha-size",	GDK_GL_ACCUM_ALPHA_SIZE,	1},
  {NULL,-1,-1}
};

void
sgtk_gl_config_finish (void* config)
{ g_free (config); }

int*
sgtk_scm2gtk_gl_config (SCM s_attrlist, int pos, char *func)
{
  int		*attrlist, *aptr;
  int		size;

  int		symlen;
  char		*symbol;

  SCM		s_head, s_value, s_err;
  sgtk_glXCC_type *ptr;

  size = scm_ilength (s_attrlist);
  aptr = attrlist = g_new (int, size + 1);

  s_head = s_attrlist;
  while (s_head != SCM_EOL)
    {
      s_err = s_head;
      s_value = SCM_CAR (s_head);
      s_head = SCM_CDR (s_head);

      if (! SCM_SYMBOLP (s_value))
	goto error;

      symbol = SCM_SYMBOL_CHARS (s_value);
      symlen = SCM_SYMBOL_LENGTH (s_value);

      for (ptr = sgtk_glXCC_params; g_strcasecmp (ptr->name, symbol) != 0; ptr++)
	{
	  if (ptr->name == NULL)
	    goto error;
	}
      
      *aptr++ = ptr -> glx_value;

      if (ptr->intparam) {
	if (s_head == SCM_EOL)
	  goto error;	  
	else	
	  {
	    s_value = SCM_CAR (s_head);
	    s_head = SCM_CDR (s_head);

	    if (! SCM_INUMP (s_value))
	      goto error;
	    
	    *aptr++ = gh_scm2int (s_value);
	  }
      }
    }

  *aptr = GDK_GL_NONE;
  return attrlist;

 error:
  g_free (attrlist);
  SCM_ASSERT (0, s_err, pos, func);

  return NULL;
}

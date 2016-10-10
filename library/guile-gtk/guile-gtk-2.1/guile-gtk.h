/* Copyright (C) 1997, 1998, 1999, 2003, 2004, 2006, 2007 Free Software
 * Foundation, Inc.
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

#ifndef GUILE_GTK_H
#define GUILE_GTK_H

#include <libguile.h>
#include <gtk/gtk.h>
#include <glib-object.h>

typedef guint32 sgtk_timestamp;

typedef struct _sgtk_type_info {
  char *name;
  GtkType type;
  SCM (*conversion) (SCM);
  GtkType (*init_func) ();
} sgtk_type_info;

typedef struct _sgtk_enum_literal {
  SCM symbol;
  char *name;
  int value;
} sgtk_enum_literal;

typedef struct _sgtk_enum_info {
  sgtk_type_info header;
  int n_literals;
  sgtk_enum_literal *literals;
} sgtk_enum_info;

/* This is like an _sgtk_enum_literal, but the values are strings.
   This is used in Gnome.  */
typedef struct _sgtk_senum_literal {
  char *name;
  char *value;
} sgtk_senum_literal;

typedef struct _sgtk_senum_info {
  sgtk_type_info header;
  int n_literals;
  sgtk_senum_literal *literals;
} sgtk_senum_info;

typedef struct _sgtk_boxed_info {
  sgtk_type_info header;
  void *(*copy) (void *);
  void (*destroy) (void *);
  void (*cleanup) (SCM);
  size_t size;
} sgtk_boxed_info;

typedef struct _sgtk_object_info {
  sgtk_type_info header;

  struct _sgtk_object_info *parent;
  guint n_args;
  GtkArg *args;
  guint *args_flags;
  char **args_short_names;
} sgtk_object_info;


/* gdk-glue.c */
extern sgtk_boxed_info sgtk_gdk_color_info;
extern sgtk_object_info sgtk_gdk_colormap_info;
extern sgtk_boxed_info sgtk_gdk_cursor_info;
extern sgtk_boxed_info sgtk_gdk_font_info;
extern sgtk_enum_info sgtk_gdk_gravity_info;
extern sgtk_object_info sgtk_gdk_pixmap_info;
extern sgtk_object_info sgtk_gdk_window_info;
extern sgtk_object_info sgtk_gdk_visual_info;
extern sgtk_enum_info sgtk_gdk_cap_style_info;
extern sgtk_enum_info sgtk_gdk_fill_info;
extern sgtk_enum_info sgtk_gdk_function_info;
extern sgtk_enum_info sgtk_gdk_join_style_info;
extern sgtk_enum_info sgtk_gdk_line_style_info;
extern sgtk_enum_info sgtk_gdk_subwindow_mode_info;
extern sgtk_enum_info sgtk_gdk_visual_type_info;
SCM sgtk_gdk_font_load (SCM font);
void sgtk_init_gtk_gdk_glue_types (void);

/* gtk-glue.c */
extern sgtk_enum_info sgtk_gtk_arg_flags_info;
void sgtk_init_gtk_gtk_glue_types (void);

/* gtk-gl/gdk-gl-glue.c */
void sgtk_init_gtk_gdk_gl_glue_types (void);


/* C string objects */
int sgtk_valid_cstr (SCM obj);
SCM sgtk_to_cstr (SCM obj);
char *sgtk_cstr2ptr (SCM obj, unsigned long pos, const char *func_name);

/* C memory objects */
SCM sgtk_make_cblk (void *p, size_t len);


void sgtk_register_type_infos (sgtk_type_info **infos);
void sgtk_register_type_infos_gtk (GtkTypeInfo **infos);
sgtk_type_info *sgtk_get_type_info (guint type_seqno);
GtkType sgtk_type_from_name (char *name);

SCM sgtk_wrap_gtkobj (GObject *obj);
SCM sgtk_wrap_gtkobj_nocopy (GObject *obj);
int sgtk_is_a_gtkobj (GType type, SCM obj);
GObject *sgtk_get_gtkobj (SCM obj);
GType sgtk_g_object_get_type (void);

void sgtk_enum_flags_init (sgtk_enum_info*);
int sgtk_enum_flags_bin_search (SCM key, sgtk_enum_info *info, int *rval);

int sgtk_valid_enum (SCM obj, sgtk_enum_info*);
SCM sgtk_enum2scm (gint val, sgtk_enum_info*);
gint sgtk_scm2enum (SCM obj, sgtk_enum_info*, int pos, char *sname);

int sgtk_valid_flags (SCM obj, sgtk_enum_info*);
SCM sgtk_flags2scm (gint val, sgtk_enum_info*);
gint sgtk_scm2flags (SCM obj, sgtk_enum_info*, int pos, char *sname);

int sgtk_valid_senum (SCM obj, sgtk_senum_info*);
SCM sgtk_senum2scm (char *val, sgtk_senum_info*);
char *sgtk_scm2senum (SCM obj, sgtk_senum_info*);

SCM sgtk_boxed2scm (gpointer ptr, sgtk_boxed_info*, int copyp);
void *sgtk_scm2boxed (SCM obj);
int sgtk_valid_boxed (SCM obj, sgtk_boxed_info*);
void sgtk_boxed_invalidate (SCM obj);

int sgtk_valid_point (SCM obj);
GdkPoint sgtk_scm2point (SCM obj);
SCM sgtk_point2scm (GdkPoint p);

struct sgtk_rectangle {
  int null;
  GdkRectangle r;
};
int sgtk_valid_rect (SCM obj);
GdkRectangle sgtk_scm2rect (SCM obj);
struct sgtk_rectangle sgtk_scm2rect_null_ok (SCM obj);
SCM sgtk_rect2scm (GdkRectangle r);

int sgtk_valid_segment (SCM obj);
GdkSegment sgtk_scm2segment (SCM obj);
SCM sgtk_segment2scm (GdkSegment seg);

int sgtk_port2fileno (SCM port);
SCM sgtk_fileno2port (int fd);

GdkAtom sgtk_scm2atom (SCM symbol);
SCM sgtk_atom2scm (GdkAtom atom);

int sgtk_valid_type (SCM obj);
GtkType sgtk_scm2type (SCM obj);
SCM sgtk_type2scm (GtkType t);

int sgtk_valid_composite (SCM obj, int (*predicate)(SCM));
int sgtk_valid_complen (SCM obj, int (*predicate)(SCM), int len);
SCM sgtk_composite_inconversion (SCM obj, SCM (*conversion)(SCM));
SCM sgtk_composite_outconversion (SCM obj, SCM (*conversion)(SCM));

SCM sgtk_slist2scm (GSList *list, SCM (*toscm)(void*));
GSList *sgtk_scm2slist (SCM obj, void (*fromscm)(SCM, void*));
void sgtk_slist_finish (GSList *list, SCM obj, SCM (*toscm)(void*));

SCM sgtk_list2scm (GList *list, SCM (*toscm)(void*));
GList *sgtk_scm2list (SCM obj, void (*fromscm)(SCM, void*));
void sgtk_list_finish (GList *list, SCM obj, SCM (*toscm)(void*));

typedef struct {
  int count;
  void *vec;
} sgtk_cvec;

sgtk_cvec sgtk_scm2cvec (SCM obj, void (*fromscm)(SCM, void*), size_t sz);
void sgtk_cvec_finish (sgtk_cvec *, SCM obj, SCM (*toscm)(void*), size_t sz);

typedef struct {
  int count;
  guchar *raw;
  SCM keep;
} sgtk_raw;

sgtk_raw sgtk_scm2raw (SCM obj, int, char*);

typedef struct sgtk_protshell sgtk_protshell;

sgtk_protshell *sgtk_protect (SCM protector, SCM obj);
void sgtk_unprotect (sgtk_protshell *);

void sgtk_callback_marshal (GtkObject *,
			    gpointer data,
			    guint n_args,
			    GtkArg *args);
void sgtk_callback_destroy (gpointer data);
SCM sgtk_callback_trampoline (SCM new_trampoline);

void sgtk_about_dialog_activate_link_marshal (GtkAboutDialog *about,
                                              const gchar *link,
                                              gpointer data);
void gtk_about_dialog_set_email_hook_interp (SCM func);
void gtk_about_dialog_set_url_hook_interp (SCM func);
void gtk_link_button_set_uri_hook_interp (SCM func);


void sgtk_closure_marshal (GClosure *closure,
                           GValue *return_value,
                           guint n_param_values,
                           const GValue *param_values,
                           gpointer invocation_hint,
                           gpointer marshal_data);
void sgtk_closure_destroy (gpointer data, GClosure *closure);

int sgtk_valid_arg (GtkArg *, SCM val);
SCM sgtk_arg2scm (GtkArg *a, int free_mem);
void sgtk_scm2ret (GtkArg *a, SCM obj);
void sgtk_throw_gerror (const char *func_name, GError *gerr);

sgtk_object_info *sgtk_find_object_info_from_type (GtkType type);
sgtk_object_info *sgtk_find_object_info (char *name);

SCM sgtk_color_conversion (SCM color);
SCM sgtk_font_conversion (SCM color);

void sgtk_set_standalone (int flag);
int sgtk_is_standalone ();
SCM sgtk_standalone_p ();

void sgtk_register_glue (char *name, void (*func)(void));
#define SGTK_REGISTER_GLUE(func) sgtk_register_glue (#func, func)

void sgtk_init_gdk_support (void);
void sgtk_init_gtk_support (void);
void sgtk_init_threads (void);

void sgtk_init ();
void sgtk_init_with_args (int *argcp, char ***argvp);

void sgtk_shell (int argc, char **argv);

/* Additional useful Gdk routines. */

SCM gdk_window_mark (GdkWindow *window);
GdkWindow *gdk_window_new_interp (GdkWindow *parent, int width, int height,
				  GdkEventMask event_mask,
				  GdkWindowClass window_class,
				  GdkWindowType window_type, SCM rest);
void gdk_window_destroy_interp (GdkWindow *window);
SCM gdk_window_get_children_interp (GdkWindow *window);

guint32 gdk_window_get_id (GdkWindow *window);
GdkEventType gdk_event_type (GdkEvent *event);
GdkWindow *gdk_event_window (GdkEvent *event);
gboolean gdk_event_send_event (GdkEvent *event);
GdkRectangle gdk_event_area (GdkEvent *event);
gint gdk_event_count (GdkEvent *event);
GdkVisibilityState gdk_event_visibility_state (GdkEvent *event);
guint32 gdk_event_time (GdkEvent *event);
gdouble gdk_event_x (GdkEvent *event);
gdouble gdk_event_y (GdkEvent *event);
gint gdk_event_button (GdkEvent *event);
guint gdk_event_state (GdkEvent *event);
gboolean gdk_event_is_hint (GdkEvent *event);
GdkInputSource gdk_event_source (GdkEvent *event);
gdouble gdk_event_x_root (GdkEvent *event);
gdouble gdk_event_y_root (GdkEvent *event);
guint gdk_event_keyval (GdkEvent *event);
SCM gdk_event_string (GdkEvent *event);
GdkWindow *gdk_event_subwindow (GdkEvent *event);
GdkCrossingMode gdk_event_crossing_mode (GdkEvent *event);
GdkNotifyType gdk_event_notify_detail (GdkEvent *event);
gboolean gdk_event_in (GdkEvent *event);
int gdk_event_focus (GdkEvent *event);
gint16 gdk_event_configure_x (GdkEvent *event);
gint16 gdk_event_configure_y (GdkEvent *event);
gint16 gdk_event_configure_width (GdkEvent *event);
gint16 gdk_event_configure_height (GdkEvent *event);
GdkAtom gdk_event_atom (GdkEvent *event);
GdkAtom gdk_event_selection (GdkEvent *event);
GdkAtom gdk_event_target (GdkEvent *event);
GdkAtom gdk_event_property (GdkEvent *event);
GdkEvent *gdk_event_new (GdkEventType type);
guint32 gdk_event_requestor (GdkEvent *event);
GdkDragContext* gdk_event_drag_context (GdkEvent *event);
GdkAtom gdk_event_message_type (GdkEvent *event);
SCM gdk_event_message (GdkEvent *event);
void gdk_add_client_message_filter_interp (GdkAtom message_type,
					   SCM filter_proc);

SCM gdk_rectangle_intersect_interp (GdkRectangle *rect1, GdkRectangle *rect2);

GdkRectangle gdk_rectangle_union_interp (GdkRectangle *rect1,
					 GdkRectangle *rect2);

GdkRectangle gdk_region_get_clipbox_interp (GdkRegion *region);

GdkImage *gdk_image_new_bitmap_interp (GdkVisual *visual,
				       guchar data[], int count,
				       gint width, gint height);

GdkBitmap *gdk_bitmap_create_from_data_interp (GdkWindow *window,
					       guchar data[], int count,
					       gint width, gint height);

GdkPixmap *gdk_pixmap_create_from_data_interp (GdkWindow *window,
					       guchar data[], int count,
					       gint width, gint height,
					       gint depth,
					       GdkColor *fg, GdkColor *bg);

GdkPixmap *gdk_pixmap_create_from_xpm_d_interp (GdkWindow *window,
						GdkBitmap **mask,
						GdkColor *transparent_color,
						char *data[],int count);

GdkPixmap *
gdk_pixmap_colormap_create_from_xpm_d_interp (GdkWindow *window,
					      GdkColormap *colormap,
					      GdkBitmap **mask,
					      GdkColor *transp_color,
					      char *data[],int count);

char *gdk_wcstombs_interp (GdkWChar src[], int count);

SCM gdk_mbstowcs_interp (const char *src);

GdkGCValuesMask sgtk_gdk_gc_values_fill (char *func_name, int argnum,
                                         GdkGCValues *values, SCM rest);
GdkGC *gdk_gc_new_with_values_interp (GdkWindow *window, SCM rest);
SCM gdk_gc_get_values_interp (GdkGC *gc);

guint32 gdk_get_leader_window_id ();

int gtk_editable_insert_text_scm (GtkEditable *editable,
                                  gchar *text, int position);

void gdk_draw_text_interp (GdkDrawable *drawable, GdkFont *font, GdkGC *gc,
			   gint x, gint y, SCM text);

void gdk_text_extents_interp (GdkFont *font, SCM text,
			      gint *lbearing, gint *rbearing, gint *width,
			      gint *ascent, gint *descent);
gint gdk_text_width_interp (GdkFont *font, SCM text);
gint gdk_text_measure_interp (GdkFont *font, SCM text);
gint gdk_text_height_interp (GdkFont *font, SCM text);

/* Gtk stuff that wouldn't be here in an ideal world. */

SCM gtk_widget_size_request_interp (GtkWidget *widget);
GtkWidget *gtk_message_dialog_new_interp (GtkWindow *parent,
                                          GtkDialogFlags flags,
                                          GtkMessageType type,
                                          GtkButtonsType buttons,
                                          const char *message);
SCM gtk_container_children_interp (GtkContainer *container);
gchar *gtk_label_get_interp (GtkLabel *label);
void gtk_menu_popup_interp (GtkMenu *menu,
			    GtkWidget *parent_menu_shell,
			    GtkWidget *parent_menu_item,
			    gint button,
			    guint32 activate_time);

GdkColor *gdk_color_new (void);
GdkColor *gdk_color_parse_interp (char *spec);
GdkColor *gtk_style_get_white_interp (GtkStyle *style);

void gtk_list_append_item (GtkList *list, GtkListItem *item);
void gtk_list_prepend_item (GtkList *list, GtkListItem *item);

gboolean gtk_type_get_info (GtkType type, GtkTypeInfo *info);
GtkType gtk_class_new (GtkType parent_type, gchar *name);
guint
gtk_signal_new_generic (const gchar     *name,
			GtkSignalRunType signal_flags,
			GtkType          type,
			GtkType          return_type,
			guint            nparams,
			GtkType         *params);
SCM sgtk_signal_emit (GObject *obj, char *name, SCM scm_args);
void gtk_signal_set_class_function_full (GtkType            type,
					 const gchar       *signal,
					 GtkSignalFunc      func,
					 GtkCallbackMarshal marshal,
					 gpointer           data,
					 GtkDestroyNotify   destroy_func);
gulong gtk_signal_connect_interp (SCM         object,
                                  const gchar *signal,
                                  SCM         func,
                                  gboolean    object_signal,
                                  gboolean    after);


void gtk_color_selection_set_color_interp (GtkColorSelection *sel, GdkColor *color);
GdkColor *gtk_color_selection_get_color_interp (GtkColorSelection *sel);

SCM gtk_selection_data_data (GtkSelectionData* data);

void gdk_draw_rgb_image_interp (GdkDrawable *drawable, GdkGC *gc,
				gint x, gint y, gint w, gint h,
				GdkRgbDither dith,
				guchar rgb_buf[], int count,
				gint rowstride);

void gdk_draw_rgb_image_dithalign_interp (GdkDrawable *drawable, GdkGC *gc,
					  gint x, gint y, gint w, gint h,
					  GdkRgbDither dith,
					  guchar rgb_buf[], int count,
					  gint rowstride,
					  gint xdith, gint ydith);

void gdk_draw_indexed_image_interp (GdkDrawable *drawable, GdkGC *gc,
				    gint x, gint y, gint w, gint h,
				    GdkRgbDither dith,
				    guchar index_buf[], int i_count,
				    gint rowstride,
				    GdkRgbCmap *cmap);

void gdk_draw_gray_image_interp (GdkDrawable *drawable, GdkGC *gc,
				 gint x, gint y, gint w, gint h,
				 GdkRgbDither dith,
				 guchar gray_buf[], int count,
				 gint rowstride);

void gdk_draw_rgb_32_image_interp (GdkDrawable *drawable, GdkGC *gc,
				   gint x, gint y, gint w, gint h,
				   GdkRgbDither dith,
				   guchar rgb_32_buf[], int count,
				   gint rowstride);

GdkRgbCmap *gdk_rgb_cmap_new_interp (guint32 colors[], int count);
void gdk_rgb_gc_set_foreground_interp (GdkGC *gc, guint32 rgb);
void gdk_rgb_gc_set_background_interp (GdkGC *gc, guint32 rgb);
void gdk_rgb_set_install_interp (int install);
void gdk_rgb_set_min_colors_interp (int min_colors);
GdkColormap *gdk_rgb_get_cmap_interp (void);
GdkVisual *gdk_rgb_get_visual_interp (void);
gboolean gdk_rgb_ditherable_interp (void);
void gdk_rgb_set_verbose_interp (gboolean verbose);

gint gdk_colormap_alloc_colors_interp (GdkColormap *colormap, SCM colors,
                                       int writable, int best_match,
                                       SCM *s_success);
void gdk_colormap_free_colors_interp (GdkColormap *colormap, SCM colors);

GdkColor *gdk_color_white_interp (GdkColormap *colormap);
GdkColor *gdk_color_black_interp (GdkColormap *colormap);

SCM gdk_query_depths_interp (void);
SCM gdk_query_visual_types_interp (void);
SCM gdk_list_visuals_interp (void);

GList* gdk_drag_context_targets (GdkDragContext*);

GtkTargetEntry sgtk_scm2gtk_target_entry (SCM obj, int pos, char* subr);
void sgtk_gtk_target_entry_free (GtkTargetEntry*);

void* gtk_fake_copy (void *);
void *gtk_no_copy (void *);
void gtk_no_free (void *);

GdkColor *gtk_style_white (GtkStyle *style);
GdkColor *gtk_style_black (GtkStyle *style);
GdkColor *gtk_style_fg (GtkStyle *style, GtkStateType state);
GdkColor *gtk_style_bg (GtkStyle *style, GtkStateType state);
GdkColor *gtk_style_light (GtkStyle *style, GtkStateType state);
GdkColor *gtk_style_dark (GtkStyle *style, GtkStateType state);
GdkColor *gtk_style_mid (GtkStyle *style, GtkStateType state);
GdkColor *gtk_style_text (GtkStyle *style, GtkStateType state);
GdkColor *gtk_style_base (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_fg_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_bg_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_light_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_dark_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_mid_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_text_gc (GtkStyle *style, GtkStateType state);
GdkGC *gtk_style_base_gc (GtkStyle *style, GtkStateType state);

void gdk_window_set_geometry_hints_interp (GdkWindow *window, SCM rest);
SCM gdk_window_get_position_interp (GdkWindow *window);
SCM gdk_window_get_root_origin_interp (GdkWindow *window);
SCM gdk_window_get_deskrelative_origin_interp (GdkWindow *window);
SCM gdk_window_get_size_interp (GdkWindow *);
SCM gdk_window_get_origin_interp (GdkWindow *);

SCM gdk_selection_property_get_interp (GdkWindow *requestor,
				       GdkAtom *prop_type, int *prop_format);
void gdk_property_change_interp (GdkWindow *window,
				 GdkAtom property, GdkAtom type,
				 gint format, GdkPropMode mode, SCM data);

SCM gdk_text_property_to_text_list_interp (GdkAtom encoding, gint format,
					   SCM text);
gint gdk_string_to_compound_text_interp (char *str, GdkAtom *encoding,
                                         gint *format, SCM *textp);
SCM gdk_property_get_interp (GdkWindow *window, GdkAtom property, GdkAtom type,
			     gulong offset, gulong length, int pdelete,
			     GdkAtom *actual_property_type,
			     gint *actual_format);

SCM gtk_rc_get_default_files_interp (void);
GtkTextIter *gtk_text_iter_new_interp (void);

void sgtk_issue_deprecation_warning (const char*);

#endif /* !GUILE_GTK_H */

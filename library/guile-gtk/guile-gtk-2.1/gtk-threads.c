/* Threading for guile-gtk
 *
 * Copyright (C) 2000, 2002, 2003, 2006, 2007 Free Software Foundation
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


/* The code here makes a compile-time selection between the following
   threads methods

       - guile 1.6 coop
       - guile 1.6 no threads
       - guile 1.8, with its pthreads or no threads noticed at runtime

   The runtime test for 1.8 can be done because glib has pthreads builtin
   already, we don't need any special pthreads compile or link flags to
   enable it (not beyond what glib already gives us).  But note that in
   guile 1.8.0 through 1.8.2 this is not enough.  A guile-gtk build against
   those versions ends up depending on the compile-time threads method due
   to inlining of scm_cell() by guile.  This afflicts all smob-using
   programs and might with luck be fixed in a future guile version.  */


/* Define the following option to include the primitive
 * `gtk-threads-update' which makes the main loop wake up.
 */
#define GUILE_GTKTHREADS_UPDATE 1

#include <config.h>
#include <libguile.h>
#include <unistd.h>
#include <glib.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "gtk-threads.h"


/*---------------------------------------------------------------------------*/
/* Guile 1.6 cooperative threads, ie ./configure --with-threads=coop
 */

#if USE_COOP_THREADS       /* Guile 1.6 */              \
  || SCM_USE_COOP_THREADS  /* future Guile 1.8 maybe */ \
  || SCM_USE_COPT_THREADS  /* future Guile 1.8 maybe */
#define FOUND 1

#if USE_COOP_THREADS && ! defined (SCM_USE_COOP_THREADS)
#define SCM_USE_COOP_THREADS 1
#endif

#ifdef FD_SET
#  define SELECT_MASK fd_set
#else /* !NO_FD_SET */
#  ifdef _IBMR2 /*fixme* not defined*/
#    define SELECT_MASK void
#  else /* !_IBMR2 */
#    define SELECT_MASK int
#  endif /* !_IBMR2 */
#endif /* !NO_FD_SET */

#ifdef GUILE_GTKTHREADS_UPDATE
static int poll_waiting = 0;
static int wake_up_pipe[2] = { -1, -1 };
static GPollFD wake_up_rec;
#endif

static gint 
g_poll (GPollFD *fds,
	guint    nfds,
	gint     timeout)
{
  struct timeval tv;
  SELECT_MASK rset, wset, xset;
  GPollFD *f;
  int ready;
  int maxfd = 0;

  FD_ZERO (&rset);
  FD_ZERO (&wset);
  FD_ZERO (&xset);

  for (f = fds; f < &fds[nfds]; ++f)
    if (f->fd >= 0)
      {
	if (f->events & G_IO_IN)
	  FD_SET (f->fd, &rset);
	if (f->events & G_IO_OUT)
	  FD_SET (f->fd, &wset);
	if (f->events & G_IO_PRI)
	  FD_SET (f->fd, &xset);
	if (f->fd > maxfd && (f->events & (G_IO_IN|G_IO_OUT|G_IO_PRI)))
	  maxfd = f->fd;
      }

  tv.tv_sec = timeout / 1000;
  tv.tv_usec = (timeout % 1000) * 1000;

#ifdef GUILE_GTKTHREADS_UPDATE
  poll_waiting = TRUE;
#endif
  
  ready = scm_internal_select (maxfd + 1, &rset, &wset, &xset,
		               timeout == -1 ? NULL : &tv);

#ifdef GUILE_GTKTHREADS_UPDATE
  if (!poll_waiting)
    {
#ifndef NATIVE_WIN32
      gchar c;
      read (wake_up_pipe[0], &c, 1);
#endif
    }
  else
    poll_waiting = FALSE;
#endif

  if (ready > 0)
    for (f = fds; f < &fds[nfds]; ++f)
      {
	f->revents = 0;
	if (f->fd >= 0)
	  {
	    if (FD_ISSET (f->fd, &rset))
	      f->revents |= G_IO_IN;
	    if (FD_ISSET (f->fd, &wset))
	      f->revents |= G_IO_OUT;
	    if (FD_ISSET (f->fd, &xset))
	      f->revents |= G_IO_PRI;
	  }
      }

  return ready;
}

#ifdef GUILE_GTKTHREADS_UPDATE
/* Wake the main loop up from a poll() */
static void
g_main_wakeup (void)
{
  if (poll_waiting)
    {
      poll_waiting = FALSE;
#ifndef NATIVE_WIN32
      write (wake_up_pipe[1], "A", 1);
#else
      ReleaseSemaphore (wake_up_semaphore, 1, NULL);
#endif
    }
}

SCM
sgtk_threads_update ()
{
  g_main_wakeup ();
  return SCM_UNSPECIFIED;
}

#else  /* !GUILE_GTKTHREADS_UPDATE */

SCM
sgtk_threads_update ()
{
  return SCM_UNSPECIFIED;
}

#endif /* !GUILE_GTKTHREADS_UPDATE */

#define guile_print_error( name, num )                          \
  g_error( "file %s: line %d (%s): error %s during %s",         \
           __FILE__, __LINE__, G_GNUC_PRETTY_FUNCTION,          \
           g_strerror((num)), #name )

#define guile_check_for_error( what ) G_STMT_START{             \
   int error = (what);                                           \
   if( error ) { guile_print_error( what, error ); }             \
   }G_STMT_END

static GMutex *
g_mutex_new_guile_impl (void)
{
  GMutex *result = (GMutex *) g_new (scm_t_mutex, 1);
#ifdef SCM_MUTEX_INIT_TWO_ARGS
  guile_check_for_error (scm_mutex_init ((scm_t_mutex *) result, 0));
#else
  guile_check_for_error (scm_mutex_init ((scm_t_mutex *) result));
#endif
  return result;
}

static void
g_mutex_lock_guile_impl (GMutex * mutex)
{
  scm_t_mutex *m = (scm_t_mutex *) mutex;
  scm_mutex_lock (m);
}

static void
g_mutex_unlock_guile_impl (GMutex * mutex)
{
  scm_t_mutex *m = (scm_t_mutex *) mutex;
  scm_mutex_unlock (m);
}

static void
g_mutex_free_guile_impl (GMutex * mutex)
{
  guile_check_for_error (scm_mutex_destroy ((scm_t_mutex *) mutex));
  g_free (mutex);
}

/* NOTE: the functions g_mutex_lock and g_mutex_unlock may not use
 * functions from gmem.c and gmessages.c;
 */

/* scm_mutex_lock, scm_mutex_unlock can be taken directly, as
 * signature and semantics are right, but without error check
 * We might want to change this.
 */

static gboolean
g_mutex_trylock_guile_impl (GMutex * mutex)
{
  int result;

  result = scm_mutex_trylock ((scm_t_mutex *) mutex);

  if (result == EBUSY)
    return FALSE;

  guile_check_for_error (result);
  return TRUE;
}

static GCond *
g_cond_new_guile_impl (void)
{
  GCond *result = (GCond *) g_new (scm_t_cond, 1);
  guile_check_for_error (scm_cond_init ((scm_t_cond *) result, NULL));
  return result;
}

/* scm_cond_signal, scm_cond_broadcast and scm_cond_wait can be taken
 * directly, as signatures and semantics are right, but without error
 * check.  We might want to change this.
 */

#define G_MICROSEC 1000000
#define G_NANOSEC 1000000000

static gboolean
g_cond_timed_wait_guile_impl (GCond * cond,
			      GMutex * entered_mutex,
			      GTimeVal * abs_time)
{
  int result;
  struct timespec end_time;
  gboolean timed_out;

  g_return_val_if_fail (cond != NULL, FALSE);
  g_return_val_if_fail (entered_mutex != NULL, FALSE);

  if (!abs_time)
    {
      g_cond_wait (cond, entered_mutex);
      return TRUE;
    }

  end_time.tv_sec = abs_time->tv_sec;
  end_time.tv_nsec = abs_time->tv_usec * (G_NANOSEC / G_MICROSEC);
  g_assert (end_time.tv_nsec < G_NANOSEC);
  result = scm_cond_timedwait ((scm_t_cond *) cond,
			       (scm_t_mutex *) entered_mutex,
			       &end_time);

  timed_out = (result == ETIME);

  if (!timed_out)
    guile_check_for_error (result);
  return !timed_out;
}

static void
g_cond_free_guile_impl (GCond * cond)
{
  guile_check_for_error (scm_cond_destroy ((scm_t_cond *) cond));
  g_free (cond);
}

static GPrivate *
g_private_new_guile_impl (GDestroyNotify destructor)
{
  GPrivate *result = (GPrivate *) g_new (scm_t_key, 1);
  guile_check_for_error (scm_key_create ((scm_t_key *) result,
					 destructor));
  return result;
}

/* NOTE: the functions g_private_get and g_private_set may not use
   functions from gmem.c and gmessages.c */

static void
g_private_set_guile_impl (GPrivate * private_key, gpointer value)
{
  if (!private_key)
    return;

  scm_setspecific (*(scm_t_key *) private_key, value);
}

static gpointer
g_private_get_guile_impl (GPrivate * private_key)
{
  if (!private_key)
    return NULL;
  return scm_getspecific (*(scm_t_key *) private_key);
}

struct spawn_data {
  GThreadFunc func;
  gpointer arg;
};

static SCM
spawn (void *arg)
{
  struct spawn_data *data = (struct spawn_data *) arg;
  data->func (data->arg);
  return SCM_UNSPECIFIED;
}

static void
g_thread_create_guile_impl (GThreadFunc thread_func, 
			    gpointer arg, 
			    gulong stack_size,
			    gboolean joinable,
			    gboolean bound,
			    GThreadPriority priority,
			    gpointer thread,
                            GError **error)
{
  struct spawn_data data;
  SCM t;
  data.func = thread_func;
  data.arg = arg;
  t = scm_spawn_thread (spawn, &data, scm_handle_by_message_noexit, 0);
  * (SCM *) thread = t;
}

static void
g_thread_join_guile_impl (gpointer thread)
{
#ifdef SCM_USE_COOP_THREADS
  coop_join ((coop_t *) SCM_THREAD_DATA ((SCM) thread));
#endif
#ifdef SCM_USE_COPT_THREADS
  scm_join_thread ((SCM) thread);
#endif
}

#ifdef SCM_USE_COOP_THREADS
extern void coop_abort (void);
#endif

static void
g_thread_exit_guile_impl (void)
{
#ifdef SCM_USE_COOP_THREADS
  coop_abort ();
#else
  fprintf (stderr, "g_thread_exit_guile_impl not implemented\n");
  abort ();
#endif
}

static void
g_thread_set_priority_guile_impl (gpointer thread, GThreadPriority priority)
{
}

static void
g_thread_self_guile_impl (gpointer thread)
{
#ifdef SCM_USE_COOP_THREADS
  scm_root_state *rs = coop_global_curr->data;
  SCM self = rs->handle;
  * (SCM *) thread = self;
#else
#ifdef SCM_USE_COPT_THREADS
  * (SCM *) thread = cur_thread;
#else
#error Oops, no g_thread_self_guile_impl code
#endif
#endif
}

static GThreadFunctions g_thread_functions =
{
  g_mutex_new_guile_impl,
  g_mutex_lock_guile_impl,
  g_mutex_trylock_guile_impl,
  g_mutex_unlock_guile_impl,
  g_mutex_free_guile_impl,
  g_cond_new_guile_impl,
  scm_cond_signal, /* cond_signal */
  scm_cond_broadcast, /* cond_broadcast */
  scm_cond_wait, /* cond_wait */
  g_cond_timed_wait_guile_impl,
  g_cond_free_guile_impl,
  g_private_new_guile_impl,
  g_private_get_guile_impl,
  g_private_set_guile_impl,
  g_thread_create_guile_impl,
  (void (*)(void)) scm_yield,
  g_thread_join_guile_impl,
  g_thread_exit_guile_impl,
  g_thread_set_priority_guile_impl,
  g_thread_self_guile_impl
};

void
sgtk_init_threads ()
{
  g_thread_init (&g_thread_functions);
  g_main_set_poll_func (g_poll);
#ifdef GUILE_GTKTHREADS_UPDATE
  if (pipe (wake_up_pipe) < 0)
    g_error ("Cannot create pipe main loop wake-up: %s\n",
	     g_strerror (errno));

  wake_up_rec.fd = wake_up_pipe[0];
  wake_up_rec.events = G_IO_IN;
  g_main_context_add_poll (g_main_context_default(), &wake_up_rec, 0);
#endif
}

#endif /* coop threads */


/*---------------------------------------------------------------------------*/
/* Guile 1.6 null threads, ie ./configure --with-threads=no (or null) */

#if (SCM_MAJOR_VERSION == 1 && SCM_MINOR_VERSION == 6 && \
     ! USE_THREADS)
#define FOUND 1

SCM
sgtk_threads_update ()
{
  return SCM_UNSPECIFIED;
}

void
sgtk_init_threads ()
{
}

#endif /* Guile 1.6 null threads */



/*---------------------------------------------------------------------------*/
/* Guile 1.8 and up runtime detection of either
       - pthreads, ie ./configure --with-threads=pthreads
       - no threads, ie. ./configure --with-threads=no (or null)
 */

#if (SCM_MAJOR_VERSION > 1 \
     || (SCM_MAJOR_VERSION == 1 && SCM_MINOR_VERSION >= 8))
#define FOUND 1

SCM
sgtk_threads_update ()
{
  return SCM_UNSPECIFIED;
}


/* There's no `scm_provided_p' to use here, `provided?' is only scheme code
   (as of guile 1.8.2). */
static int
sgtk_scm_c_provided_p (const char *name)
{
  SCM module = scm_c_resolve_module ("guile");
  SCM features = scm_variable_ref (scm_c_module_lookup (module, "*features*"));
  return scm_is_true (scm_memq (scm_from_locale_symbol (name), features));
}

void
sgtk_init_threads ()
{
  /* The "threads" feature means some sort of threading.  Would like to
     check for some more definite indication that it's pthreads, so we don't
     do the wrong thing if there's a new style of threads in the future.
     But a hypothetical new style would need work done on the code in this
     file anyway, so no need to worry for now.

     g_thread_init(NULL) asks for the default threading, which is posix
     pthreads on a GNU/Linux system.*/

  if (sgtk_scm_c_provided_p ("threads"))
    g_thread_init (NULL);
}

#endif /* Guile 1.8 and up */



/*---------------------------------------------------------------------------*/
#if ! FOUND
#error "Oops, Guile threading system recognised"
#endif

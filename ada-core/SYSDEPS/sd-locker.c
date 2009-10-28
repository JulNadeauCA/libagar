/*
 * Attempt to check OS type.
 */

#define SD_LOCKER_OS_POSIX 0x0000
#define SD_LOCKER_OS_WIN32 0x0001

/* Win32 */
#ifndef SD_LOCKER_OS_TYPE
#  if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(__WIN64__) || defined(__TOS_WIN__)
#    define SD_LOCKER_OS_TYPE SD_LOCKER_OS_WIN32
#  endif
#endif

/* Fallback OS type - POSIX */
#ifndef SD_LOCKER_OS_TYPE
#  define SD_LOCKER_OS_TYPE SD_LOCKER_OS_POSIX
#endif

/*
 * OS-independent headers.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct sd_locker_state_t {
  char *lock_file;
  int   id;
  int   exit_code;
  int   locked;
};

static void
sd_locker_fatal (const struct sd_locker_state_t *, const char *);

/* Select implementation based on OS. */
#if SD_LOCKER_OS_TYPE == SD_LOCKER_OS_WIN32
#  include "sd-locker_win32.c"
#else
#  include "sd-locker_posix.c"
#endif

static const char *
sd_locker_error_message (void)
{
#if SD_LOCKER_OS_TYPE == SD_LOCKER_OS_WIN32
  return sd_locker_win32_error_message ();
#else
  return sd_locker_posix_error_message ();
#endif
}

static void
sd_locker_fatal
  (const struct sd_locker_state_t *state,
   const char *message)
{
  assert (state != NULL);
  assert (message != NULL);

  (void) fprintf (stderr, "sd-locker: %d: %s - %s\n", state->id, message, sd_locker_error_message ());
  (void) fflush (stderr);
  exit (EXIT_FAILURE);
}

static void
sd_locker_lock_announce
  (const struct sd_locker_state_t *state,
   const char *message)
{
  assert (state   != NULL);
  assert (message != NULL);

#ifdef SD_LOCKER_DEBUGGING
  (void) fprintf (stderr, "sd-locker: %d: %s - %s\n", state->id, state->lock_file, message);
  (void) fflush (stderr);
#endif
}

static void
sd_locker_lock_file_open
  (struct sd_locker_state_t *state)
{
  assert (state != NULL);
#if SD_LOCKER_OS_TYPE == SD_LOCKER_OS_WIN32
  sd_locker_win32_lock_file_open (state);
#else
  sd_locker_posix_lock_file_open (state);
#endif
}

static void
sd_locker_lock_file_close
  (struct sd_locker_state_t *state)
{
  assert (state != NULL);
#if SD_LOCKER_OS_TYPE == SD_LOCKER_OS_WIN32
  sd_locker_win32_lock_file_close (state);
#else
  sd_locker_posix_lock_file_close (state);
#endif
}

static void
sd_locker_lock_acquire (struct sd_locker_state_t *state)
{
  assert (state != NULL);
  sd_locker_lock_announce (state, "acquiring");
#if SD_LOCKER_OS_TYPE == SD_LOCKER_OS_WIN32
  sd_locker_win32_lock_acquire (state);
#else
  sd_locker_posix_lock_acquire (state);
#endif
  state->locked = 1;
  sd_locker_lock_announce (state, "acquired");
}

static void
sd_locker_lock_release (struct sd_locker_state_t *state)
{
  assert (state != NULL);
  sd_locker_lock_announce (state, "releasing");
#if SD_LOCKER_OS_TYPE == SD_LOCKER_OS_WIN32
  sd_locker_win32_lock_release (state);
#else
  sd_locker_posix_lock_release (state);
#endif
  state->locked = 0;
  sd_locker_lock_announce (state, "released");
}

#ifdef SD_LOCKER_DEBUGGING
static void
sd_locker_dump_arguments
  (const struct sd_locker_state_t *state, int argc, char *argv[])
{
  int index;

  assert (state != NULL);
  assert (state->lock_file != NULL);

  for (index = 0; index < argc; ++index)
    (void) fprintf (stderr, "sd-locker: %d: %s - [%d] %s\n",
      state->id, state->lock_file, index, argv [index]);

  (void) fflush (stderr);
}
#else
static void
sd_locker_dump_arguments
  (const struct sd_locker_state_t *state, int argc, char *argv[])
{
  int index;

  assert (state != NULL);
  assert (state->lock_file != NULL);

  for (index = 0; index < argc; ++index)
    assert (argv [index] != NULL);
}
#endif

static void
sd_locker_execute
  (struct sd_locker_state_t *state, int argc, char *argv[])
{
  assert (state         != NULL);
  assert (state->locked == 1);
  assert (argc          > 0);
  assert (argv          != NULL);

  sd_locker_dump_arguments (state, argc, argv);

#if SD_LOCKER_OS_TYPE == SD_LOCKER_OS_WIN32
  sd_locker_win32_execute (state, argc, argv);
#else
  sd_locker_posix_execute (state, argc, argv);
#endif
}

static void
usage (void)
{
  (void) fprintf (stderr, "sd-locker: usage: id file command [arguments]\n");
  (void) fflush (stderr);
  exit (EXIT_FAILURE);
}

static struct sd_locker_state_t sd_locker_state;

int
main (int argc, char *argv[])
{
  int index;

  if (argc < 4) usage ();

  for (index = 0; index < argc; ++index)
    assert (argv [index] != NULL);

  sd_locker_state.id        = atoi (argv [1]);
  sd_locker_state.lock_file = argv [2];
  sd_locker_state.exit_code = EXIT_SUCCESS;

  argc -= 3;
  argv += 3;

  sd_locker_lock_file_open  (&sd_locker_state);
  sd_locker_lock_acquire    (&sd_locker_state);
  sd_locker_execute         (&sd_locker_state, argc, argv);
  sd_locker_lock_release    (&sd_locker_state);
  sd_locker_lock_file_close (&sd_locker_state);

  return sd_locker_state.exit_code;
}

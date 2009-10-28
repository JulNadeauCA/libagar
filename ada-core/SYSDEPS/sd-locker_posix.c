/*
 * POSIX version.
 */

#include <sys/types.h>
#include <sys/wait.h>

#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

struct sd_locker_posix_state_t {
  int lock_fd;
};

static struct sd_locker_posix_state_t posix_state;

/*
 * Acquire current error message.
 */

static const char *
sd_locker_posix_error_message (void)
  /*@globals errno @*/
{
  return strerror (errno);
}

/*
 * Lock handle, wait indefinitely to acquire lock.
 */

static void
sd_locker_posix_lock_acquire 
  (struct sd_locker_state_t *state)
  /*@globals errno, posix_state @*/
  /*@modifies errno, posix_state @*/
{
  struct flock fl;

  assert (state         != NULL);
  assert (state->locked == 0);

  memset (&fl, 0, sizeof (fl));

  fl.l_type   = (short) F_WRLCK;
  fl.l_whence = (short) SEEK_SET;
  fl.l_start  = 0;
  fl.l_len    = 0;
  fl.l_pid    = getpid();

  if (fcntl (posix_state.lock_fd, F_SETLKW, &fl) == -1)
    sd_locker_fatal (state, "fcntl");
}

/*
 * Unlock handle.
 */

static void
sd_locker_posix_lock_release
  (struct sd_locker_state_t *state)
  /*@globals errno, posix_state @*/
  /*@modifies errno, posix_state @*/
{
  struct flock fl;

  assert (state         != NULL);
  assert (state->locked == 1);

  memset (&fl, 0, sizeof (fl));

  fl.l_type   = (short) F_UNLCK;
  fl.l_whence = (short) SEEK_SET;
  fl.l_start  = 0;
  fl.l_len    = 0;
  fl.l_pid    = getpid();

  if (fcntl (posix_state.lock_fd, F_SETLKW, &fl) == -1)
    sd_locker_fatal (state, "fcntl");
}

/*
 * Create/open lock file.
 */

static void
sd_locker_posix_lock_file_open
  (struct sd_locker_state_t *state)
  /*@globals errno, posix_state @*/
  /*@modifies errno, posix_state @*/
{
  assert (state            != NULL);
  assert (state->lock_file != NULL);

  posix_state.lock_fd = open (state->lock_file, O_WRONLY | O_TRUNC | O_CREAT, 0600);
  if (posix_state.lock_fd == -1)
    sd_locker_fatal (state, "open");
}

/*
 * Close lock file.
 */

static void
sd_locker_posix_lock_file_close
  (struct sd_locker_state_t *state)
  /*@globals errno, posix_state @*/
  /*@modifies errno, posix_state @*/
{
  assert (state               != NULL);
  assert (state->lock_file    != NULL);
  assert (posix_state.lock_fd != -1);

  if (close (posix_state.lock_fd) == -1)
    sd_locker_fatal (state, "close");
}

static void
sd_locker_posix_execute
  (struct sd_locker_state_t *state, int argc, char *argv[])
  /*@globals errno, posix_state @*/
  /*@modifies errno, posix_state @*/
{
  pid_t process_id;
  int status;

  assert (state != NULL);
  assert (argc > 0);
  assert (argv != NULL);

  process_id = fork ();
  if (process_id == (pid_t) -1)
    sd_locker_fatal (state, "fork");

  if (process_id == 0) {
    if (close (posix_state.lock_fd) == -1)
      sd_locker_fatal (state, "close");
    if (execvp (*argv, argv) == -1)
      sd_locker_fatal (state, "execvp");
  } else {
    if (waitpid (process_id, &status, 0) == (pid_t) -1)
      sd_locker_fatal (state, "waitpid");
    state->exit_code = WEXITSTATUS (status);
  }
}


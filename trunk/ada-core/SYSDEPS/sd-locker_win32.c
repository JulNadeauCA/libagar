/*
 * Win32 version.
 */

/* Require >= Windows 2000 */
#ifndef WINVER
#  define WINVER 0x0500
#endif

#include <windows.h>

struct sd_locker_win32_state_t {
  OVERLAPPED lock_overlap;
  HANDLE     lock_handle;
};

static struct sd_locker_win32_state_t win32_state;

/*
 * Acquire current error message.
 */

static const char *
sd_locker_win32_error_message (void)
{
  char *buffer = NULL;
  DWORD error_code = GetLastError ();

  FormatMessage
   (FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
    NULL,
    error_code,
    0,
    (LPTSTR) &buffer,
    0,
    0);

  assert (buffer != NULL);
  return buffer;
}

/*
 * Lock handle, wait indefinitely to acquire lock.
 */

static void
sd_locker_win32_lock_acquire 
  (struct sd_locker_state_t *state)
{
  BOOL lock_result;

  assert (state         != NULL);
  assert (state->locked == 0);

  win32_state.lock_overlap.hEvent = win32_state.lock_handle;

  lock_result = LockFileEx
   (win32_state.lock_handle,
    LOCKFILE_EXCLUSIVE_LOCK,    /* Exclusive lock, wait for lock to succeed. */
    0,                          /* Reserved. */
    0,                          /* Low 32 bits of range to lock. */
    0xffffffff,                 /* High 32 bits of range to lock. */
    &win32_state.lock_overlap);

  if (lock_result != TRUE) sd_locker_fatal (state, "lock_file_ex");
}

/*
 * Unlock handle.
 */

static void
sd_locker_win32_lock_release
  (struct sd_locker_state_t *state)
{
  BOOL unlock_result;

  assert (state         != NULL);
  assert (state->locked == 1);

  unlock_result = UnlockFileEx
   (win32_state.lock_handle,
    0,                          /* Reserved. */
    0,                          /* Low 32 bits of range to lock. */
    0xffffffff,                 /* High 32 bits of range to unlock. */
    &win32_state.lock_overlap);

  if (unlock_result != TRUE) sd_locker_fatal (state, "unlock_file_ex");
}

/*
 * Create/open lock file.
 */

static void
sd_locker_win32_lock_file_open
  (struct sd_locker_state_t *state)
{
  assert (state            != NULL);
  assert (state->lock_file != NULL);

  win32_state.lock_handle = CreateFile
    (state->lock_file,
     GENERIC_READ | GENERIC_WRITE,
     FILE_SHARE_READ | FILE_SHARE_WRITE,
     NULL,
     OPEN_ALWAYS,
     FILE_ATTRIBUTE_NORMAL,
     NULL);

  if (win32_state.lock_handle == INVALID_HANDLE_VALUE)
    sd_locker_fatal (state, "create_file");
}

/*
 * Close lock file.
 */

static void
sd_locker_win32_lock_file_close
  (struct sd_locker_state_t *state)
{
  assert (state            != NULL);
  assert (state->lock_file != NULL);
  assert (win32_state.lock_handle != NULL);

  if (CloseHandle (win32_state.lock_handle) == FALSE)
    sd_locker_fatal (state, "close_handle");
}

#define SD_LOCKER_WIN32_POSIX_SHELL_PREFIX      "sh -c \""
#define SD_LOCKER_WIN32_POSIX_SHELL_PREFIX_SIZE (sizeof (SD_LOCKER_WIN32_POSIX_SHELL_PREFIX) - 1)
#define SD_LOCKER_WIN32_POSIX_SHELL_QUOTE       "'"
#define SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE  (sizeof (SD_LOCKER_WIN32_POSIX_SHELL_QUOTE) - 1)

static char *
sd_locker_win32_convert_command
  (int argc, char *argv[])
{
  size_t length_total;
  size_t length_param;
  size_t length_used;
  char *buffer;
  char *ptr;
  int index;

  /* Calculate required command line length. */
  length_used  = 0;
  length_total = SD_LOCKER_WIN32_POSIX_SHELL_PREFIX_SIZE + SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE;

  /* Storage required for each parameter includes two quotes and a space */
  for (index = 0; index < argc; ++index)
    length_total += SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE
      + strlen (argv [index])
      + SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE
      + sizeof (' ');

  /* Allocate space for command line. */
  assert (length_total > SD_LOCKER_WIN32_POSIX_SHELL_PREFIX_SIZE);
  buffer = malloc (length_total);
  if (buffer == NULL) return NULL;

  /* Copy POSIX shell prefix to command line. */
  memcpy (buffer, SD_LOCKER_WIN32_POSIX_SHELL_PREFIX, SD_LOCKER_WIN32_POSIX_SHELL_PREFIX_SIZE);
  ptr         = buffer + SD_LOCKER_WIN32_POSIX_SHELL_PREFIX_SIZE;
  length_used = SD_LOCKER_WIN32_POSIX_SHELL_PREFIX_SIZE;

  /* Copy each string into buffer from argument vector. */
  for (index = 0; index < argc; ++index) {
    length_param = strlen (argv [index]);

    /* Open quote. */
    memcpy (ptr, SD_LOCKER_WIN32_POSIX_SHELL_QUOTE, SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE);
    ptr         += SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE;
    length_used += SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE;

    /* Copy argument. */
    memcpy (ptr, argv [index], length_param);
    ptr         += length_param;
    length_used += length_param;

    /* Close quote. */
    memcpy (ptr, SD_LOCKER_WIN32_POSIX_SHELL_QUOTE, SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE);
    ptr         += SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE;
    length_used += SD_LOCKER_WIN32_POSIX_SHELL_QUOTE_SIZE;

    /* Terminating space. */
    *ptr         = ' ';
    ptr         += 1;
    length_used += 1;
  }

  /* End quoting. */
  *ptr         = '"';
  ptr         += 1;
  length_used += 1;

  /* Null terminate. */
  *ptr         = 0;
  length_used += 1;

  /* Various buffer assertions. */
  assert (buffer [length_used - 1] == 0);
  assert (length_used <= length_total);

  return buffer;
}

static void
sd_locker_win32_execute
  (struct sd_locker_state_t *state, int argc, char *argv[])
{
  STARTUPINFO         info_startup;
  PROCESS_INFORMATION info_process;
  char *command;
  BOOL exec_result;
  DWORD wait_result;

  assert (state != NULL);
  assert (argc > 0);
  assert (argv != NULL);

  ZeroMemory (&info_startup, sizeof (info_startup));
  ZeroMemory (&info_process, sizeof (info_process));
  info_startup.cb = sizeof (info_startup);

  /* Transform command into flattened copy for win32 command line. */
  command = sd_locker_win32_convert_command (argc, argv);
  if (command == NULL) sd_locker_fatal (state, "malloc");

  /* Execute shell to execute command and arguments. */
  exec_result = CreateProcess
    (NULL,           /* Command name. NULL == Use command line. */
     command,        /* Command line. */
     NULL,           /* Process security attributes. */
     NULL,           /* Thread security attributes. */
     TRUE,           /* Inherit file handles. */
     0,              /* Creation flags. */
     NULL,           /* Environment. */
     NULL,           /* Current directory. */
     &info_startup,
     &info_process);

  if (exec_result == FALSE)
    sd_locker_fatal (state, "create_process");

  wait_result = WaitForSingleObject (info_process.hProcess, INFINITE);
  /* XXX: Do something with wait_result here? */

  if (GetExitCodeProcess (info_process.hProcess, (PDWORD) &state->exit_code) == FALSE)
    sd_locker_fatal (state, "exit_code");
  if (CloseHandle (info_process.hProcess) == FALSE)
    sd_locker_fatal (state, "close_process");
  if (CloseHandle (info_process.hThread) == FALSE)
    sd_locker_fatal (state, "close_thread");
}

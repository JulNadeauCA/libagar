#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "ctxt.h"
#include "install.h"

static const char progname[] = "instchk";

static void
cb_info (const char *str, /*@unused@*/ void *data)
  /*@globals stderr @*/
  /*@modifies stderr @*/
{
  (void) fprintf (stderr, "%s\n", str);
}

static void
cb_warn (const char *str, /*@unused@*/ void *data)
  /*@globals stderr, progname @*/
  /*@modifies stderr @*/
{
  (void) fprintf (stderr, "%s: warning: %s\n", progname, str);
}

int
main (void)
  /*@globals errno, insthier_len, progname, stderr, insthier, ctxt_fakeroot, install_failed @*/
  /*@modifies stderr @*/
{
  unsigned long index;
  struct install_status_t status;

  if (ctxt_fakeroot[0] != (char) 0) {
    (void) fprintf (stderr, "%s: info: using fake root - %s\n", progname, ctxt_fakeroot);
    install_fake_root (ctxt_fakeroot);
  }

  status = install_init ("conf-sosuffix");
  if (status.status != INSTALL_STATUS_OK) {
    assert (status.message != NULL);
    (void) fprintf (stderr, "%s: fatal: init: %s - %s\n", progname,
      status.message, install_error (errno));
    exit (EXIT_FAILURE);
  }

  install_callback_warn_set (cb_warn);
  install_callback_info_set (cb_info);

  for (index = 0; index < insthier_len; ++index) {
    status = install_check (&insthier[index]);
    switch (status.status) {
      case INSTALL_STATUS_OK:
        break;
      case INSTALL_STATUS_ERROR:
        assert (status.message != NULL);
        (void) fprintf (stderr, "%s: error: %s - %s\n", progname,
          status.message, install_error (errno));
        break;
      case INSTALL_STATUS_FATAL:
        assert (status.message != NULL);
        (void) fprintf (stderr, "%s: fatal: %s - %s\n", progname,
          status.message, install_error (errno));
        exit (EXIT_FAILURE);
    }
  }

  if (install_failed != 0) {
    (void) fprintf (stderr, "%s: %lu of %lu files failed\n", progname,
      install_failed, insthier_len);
    return 1;
  }
  return 0;
}

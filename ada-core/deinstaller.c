#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "install.h"
#include "ctxt.h"

static const char progname[] = "deinstaller";

static void
cb_info (const char *str, /*@unused@*/ void *data)
  /*@globals stderr @*/
  /*@modifies stderr @*/
{
  (void) fprintf (stderr, "%s\n", str);
}

static void
cb_warn (const char *str, /*@unused@*/ void *data)
  /*@globals progname, stderr @*/
  /*@modifies stderr @*/
{
  (void) fprintf (stderr, "%s: warning: %s\n", progname, str);
}

int
main (int argc, char *argv[])
  /*@globals errno, insthier_len, progname, stderr, insthier, ctxt_fakeroot @*/
  /*@modifies stderr @*/
{
  unsigned long index;
  unsigned int flag;
  struct install_status_t status;

  argv = 0;

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

  flag = (argc > 1) ? INSTALL_DRYRUN : 0;
  for (index = insthier_len - 1;; --index) {
    status = deinstall (&insthier[index], flag);
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
    if (index == 0) break;
  }

  return 0;
}

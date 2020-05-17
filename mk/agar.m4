# Configure paths for Agar
# stolen from Sam Lantinga 9/21/99
# stolen from Manish Singh
# stolen back from Frank Belew
# stolen from Manish Singh
# Shamelessly stolen from Owen Taylor

dnl AM_PATH_AGAR([MINIMUM-VERSION, [ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]]])
dnl Test for Agar, and define AGAR_CFLAGS and AGAR_LIBS
dnl
AC_DEFUN([AM_PATH_AGAR],
[dnl 
dnl Get the cflags and libraries from the agar-config script
dnl
AC_ARG_WITH(agar-prefix,[  --with-agar-prefix=PFX   Prefix where Agar is installed (optional)],
            agar_prefix="$withval", agar_prefix="")
AC_ARG_WITH(agar-exec-prefix,[  --with-agar-exec-prefix=PFX Exec prefix where Agar is installed (optional)],
            agar_exec_prefix="$withval", agar_exec_prefix="")
AC_ARG_ENABLE(agartest, [  --disable-agartest       Do not try to compile and run a test Agar program],
		    , enable_agartest=yes)

  if test x$agar_exec_prefix != x ; then
    agar_config_args="$agar_config_args --exec-prefix=$agar_exec_prefix"
    if test x${AGAR_CONFIG+set} != xset ; then
      AGAR_CONFIG=$agar_exec_prefix/bin/agar-config
    fi
  fi
  if test x$agar_prefix != x ; then
    agar_config_args="$agar_config_args --prefix=$agar_prefix"
    if test x${AGAR_CONFIG+set} != xset ; then
      AGAR_CONFIG=$agar_prefix/bin/agar-config
    fi
  fi

  if test "x$prefix" != xNONE; then
    PATH="$prefix/bin:$prefix/usr/bin:$PATH"
  fi
  AC_PATH_PROG(AGAR_CONFIG, agar-config, no, [$PATH])
  min_agar_version=ifelse([$1], ,0.11.0,$1)
  AC_MSG_CHECKING(for Agar - version >= $min_agar_version)
  no_agar=""
  if test "$AGAR_CONFIG" = "no" ; then
    no_agar=yes
  else
    AGAR_CFLAGS=`$AGAR_CONFIG $agar_config_args --cflags`
    AGAR_LIBS=`$AGAR_CONFIG $agar_config_args --libs`

    agar_major_version=`$AGAR_CONFIG $agar_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\).*/\1/'`
    agar_minor_version=`$AGAR_CONFIG $agar_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\).*/\2/'`
    agar_micro_version=`$AGAR_CONFIG $agar_config_args --version | \
           sed 's/\([[0-9]]*\).\([[0-9]]*\).\([[0-9]]*\).*/\3/'`
    if test "x$enable_agartest" = "xyes" ; then
      ac_save_CFLAGS="$CFLAGS"
      ac_save_CXXFLAGS="$CXXFLAGS"
      ac_save_LIBS="$LIBS"
      CFLAGS="$CFLAGS $AGAR_CFLAGS"
      CXXFLAGS="$CXXFLAGS $AGAR_CFLAGS"
      LIBS="$LIBS $AGAR_LIBS"
dnl
dnl Now check if the installed Agar is sufficiently new. (Also sanity
dnl checks the results of agar-config to some extent
dnl
      rm -f conf.agartest
      AC_TRY_RUN([
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <agar/core.h>
#include <agar/gui.h>

char*
my_strdup (char *str)
{
  char *new_str;
  
  if (str)
    {
      new_str = (char *)malloc ((strlen (str) + 1) * sizeof(char));
      strcpy (new_str, str);
    }
  else
    new_str = NULL;
  
  return new_str;
}

int main (int argc, char *argv[])
{
  int major, minor, micro;
  char *tmp_version;

  /* This hangs on some systems (?)
  system ("touch conf.agartest");
  */
  { FILE *fp = fopen("conf.agartest", "a"); if ( fp ) fclose(fp); }

  /* HP/UX 9 (%@#!) writes to sscanf strings */
  tmp_version = my_strdup("$min_agar_version");
  if (sscanf(tmp_version, "%d.%d.%d", &major, &minor, &micro) != 3) {
     printf("%s, bad version string\n", "$min_agar_version");
     exit(1);
   }

   if (($agar_major_version > major) ||
      (($agar_major_version == major) && ($agar_minor_version > minor)) ||
      (($agar_major_version == major) && ($agar_minor_version == minor) && ($agar_micro_version >= micro)))
    {
      return 0;
    }
  else
    {
      printf("\n*** 'agar-config --version' returned %d.%d.%d, but the minimum version\n", $agar_major_version, $agar_minor_version, $agar_micro_version);
      printf("*** of Agar required is %d.%d.%d. If agar-config is correct, then it is\n", major, minor, micro);
      printf("*** best to upgrade to the required version.\n");
      printf("*** If agar-config was wrong, set the environment variable AGAR_CONFIG\n");
      printf("*** to point to the correct copy of agar-config, and remove the file\n");
      printf("*** config.cache before re-running configure\n");
      return 1;
    }
}

],, no_agar=yes,[echo $ac_n "cross compiling; assumed OK... $ac_c"])
       CFLAGS="$ac_save_CFLAGS"
       CXXFLAGS="$ac_save_CXXFLAGS"
       LIBS="$ac_save_LIBS"
     fi
  fi
  if test "x$no_agar" = x ; then
     AC_MSG_RESULT(yes)
     ifelse([$2], , :, [$2])     
  else
     AC_MSG_RESULT(no)
     if test "$AGAR_CONFIG" = "no" ; then
       echo "*** The agar-config script installed by Agar could not be found"
       echo "*** If Agar was installed in PREFIX, make sure PREFIX/bin is in"
       echo "*** your path, or set the AGAR_CONFIG environment variable to the"
       echo "*** full path to agar-config."
     else
       if test -f conf.agartest ; then
        :
       else
          echo "*** Could not run Agar test program, checking why..."
          CFLAGS="$CFLAGS $AGAR_CFLAGS"
          CXXFLAGS="$CXXFLAGS $AGAR_CFLAGS"
          LIBS="$LIBS $AGAR_LIBS"
          AC_TRY_LINK([
#include <stdio.h>
#include <agar/core.h>
#include <agar/gui.h>

int main(int argc, char *argv[])
{ return 0; }
#undef  main
#define main K_and_R_C_main
],      [ return 0; ],
        [ echo "*** The test program compiled, but did not run. This usually means"
          echo "*** that the run-time linker is not finding Agar or finding the wrong"
          echo "*** version of Agar. If it is not finding Agar, you'll need to set your"
          echo "*** LD_LIBRARY_PATH environment variable, or edit /etc/ld.so.conf to point"
          echo "*** to the installed location  Also, make sure you have run ldconfig if that"
          echo "*** is required on your system"
	  echo "***"
          echo "*** If you have an old version installed, it is best to remove it, although"
          echo "*** you may also be able to get things to work by modifying LD_LIBRARY_PATH"],
        [ echo "*** The test program failed to compile or link. See the file config.log for the"
          echo "*** exact error that occurred. This usually means Agar was incorrectly installed"
          echo "*** or that you have moved Agar since it was installed. In the latter case, you"
          echo "*** may want to edit the agar-config script: $AGAR_CONFIG" ])
          CFLAGS="$ac_save_CFLAGS"
          CXXFLAGS="$ac_save_CXXFLAGS"
          LIBS="$ac_save_LIBS"
       fi
     fi
     AGAR_CFLAGS=""
     AGAR_LIBS=""
     ifelse([$3], , :, [$3])
  fi
  AC_SUBST(AGAR_CFLAGS)
  AC_SUBST(AGAR_LIBS)
  rm -f conf.agartest
])

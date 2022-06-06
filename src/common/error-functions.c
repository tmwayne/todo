//
// -----------------------------------------------------------------------------
// error-functions.c
// -----------------------------------------------------------------------------
//
// Copyright (c) 2019 Michael Kerrisk
//
// This program is free software. You may use, modify, and redistribute it
// under the terms of the GNU Lesser General Public License as published
// by the Free Software Foundation, either version 3 or (at your option)
// any later version. This program is distributed without any warranty.
//

#include <stdio.h>   // vsnprintf, snprintf, fprintf, vfprintf
#include <stdlib.h>  // EXIT_FAILURE
#include <unistd.h>  // _exit
#include <string.h>  // strerror
#include <errno.h>   // errno
#include <stdarg.h>
#include <stdbool.h> // true, false
#include "error-functions.h"

static char *ename[] = {
  "", 
  "EPERM", "ENOENT", "ESRCH", "EINTR", "EIO", "ENXIO", "E2BIG", "ENOEXEC", 
  "EBADF", "ECHILD", "EAGAIN/EWOULDBLOCK", "ENOMEM", "EACCES", "EFAULT", 
  "ENOTBLK", "EBUSY", "EEXIST", "EXDEV", "ENODEV", "ENOTDIR", "EISDIR", 
  "EINVAL", "ENFILE", "EMFILE", "ENOTTY", "ETXTBSY", "EFBIG", "ENOSPC", 
  "ESPIPE", "EROFS", "EMLINK", "EPIPE", "EDOM", "ERANGE", "EDEADLK/EDEADLOCK", 
  "ENAMETOOLONG", "ENOLCK", "ENOSYS", "ENOTEMPTY", "ELOOP", "", "ENOMSG", 
  "EIDRM", "ECHRNG", "EL2NSYNC", "EL3HLT", "EL3RST", "ELNRNG", "EUNATCH", 
  "ENOCSI", "EL2HLT", "EBADE", "EBADR", "EXFULL", "ENOANO", "EBADRQC", 
  "EBADSLT", "", "EBFONT", "ENOSTR", "ENODATA", "ETIME", "ENOSR", "ENONET", 
  "ENOPKG", "EREMOTE", "ENOLINK", "EADV", "ESRMNT", "ECOMM", "EPROTO", 
  "EMULTIHOP", "EDOTDOT", "EBADMSG", "EOVERFLOW", "ENOTUNIQ", "EBADFD",  
  "EREMCHG", "ELIBACC", "ELIBBAD", "ELIBSCN", "ELIBMAX", "ELIBEXEC", "EILSEQ", 
  "ERESTART", "ESTRPIPE", "EUSERS", "ENOTSOCK", "EDESTADDRREQ", "EMSGSIZE", 
  "EPROTOTYPE", "ENOPROTOOPT", "EPROTONOSUPPORT", "ESOCKTNOSUPPORT", 
  "EOPNOTSUPP/ENOTSUP", "EPFNOSUPPORT", "EAFNOSUPPORT", "EADDRINUSE", 
  "EADDRNOTAVAIL", "ENETDOWN", "ENETUNREACH", "ENETRESET", "ECONNABORTED", 
  "ECONNRESET", "ENOBUFS", "EISCONN", "ENOTCONN", "ESHUTDOWN", "ETOOMANYREFS", 
  "ETIMEDOUT", "ECONNREFUSED", "EHOSTDOWN", "EHOSTUNREACH", "EALREADY", 
  "EINPROGRESS", "ESTALE", "EUCLEAN", "ENOTNAM", "ENAVAIL", "EISNAM", 
  "EREMOTEIO", "EDQUOT", "ENOMEDIUM", "EMEDIUMTYPE", "ECANCELED", "ENOKEY", 
  "EKEYEXPIRED", "EKEYREVOKED", "EKEYREJECTED", "EOWNERDEAD", "ENOTRECOVERABLE", 
  "ERFKILL", "EHWPOISON"
};

#define MAX_ENAME 132

#ifdef __GNUC__
__attribute__ ((__noreturn__))
#endif
static void
terminate(bool useExit3)
{
  char *s;

  // Dump core if EF_DUMPCORE environment variable is defined and
  // is a nonempty string; otherwise call exit(3) or _exit(2),
  // depending on the value of useExit3
  
  s = getenv("EF_DUMPCORE");

  if (s != NULL && *s != '\0')
    abort();
  else if (useExit3)
    exit(EXIT_FAILURE);
  else
    _exit(EXIT_FAILURE);
}

static void
outputError(bool useErr, int err, bool flushStdout,
  const char *format, va_list ap)
{
// TODO: double check that this is safe
#define BUF_SIZE 500
  char buf[BUF_SIZE], userMsg[BUF_SIZE], errText[BUF_SIZE];

  vsnprintf(userMsg, BUF_SIZE, format, ap);

  if (useErr)
    snprintf(errText, BUF_SIZE, " [%s %s]",
      (err > 0 && err < MAX_ENAME) ? 
      ename[err] : "Unknown error code", strerror(err));
  else
    snprintf(errText, BUF_SIZE, ":");

  snprintf(buf, BUF_SIZE, "Error%s %s\n", errText, userMsg);

  if (flushStdout) fflush(stdout); // flush any pending stdout

  fputs(buf, stderr);
  fflush(stderr);   // in case stderr is not line-buffered
}

void
errMsg(const char *format, ...)
{
  va_list argList;
  int savedErrno;

  savedErrno = errno; // in case we change it here

  va_start(argList, format);
  outputError(true, errno, true, format, argList);
  va_end(argList);

  errno = savedErrno;
}

void
sysErrExit(const char *format, ...)
{
  va_list argList;

  va_start(argList, format);
  outputError(true, errno, true, format, argList);
  va_end(argList);

  terminate(true);
}

void
sysErr_Exit(const char *format, ...)
{
  va_list argList;

  va_start(argList, format);
  outputError(true, errno, false, format, argList);
  va_end(argList);

  terminate(false);
}

void
sysErrExitEN(int errno, const char *format, ...)
{
  va_list argList;

  va_start(argList, format);
  outputError(true, errno, true, format, argList);
  va_end(argList);

  terminate(true);
}

void
errExit(const char *format, ...)
{
  va_list argList;

  va_start(argList, format);
  outputError(false, 0, true, format, argList);
  va_end(argList);

  terminate(true);
}

void
usageErr(const char *format, ...)
{
  va_list argList;

  fflush(stdout); // flush any pending stdout

  fprintf(stderr, "Usage: ");
  va_start(argList, format);
  vfprintf(stderr, format, argList);
  va_end(argList);

  fflush(stderr); // in case stderr is not line-buffered
  exit(EXIT_FAILURE);
}

void
cmdLineErr(const char *format, ...)
{
  va_list argList;

  fflush(stdout); // flush any pending stdout

  fprintf(stderr, "Command-line usage error: ");
  va_start(argList, format);
  vfprintf(stderr, format, argList);
  va_end(argList);

  fflush(stderr); // in case stderr is not line-buffered
  exit(EXIT_FAILURE);
}

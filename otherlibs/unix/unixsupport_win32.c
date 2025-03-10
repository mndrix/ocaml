/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*             Xavier Leroy, projet Cristal, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1996 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#define CAML_INTERNALS

#include <stddef.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/alloc.h>
#include <caml/memory.h>
#include <caml/fail.h>
#include <caml/custom.h>
#include <caml/platform.h>
#include <caml/osdeps.h>
#include "caml/unixsupport.h"
#include "cst2constr.h"
#include <errno.h>

/* Heap-allocation of Windows file handles */

static int handle_compare(value v1, value v2)
{
  HANDLE h1 = Handle_val(v1);
  HANDLE h2 = Handle_val(v2);
  return h1 == h2 ? 0 : h1 < h2 ? -1 : 1;
}

static intnat handle_hash(value v)
{
  return (intnat) Handle_val(v);
}

static struct custom_operations handle_ops = {
  "_handle",
  custom_finalize_default,
  handle_compare,
  handle_hash,
  custom_serialize_default,
  custom_deserialize_default,
  custom_compare_ext_default,
  custom_fixed_length_default
};

value caml_win32_alloc_handle(HANDLE h)
{
  value res =
    caml_alloc_custom(&handle_ops, sizeof(struct filedescr), 0, 1);
  Handle_val(res) = h;
  Descr_kind_val(res) = KIND_HANDLE;
  ((struct filedescr *) Data_custom_val(res))->crt_fd = NO_CRT_FD;
  Flags_fd_val(res) = FLAGS_FD_IS_BLOCKING;
  return res;
}

value caml_win32_alloc_socket(SOCKET s)
{
  value res =
    caml_alloc_custom(&handle_ops, sizeof(struct filedescr), 0, 1);
  Socket_val(res) = s;
  Descr_kind_val(res) = KIND_SOCKET;
  ((struct filedescr *) Data_custom_val(res))->crt_fd = NO_CRT_FD;
  Flags_fd_val(res) = FLAGS_FD_IS_BLOCKING;
  return res;
}

#if 0
/* PR#4750: this function is no longer used */
value win_alloc_handle_or_socket(HANDLE h)
{
  value res = win_alloc_handle(h);
  int opt;
  int optlen = sizeof(opt);
  if (getsockopt((SOCKET) h, SOL_SOCKET, SO_TYPE, (char *)&opt, &optlen) == 0)
    Descr_kind_val(res) = KIND_SOCKET;
  return res;
}
#endif

/* Mapping of Windows error codes to POSIX error codes */

void caml_win32_maperr(DWORD win32err)
{
  int posixerr = caml_posixerr_of_win32err(win32err);
  if (posixerr != 0) {
    errno = posixerr;
  } else {
    /* Not found: save original error code, negated so that we can
       recognize it in caml_unix_error_message */
    errno = -(int)win32err;
  }
}

/* Windows socket errors */
#undef EWOULDBLOCK
#define EWOULDBLOCK             -WSAEWOULDBLOCK
#undef EINPROGRESS
#define EINPROGRESS             -WSAEINPROGRESS
#undef EALREADY
#define EALREADY                -WSAEALREADY
#undef ENOTSOCK
#define ENOTSOCK                -WSAENOTSOCK
#undef EDESTADDRREQ
#define EDESTADDRREQ            -WSAEDESTADDRREQ
#undef EMSGSIZE
#define EMSGSIZE                -WSAEMSGSIZE
#undef EPROTOTYPE
#define EPROTOTYPE              -WSAEPROTOTYPE
#undef ENOPROTOOPT
#define ENOPROTOOPT             -WSAENOPROTOOPT
#undef EPROTONOSUPPORT
#define EPROTONOSUPPORT         -WSAEPROTONOSUPPORT
#undef ESOCKTNOSUPPORT
#define ESOCKTNOSUPPORT         -WSAESOCKTNOSUPPORT
#undef EOPNOTSUPP
#define EOPNOTSUPP              -WSAEOPNOTSUPP
#undef EPFNOSUPPORT
#define EPFNOSUPPORT            -WSAEPFNOSUPPORT
#undef EAFNOSUPPORT
#define EAFNOSUPPORT            -WSAEAFNOSUPPORT
#undef EADDRINUSE
#define EADDRINUSE              -WSAEADDRINUSE
#undef EADDRNOTAVAIL
#define EADDRNOTAVAIL           -WSAEADDRNOTAVAIL
#undef ENETDOWN
#define ENETDOWN                -WSAENETDOWN
#undef ENETUNREACH
#define ENETUNREACH             -WSAENETUNREACH
#undef ENETRESET
#define ENETRESET               -WSAENETRESET
#undef ECONNABORTED
#define ECONNABORTED            -WSAECONNABORTED
#undef ECONNRESET
#define ECONNRESET              -WSAECONNRESET
#undef ENOBUFS
#define ENOBUFS                 -WSAENOBUFS
#undef EISCONN
#define EISCONN                 -WSAEISCONN
#undef ENOTCONN
#define ENOTCONN                -WSAENOTCONN
#undef ESHUTDOWN
#define ESHUTDOWN               -WSAESHUTDOWN
#undef ETOOMANYREFS
#define ETOOMANYREFS            -WSAETOOMANYREFS
#undef ETIMEDOUT
#define ETIMEDOUT               -WSAETIMEDOUT
#undef ECONNREFUSED
#define ECONNREFUSED            -WSAECONNREFUSED
#undef ELOOP
#define ELOOP                   -WSAELOOP
#undef EHOSTDOWN
#define EHOSTDOWN               -WSAEHOSTDOWN
#undef EHOSTUNREACH
#define EHOSTUNREACH            -WSAEHOSTUNREACH
#undef EPROCLIM
#define EPROCLIM                -WSAEPROCLIM
#undef EUSERS
#define EUSERS                  -WSAEUSERS
#undef EDQUOT
#define EDQUOT                  -WSAEDQUOT
#undef ESTALE
#define ESTALE                  -WSAESTALE
#undef EREMOTE
#define EREMOTE                 -WSAEREMOTE

#undef EOVERFLOW
#define EOVERFLOW -ERROR_ARITHMETIC_OVERFLOW
#undef EACCESS
#define EACCESS EACCES

static const int error_table[] = {
  E2BIG, EACCESS, EAGAIN, EBADF, EBUSY, ECHILD, EDEADLK, EDOM,
  EEXIST, EFAULT, EFBIG, EINTR, EINVAL, EIO, EISDIR, EMFILE, EMLINK,
  ENAMETOOLONG, ENFILE, ENODEV, ENOENT, ENOEXEC, ENOLCK, ENOMEM, ENOSPC,
  ENOSYS, ENOTDIR, ENOTEMPTY, ENOTTY, ENXIO, EPERM, EPIPE, ERANGE,
  EROFS, ESPIPE, ESRCH, EXDEV, EWOULDBLOCK, EINPROGRESS, EALREADY,
  ENOTSOCK, EDESTADDRREQ, EMSGSIZE, EPROTOTYPE, ENOPROTOOPT,
  EPROTONOSUPPORT, ESOCKTNOSUPPORT, EOPNOTSUPP, EPFNOSUPPORT,
  EAFNOSUPPORT, EADDRINUSE, EADDRNOTAVAIL, ENETDOWN, ENETUNREACH,
  ENETRESET, ECONNABORTED, ECONNRESET, ENOBUFS, EISCONN, ENOTCONN,
  ESHUTDOWN, ETOOMANYREFS, ETIMEDOUT, ECONNREFUSED, EHOSTDOWN,
  EHOSTUNREACH, ELOOP, EOVERFLOW /*, EUNKNOWNERR */
};

value caml_unix_error_of_code (int errcode)
{
  int errconstr;
  value err;

  errconstr =
      caml_unix_cst_to_constr(errcode, error_table,
                         sizeof(error_table)/sizeof(int), -1);
  if (errconstr == Val_int(-1)) {
    err = caml_alloc_small(1, 0);
    Field(err, 0) = Val_int(errcode);
  } else {
    err = errconstr;
  }
  return err;
}

int caml_unix_code_of_unix_error (value error)
{
  if (Is_block(error)) {
    return Int_val(Field(error, 0));
  } else {
    return error_table[Int_val(error)];
  }
}

static const value * _Atomic caml_unix_error_exn = NULL;

void caml_unix_error(int errcode, const char *cmdname, value cmdarg)
{
  CAMLparam0();
  CAMLlocal3(name, err, arg);
  value res;
  const value * exn;

  exn = atomic_load_acquire(&caml_unix_error_exn);
  if (exn == NULL) {
    exn = caml_named_value("Unix.Unix_error");
    if (exn == NULL)
      caml_invalid_argument("Exception Unix.Unix_error not initialized,"
                            " please link unix.cma");
    atomic_store(&caml_unix_error_exn, exn);
  }
  arg = cmdarg == Nothing ? caml_copy_string("") : cmdarg;
  name = caml_copy_string(cmdname);
  err = caml_unix_error_of_code (errcode);
  res = caml_alloc_small(4, 0);
  Field(res, 0) = *exn;
  Field(res, 1) = err;
  Field(res, 2) = name;
  Field(res, 3) = arg;
  caml_raise(res);
  CAMLnoreturn;
}

void caml_uerror(const char * cmdname, value cmdarg)
{
  caml_unix_error(errno, cmdname, cmdarg);
}

void caml_unix_check_path(value path, const char * cmdname)
{
  if (! caml_string_is_c_safe(path)) caml_unix_error(ENOENT, cmdname, path);
}

int caml_unix_cloexec_default = 0;

int caml_unix_cloexec_p(value cloexec)
{
  if (Is_some(cloexec))
    return Bool_val(Some_val(cloexec));
  else
    return caml_unix_cloexec_default;
}

int caml_win32_set_inherit(HANDLE fd, BOOL inherit)
{
  /* According to the MSDN, SetHandleInformation may not work
     for console handles on WinNT4 and earlier versions. */
  if (! SetHandleInformation(fd,
                             HANDLE_FLAG_INHERIT,
                             inherit ? HANDLE_FLAG_INHERIT : 0)) {
    caml_win32_maperr(GetLastError());
    return -1;
  }
  return 0;
}

/**************************************************************************/
/*                                                                        */
/*                                 OCaml                                  */
/*                                                                        */
/*             Xavier Leroy, projet Cristal, INRIA Rocquencourt           */
/*                                                                        */
/*   Copyright 1997 Institut National de Recherche en Informatique et     */
/*     en Automatique.                                                    */
/*                                                                        */
/*   All rights reserved.  This file is distributed under the terms of    */
/*   the GNU Lesser General Public License version 2.1, with the          */
/*   special exception on linking described in the file LICENSE.          */
/*                                                                        */
/**************************************************************************/

#include <caml/fail.h>
#include <caml/mlvalues.h>
#include "caml/unixsupport.h"
#ifndef _WIN32
#include <unistd.h>
#endif

CAMLprim value caml_unix_setsid(value unit)
{
#ifdef HAS_SETSID
  pid_t pid = setsid();
  if (pid == (pid_t)(-1)) caml_uerror("setsid", Nothing);
  return Val_long(pid);
#else
  caml_invalid_argument("setsid not implemented");
  return Val_unit;
#endif
}

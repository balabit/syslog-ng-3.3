/*
 * Copyright (c) 2002-2011 BalaBit IT Ltd, Budapest, Hungary
 * Copyright (c) 1998-2011 Balázs Scheidler
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As an additional exemption you are allowed to compile & link against the
 * OpenSSL libraries as published by the OpenSSL project. See the file
 * COPYING for details.
 */

/* This file becomes part of libsyslog-ng-crypto.so, the shared object
 * that contains all crypto related stuff to be used by plugins. This
 * includes the TLS wrappers, random number initialization, and so on.
 */

#include "crypto.h"
#include "apphook.h"

#include <openssl/rand.h>
#include <openssl/ssl.h>

extern gboolean seed_rng; /* defined in main.c */

static void
crypto_deinit(void *c)
{
  char rnd_file[256];

  if (seed_rng)
    {
      RAND_file_name(rnd_file, sizeof(rnd_file));
      if (rnd_file[0])
        RAND_write_file(rnd_file);
    }
}

static void __attribute__((constructor))
crypto_init(void)
{
  char rnd_file[256];

  if (seed_rng)
    {
      RAND_file_name(rnd_file, sizeof(rnd_file));
      if (rnd_file[0])
        RAND_load_file(rnd_file, -1);
    }
  SSL_library_init();
  SSL_load_error_strings();
  OpenSSL_add_all_algorithms();
  register_application_hook(AH_SHUTDOWN, (ApplicationHookFunc) crypto_deinit, NULL);
}

/* the crypto options (seed) are handled in main.c */
/* Mcaster1DNAS - SSL Certificate Generator
 *
 * Cross-platform: Linux, macOS, Windows.
 * Compiled only when HAVE_OPENSSL is defined.
 *
 * Copyright 2026, Saint John (David St John) <davestj@gmail.com>
 * Distributed under the GNU General Public License, version 2.
 */

#ifndef __SSL_GEN_H__
#define __SSL_GEN_H__

#ifdef HAVE_OPENSSL

/* Parameters for SSL cert/CSR generation */
typedef struct ssl_gen_params_tag
{
    const char *gentype;        /* "selfsigned" or "csr"                          */
    const char *subj;           /* "/C=US/ST=CA/L=City/O=Org/OU=Unit/CN=hostname" */
    const char *savepath;       /* output directory (created if absent)            */
    int         key_bits;       /* RSA key size — 0 defaults to 2048              */
    int         days;           /* cert validity in days (selfsigned) — 0→365     */
    int         add_to_config;  /* 1 = also update the config file paths section  */
    const char *config_path;    /* YAML or XML config file to patch               */
} ssl_gen_params_t;

/*
 * ssl_gen_run() — generate keys and cert/CSR according to params.
 * Returns 0 on success, -1 on error.  Call ssl_gen_last_error() for details.
 */
int ssl_gen_run(const ssl_gen_params_t *p);

/*
 * ssl_gen_parse_args() — scan argv[] for --ssl-* flags and populate *p.
 * Returns 1 if --ssl-gencert was found (caller should run ssl_gen_run then exit),
 * 0 if not found (normal server startup).
 *
 * Recognised flags:
 *   --ssl-gencert
 *   --ssl-gentype=selfsigned|csr
 *   --subj="/C=US/..."
 *   --ssl-gencert-savepath=<dir>
 *   --ssl-gencert-addtoconfig=true|false
 *
 * The -c <file> flag is NOT consumed here — the caller reads it separately.
 */
int ssl_gen_parse_args(int argc, char **argv, ssl_gen_params_t *p);

/* Returns a human-readable description of the last error. */
const char *ssl_gen_last_error(void);

#endif /* HAVE_OPENSSL */

#endif /* __SSL_GEN_H__ */

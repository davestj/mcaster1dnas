/* Mcaster1DNAS - SSL Certificate Generator
 *
 * Cross-platform implementation (Linux, macOS, Windows).
 * Compiled only when HAVE_OPENSSL is defined.
 *
 * Generates:
 *   selfsigned: RSA private key + self-signed X.509 cert + combined PEM
 *   csr:        RSA private key + Certificate Signing Request
 *
 * Copyright 2026, Saint John (David St John) <davestj@gmail.com>
 * Distributed under the GNU General Public License, version 2.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_OPENSSL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#ifdef _WIN32
#  include <direct.h>   /* _mkdir */
#  include <io.h>       /* _access */
#  define  PATH_SEP '\\'
#  define  mkdir_compat(p)  _mkdir(p)
#  define  access_compat(p) _access((p), 0)
#else
#  include <sys/stat.h> /* mkdir */
#  include <unistd.h>   /* access */
#  define  PATH_SEP '/'
#  define  mkdir_compat(p)  mkdir((p), 0755)
#  define  access_compat(p) access((p), F_OK)
#endif

#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/rand.h>
#include <openssl/x509.h>
#include <openssl/x509v3.h>
#include <openssl/err.h>
#include <openssl/bn.h>

#ifndef _WIN32
#include <fcntl.h>
#endif

#include "ssl_gen.h"

/* ---- internal state ---- */
static char s_last_error[512] = "No error";

static void set_error(const char *msg)
{
    strncpy(s_last_error, msg, sizeof(s_last_error) - 1);
    s_last_error[sizeof(s_last_error) - 1] = '\0';
}

static void set_error_ssl(const char *prefix)
{
    char buf[256] = "";
    unsigned long e = ERR_get_error();
    if (e)
        ERR_error_string_n(e, buf, sizeof(buf));
    else
        strncpy(buf, "unknown OpenSSL error", sizeof(buf) - 1);

    snprintf(s_last_error, sizeof(s_last_error), "%s: %s", prefix, buf);
}

const char *ssl_gen_last_error(void)
{
    return s_last_error;
}

/* ---- helpers ---- */

/* Recursive mkdir -p: create path and all intermediate directories. */
static int make_directory(const char *path)
{
    char tmp[1024];
    char *p;
    size_t len;

    if (access_compat(path) == 0)
        return 0;   /* already exists */

    strncpy(tmp, path, sizeof(tmp) - 1);
    tmp[sizeof(tmp) - 1] = '\0';
    len = strlen(tmp);

    /* Strip trailing separators */
    while (len > 1 && (tmp[len-1] == '/' || tmp[len-1] == '\\'))
        tmp[--len] = '\0';

    /* Walk each component and mkdir as needed */
    for (p = tmp + 1; *p; p++)
    {
        if (*p == '/' || *p == '\\')
        {
            *p = '\0';
            if (access_compat(tmp) != 0)
                mkdir_compat(tmp);   /* ignore error; final mkdir will catch it */
            *p = PATH_SEP;
        }
    }

    if (mkdir_compat(tmp) != 0 && access_compat(tmp) != 0)
    {
        snprintf(s_last_error, sizeof(s_last_error),
                 "mkdir(%s) failed: %s", path, strerror(errno));
        return -1;
    }
    return 0;
}

/* Build a full path: dir + sep + filename */
static void build_path(char *out, size_t sz, const char *dir, const char *file)
{
    size_t dlen = strlen(dir);
    if (dlen > 0 && (dir[dlen-1] == '/' || dir[dlen-1] == '\\'))
        snprintf(out, sz, "%s%s", dir, file);
    else
        snprintf(out, sz, "%s%c%s", dir, PATH_SEP, file);
}

/*
 * Parse an OpenSSL-style subject string "/C=US/ST=CA/O=Org/CN=host"
 * into an X509_NAME.  Tokens are separated by '/'; each is "KEY=value".
 */
static int parse_subject_into_name(const char *subj, X509_NAME *name)
{
    if (!subj || !name) return -1;

    char buf[1024];
    strncpy(buf, subj, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char *p = buf;
    /* skip leading slash(es) */
    while (*p == '/') p++;

    while (*p)
    {
        /* find end of this token */
        char *slash = strchr(p, '/');
        if (slash) *slash = '\0';

        char *eq = strchr(p, '=');
        if (eq)
        {
            *eq = '\0';
            const char *key = p;
            const char *val = eq + 1;
            if (*key && *val)
            {
                if (X509_NAME_add_entry_by_txt(name, key, MBSTRING_ASC,
                                               (const unsigned char *)val,
                                               -1, -1, 0) != 1)
                {
                    set_error_ssl("X509_NAME_add_entry_by_txt");
                    return -1;
                }
            }
        }

        if (!slash) break;
        p = slash + 1;
    }
    return 0;
}

/* Generate an RSA private key */
static EVP_PKEY *generate_rsa_key(int bits)
{
    EVP_PKEY *pkey = EVP_RSA_gen((unsigned int)bits);
    if (!pkey)
        set_error_ssl("EVP_RSA_gen");
    return pkey;
}

/* Open a file for writing with restricted permissions (0600) for sensitive data */
static FILE *fopen_secure(const char *path)
{
#ifndef _WIN32
    int fd = open(path, O_WRONLY | O_CREAT | O_TRUNC, 0600);
    if (fd < 0) return NULL;
    return fdopen(fd, "wb");
#else
    return fopen(path, "wb");
#endif
}

/* Write PEM private key to file */
static int write_pem_key(EVP_PKEY *pkey, const char *path)
{
    FILE *f = fopen_secure(path);
    if (!f)
    {
        snprintf(s_last_error, sizeof(s_last_error),
                 "Cannot open %s for writing: %s", path, strerror(errno));
        return -1;
    }
    int ok = PEM_write_PrivateKey(f, pkey, NULL, NULL, 0, NULL, NULL);
    fclose(f);
    if (!ok) { set_error_ssl("PEM_write_PrivateKey"); return -1; }
    return 0;
}

/* Concatenate cert_file and key_file into combined_file (combined PEM) */
static int write_combined_pem(const char *cert_path, const char *key_path,
                               const char *combined_path)
{
    FILE *out = fopen_secure(combined_path);
    if (!out)
    {
        snprintf(s_last_error, sizeof(s_last_error),
                 "Cannot open %s for writing: %s", combined_path, strerror(errno));
        return -1;
    }

    const char *sources[2] = { cert_path, key_path };
    int i;
    for (i = 0; i < 2; i++)
    {
        FILE *in = fopen(sources[i], "rb");
        if (!in) { fclose(out); return -1; }
        char tmp[4096];
        size_t n;
        while ((n = fread(tmp, 1, sizeof(tmp), in)) > 0)
            fwrite(tmp, 1, n, out);
        fclose(in);
    }
    fclose(out);
    return 0;
}

/* ---- config patching ---- */

/*
 * Patch a YAML config file: replace or append ssl-certificate / ssl-private-key
 * under the paths: section.  Uses simple line-by-line text substitution.
 */
static int patch_yaml_config(const char *config_path,
                              const char *cert_path, const char *key_path)
{
    FILE *f = fopen(config_path, "rb");
    if (!f)
    {
        snprintf(s_last_error, sizeof(s_last_error),
                 "Cannot open config %s: %s", config_path, strerror(errno));
        return -1;
    }

    /* Read entire file */
    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = (char *)malloc((size_t)fsize + 1);
    if (!content) { fclose(f); set_error("out of memory"); return -1; }
    size_t nread = fread(content, 1, (size_t)fsize, f);
    fclose(f);
    content[nread] = '\0';

    /* Calculate safe output buffer: original + generous space for new paths */
    size_t extra = strlen(cert_path) + strlen(key_path) + 4096;
    size_t out_alloc = nread + extra;
    char *out = (char *)malloc(out_alloc);
    if (!out) { free(content); set_error("out of memory"); return -1; }
    size_t out_pos = 0;

    int found_cert = 0, found_key = 0;
    char *line = content;
    char *next;

    while (*line)
    {
        /* find end of line */
        next = strchr(line, '\n');
        if (next) *(next) = '\0';

        /* check for ssl-certificate: or ssl-private-key: */
        char *t = line;
        while (*t == ' ' || *t == '\t') t++;

        char newline[512];
        if (strncmp(t, "ssl-certificate:", 16) == 0)
        {
            int indent = (int)(t - line);
            snprintf(newline, sizeof(newline), "%*ssl-certificate: \"%s\"",
                     indent, "", cert_path);
            size_t nl = strlen(newline);
            if (out_pos + nl + 2 < out_alloc) { memcpy(out + out_pos, newline, nl); out_pos += nl; }
            found_cert = 1;
        }
        else if (strncmp(t, "ssl-private-key:", 16) == 0)
        {
            int indent = (int)(t - line);
            snprintf(newline, sizeof(newline), "%*ssl-private-key: \"%s\"",
                     indent, "", key_path);
            size_t nl = strlen(newline);
            if (out_pos + nl + 2 < out_alloc) { memcpy(out + out_pos, newline, nl); out_pos += nl; }
            found_key = 1;
        }
        else
        {
            size_t ll = strlen(line);
            if (out_pos + ll + 2 < out_alloc) { memcpy(out + out_pos, line, ll); out_pos += ll; }
        }

        if (out_pos + 1 < out_alloc) out[out_pos++] = '\n';

        if (!next) break;
        line = next + 1;
    }

    /* If lines were not found, append after paths: section (best-effort) */
    if (!found_cert)
    {
        char append[512];
        int r = snprintf(append, sizeof(append), "  ssl-certificate: \"%s\"\n", cert_path);
        if (r > 0 && out_pos + (size_t)r < out_alloc) { memcpy(out + out_pos, append, r); out_pos += r; }
    }
    if (!found_key && strcmp(cert_path, key_path) != 0)
    {
        char append[512];
        int r = snprintf(append, sizeof(append), "  ssl-private-key: \"%s\"\n", key_path);
        if (r > 0 && out_pos + (size_t)r < out_alloc) { memcpy(out + out_pos, append, r); out_pos += r; }
    }

    out[out_pos] = '\0';
    free(content);

    /* Write back */
    f = fopen(config_path, "wb");
    if (!f)
    {
        free(out);
        snprintf(s_last_error, sizeof(s_last_error),
                 "Cannot write config %s: %s", config_path, strerror(errno));
        return -1;
    }
    fputs(out, f);
    fclose(f);
    free(out);
    return 0;
}

/*
 * Patch an XML config file: replace ssl-certificate and ssl-private-key element values.
 */
static int patch_xml_config(const char *config_path,
                             const char *cert_path, const char *key_path)
{
    FILE *f = fopen(config_path, "rb");
    if (!f)
    {
        snprintf(s_last_error, sizeof(s_last_error),
                 "Cannot open config %s: %s", config_path, strerror(errno));
        return -1;
    }

    fseek(f, 0, SEEK_END);
    long fsize = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *content = (char *)malloc((size_t)fsize + 1);
    if (!content) { fclose(f); set_error("out of memory"); return -1; }
    size_t nread = fread(content, 1, (size_t)fsize, f);
    fclose(f);
    content[nread] = '\0';

    /* Safe output buffer with space for added paths */
    size_t extra = strlen(cert_path) + strlen(key_path) + 4096;
    size_t out_alloc = nread + extra;
    char *out = (char *)malloc(out_alloc);
    if (!out) { free(content); set_error("out of memory"); return -1; }
    size_t out_pos = 0;

    char *p = content;
    int found_cert = 0, found_key = 0;

    while (*p)
    {
        /* Look for <ssl-certificate> or <ssl-private-key> tags */
        char *tag_cert = strstr(p, "<ssl-certificate>");
        char *tag_key  = strstr(p, "<ssl-private-key>");
        char *earliest = NULL;
        int is_cert = 0;

        if (tag_cert && (!tag_key || tag_cert < tag_key)) { earliest = tag_cert; is_cert = 1; }
        else if (tag_key)                                  { earliest = tag_key;  is_cert = 0; }

        if (!earliest)
        {
            size_t rem = strlen(p);
            if (out_pos + rem < out_alloc) { memcpy(out + out_pos, p, rem); out_pos += rem; }
            break;
        }

        /* copy up to the tag */
        size_t pre = (size_t)(earliest - p);
        if (out_pos + pre < out_alloc) { memcpy(out + out_pos, p, pre); out_pos += pre; }

        const char *open_tag  = is_cert ? "<ssl-certificate>" : "<ssl-private-key>";
        const char *close_tag = is_cert ? "</ssl-certificate>" : "</ssl-private-key>";
        const char *newval    = is_cert ? cert_path : key_path;

        char newelem[1024];
        int ne = snprintf(newelem, sizeof(newelem), "%s%s%s", open_tag, newval, close_tag);
        if (ne > 0 && out_pos + (size_t)ne < out_alloc) { memcpy(out + out_pos, newelem, ne); out_pos += ne; }

        if (is_cert) found_cert = 1;
        else         found_key  = 1;

        /* skip past old closing tag */
        char *close = strstr(earliest + strlen(open_tag), close_tag);
        if (close)
            p = close + strlen(close_tag);
        else
            p = earliest + strlen(open_tag);
    }

    out[out_pos] = '\0';

    /* Append missing tags before </paths> (best-effort) */
    if (!found_cert || !found_key)
    {
        char *paths_close = strstr(out, "</paths>");
        if (paths_close)
        {
            char insert[1024] = "";
            size_t ins_pos = 0;
            if (!found_cert)
            {
                int r = snprintf(insert + ins_pos, sizeof(insert) - ins_pos,
                    "    <ssl-certificate>%s</ssl-certificate>\n", cert_path);
                if (r > 0) ins_pos += r;
            }
            if (!found_key && strcmp(cert_path, key_path) != 0)
            {
                int r = snprintf(insert + ins_pos, sizeof(insert) - ins_pos,
                    "    <ssl-private-key>%s</ssl-private-key>\n", key_path);
                if (r > 0) ins_pos += r;
            }
            size_t tail_len = strlen(paths_close);
            if (out_pos + ins_pos + tail_len < out_alloc)
            {
                memmove(paths_close + ins_pos, paths_close, tail_len + 1);
                memcpy(paths_close, insert, ins_pos);
                out_pos += ins_pos;
            }
        }
    }

    free(content);

    f = fopen(config_path, "wb");
    if (!f)
    {
        free(out);
        snprintf(s_last_error, sizeof(s_last_error),
                 "Cannot write config %s: %s", config_path, strerror(errno));
        return -1;
    }
    fputs(out, f);
    fclose(f);
    free(out);
    return 0;
}

static int patch_config(const char *config_path,
                         const char *cert_path, const char *key_path)
{
    /* Auto-detect YAML vs XML by extension */
    const char *ext = strrchr(config_path, '.');
    if (ext && (strcmp(ext, ".yaml") == 0 || strcmp(ext, ".yml") == 0))
        return patch_yaml_config(config_path, cert_path, key_path);
    else
        return patch_xml_config(config_path, cert_path, key_path);
}

/* ================================================================
 * Public API
 * ================================================================ */

static int generate_selfsigned(const ssl_gen_params_t *p,
                                int key_bits, int days)
{
    char key_path[1024], cert_path[1024], pem_path[1024];
    build_path(key_path,  sizeof(key_path),  p->savepath, "selfsigned.key");
    build_path(cert_path, sizeof(cert_path), p->savepath, "selfsigned.crt");
    build_path(pem_path,  sizeof(pem_path),  p->savepath, "selfsigned.pem");

    /* Generate key */
    EVP_PKEY *pkey = generate_rsa_key(key_bits);
    if (!pkey) return -1;

    /* Create certificate */
    X509 *cert = X509_new();
    if (!cert)
    {
        set_error_ssl("X509_new");
        EVP_PKEY_free(pkey);
        return -1;
    }

    X509_set_version(cert, 2);  /* v3 */
    /* Use a random 128-bit serial number per RFC 5280 (CWE-330) */
    {
        BIGNUM *bn_serial = BN_new();
        if (bn_serial) {
            BN_rand(bn_serial, 128, BN_RAND_TOP_ANY, BN_RAND_BOTTOM_ANY);
            BN_to_ASN1_INTEGER(bn_serial, X509_get_serialNumber(cert));
            BN_free(bn_serial);
        }
    }
    X509_gmtime_adj(X509_getm_notBefore(cert), 0);
    X509_gmtime_adj(X509_getm_notAfter(cert), (long)60 * 60 * 24 * days);
    X509_set_pubkey(cert, pkey);

    X509_NAME *name = X509_get_subject_name(cert);
    if (parse_subject_into_name(p->subj, name) < 0)
    {
        X509_free(cert); EVP_PKEY_free(pkey); return -1;
    }
    X509_set_issuer_name(cert, name);   /* self-signed: issuer == subject */

    if (X509_sign(cert, pkey, EVP_sha256()) == 0)
    {
        set_error_ssl("X509_sign");
        X509_free(cert); EVP_PKEY_free(pkey); return -1;
    }

    /* Write files */
    int rc = 0;
    if (write_pem_key(pkey, key_path) < 0) rc = -1;

    if (rc == 0)
    {
        FILE *f = fopen_secure(cert_path);
        if (!f)
        {
            snprintf(s_last_error, sizeof(s_last_error),
                     "Cannot open %s for writing: %s", cert_path, strerror(errno));
            rc = -1;
        }
        else
        {
            if (!PEM_write_X509(f, cert))
            {
                set_error_ssl("PEM_write_X509");
                rc = -1;
            }
            fclose(f);
        }
    }

    if (rc == 0)
    {
        rc = write_combined_pem(cert_path, key_path, pem_path);
        if (rc == 0)
            printf("ssl-gen: Created %s, %s, %s\n", key_path, cert_path, pem_path);
    }

    X509_free(cert);
    EVP_PKEY_free(pkey);

    if (rc == 0 && p->add_to_config && p->config_path && p->config_path[0])
        rc = patch_config(p->config_path, pem_path, pem_path);

    return rc;
}

static int generate_csr(const ssl_gen_params_t *p, int key_bits)
{
    char key_path[1024], csr_path[1024];
    build_path(key_path, sizeof(key_path), p->savepath, "server.key");
    build_path(csr_path, sizeof(csr_path), p->savepath, "server.csr");

    EVP_PKEY *pkey = generate_rsa_key(key_bits);
    if (!pkey) return -1;

    X509_REQ *req = X509_REQ_new();
    if (!req)
    {
        set_error_ssl("X509_REQ_new");
        EVP_PKEY_free(pkey);
        return -1;
    }

    X509_REQ_set_version(req, 1);
    X509_REQ_set_pubkey(req, pkey);

    X509_NAME *name = X509_REQ_get_subject_name(req);
    if (parse_subject_into_name(p->subj, name) < 0)
    {
        X509_REQ_free(req); EVP_PKEY_free(pkey); return -1;
    }

    if (X509_REQ_sign(req, pkey, EVP_sha256()) == 0)
    {
        set_error_ssl("X509_REQ_sign");
        X509_REQ_free(req); EVP_PKEY_free(pkey); return -1;
    }

    int rc = 0;
    if (write_pem_key(pkey, key_path) < 0) rc = -1;

    if (rc == 0)
    {
        FILE *f = fopen_secure(csr_path);
        if (!f)
        {
            snprintf(s_last_error, sizeof(s_last_error),
                     "Cannot open %s for writing: %s", csr_path, strerror(errno));
            rc = -1;
        }
        else
        {
            if (!PEM_write_X509_REQ(f, req))
            {
                set_error_ssl("PEM_write_X509_REQ");
                rc = -1;
            }
            fclose(f);
        }
    }

    if (rc == 0)
        printf("ssl-gen: Created %s, %s\n", key_path, csr_path);

    X509_REQ_free(req);
    EVP_PKEY_free(pkey);

    if (rc == 0 && p->add_to_config && p->config_path && p->config_path[0])
    {
        /* For CSR, we patch with the key path; cert will be supplied by the CA */
        rc = patch_config(p->config_path, key_path, key_path);
        printf("ssl-gen: Note — CSR must be signed by a CA before using as certificate.\n");
    }

    return rc;
}

int ssl_gen_run(const ssl_gen_params_t *p)
{
    if (!p || !p->savepath || !p->savepath[0])
    {
        set_error("ssl_gen_run: savepath is required");
        return -1;
    }

    int key_bits = (p->key_bits > 0) ? p->key_bits : 2048;
    int days     = (p->days > 0)     ? p->days     : 365;
    const char *gentype = p->gentype ? p->gentype : "selfsigned";
    const char *subj    = p->subj    ? p->subj    : "/CN=localhost";

    ssl_gen_params_t local = *p;
    local.key_bits = key_bits;
    local.days     = days;
    local.gentype  = gentype;
    local.subj     = subj;

    if (make_directory(p->savepath) < 0)
        return -1;

    if (strcmp(gentype, "csr") == 0)
        return generate_csr(&local, key_bits);
    else
        return generate_selfsigned(&local, key_bits, days);
}

int ssl_gen_parse_args(int argc, char **argv, ssl_gen_params_t *p)
{
    int found = 0;
    int i;

    if (!p) return 0;
    memset(p, 0, sizeof(*p));
    p->key_bits = 2048;
    p->days     = 365;

    for (i = 1; i < argc; i++)
    {
        const char *a = argv[i];
        if (strcmp(a, "--ssl-gencert") == 0)
        {
            found = 1;
        }
        else if (strncmp(a, "--ssl-gentype=", 14) == 0)
        {
            p->gentype = a + 14;
        }
        else if (strncmp(a, "--subj=", 7) == 0)
        {
            p->subj = a + 7;
        }
        else if (strncmp(a, "--ssl-gencert-savepath=", 23) == 0)
        {
            p->savepath = a + 23;
        }
        else if (strncmp(a, "--ssl-gencert-addtoconfig=", 26) == 0)
        {
            p->add_to_config = (strcmp(a + 26, "true") == 0) ? 1 : 0;
        }
        else if (strcmp(a, "-c") == 0 && i + 1 < argc)
        {
            p->config_path = argv[i + 1];
        }
    }

    return found;
}

#endif /* HAVE_OPENSSL */

// SslGen.cpp — Windows C++ wrapper around the cross-platform ssl_gen C module.
//
// The actual OpenSSL work lives in ../src/ssl_gen.c (compiled into each vcxproj).
// This file provides:
//   - CSslGen::Run()  / CSslGen::GetLastError()   for the MFC WinUI
//   - win_ssl_gencert()                            called from main.c on Windows

#include "stdafx.h"
#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "SslGen.h"

#ifdef HAVE_OPENSSL
extern "C" {
#include "../src/ssl_gen.h"
}
#endif

// ---------------------------------------------------------------------------
// CSslGen
// ---------------------------------------------------------------------------

int CSslGen::Run(const SslGenParams& p)
{
#ifdef HAVE_OPENSSL
    ssl_gen_params_t params = {};
    params.gentype       = p.gentype.empty()     ? "selfsigned"      : p.gentype.c_str();
    params.subj          = p.subj.empty()         ? "/CN=localhost"   : p.subj.c_str();
    params.savepath      = p.savepath.empty()     ? "."               : p.savepath.c_str();
    params.config_path   = p.configPath.empty()   ? nullptr           : p.configPath.c_str();
    params.key_bits      = p.key_bits;
    params.days          = p.days;
    params.add_to_config = p.addToConfig ? 1 : 0;

    return ssl_gen_run(&params);
#else
    (void)p;
    return -1;  // OpenSSL not compiled in
#endif
}

std::string CSslGen::GetLastError()
{
#ifdef HAVE_OPENSSL
    return ssl_gen_last_error();
#else
    return "OpenSSL not compiled in";
#endif
}

// ---------------------------------------------------------------------------
// win_ssl_gencert — C entry point called from main.c
// ---------------------------------------------------------------------------

extern "C" int win_ssl_gencert(int argc, char **argv)
{
#ifdef HAVE_OPENSSL
    ssl_gen_params_t params = {};
    int found = ssl_gen_parse_args(argc, argv, &params);

    if (!found)
        return 0;   // --ssl-gencert not present — no-op

    if (!params.savepath || !params.savepath[0])
    {
        fprintf(stderr, "ssl-gen: --ssl-gencert-savepath=<dir> is required\n");
        return 1;
    }

    printf("ssl-gen: type=%s  subj=%s  savepath=%s\n",
           params.gentype ? params.gentype : "selfsigned",
           params.subj    ? params.subj    : "/CN=localhost",
           params.savepath);

    int rc = ssl_gen_run(&params);
    if (rc != 0)
        fprintf(stderr, "ssl-gen error: %s\n", ssl_gen_last_error());
    else
        printf("ssl-gen: Done.\n");

    return rc;
#else
    (void)argc; (void)argv;
    fprintf(stderr, "ssl-gen: OpenSSL not compiled in\n");
    return 1;
#endif
}

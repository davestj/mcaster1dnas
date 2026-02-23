#pragma once
// SslGen.h — Windows C++ wrapper around the cross-platform ssl_gen C module.
// Used by both Mcaster1Win (WinUI MFC) and Mcaster1Console (via win_ssl_gencert).

#include <string>

// Parameters for SSL certificate / CSR generation.
struct SslGenParams
{
    std::string gentype;        // "selfsigned" or "csr"
    std::string subj;           // "/C=US/ST=TX/O=Org/CN=hostname"
    std::string savepath;       // output directory (created if absent)
    std::string configPath;     // YAML or XML config file to patch (optional)
    int  key_bits    = 2048;
    int  days        = 365;
    bool addToConfig = false;
};

// CSslGen — thin C++ wrapper over ssl_gen_run() / ssl_gen_parse_args().
class CSslGen
{
public:
    // Run SSL generation with the given parameters.
    // Returns 0 on success, -1 on error.  Use GetLastError() for details.
    static int         Run(const SslGenParams& params);

    // Returns the last error message from the C layer.
    static std::string GetLastError();
};

// C-linkage entry point called from main.c on Windows when --ssl-gencert is present.
// Parses argv[], runs ssl_gen_run(), prints result, and returns exit code.
#ifdef __cplusplus
extern "C" {
#endif
int win_ssl_gencert(int argc, char **argv);
#ifdef __cplusplus
}
#endif

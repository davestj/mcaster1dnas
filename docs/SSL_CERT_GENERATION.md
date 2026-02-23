# SSL Certificate Generation — Mcaster1DNAS

**Applies to:** Mcaster1DNAS v2.5.1-beta.2+
**Platforms:** Windows (Console + GUI), Linux, macOS
**Branch:** windows-dev / development

---

## Overview

Mcaster1DNAS includes a built-in OpenSSL-based certificate and CSR generator accessible
from the command line (all platforms) and from the Windows GUI (`mcaster1win.exe`). This
eliminates the need for separate openssl CLI invocations when bootstrapping a new server
with TLS.

The feature is compiled only when `HAVE_OPENSSL` is defined (always true on Windows with
vcpkg openssl; on Linux/macOS requires `--with-openssl` configure flag).

---

## CLI Flags (All Platforms)

| Flag | Type | Description |
|------|------|-------------|
| `--ssl-gencert` | presence flag | Trigger SSL generation mode; no server starts |
| `--ssl-gentype=<type>` | `selfsigned` or `csr` | Output type (default: `selfsigned`) |
| `--subj=<subject>` | string | X.509 subject in OpenSSL format (e.g. `/C=US/ST=TX/O=Org/CN=localhost`) |
| `--ssl-gencert-savepath=<dir>` | path | Output directory (created if absent) |
| `--ssl-gencert-addtoconfig=true` | boolean | Patch `-c <config>` file after generation |

When `--ssl-gencert` is present the server **does not start** — it generates files, optionally
patches the config, reports result, and exits.

---

## Windows Console (mcaster1.exe)

```powershell
# Self-signed cert — saves to ssl\mycert\
mcaster1.exe --ssl-gencert `
    --ssl-gentype=selfsigned `
    --subj="/C=US/ST=TX/L=Dallas/O=Acme Radio/CN=stream.example.com" `
    --ssl-gencert-savepath="ssl\mycert"

# Self-signed cert + auto-patch config
mcaster1.exe --ssl-gencert `
    --ssl-gentype=selfsigned `
    --subj="/C=US/ST=TX/O=Acme Radio/CN=stream.example.com" `
    --ssl-gencert-savepath="ssl\mycert" `
    --ssl-gencert-addtoconfig=true `
    -c windows\mcaster1dnas.yaml

# CSR (for submission to a CA)
mcaster1.exe --ssl-gencert `
    --ssl-gentype=csr `
    --subj="/C=US/ST=TX/O=Acme Radio/CN=stream.example.com" `
    --ssl-gencert-savepath="ssl\csr"
```

---

## Windows GUI (mcaster1win.exe)

The GUI passes the same flags at launch. On completion a MessageBox reports success or
failure, then the application exits (no server GUI is shown).

```
mcaster1win.exe --ssl-gencert --ssl-gentype=selfsigned ^
    --subj="/C=US/ST=TX/O=Acme Radio/CN=stream.example.com" ^
    --ssl-gencert-savepath=ssl\mycert ^
    --ssl-gencert-addtoconfig=true ^
    -c windows\mcaster1dnas.yaml
```

You can also invoke it from within the GUI via **Configuration → Generate SSL Certificate**
(planned, not yet in UI — use command-line for now).

---

## Linux / macOS (mcaster1)

Same flags, identical behavior. Build must have been configured with `--with-openssl`.

```bash
# Self-signed cert
./mcaster1 --ssl-gencert \
    --ssl-gentype=selfsigned \
    --subj="/C=US/ST=CA/O=Acme Radio/CN=stream.example.com" \
    --ssl-gencert-savepath=/etc/mcaster1dnas/ssl

# CSR for a commercial CA
./mcaster1 --ssl-gencert \
    --ssl-gentype=csr \
    --subj="/C=US/ST=CA/O=Acme Radio/CN=stream.example.com" \
    --ssl-gencert-savepath=/etc/mcaster1dnas/ssl

# Verify the generated CSR
openssl req -verify -in /etc/mcaster1dnas/ssl/server.csr -noout -text
```

---

## Output Files

### Self-signed (`--ssl-gentype=selfsigned`)

| File | Description |
|------|-------------|
| `<savepath>/selfsigned.key` | PEM private key (RSA 2048-bit, unencrypted) |
| `<savepath>/selfsigned.crt` | PEM certificate (X.509, valid 365 days by default) |
| `<savepath>/selfsigned.pem` | Combined cert + key (for mcaster1dnas `ssl-certificate` config key) |

### CSR (`--ssl-gentype=csr`)

| File | Description |
|------|-------------|
| `<savepath>/server.key` | PEM private key |
| `<savepath>/server.csr` | PEM Certificate Signing Request |

Submit `server.csr` to your CA (Let's Encrypt, DigiCert, etc.). When you receive the signed
cert, combine it with `server.key` into a `.pem` file and point `ssl-certificate:` at it.

---

## Config Patching (`--ssl-gencert-addtoconfig=true`)

When this flag is present alongside `-c <configfile>`, the generator line-scans the YAML or
XML config and updates (or inserts) the `ssl-certificate` key:

**YAML** — updates `ssl-certificate:` under `paths:`:
```yaml
paths:
  ssl-certificate: "ssl/mycert/selfsigned.pem"   # ← patched in-place
```

**XML** — updates `<ssl-certificate>` inside `<paths>`:
```xml
<paths>
  <ssl-certificate>ssl/mycert/selfsigned.pem</ssl-certificate>
</paths>
```

If the key is absent it is inserted after the `paths:` / `<paths>` block opener.

---

## Using Test Certificates in Development

The `ssl/temp/` directory contains pre-generated test files for local development
(created with vcpkg `openssl.exe`, subject `CN=localhost`):

```
ssl/temp/selfsigned.key   — private key
ssl/temp/selfsigned.crt   — certificate
ssl/temp/selfsigned.pem   — combined (use this in config)
ssl/temp/test.key         — CSR private key
ssl/temp/test.csr         — Certificate Signing Request
```

Point `ssl-certificate` in your local config at `ssl/temp/selfsigned.pem` for quick
test builds:

```yaml
paths:
  ssl-certificate: "../../ssl/temp/selfsigned.pem"
```

Browsers will show a certificate warning for self-signed certs. Use `curl -k` or add the
cert to your browser's trust store for testing. **Never use these files in production.**

---

## Per-Listener SSL Enforcement

Beyond generating certs, Mcaster1DNAS v2.5.1-beta.2+ supports per-listener `ssl:` flags
in the config. See [../docs/PER_LISTENER_SSL.md](PER_LISTENER_SSL.md) or the
[YAML Configuration section](#per-listener-ssl-yaml) below.

### YAML
```yaml
listen-sockets:
  - port: 9330
    bind-address: "0.0.0.0"
    ssl: false      # plain HTTP only — TLS connections are rejected with a WARN log
  - port: 9443
    bind-address: "0.0.0.0"
    ssl: true       # TLS only — plain HTTP connections are rejected with a WARN log
```

### XML
```xml
<listen-socket><port>9330</port><ssl>0</ssl></listen-socket>
<listen-socket><port>9443</port><ssl>1</ssl></listen-socket>
```

The default (`ssl:` absent or `-1`) is the previous auto-detect behavior — the server
peeks at the first bytes to determine whether the client is speaking TLS.

---

## Troubleshooting

| Symptom | Likely cause | Fix |
|---------|--------------|-----|
| `ssl_gen_run: EVP_RSA_gen failed` | OpenSSL not linked / HAVE_OPENSSL not defined | Rebuild with `--with-openssl` (Linux) or ensure vcpkg openssl is linked (Windows) |
| `mkdir failed: permission denied` | Savepath parent not writable | Choose a writeable directory or run with elevated permissions |
| `config patch: ssl-certificate key not found` | `paths:` block missing in YAML | Add a `paths:` section manually first |
| Browser says cert invalid | Self-signed cert used in production | Use Let's Encrypt or a commercial CA for production |
| `curl: (60) SSL certificate problem` | Self-signed cert not trusted | Use `curl -k` for testing, or import cert into OS trust store |

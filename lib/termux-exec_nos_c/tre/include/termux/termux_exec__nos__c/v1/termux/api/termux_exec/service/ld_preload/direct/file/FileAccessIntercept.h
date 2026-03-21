#ifndef LIBTERMUX_EXEC__NOS__C__FILE_ACCESS_INTERCEPT___H
#define LIBTERMUX_EXEC__NOS__C__FILE_ACCESS_INTERCEPT___H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif



/**
 * Redirect table entry: maps a source path to a destination path
 * relative to the Termux prefix directory.
 */
typedef struct {
    const char *src;  /** Path the program tries to access (e.g. "/etc/resolv.conf"). */
    const char *dest; /** Path relative to $PREFIX to redirect to (e.g. "/etc/resolv.conf"). */
} FileAccessRedirectEntry;



/**
 * Check if `path` should be redirected to a Termux prefix equivalent.
 *
 * Redirects accesses of standard Linux `/etc/` configuration files to
 * their Termux equivalents under `$TERMUX__PREFIX/etc/`. All access
 * modes (read, write, stat) are redirected. This is needed because
 * Android does not provide `/etc/resolv.conf`, `/etc/nsswitch.conf`,
 * or standard SSL certificate paths.
 *
 * Currently redirected:
 * - `/etc/resolv.conf`       -> `$PREFIX/etc/resolv.conf`
 * - `/etc/hosts`             -> `$PREFIX/etc/hosts`
 * - `/etc/nsswitch.conf`     -> `$PREFIX/etc/nsswitch.conf`
 * - `/etc/ssl/certs/ca-certificates.crt`              -> `$PREFIX/etc/tls/cert.pem`
 * - `/etc/pki/tls/certs/ca-bundle.crt`                -> `$PREFIX/etc/tls/cert.pem`
 * - `/etc/ssl/ca-bundle.pem`                          -> `$PREFIX/etc/tls/cert.pem`
 * - `/etc/pki/tls/cacert.pem`                         -> `$PREFIX/etc/tls/cert.pem`
 * - `/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem` -> `$PREFIX/etc/tls/cert.pem`
 * - `/etc/ssl/cert.pem`                               -> `$PREFIX/etc/tls/cert.pem`
 *
 * @param path The path being accessed.
 * @param buffer Output buffer for the redirected path (should be PATH_MAX).
 * @param bufferSize Size of the output buffer.
 * @return The redirected path if applicable, otherwise the original path.
 */
const char *fileAccess_redirectPath(const char *path,
    char *buffer, size_t bufferSize);



#ifdef __cplusplus
}
#endif

#endif /* LIBTERMUX_EXEC__NOS__C__FILE_ACCESS_INTERCEPT___H */

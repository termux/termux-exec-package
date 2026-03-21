#define _GNU_SOURCE
#include <fcntl.h>
#include <linux/limits.h>
#include <string.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <termux/termux_core__nos__c/v1/termux/file/TermuxFile.h>
#include <termux/termux_exec__nos__c/v1/termux/api/termux_exec/service/ld_preload/direct/file/FileAccessIntercept.h>

/**
 * Redirect table for standard Linux `/etc/` files to Termux prefix equivalents.
 *
 * For SSL certificates, Go's `crypto/x509/root_linux.go` checks several
 * hardcoded paths. We redirect them all to Termux's cert bundle at
 * `$PREFIX/etc/tls/cert.pem`.
 */
static const FileAccessRedirectEntry REDIRECT_TABLE[] = {
    /* DNS and network configuration */
    { "/etc/resolv.conf",       "/etc/resolv.conf" },
    { "/etc/hosts",             "/etc/hosts" },
    { "/etc/nsswitch.conf",     "/etc/nsswitch.conf" },
    /* SSL CA certificates */
    { "/etc/ssl/certs/ca-certificates.crt", "/etc/tls/cert.pem" },
    { "/etc/pki/tls/certs/ca-bundle.crt",   "/etc/tls/cert.pem" },
    { "/etc/ssl/ca-bundle.pem",              "/etc/tls/cert.pem" },
    { "/etc/pki/tls/cacert.pem",             "/etc/tls/cert.pem" },
    { "/etc/pki/ca-trust/extracted/pem/tls-ca-bundle.pem", "/etc/tls/cert.pem" },
    { "/etc/ssl/cert.pem",                   "/etc/tls/cert.pem" },
    { NULL, NULL }
};

const char *fileAccess_redirectPath(const char *path,
    char *buffer, size_t bufferSize) {
    if (path == NULL || path[0] != '/') return path;

    for (int i = 0; REDIRECT_TABLE[i].src != NULL; i++) {
        if (strcmp(path, REDIRECT_TABLE[i].src) != 0) continue;

        const char *prefixDir = termux_prefixDir_get("FileAccessIntercept");
        if (prefixDir == NULL) return path;

        size_t prefixLen = strlen(prefixDir);
        size_t destLen = strlen(REDIRECT_TABLE[i].dest);
        if (prefixLen + destLen + 1 > bufferSize) return path;

        memcpy(buffer, prefixDir, prefixLen);
        memcpy(buffer + prefixLen, REDIRECT_TABLE[i].dest, destLen + 1);

        /* Only redirect if the Termux file exists.
         * Use raw syscall to avoid recursing into our own hook. */
        if (syscall(__NR_faccessat, AT_FDCWD, buffer, F_OK, 0) == 0)
            return buffer;

        return path;
    }

    return path;
}

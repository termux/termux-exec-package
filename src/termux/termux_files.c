#define _GNU_SOURCE
#include <errno.h>
#include <libgen.h>
#include <regex.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#include <linux/limits.h>

#include "../data/data_utils.h"
#include "../file/file_utils.h"
#include "../logger/logger.h"
#include "termux_env.h"
#include "termux_files.h"

static const char* LOG_TAG = "termux_files";



static char sTermuxAppDataDirBuffer[TERMUX_APP__DATA_DIR_MAX_LEN];

/** The Termux app data directory path. */
static const char* sTermuxAppDataDir = NULL;


static char sTermuxLegacyAppDataDirBuffer[TERMUX_APP__DATA_DIR_MAX_LEN];

/** The Termux legacy app data directory path. */
static const char* sTermuxLegacyAppDataDir = NULL;


static char sTermuxRootfsDirBuffer[TERMUX__ROOTFS_DIR_MAX_LEN];

/** The Termux rootfs directory path. */
static const char* sTermuxRootfsDir = NULL;


static char sTermuxPrefixDirBuffer[TERMUX__PREFIX_DIR_MAX_LEN];

/** The Termux prefix directory path. */
static const char* sTermuxPrefixDir = NULL;



const char* get_termux_app_data_dir(const char* log_tag) {
    if (sTermuxAppDataDir == NULL) {
        sTermuxAppDataDir = get_termux_app_data_dir_from_env_or_default(log_tag,
            sTermuxAppDataDirBuffer, sizeof(sTermuxAppDataDirBuffer));
    }

    return sTermuxAppDataDir;
}

int get_termux_app_data_dir_from_env(const char* log_tag, char *buffer, size_t buffer_len) {
    return get_path_from_env(log_tag, "app_data_dir",
        ENV__TERMUX_APP__DATA_DIR, TERMUX_APP__DATA_DIR_MAX_LEN,
        buffer, buffer_len);
}

const char* get_termux_app_data_dir_from_env_or_default(const char* log_tag, char *buffer, size_t buffer_len) {
    return get_path_from_env_or_default(log_tag, "app_data_dir",
        ENV__TERMUX_APP__DATA_DIR, TERMUX_APP__DATA_DIR_MAX_LEN,
        TERMUX_APP__DATA_DIR, -1,
        buffer, buffer_len);
}


const char* get_termux_legacy_app_data_dir(const char* log_tag) {
    if (sTermuxLegacyAppDataDir == NULL) {
        sTermuxLegacyAppDataDir = get_termux_legacy_app_data_dir_from_env_or_default(log_tag,
            sTermuxLegacyAppDataDirBuffer, sizeof(sTermuxLegacyAppDataDirBuffer));
    }

    return sTermuxLegacyAppDataDir;
}

int get_termux_legacy_app_data_dir_from_env(const char* log_tag, char *buffer, size_t buffer_len) {
    return get_path_from_env(log_tag, "legacy_app_data_dir",
        ENV__TERMUX_APP__LEGACY_DATA_DIR, TERMUX_APP__DATA_DIR_MAX_LEN,
        buffer, buffer_len);
}

const char* get_termux_legacy_app_data_dir_from_env_or_default(const char* log_tag, char *buffer, size_t buffer_len) {
    const char*  termux_legacy_app_data_dir = get_path_from_env_or_default(
        log_tag, "legacy_app_data_dir",
        ENV__TERMUX_APP__LEGACY_DATA_DIR, TERMUX_APP__DATA_DIR_MAX_LEN,
        TERMUX_APP__DATA_DIR, -1,
        buffer, buffer_len);
    if (termux_legacy_app_data_dir == NULL) {
        return NULL;
    }

    // If default path is not for a legacy app data directory path,
    // then convert it into a legacy path by extracting the package
    // name from the `basename` of path found and appending it to
    // `/data/data/`.
    if (!string_starts_with(termux_legacy_app_data_dir, "/data/data/")) {
        char termux_legacy_app_data_dir_copy[buffer_len];
        strncpy(termux_legacy_app_data_dir_copy, termux_legacy_app_data_dir, buffer_len - 1);

        termux_legacy_app_data_dir = convert_termux_app_data_dir_to_legacy_path(log_tag,
            termux_legacy_app_data_dir_copy, buffer, buffer_len);
        if (termux_legacy_app_data_dir == NULL) {
            return NULL;
        }

        #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorVVerbose(log_tag, "updated_legacy_app_data_dir: '%s'", termux_legacy_app_data_dir);
        #endif
    }

    return termux_legacy_app_data_dir;

}


const char* convert_termux_app_data_dir_to_legacy_path(const char* log_tag,
    const char *termux_app_data_dir, char *buffer, size_t buffer_len) {
    (void)log_tag;

    if (termux_app_data_dir == NULL || *termux_app_data_dir != '/') {
        #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorDebug(log_tag, "The app_data_dir '%s' to be converted to legacy path is not an absolute path",
            termux_app_data_dir == NULL ? "" : termux_app_data_dir);
        #endif
        errno = EINVAL;
        return NULL;
    }

    char *last_path_separator = strrchr(termux_app_data_dir, '/');
    if (!last_path_separator) {
        #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorDebug(log_tag, "Failed to find last path separator '/' in app_data_dir '%s' to be converted to legacy path",
            termux_app_data_dir);
        #endif
        errno = EINVAL;
        return NULL;
    }

    size_t last_path_separator_position = last_path_separator - termux_app_data_dir;
    size_t termux_app_data_dir_len = strlen(termux_app_data_dir);

    if (last_path_separator_position == 0) {
        #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorDebug(log_tag, "The last path separator '/' is at rootfs in app_data_dir '%s' to be converted to legacy path",
            termux_app_data_dir);
        #endif
        errno = EINVAL;
        return NULL;
    }

    // If path contains at least one character after last path
    // separator `/`, ideally for package name.
    if ((last_path_separator_position + 1) >= termux_app_data_dir_len) {
      #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorDebug(log_tag, "No basename found in app_data_dir '%s' to be converted to legacy path",
            termux_app_data_dir);
        #endif
        errno = EINVAL;
        return NULL;
    }

    strncpy(buffer, "/data/data/", buffer_len - 1);

    char *dest = buffer + 11;

    // Copy package name from basename.
    const char *src = last_path_separator + 1;
    size_t termux_legacy_app_data_dir_len = 11 + strlen(src);
    if (buffer_len <= termux_legacy_app_data_dir_len) {
        #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorDebug(log_tag, "The legacy_app_data_dir '/data/data/%s' with length '%zu' is too long to fit in the buffer with length '%zu'",
            src, termux_legacy_app_data_dir_len, buffer_len);
        #endif
        errno = ENAMETOOLONG;
        return NULL;
    }

    strcpy(dest, src);

    return buffer;
}



const char* get_termux_rootfs_dir(const char* log_tag) {
    if (sTermuxRootfsDir == NULL) {
        sTermuxRootfsDir = get_termux_rootfs_dir_from_env_or_default(log_tag,
            sTermuxRootfsDirBuffer, sizeof(sTermuxRootfsDirBuffer));
    }

    return sTermuxRootfsDir;
}

int get_termux_rootfs_dir_from_env(const char* log_tag, char *buffer, size_t buffer_len) {
    return get_path_from_env(log_tag, "rootfs_dir",
        ENV__TERMUX__ROOTFS, TERMUX__ROOTFS_DIR_MAX_LEN,
        buffer, buffer_len);
}

const char* get_termux_rootfs_dir_from_env_or_default(const char* log_tag, char *buffer, size_t buffer_len) {
    return get_path_from_env_or_default(log_tag, "rootfs_dir",
        ENV__TERMUX__ROOTFS, TERMUX__ROOTFS_DIR_MAX_LEN,
        TERMUX__ROOTFS, 3 /* (R_OK | X_OK) */,
        buffer, buffer_len);
}



const char* get_termux_prefix_dir(const char* log_tag) {
    if (sTermuxPrefixDir == NULL) {
        sTermuxPrefixDir = get_termux_prefix_dir_from_env_or_default(log_tag,
            sTermuxPrefixDirBuffer, sizeof(sTermuxPrefixDirBuffer));
    }

    return sTermuxPrefixDir;
}

int get_termux_prefix_dir_from_env(const char* log_tag, char *buffer, size_t buffer_len) {
    return get_path_from_env(log_tag, "prefix_dir",
        ENV__TERMUX__PREFIX, TERMUX__PREFIX_DIR_MAX_LEN,
        buffer, buffer_len);
}

const char* get_termux_prefix_dir_from_env_or_default(const char* log_tag, char *buffer, size_t buffer_len) {
    return get_path_from_env_or_default(log_tag, "prefix_dir",
        ENV__TERMUX__PREFIX, TERMUX__PREFIX_DIR_MAX_LEN,
        TERMUX__PREFIX, 3 /* (R_OK | X_OK) */,
        buffer, buffer_len);
}





char *termux_prefix_path(const char* log_tag, const char *termux_prefix_dir, const char *executable_path,
    char *buffer, size_t buffer_len) {
    (void)log_tag;
    (void)LOG_TAG;

    size_t executable_path_len = strlen(executable_path);
    if (buffer_len <= executable_path_len) {
        #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorDebug(LOG_TAG, "The original executable_path '%s' with length '%zu' to prefix is too long to fit in the buffer with length '%zu'",
            executable_path, executable_path_len, buffer_len);
        #endif
        errno = ENAMETOOLONG;
        return NULL;
    }

    // If `executable_path` is not an absolute path.
    if (executable_path[0] != '/') {
        strcpy(buffer, executable_path);
        return buffer;
    }

    char termux_bin_path[TERMUX__BIN_DIR_MAX_LEN + 1];

    // If `executable_path` equals with `/bin` or `/usr/bin` (currently not `/xxx/bin`).
    if (strcmp(executable_path, "/bin") == 0 || strcmp(executable_path, "/usr/bin") == 0) {
        if (termux_prefix_dir == NULL) {
            termux_prefix_dir = get_termux_prefix_dir(log_tag);
            if (termux_prefix_dir == NULL) { return NULL; }
        }

        // If `termux_prefix_dir` equals `/`.
        if (strlen(termux_prefix_dir) == 1 && termux_prefix_dir[0] == '/') {
            strcpy(buffer, executable_path);
            return buffer;
        }

        snprintf(termux_bin_path, sizeof(termux_bin_path), "%s/bin", termux_prefix_dir);
        strcpy(buffer, termux_bin_path);
        return buffer;
    }

    char *bin_match = strstr(executable_path, "/bin/");
    // If `executable_path` starts with `/bin/` or `/xxx/bin`.
    if (bin_match == executable_path || bin_match == (executable_path + 4)) {
        if (termux_prefix_dir == NULL) {
            termux_prefix_dir = get_termux_prefix_dir(log_tag);
            if (termux_prefix_dir == NULL) { return NULL; }
        }

        // If `termux_prefix_dir` equals `/`.
        if (strlen(termux_prefix_dir) == 1 && termux_prefix_dir[0] == '/') {
            strcpy(buffer, executable_path);
            return buffer;
        }

        int termux_bin_path_len = snprintf(termux_bin_path, sizeof(termux_bin_path),
            "%s/bin/", termux_prefix_dir);

        strcpy(buffer, termux_bin_path);
        char *dest = buffer + termux_bin_path_len;
        // Copy what comes after `/bin/`.
        const char *src = bin_match + 5;
        size_t prefixed_path_len = termux_bin_path_len + strlen(src);
        if (buffer_len <= prefixed_path_len) {
            #ifndef TERMUX_EXEC__RUNNING_TESTS
            logErrorDebug(log_tag, "The prefixed_path '%s%s' with length '%zu' is too long to fit in the buffer with length '%zu'",
                termux_bin_path, src, prefixed_path_len, buffer_len);
            #endif
            errno = ENAMETOOLONG;
            return NULL;
        }

        strcpy(dest, src);
        return buffer;
    } else {
        strcpy(buffer, executable_path);
        return buffer;
    }
}





int is_path_under_termux_app_data_dir(const char* log_tag, const char *path,
    const char *termux_app_data_dir, const char *termux_legacy_app_data_dir) {
    if (path == NULL || *path == '\0') {
        return 1;
    }


    const char* real_path;
    char real_path_buffer[PATH_MAX];
    (void)real_path_buffer;
    if (strstr(path, "/fd/") != NULL && regex_match(path, REGEX__PROC_FD_PATH, REG_EXTENDED) == 0) {
        real_path = get_fd_realpath(log_tag, path, real_path_buffer, sizeof(real_path_buffer));
        if (real_path == NULL) {
            return -1;
        }

        path = real_path;
    }


    // Termux app for version `>= 0.119.0` will export
    // `TERMUX_APP__DATA_DIR` with the non-legacy app data directory
    // paths (`/data/user/<user_id>/<package_name>` or
    // `/mnt/expand/<volume_uuid>/user/0/<package_name>`) that
    // are normally returned by Android `ApplicationInfo.dataDir` call.
    // It will also export `TERMUX_APP__LEGACY_DATA_DIR` with the
    // legacy app data directory path (`/data/data/<package_name>`),
    // however, it will only be accessible if app is running on
    // primary user `0`, or if Android vendor does a bind mount for
    // secondary users/profiles as well or if
    // bind mount was done manually on rooted devices with
    // `MountLegacyAppDataDirPaths.java`.
    //
    // The checking if `path` passed to this function is under termux
    // app data directory is done based on following logic, assuming
    // `termux_app_data_dir` and `termux_legacy_app_data_dir` passed
    // are `NULL`. The below `FAIL` condition, like where `termux-exec`
    // was compiled with the default value of a legacy path
    // `/data/data/<package_name>` and a path starting with
    // `/data/user/0/<package_name>` is executed instead
    // of it starting with `/data/data/<package_name>`, would not
    // normally occur as rootfs would be for a legacy path as well,
    // and if solving it is needed, then user should used the app
    // version (like `>= 0.119.0`) where `TERMUX_APP__DATA_DIR`
    // variable is exported automatically.
    // - Executing path under legacy app data directory:
    //   - If `ENV__TERMUX_APP__LEGACY_DATA_DIR` is set and valid:
    //       - The env path will be used as is.
    //   - Else:
    //       - A non-legacy default path will be used after being
    //         converted to a legacy path.
    //       - A legacy default path will be used as is.
    // - Executing path under non-legacy app data directory:
    //   - If `ENV__TERMUX_APP__DATA_DIR` is set and valid:
    //       - The env path will be used as is.
    //   - Else:
    //       - A non-legacy default path will be used as is.
    //       - A legacy default path will be used as is. (FAIL)
    if (string_starts_with(path, "/data/data/")) {
        if (termux_legacy_app_data_dir == NULL) {
            termux_legacy_app_data_dir = get_termux_legacy_app_data_dir(log_tag);
            if (termux_legacy_app_data_dir == NULL) { return -1; }
        }

        termux_app_data_dir = termux_legacy_app_data_dir;
    } else {
        if (termux_app_data_dir == NULL) {
            termux_app_data_dir = get_termux_app_data_dir(log_tag);
            if (termux_app_data_dir == NULL) { return -1; }
        }
    }

    return is_path_in_dir_path("app_data_dir", path, termux_app_data_dir, true);
}



int is_path_under_termux_rootfs_dir(const char* log_tag, const char *path,
    const char *termux_rootfs_dir) {
    if (termux_rootfs_dir == NULL) {
        termux_rootfs_dir = get_termux_rootfs_dir(log_tag);
        if (termux_rootfs_dir == NULL) { return -1; }
    }

    return is_path_or_fd_path_in_dir_path(log_tag, "rootfs_dir", path, termux_rootfs_dir, true);
}

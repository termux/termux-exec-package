#define _GNU_SOURCE
#include <errno.h>
#include <limits.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "../logger/logger.h"
#include "file_utils.h"

static const char* LOG_TAG = "file_utils";

#include "canonicalize.c"

char* absolutize_path(const char *path, char *absolute_path, int buffer_len) {
    if (buffer_len < PATH_MAX) {
        errno = EINVAL;
        return NULL;
    }

    if (path == NULL) {
        errno = EINVAL;
        return NULL;
    }

    size_t path_len = strlen(path);
    if (path_len < 1) {
        errno = EINVAL;
        return NULL;
    }

    if (path_len >= PATH_MAX) {
        errno = ENAMETOOLONG;
        return NULL;
    }

    if (path[0] != '/') {
        char pwd[PATH_MAX];
        if (getcwd(pwd, sizeof(pwd)) == NULL) {
            return NULL;
        }

        size_t pwd_len = strlen(pwd);
        // Following a change in Linux 2.6.36, the pathname returned by the
        // getcwd() system call will be prefixed with the string
        // "(unreachable)" if the current directory is not below the root
        // directory of the current process (e.g., because the process set a
        // new filesystem root using chroot(2) without changing its current
        // directory into the new root).  Such behavior can also be caused
        // by an unprivileged user by changing the current directory into
        // another mount namespace.  When dealing with pathname from
        // untrusted sources, callers of the functions described in this
        // page should consider checking whether the returned pathname
        // starts with '/' or '(' to avoid misinterpreting an unreachable
        // path as a relative pathname.
        // - https://man7.org/linux/man-pages/man3/getcwd.3.html
        if (pwd_len < 1 || pwd[0] != '/') {
            errno = ENOENT;
            return NULL;
        }

        memcpy(absolute_path, pwd, pwd_len);
        if (absolute_path[pwd_len - 1] != '/') {
            absolute_path[pwd_len] = '/';
            pwd_len++;
        }

        size_t absolutized_path_len = pwd_len + path_len;
        if (absolutized_path_len >= PATH_MAX) {
            errno = ENAMETOOLONG;
            return NULL;
        }

        memcpy(absolute_path + pwd_len, path, path_len);
        absolute_path[absolutized_path_len] = '\0';
    } else {
        strcpy(absolute_path, path);
    }

    return absolute_path;
}





char* normalize_path(char* path, bool keepEndSeparator, bool removeDoubleDot) {
    return __normalizePath(path, keepEndSeparator, removeDoubleDot);
}




char* remove_dup_separator(char* path, bool keepEndSeparator) {
    if (path == NULL || *path == '\0') {
        return NULL;
    }

    char* in = path;
    char* out = path;
    char prevChar = 0;
    int n = 0;
    for (; *in != '\0'; in++) {
        // Remove duplicate path separators.
        if (!(*in == '/' && prevChar == '/')) {
            *(out++) = *in;
            n++;
        }
        prevChar = *in;
    }
    *out = '\0';

    // Remove the trailing path separator, except when path equals `/`.
    if (!keepEndSeparator && prevChar == '/' && n > 1) {
        *(--out) = '\0';
    }

    return path;
}





char* get_fd_realpath(const char* log_tag, const char *path, char *real_path, size_t buffer_len) {
    char path_string[strlen(path) + 1];
    strncpy(path_string, path, sizeof(path_string));
    char* fd_string = basename(path_string);

    int fd = string_to_int(fd_string, -1, log_tag, "Failed to convert fd string '%s' to fd for fd path '%s'", fd_string, path);
    if (fd < 0) {
        return NULL;
    }

    #ifndef TERMUX_EXEC__RUNNING_TESTS
    logErrorVVerbose(log_tag, "fd_path: '%s', fd: '%d'", path, fd);
    #endif

    struct stat fd_statbuf;
    int fd_result = fstat(fd, &fd_statbuf);
    if (fd_result < 0) {
        logStrerrorDebug(log_tag, "Failed to stat fd '%d' for fd path '%s'", fd, path);
        return NULL;
    }

    ssize_t length = readlink(path, real_path, buffer_len - 1);
    if (length < 0) {
        logStrerrorDebug(log_tag, "Failed to get real path for fd path '%s'", path);
        return NULL;
    } else {
        real_path[length] = '\0';
    }

    #ifndef TERMUX_EXEC__RUNNING_TESTS
    logErrorVVerbose(log_tag, "real_path: '%s'", real_path);
    #endif

    // Check if fd is for a regular file.
    // We should log real path first before doing this check.
    if (!S_ISREG(fd_statbuf.st_mode)) {
        #ifndef TERMUX_EXEC__RUNNING_TESTS
        logErrorDebug(log_tag, "The real path '%s' for fd path '%s' is of type '%d' instead of a regular file",
        real_path, path, fd_statbuf.st_mode & S_IFMT);
        #endif
        errno = ENOEXEC;
        return NULL;
    }

    size_t real_path_length = strlen(real_path);
    if (real_path_length < 1 || real_path[0] != '/') {
        logErrorDebug(log_tag, "A non absolute real path '%s' returned for fd path '%s'", real_path, path);
        errno = ENOEXEC;
        return NULL;
    }

    struct stat path_statbuf;
    int path_result = stat(real_path, &path_statbuf);
    if (path_result < 0) {
        logStrerrorDebug(log_tag, "Failed to stat real path '%s' returned for fd path '%s'", real_path, path);
        return NULL;
    }

    // If the original file when fd was opened has now been replaced with a different file.
    if (fd_statbuf.st_dev != path_statbuf.st_dev || fd_statbuf.st_ino != path_statbuf.st_ino) {
        logErrorDebug(log_tag, "The file at real path '%s' is not for the original fd '%d'", real_path, fd);
        errno = ENXIO;
        return NULL;
    }

    return real_path;
}





bool is_path_in_dir_path(const char* label, const char* path, const char* dir_path, bool ensure_under) {
    if (path == NULL || *path != '/' || dir_path == NULL || *dir_path != '/' ) {
        return 1;
    }

    // If root `/`, preserve it as is, otherwise append `/` to dir_path.
    char *dir_sub_path;
    bool is_rootfs = strcmp(dir_path, "/") == 0;
    if (asprintf(&dir_sub_path, is_rootfs ? "%s" : "%s/", dir_path) == -1) {
        errno = ENOMEM;
        logStrerrorDebug(LOG_TAG, "asprintf failed while checking if the path '%s' is under %s '%s'", path, label, dir_path);
        return -1;
    }

    int result;
    if (ensure_under) {
        result = strcmp(dir_sub_path, path) != 0 && string_starts_with(path, dir_sub_path) ? 0 : 1;
    } else {
        result = strcmp(dir_sub_path, path) == 0 || string_starts_with(path, dir_sub_path) ? 0 : 1;
    }

    free(dir_sub_path);

    return result;
}

int is_path_or_fd_path_in_dir_path(const char* log_tag, const char* label,
    const char *path, const char *dir_path, bool ensure_under) {
    if (path == NULL || *path == '\0') {
        return 1;
    }

    if (strstr(path, "/fd/") != NULL && regex_match(path, REGEX__PROC_FD_PATH, REG_EXTENDED) == 0) {
        char real_path_buffer[PATH_MAX];
        (void)real_path_buffer;
        const char* real_path = get_fd_realpath(log_tag, path, real_path_buffer, sizeof(real_path_buffer));
        if (real_path == NULL) {
            return -1;
        }

        return is_path_in_dir_path(label, real_path, dir_path, ensure_under);
    } else {
        return is_path_in_dir_path(label, path, dir_path, ensure_under);
    }
}





int get_path_from_env(const char* log_tag, const char* label,
    const char * env_variable_name, size_t env_path_max_len,
    char *buffer, size_t buffer_len) {
    (void)label;
    (void)LOG_TAG;
    (void)log_tag;

    const char* env_path = getenv(env_variable_name);
    if (env_path != NULL) {
        size_t env_path_len = strlen(env_path);
        if (env_path_len > 0) {
            if (env_path[0] == '/' && (env_path_max_len < 1 || env_path_len < env_path_max_len)) {
                if (buffer_len <= env_path_len) {
                    #ifndef TERMUX_EXEC__RUNNING_TESTS
                    logErrorDebug(LOG_TAG, "The %s '%s' with length '%zu' too long to fit in the buffer with length '%zu'",
                        label, env_path, env_path_len, buffer_len);
                    #endif
                    errno = EINVAL;
                    return -1;
                }

                strcpy(buffer, env_path);
                return 0;
            }

            #ifndef TERMUX_EXEC__RUNNING_TESTS
            logErrorVVerbose(log_tag, "Ignoring invalid %s with length '%zu' set in '%s' env variable: '%s'",
                label, env_path_len, env_variable_name, env_path);
            if (env_path_max_len >= 1) {
                logErrorVVerbose(log_tag, "The %s must be an absolute path starting with a '/' with max length '%d' including the null '\0' terminator",
                    label, env_path_max_len);
            } else {
                logErrorVVerbose(log_tag, "The %s must be an absolute path starting with a '/'",
                    label);
            }
            #endif
        }
    }

    return 1;
}

const char* get_path_from_env_or_default(const char* log_tag, const char* label,
    const char * env_variable_name, size_t env_path_max_len,
    const char* default_path, int default_path_access_check_type,
    char *buffer, size_t buffer_len) {

    const char *path;
    int result = get_path_from_env(log_tag, label,
        env_variable_name, env_path_max_len, buffer, buffer_len);
    if (result < 0) {
        return NULL;
    } else if (result == 0) {
        normalize_path(buffer, false, true);
        path = buffer;
        logErrorVVerbose(log_tag, "%s: '%s'", label, path);
        return path;
    }

    // Use default path.
    size_t path_len = strlen(default_path);
    if (default_path[0] != '/' || buffer_len <= path_len) {
        logErrorDebug(log_tag, "The default_%s '%s' with length '%zu' must be an absolute path starting with a '/' with max length '%zu'",
            label, default_path, path_len, buffer_len);
        errno = EINVAL;
        return NULL;
    }

    strcpy(buffer, default_path);
    normalize_path(buffer, false, true);
    path = default_path;
    logErrorVVerbose(log_tag, "default_%s: '%s'", label, path);

    // Check default path access if required.
    if (default_path_access_check_type >= 0) {
        if (default_path_access_check_type == 0) {
            if (access(path, F_OK) != 0) {
                logStrerrorDebug(log_tag, "The default_%s '%s' does not exist", label, default_path);
                return NULL;
            }
        } else if (default_path_access_check_type == 1) {
            if (access(path, R_OK) != 0) {
                logStrerrorDebug(log_tag, "The default_%s '%s' is not readable", label, default_path);
                return NULL;
            }
        } else if (default_path_access_check_type == 2) {
            if (access(path, (R_OK | W_OK)) != 0) {
                logStrerrorDebug(log_tag, "The default_%s '%s' is not readable or writable", label, default_path);
                return NULL;
            }
        } else if (default_path_access_check_type == 3) {
            if (access(path, (R_OK | X_OK)) != 0) {
                logStrerrorDebug(log_tag, "The default_%s '%s' is not readable or executable", label, default_path);
                return NULL;
            }
        } else if (default_path_access_check_type == 4) {
            if (access(path, (R_OK | W_OK | X_OK)) != 0) {
                logStrerrorDebug(log_tag, "The default_%s '%s' is not readable, writable or executable", label, default_path);
                return NULL;
            }
        }
    }

    return path;
}

#ifndef TERMUX_FILES_H
#define TERMUX_FILES_H

/**
 * The max length for the `TERMUX_APP__DATA_DIR` directory including
 * the null '\0' terminator.
 *
 * Check https://github.com/termux/termux-packages/wiki/Termux-file-system-layout#file-path-limits
 * for why the value `69` is chosen.
 *
 * Default value: `69`
 */
#define TERMUX_APP__DATA_DIR_MAX_LEN 69

/**
 * The max length for the `TERMUX__ROOTFS` directory including the
 * null '\0' terminator.
 *
 * Check https://github.com/termux/termux-packages/wiki/Termux-file-system-layout#file-path-limits
 * for why the value `86` is chosen.
 *
 * Default value: `86`
 */
#define TERMUX__ROOTFS_DIR_MAX_LEN 86

/**
 * The max length for the `TERMUX__PREFIX` directory including the
 * null '\0' terminator.
 *
 * Default value: `90`
 */
#define TERMUX__PREFIX_DIR_MAX_LEN (TERMUX__ROOTFS_DIR_MAX_LEN + 4) // "/usr" (90)

/**
 * The max length for the `TERMUX__BIN_DIR` including the null '\0' terminator.
 *
 * Default value: `94`
 */
#define TERMUX__BIN_DIR_MAX_LEN (TERMUX__PREFIX_DIR_MAX_LEN + 4) // "/bin" (94)

/**
 * The max safe length for a sub file path under the `TERMUX__BIN_DIR`
 * including the null '\0' terminator.
 *
 * This allows for a filename with max length `32` so that the path
 * length is under `128` (`BINPRM_BUF_SIZE`) for Linux kernel `< 5.1`.
 * Check `exec.h` docs.
 *
 * Default value: `127`
 */
#define TERMUX__BIN_FILE_SAFE_MAX_LEN (TERMUX__BIN_DIR_MAX_LEN + 34) // "/<filename_with_len_32>" (127)



/**
 * Get cached `sTermuxAppDataDir` that is set to the path returned by
 * `get_termux_app_data_dir_from_env_or_default()` on its first call.
 */
const char* get_termux_app_data_dir(const char* log_tag);

/**
 * Get the termux app data directory from the `ENV__TERMUX_APP__DATA_DIR`
 * env variable value if its set to a valid absolute path with max
 * length `TERMUX_APP__DATA_DIR_DIR_MAX_LEN`.
 *
 *
 * @return Returns `0` if a valid app data directory is found and
 * copied to the buffer, `1` if valid app data directory is not found,
 * otherwise `-1` on other failures.
 */
int get_termux_app_data_dir_from_env(const char* log_tag,
    char *buffer, size_t buffer_len);

/**
 * Get the termux app data directory from the `ENV__TERMUX_APP__DATA_DIR`
 * env variable if its set to a valid absolute path with max length
 * `TERMUX_APP__DATA_DIR_DIR_MAX_LEN`, otherwise if it fails, then
 * return `TERMUX_APP__DATA_DIR` set by `Makefile` if it is readable
 * and executable.
 *
 * @return Returns the `char *` to app data directory on success,
 * otherwise `NULL`.
 */
const char* get_termux_app_data_dir_from_env_or_default(const char* log_tag,
    char *buffer, size_t buffer_len);


/**
 * Get cached `sTermuxLegacyAppDataDir` that is set to the path
 * returned by `get_termux_legacy_app_data_dir_from_env_or_default()`
 * on its first call.
 */
const char* get_termux_legacy_app_data_dir(const char* log_tag);

/**
 * Get the termux legacy app data directory from the
 * `ENV__TERMUX_APP__LEGACY_DATA_DIR` env variable value if its set to
 * a valid absolute path with max length `TERMUX_APP__DATA_DIR_DIR_MAX_LEN`.
 *
 *
 * @return Returns `0` if a valid app data directory is found and
 * copied to the buffer, `1` if valid app data directory is not found,
 * otherwise `-1` on other failures.
 */
int get_termux_legacy_app_data_dir_from_env(const char* log_tag,
    char *buffer, size_t buffer_len);

/**
 * Get the termux legacy app data directory from the
 * `ENV__TERMUX_APP__LEGACY_DATA_DIR` env variable if its set to a
 * valid absolute path with max length `TERMUX_APP__DATA_DIR_DIR_MAX_LEN`,
 * otherwise if it fails, then return `TERMUX_APP__DATA_DIR` set by
 * `Makefile` if it is readable and executable after converting it to
 * a legacy path if required by calling
 * `convert_termux_app_data_dir_to_legacy_path()`.
 *
 * @return Returns the `char *` to app data directory on success,
 * otherwise `NULL`.
 */
const char* get_termux_legacy_app_data_dir_from_env_or_default(const char* log_tag,
    char *buffer, size_t buffer_len);


/**
 * Convert a termux app data directory in the format
 * `/data/user/<user_id>/<package_name>` or
 * `/mnt/expand/<volume_uuid>/user/0/<package_name>` to a legacy path
 * `/data/data/<package_name>` by extracting the package name from
 * the `basename` and appending it to `/data/data/`.
 *
 * No validation is currently done to ensure `termux_app_data_dir`
 * is in correct format and `basename` is used even if it is not a for
 * a package name. So its callers responsibility to pass a valid path.
 *
 * @return Returns the `char *` to app data directory on success,
 * otherwise `NULL`.
 */
const char* convert_termux_app_data_dir_to_legacy_path(const char* log_tag,
    const char *termux_app_data_dir, char *buffer, size_t buffer_len);



/**
 * Get cached `sTermuxRootfsDir` that is set to the path returned by
 * `get_termux_rootfs_dir_from_env_or_default()` on its first call.
 */
const char* get_termux_rootfs_dir(const char* log_tag);

/**
 * Get the termux rootfs directory from the `ENV__TERMUX__ROOTFS` env
 * variable value if its set to a valid absolute path with max length
 * `TERMUX__ROOTFS_DIR_MAX_LEN`.
 *
 *
 * @return Returns `0` if a valid rootfs directory is found and copied
 * to the buffer, `1` if valid rootfs directory is not found, otherwise
 * `-1` on other failures.
 */
int get_termux_rootfs_dir_from_env(const char* log_tag,
    char *buffer, size_t buffer_len);

/**
 * Get the termux rootfs directory from the `ENV__TERMUX__ROOTFS`
 * env variable if its set to a valid absolute path with max length
 * `TERMUX__ROOTFS_DIR_MAX_LEN`, otherwise if it fails, then return
 * `TERMUX__ROOTFS` set by `Makefile` if it is readable and executable.
 *
 * @return Returns the `char *` to rootfs directory on success,
 * otherwise `NULL`.
 */
const char* get_termux_rootfs_dir_from_env_or_default(const char* log_tag,
    char *buffer, size_t buffer_len);



/**
 * Get cached `sTermuxPrefixDir` that is set to the path returned by
 * `get_termux_prefix_dir_from_env_or_default()` on its first call.
 */
const char* get_termux_prefix_dir(const char* log_tag);

/**
 * Get the termux prefix directory from the `ENV__TERMUX__PREFIX` env
 * variable value if its set to a valid absolute path with max length
 * `TERMUX__PREFIX_DIR_MAX_LEN`.
 *
 *
 * @return Returns `0` if a valid prefix directory is found and copied
 * to the buffer, `1` if valid prefix directory is not found, otherwise
 * `-1` on other failures.
 */
int get_termux_prefix_dir_from_env(const char* log_tag,
    char *buffer, size_t buffer_len);

/**
 * Get the termux app data directory from the `ENV__TERMUX__PREFIX`
 * env variable if its set to a valid absolute path with max length
 * `TERMUX__PREFIX_DIR_MAX_LEN`, otherwise if it fails, then return
 * `TERMUX__PREFIX` set by `Makefile` if it is readable and executable.
 *
 * @return Returns the `char *` to prefix directory on success,
 * otherwise `NULL`.
 */
const char* get_termux_prefix_dir_from_env_or_default(const char* log_tag,
    char *buffer, size_t buffer_len);





/**
 * Prefix `bin` directory paths like `/bin` and `/usr/bin` with
 * termux prefix directory, like convert `/bin/sh` to `<termux_prefix_dir>/usr/bin/sh`.
 *
 * The buffer size must at least be `TERMUX__PREFIX_DIR_MAX_LEN` + strlen(executable_path)`,
 * or preferably `PATH_MAX`.
 *
 * @param termux_prefix_dir The **normalized** path to termux prefix
 *                          directory. If `NULL`, then path returned by
 *                          `get_termux_prefix_dir_from_env_or_default()`
 *                          will be used by calling `get_termux_prefix_dir()`.
 * @param executable_path The executable path to prefix.
 * @param buffer The output path buffer.
 * @param buffer_len The output path buffer length.
 * @return Returns the `char *` to original or prefixed directory on
 * success, otherwise `NULL` on failures.
 */
char *termux_prefix_path(const char* log_tag, const char *termux_prefix_dir,
    const char *executable_path, char *buffer, size_t buffer_len);





/**
 * Check whether the `path` is in `termux_app_data_dir`. If path is
 * a fd path matched by `REGEX__PROC_FD_PATH`, then the real path
 * of the fd returned by `get_fd_realpath()` will be checked instead.
 *
 * **Both `path` and `termux_app_data_dir` must be normalized paths,
 * as `is_path_or_fd_path_in_dir_path()` called by this function will
 * currently not normalize either path by itself.**
 *
 * @param log_tag The log tag to use for logging.
 * @param path The `path` to check.
 * @param termux_app_data_dir The **normalized** path to termux app
 *                data directory. If `NULL`, then path returned by
 *                `get_termux_app_data_dir_from_env_or_default()`
 *                will be used by calling `get_termux_app_data_dir()`.
 * @param termux_legacy_app_data_dir The **normalized** path to termux
 *                legacy app data directory in the format
 *                `/data/data/<package_name>`. If `NULL`, then path
 *                returned by `get_termux_legacy_app_data_dir_from_env_or_default()`
 *                will be used by calling `get_termux_legacy_app_data_dir()`.
 * @return Returns `0` if `path` is in `termux_app_data_dir`, `1` if
 * `path` is not in `termux_app_data_dir`, otherwise `-1` on other
 * failures.
 */
int is_path_under_termux_app_data_dir(const char* log_tag, const char *path,
    const char *termux_app_data_dir, const char *termux_legacy_app_data_dir);





/**
 * Check whether the `path` is in `termux_rootfs_dir`. If path is
 * a fd path matched by `REGEX__PROC_FD_PATH`, then the real path
 * of the fd returned by `get_fd_realpath()` will be checked instead.
 *
 * **Both `path` and `termux_rootfs_dir` must be normalized paths, as
 * `is_path_or_fd_path_in_dir_path()` called by this function will
 * currently not normalize either path by itself.**
 *
 * @param log_tag The log tag to use for logging.
 * @param path The `path` to check.
 * @param termux_rootfs_dir The **normalized** path to termux rootfs
 *                  directory. If `NULL`, then path returned by
 *                  `get_termux_rootfs_dir_from_env_or_default()`
 *                  will be used by calling `get_termux_rootfs_dir()`.
 * @return Returns `0` if `path` is in `termux_rootfs_dir`, `1` if
 * `path` is not in `termux_rootfs_dir`, otherwise `-1` on other
 * failures.
 */
int is_path_under_termux_rootfs_dir(const char* log_tag, const char *path,
    const char *termux_rootfs_dir);

#endif // TERMUX_FILES_H

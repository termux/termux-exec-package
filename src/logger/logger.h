#ifndef LOGGER_H
#define LOGGER_H

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>

/* Log Levels */

/**
 * The `off` log level for `logMessage()` to log nothing.
 */
static const int LOG_LEVEL__OFF = 0;

/**
 * The `normal` log level for `logMessage()` to log error, warn and
 * info messages and stacktraces.
 */
static const int LOG_LEVEL__NORMAL = 1;

/**
 * The `debug` log level for `logMessage()` to log debug messages.
 */
static const int LOG_LEVEL__DEBUG = 2;

/**
 * The `verbose` log level for `logMessage()` to log verbose messages.
 */
static const int LOG_LEVEL__VERBOSE = 3;

/**
 * The `vverbose` log level for `logMessage()` to log very verbose
 * messages.
 */
static const int LOG_LEVEL__VVERBOSE = 4;

/**
 * The `vvverbose` log level for `logMessage()` to log very verbose
 * messages.
 */
static const int LOG_LEVEL__VVVERBOSE = 5;


/**
 * The minimum log level.
 */
static const int MIN_LOG_LEVEL = LOG_LEVEL__OFF;

/**
 * The maximum log level.
 */
static const int MAX_LOG_LEVEL = LOG_LEVEL__VVVERBOSE;

/**
 * The default log level.
 */
static const int DEFAULT_LOG_LEVEL = LOG_LEVEL__NORMAL;



/* Log Priorities */

/**
 * The `error` log priority for `logMessage()` to use `logVPrintError()`
 * if `sCurrentLogLevel` is greater or equal to `LOG_LEVEL__NORMAL`.
 */
static const int LOG_PRIORITY__ERROR = 0;

/**
 * The `warn` log priority for `logMessage()` to use `logVPrintWarn()`
 * if `sCurrentLogLevel` is greater or equal to `LOG_LEVEL__NORMAL`.
 */
static const int LOG_PRIORITY__WARN = 1;

/**
 * The `info` log priority for `logMessage()` to use `logVPrintInfo()`
 * if `sCurrentLogLevel` is greater or equal to `LOG_LEVEL__NORMAL`.
 */
static const int LOG_PRIORITY__INFO = 2;

/**
 * The `debug` log priority for `logMessage()` to use `logVPrintDebug()`
 * if `sCurrentLogLevel` is greater or equal to `LOG_LEVEL__DEBUG`.
 */
static const int LOG_PRIORITY__DEBUG = 3;

/**
 * The `verbose` log priority for `logMessage()` to use `logVPrintVerbose()`
 * if `sCurrentLogLevel` is greater or equal to `LOG_LEVEL__VERBOSE`.
 */
static const int LOG_PRIORITY__VERBOSE = 4;

/**
 * The `vverbose` log priority for `logMessage()` to use `logVPrintVerbose()`
 * if `sCurrentLogLevel` is greater or equal to `LOG_LEVEL__VVERBOSE`.
 */
static const int LOG_PRIORITY__VVERBOSE = 5;

/**
 * The `vvverbose` log priority for `logMessage()` to use `logVPrintVerbose()`
 * if `sCurrentLogLevel` is greater or equal to `LOG_LEVEL__VVVERBOSE`.
 */
static const int LOG_PRIORITY__VVVERBOSE = 6;



/**
 * The `message` log format mode to log entries
 * in the format `message`.
 */
static const int LOG_FORMAT_MODE__MESSAGE = 0;

/**
 * The `tag_and_message` log format mode to log entries
 * in the format `tag: message`.
 */
static const int LOG_FORMAT_MODE__TAG_AND_MESSAGE = 1;

/**
 * The `pid_priority_tag_and_message` log format mode to log entries
 * in the format `pid priority tag: message`.
 */
static const int LOG_FORMAT_MODE__PID_PRIORITY_TAG_AND_MESSAGE = 2;



/*
 * The maximum size of the log entry tag.
 */
#define LOGGER_TAG_MAX_LENGTH 23 + 1

/*
 * The maximum size of the log entry payload that can be written to
 * the logger. An attempt to write more than this amount will result
 * in a truncated log entry.
 *
 * The limit is 4068 but this includes log tag and log level
 * prefix `D/` before log tag and `: ` suffix after it.
 *
 * #define LOGGER_ENTRY_MAX_PAYLOAD 4068
 * - https://cs.android.com/android/platform/superproject/+/android-13.0.0_r54:system/logging/liblog/include/log/log.h;l=66
 */
#define LOGGER_ENTRY_MAX_PAYLOAD 4068



/** Get logger `sCurrentLogLevel`. */
int getCurrentLogLevel();

/**
 * Set logger `sCurrentLogLevel` if a valid `logLevel` is passed.
 *
 * @return Returns the new log level set.
 */
int setCurrentLogLevel(int logLevel);

/**
 * Returns `true` if `logLevel` passed is a valid log level, otherwise `false`.
 */
bool isLogLevelValid(int logLevel);

/**
 * Returns `true` if `logLevel` passed is a valid log level, otherwise `DEFAULT_LOG_LEVEL`.
 */
int getLogLevelIfValidOtherwiseDefault(int logLevel);



/**
 * Set `sDefaultLogTag` by calling `setDefaultLogTag()` and set
 * `sLogTagPrefix` by calling `setLogTagPrefix()`.
 * The `sLogTagPrefix` will be set to `defaultLogTag` followed by a dot `.`.
 */
void setDefaultLogTagAndPrefix(const char* defaultLogTag);



/** Get logger `sDefaultLogTag`. */
const char* getDefaultLogTag();
/**
 * Set `sDefaultLogTag`.
 *
 * See also `getFullLogTag()`.
 */
void setDefaultLogTag(const char* defaultLogTag);



/** Get logger `sLogTagPrefix`. */
const char* getLogTagPrefix();

/**
 * Set `sLogTagPrefix`.
 *
 * See also `getFullLogTag()`.
 */
void setLogTagPrefix(const char* logTagPrefix);



/** Get logger `sLogFormatMode`. */
int getLogFormatMode();

/**
 * Set `sLogFormatMode`.
 */
void setLogFormatMode(int logFormatMode);





/**
 * Log a message with a specific priority.
 *
 * @param logPriority The priority to log the message with. Check `LOG_PRIORITY__*` constants
 *                    to know above which `sCurrentLogLevel` they will start logging.
 * @param tag The tag with which to log after passing it to `getFullLogTag()`.
 * @param fmt The format string for the message.
 * @param args The `va_list` of the arguments for the message.
 */
void logMessage(int logPriority, const char* tag, const char* fmt, va_list args);



/* Log a message with `LOG_PRIORITY__ERROR`. */
void logError(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__WARN`. */
void logWarn(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__INFO`. */
void logInfo(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__DEBUG`. */
void logDebug(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__VERBOSE`. */
void logVerbose(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__VVERBOSE`. */
void logVVerbose(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__VVVERBOSE`. */
void logVVVerbose(const char* tag, const char* fmt, ...);



/* Log a message with `LOG_PRIORITY__ERROR` if current log level is `>=` `LOG_LEVEL__DEBUG`. */
bool logErrorDebug(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__ERROR` if current log level is `>=` `LOG_LEVEL__VERBOSE`. */
bool logErrorVerbose(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__ERROR` if current log level is `>=` `LOG_LEVEL__VVERBOSE`. */
bool logErrorVVerbose(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__ERROR` if current log level is `>=` `LOG_LEVEL__VVVERBOSE`. */
bool logErrorVVVerbose(const char* tag, const char* fmt, ...);

/* Log a private message with `LOG_PRIORITY__ERROR`. This is equivalent to `logErrorDebug*()` methods. */
bool logErrorPrivate(const char* tag, const char* fmt, ...);





/* Log a message with `LOG_PRIORITY__ERROR` with `: strerror(errno)` appended to it.
 *
 * @param errnoCode The errno to use for `strerror(errno)`. If this
                    equals `0`, then `logMessage()` will be called
                    instead to log to message without any `strerror` appended.
 * @param tag The tag with which to log after passing it to `getFullLogTag()`.
 * @param fmt The format string for the message.
 * @param args The `va_list` of the arguments for the message.
 */
void logStrerrorMessage(int errnoCode, const char* tag, const char* fmt, va_list args);

/* Log a message with `LOG_PRIORITY__ERROR` with `: strerror(errno)` appended to it. */
void logStrerror(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__ERROR` with `: strerror(errno)` appended to it if current log level is `>=` `LOG_LEVEL__DEBUG`. */
bool logStrerrorDebug(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__ERROR` with `: strerror(errno)` appended to it if current log level is `>=` `LOG_LEVEL__VERBOSE`. */
bool logStrerrorVerbose(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__ERROR` with `: strerror(errno)` appended to it if current log level is `>=` `LOG_LEVEL__VVERBOSE`. */
bool logStrerrorVVerbose(const char* tag, const char* fmt, ...);

/* Log a message with `LOG_PRIORITY__ERROR` with `: strerror(errno)` appended to it if current log level is `>=` `LOG_LEVEL__VVVERBOSE`. */
bool logStrerrorVVVerbose(const char* tag, const char* fmt, ...);

/* Log a private message with `LOG_PRIORITY__ERROR` with `: strerror(errno)` appended to it. This is equivalent to `logStrerrorDebug*()` methods. */
bool logStrerrorPrivate(const char* tag, const char* fmt, ...);

#endif // LOGGER_H

#include "cren_error.h"

#if defined(_WIN32) || (defined(__linux__) && !defined(__ANDROID__)) || (defined(__APPLE__) && defined(__MACH__))
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#elif defined(__ANDROID__)
#include <android/log.h>
#endif
#include <time.h>

/// @brief defines how many characters a log message may have (vulkan messages can be more than 1024)
#define CREN_ERRORLOG_MAX_CHARS 2048

/// @brief string-fy the log severity
/// @param severity the severity level
const char* internal_log_cstr(CRenLogSeverity severity)
{
    switch (severity)
    {
        case CRenLogSeverity_Trace: return "TRACE";
        case CRenLogSeverity_Todo: return "TODO";
        case CRenLogSeverity_Info: return "INFO";
        case CRenLogSeverity_Warn: return "WARN";
        case CRenLogSeverity_Error: return "ERROR";
        case CRenLogSeverity_Fatal: return "FATAL";
    }
    return "UNKNOWN";
}

/// @brief formats a log message into the expected layout
/// @param outBuffer the formated message
/// @param size the buffer size
/// @param localTime the time structure
/// @param file from where the log originated
/// @param line from the log originated
/// @param severity the level of severity
/// @param buffer the custom message that describes the error
static void internal_log_format(char* outBuffer, size_t size, const struct tm* localTime, const char* file, int line, CRenLogSeverity severity, const char* buffer)
{
    snprintf
    (
        outBuffer,
        size,
        "[%02d/%02d/%04d - %02d:%02d:%02d][%s - %d][%s]: %s",
        localTime->tm_mday,
        localTime->tm_mon + 1,
        localTime->tm_year + 1900,
        localTime->tm_hour,
        localTime->tm_min,
        localTime->tm_sec,
        file,
        line,
        internal_log_cstr(severity),
        buffer
    );
}

void cren_log_message(CRenLogSeverity severity, const char* file, int line, const char* fmt, ...)
{
    char buffer[CREN_ERRORLOG_MAX_CHARS];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, CREN_ERRORLOG_MAX_CHARS, fmt, args);
    va_end(args);

    char log_message[CREN_ERRORLOG_MAX_CHARS + 256];
    time_t now = time(NULL);
    struct tm* local_time = localtime(&now);

    internal_log_format(log_message, sizeof(log_message), local_time, file, line, severity, buffer);

    if (severity == CRenLogSeverity_Fatal) {
        #ifdef CREN_PLATFORM_ANDROID
        __android_log_print(ANDROID_LOG_ERROR, "CRen", log_message);
        #else
        printf("%s\n", log_message);
        #endif
        abort();
    }

    else
    {
        #ifdef CREN_PLATFORM_ANDROID
        __android_log_print(ANDROID_LOG_DEBUG, "CRen", log_message)
        #else
        printf("%s\n", log_message);
        #endif
    }
}

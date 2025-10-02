#ifndef CREN_ERROR_INCLUDED
#define CREN_ERROR_INCLUDED

#include "cren_defines.h"
#include "cren_types.h"

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief logs a message error into the platform output/console (this function is designed to be called by the library, use macros instead)
/// @param severity the severity of the error
/// @param file the file from where to throw the message
/// @param line the line from where the log was thrown
/// @fmt va_args with information about the error
CREN_API void cren_log_message(CRen_LogSeverity severity, const char* file, int line, const char* fmt, ...);

#ifdef __cplusplus 
}
#endif

/// @brief macro for assets and message logging
#ifdef CREN_LOG_DISABLED
	#define CREN_LOG(...)
	#define CREN_ASSERT(condition, msg) ((void)0)
#else
	#define CREN_LOG(severity, ...) cren_log_message(severity, __FILE__, __LINE__, __VA_ARGS__);
	#define CREN_ASSERT(condition, ...) if (!(condition)) { cren_log_message(CREN_LOG_SEVERITY_FATAL, __FILE__, __LINE__, __VA_ARGS__); }
#endif

#endif // CREN_ERROR_INCLUDED
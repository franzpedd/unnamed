#ifndef CREN_ERROR_INCLUDED
#define CREN_ERROR_INCLUDED

#include "cren_defines.h"

/// @brief all kinds of errors severity exists, a fatal error will always trigger a crash
typedef enum CRenLogSeverity
{
	CRenLogSeverity_Trace,
	CRenLogSeverity_Todo,
	CRenLogSeverity_Info,
	CRenLogSeverity_Warn,
	CRenLogSeverity_Error,
	CRenLogSeverity_Fatal
} CRenLogSeverity;

#ifdef __cplusplus 
extern "C" {
#endif

/// @brief logs a message error into the platform output/console (this function is designed to be called by the library, use macros instead)
/// @param severity the severity of the error
/// @param file the file from where to throw the message
/// @param line the line from where the log was thrown
/// @fmt va_args with information about the error
CREN_API void cren_log_message(CRenLogSeverity severity, const char* file, int line, const char* fmt, ...);

#ifdef __cplusplus 
}
#endif

/// @brief macro for assets and message logging
#ifdef CREN_LOG_DISABLED
	#define CREN_LOG(...)
	#define CREN_ASSERT(condition, msg) ((void)0)
#else
	#define CREN_LOG(severity, ...) cren_log_message(severity, __FILE__, __LINE__, __VA_ARGS__);
	#define CREN_ASSERT(condition, msg) do { if (!(condition)) { cren_log_message(CRenLogSeverity_Fatal, __FILE__, __LINE__, msg); } } while (0);	
#endif

#endif // CREN_ERROR_INCLUDED
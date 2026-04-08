#ifndef _DEFINITIONS_H_
#define _DEFINITIONS_H_

/* TODO: Move STATIC to a dedicated common defs header (not logging). */
#ifdef UNIT_TESTING
#define STATIC
#else
#define STATIC static
#endif

/* 
 * Logging
 */

#define LOG_LEVEL_FATAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARN 3
#define LOG_LEVEL_INFO 4
#define LOG_LEVEL_DEBUG 5
#define LOG_LEVEL_TRACE 6

#ifndef DEBUG_LEVEL
#define DEBUG_LEVEL LOG_LEVEL_TRACE
#endif

#if defined(SERIAL_DEBUG)
void serialprintf(const char *fmt, ...);
#endif

#if defined(SERIAL_DEBUG) && DEBUG_LEVEL >= LOG_LEVEL_FATAL
#define LOG_FATAL(x) serialprintf("[FATAL]"),serialprintf x
#else
#define LOG_FATAL(x)
#endif

#if defined(SERIAL_DEBUG) && DEBUG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(x) serialprintf("[ERROR]"),serialprintf x
#else
#define LOG_ERROR(x)
#endif

#if defined(SERIAL_DEBUG) && DEBUG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN(x) serialprintf("[WARN ]"),serialprintf x
#else
#define LOG_WARN(x)
#endif

#if defined(SERIAL_DEBUG) && DEBUG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(x) serialprintf("[INFO ]"),serialprintf x
#else
#define LOG_INFO(x)
#endif

#if defined(SERIAL_DEBUG) && DEBUG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(x) serialprintf("[DEBUG]"),serialprintf x
#else
#define LOG_DEBUG(x)
#endif

#if defined(SERIAL_DEBUG) && DEBUG_LEVEL >= LOG_LEVEL_TRACE
#define LOG_TRACE(x) serialprintf("[TRACE]"),serialprintf x
#else
#define LOG_TRACE(x)
#endif

#endif /* _DEFINITIONS_H_ */

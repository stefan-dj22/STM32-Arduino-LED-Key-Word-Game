#ifndef __DEBUG_LOG_H
#define __DEBUG_LOG_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>

// Debug log levels
typedef enum {
    DBG_NONE = 0,
    DBG_ERROR,
    DBG_WARN,
    DBG_INFO,
    DBG_DEBUG
} DebugLevel_t;

// Debug categories
typedef enum {
    DBG_CAT_NONE = 0,
    DBG_CAT_BUTTON,
    DBG_CAT_COMMAND,
    DBG_CAT_TIMER,
    DBG_CAT_ALL
} DebugCategory_t;

// Configuration
#define DEBUG_ENABLED 1  // Set to 0 to disable all debug output
#define DEBUG_LEVEL DBG_DEBUG
#define DEBUG_CATEGORY DBG_CAT_ALL

// Debug macros
#if DEBUG_ENABLED
    #define DBG_LOG(level, cat, fmt, ...) \
        if (level <= DEBUG_LEVEL && cat <= DBG_CAT_ALL ) { \
            printf("[%s][%s] " fmt "\n", \
                level == DBG_ERROR ? "ERROR" : \
                level == DBG_WARN ? "WARN" : \
                level == DBG_INFO ? "INFO" : "DEBUG", \
                cat == DBG_CAT_BUTTON ? "BTN" : \
                cat == DBG_CAT_COMMAND ? "CMD" : \
                cat == DBG_CAT_TIMER ? "TMR" : "ALL", \
                ##__VA_ARGS__); \
        }
#else
    #define DBG_LOG(level, cat, fmt, ...)
#endif
// Convenience macros for different log levels
#define DBG_ERROR(cat, fmt, ...) DBG_LOG(DBG_ERROR, cat, fmt, ##__VA_ARGS__)
#define DBG_WARN(cat, fmt, ...)  DBG_LOG(DBG_WARN, cat, fmt, ##__VA_ARGS__)
#define DBG_INFO(cat, fmt, ...)  DBG_LOG(DBG_INFO, cat, fmt, ##__VA_ARGS__)
#define DBG_DEBUG(cat, fmt, ...) DBG_LOG(DBG_DEBUG, cat, fmt, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif

#endif /* __DEBUG_LOG_H */ 

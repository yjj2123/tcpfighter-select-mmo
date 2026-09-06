#pragma once

#include <cstdio>

#define dfLOG_LEVEL_DEBUG   0
#define dfLOG_LEVEL_ERROR   1
#define dfLOG_LEVEL_SYSTEM  2

extern int g_iLogLevel;

#define _LOG(LogLevel, fmt, ...)                            \
    do {                                                    \
        if (g_iLogLevel <= (LogLevel))                      \
        {                                                   \
            std::printf("[L%d] " fmt "\n",                  \
                (LogLevel), __VA_ARGS__);                   \
        }                                                   \
    } while (0)

#define _LOG0(LogLevel, text)                               \
    do {                                                    \
        if (g_iLogLevel <= (LogLevel))                      \
        {                                                   \
            std::printf("[L%d] " text "\n", (LogLevel));    \
        }                                                   \
    } while (0)

#define LOG(fmt, ...)   _LOG(dfLOG_LEVEL_DEBUG, fmt, __VA_ARGS__)
#define LOG0(text)      _LOG0(dfLOG_LEVEL_DEBUG, text)

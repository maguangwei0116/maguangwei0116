/*******************************************************************************
 * Copyright (c) 2009, 2013 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Ian Craggs - updates for the async client
 *******************************************************************************/

#if !defined(LOG_H)
#define LOG_H
#include "MQTTClient.h"
/*BE
map LOG_LEVELS
{
    "TRACE_MAXIMUM" 1
    "TRACE_MEDIUM" 2
    "TRACE_MINIMUM" 3
    "TRACE_PROTOCOL" 4

    "ERROR" 5
    "SEVERE" 6
    "FATAL" 7
}
BE*/

enum LOG_LEVELS {
    TRACE_MAXIMUM = 1,
    TRACE_MEDIUM,
    TRACE_MINIMUM,
    TRACE_PROTOCOL,
    LOG_ERROR,
    LOG_SEVERE,
    LOG_FATAL,
} Log_levels;


/*BE
def trace_settings_type
{
   n32 map LOG_LEVELS "trace_level"
   n32 dec "max_trace_entries"
   n32 dec "trace_output_level"
}
BE*/
typedef struct
{
    int trace_level;            /**< trace level */
    int max_trace_entries;      /**< max no of entries in the trace buffer */
    int trace_output_level;     /**< trace level to output to destination */
} trace_settings_type;

//extern trace_settings_type trace_settings;

#define LOG_PROTOCOL TRACE_PROTOCOL
#define TRACE_MAX TRACE_MAXIMUM
#define TRACE_MIN TRACE_MINIMUM
#define TRACE_MED TRACE_MEDIUM

typedef struct
{
    const char* name;
    const char* value;
} Log_nameValue;

#define REDTEA_MQTT_LOG_ON  1 /* set mqtt log switch status */

int Log_initialize(Log_nameValue*);
void Log_terminate();
#ifdef REDTEA_MQTT_LOG_ON
void _Log(const char *file, int line, int log_level, int msgno, char* format, ...);
#define __FILENAME__                    (strrchr("/"__FILE__, '/') + 1)
#define Log(level, no, format, ...)     _Log(__FILENAME__, __LINE__, level, no, format, ##__VA_ARGS__)
#else
void Log(int, int, char *, ...);
#endif
void Log_stackTrace(int, int, int, int, const char*, int, int*);
typedef void Log_traceCallback(enum LOG_LEVELS level, char* message);
void Log_setTraceCallback(Log_traceCallback* callback);
DLLExport void Log_setTraceLevel(enum LOG_LEVELS level);

#endif

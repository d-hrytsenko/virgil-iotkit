#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <assert.h>

#include <logger.h>
#include <logger_implement.h>

#define ASSERT assert
#define EOL "\r\n"

static vs_log_level_t _log_level = VS_LOGLEV_UNKNOWN;
static bool _use_heap_buffer = false;
static size_t _max_buf_size = 0;

/******************************************************************************/
bool
vs_logger_init(vs_log_level_t log_level, bool use_heap_buffer, size_t max_buf_size) {
    vs_logger_set_loglev(log_level);

    _use_heap_buffer = use_heap_buffer;
    _max_buf_size = max_buf_size;
    if (!max_buf_size) {
        return false;
    }

    return true;
}

/******************************************************************************/
vs_log_level_t
vs_logger_set_loglev(vs_log_level_t new_level) {
    vs_log_level_t prev_level = _log_level;

    _log_level = new_level;

    return prev_level;
}

/******************************************************************************/
vs_log_level_t
vs_logger_get_loglev(void) {
    return _log_level;
}

/******************************************************************************/
bool
vs_logger_is_loglev(vs_log_level_t level) {

    ASSERT(_log_level != VS_LOGLEV_UNKNOWN);

    return level <= _log_level;
}

/******************************************************************************/
static const char *
_get_level_str(vs_log_level_t log_level) {

    switch (log_level) {
    case VS_LOGLEV_INFO:
        return "INFO";
    case VS_LOGLEV_FATAL:
        return "FATAL";
    case VS_LOGLEV_ALERT:
        return "ALERT";
    case VS_LOGLEV_CRITICAL:
        return "CRITICAL";
    case VS_LOGLEV_ERROR:
        return "ERROR";
    case VS_LOGLEV_WARNING:
        return "WARNING";
    case VS_LOGLEV_NOTICE:
        return "NOTICE";
    case VS_LOGLEV_TRACE:
        return "TRACE";
    case VS_LOGLEV_DEBUG:
        return "DEBUG";

    default:
        ASSERT(false && "Unsupported logging level");
        return "";
    }
}

/******************************************************************************/
static bool
_output_preface(vs_log_level_t level, const char *cur_filename, size_t line_num) {
    int str_size;
    const char *level_str = NULL;
    int snprintf_res;
    bool res = false;

    level_str = _get_level_str(level);

    // Output time string
    if (!vs_logger_output_time()) {
        return false;
    }

    // Calculate preface string size
    // TODO : snprintf - since C99
    if (!cur_filename || !line_num) {
        str_size = snprintf(NULL, 0, " [%s] ", level_str) + 1;
    } else {
        str_size = snprintf(NULL, 0, " [%s] [%s:%d] ", level_str, cur_filename, (int)line_num) + 1;
    }

    ASSERT(str_size > 0 && str_size <= _max_buf_size);

    // TODO : VAL, variable not at the function begin - since C99
    char stack_buf[str_size];

    // TODO : snprintf - since C99
    if (!cur_filename || !line_num) {
        snprintf_res = snprintf(stack_buf, str_size, " [%s] ", level_str);
    } else {
        snprintf_res = snprintf(stack_buf, str_size, " [%s] [%s:%d] ", level_str, cur_filename, (int)line_num);
    }

    if (snprintf_res < 0) {
        ASSERT(false);
        return false;
    }

    // Output string
    res = vs_logger_implement(stack_buf);

    return res;
}

/******************************************************************************/
bool
vs_logger_message(vs_log_level_t level, const char *cur_filename, size_t line_num, const char *format, ...) {
    static const char *CUTTED_STR = "...";
    static const size_t CUTTED_STR_SIZE = 3;
    va_list args1;
    va_list args2;
    int str_size;
    char *output_str = NULL;
    size_t stack_buf_size = 0;
    int snprintf_res;
    bool res = true;
    bool cutted_str = false;

    if (!vs_logger_is_loglev(level)) {
        return true;
    }

    ASSERT(cur_filename);
    ASSERT(format);

    if (!_output_preface(level, cur_filename, line_num)) {
        return false;
    }

    // Calculate string size
    va_start(args1, format);
    va_copy(args2, args1);

    str_size = vsnprintf(NULL, 0, format, args1) /* format ... */ + 1;

    va_end(args1);

    ASSERT(str_size > 0);

    if (str_size > _max_buf_size) {
        str_size = _max_buf_size;
    }

    // Allocate heap or stack buffer
    if (!_use_heap_buffer) {
        stack_buf_size = str_size;
    }

    // TODO : VAL, variable not at the function begin - since C99
    char stack_buf[stack_buf_size];
    output_str = stack_buf;

    if (_use_heap_buffer) {
        output_str = malloc(str_size);
    }

    // Make full string

    // TODO : vsnprintf - since C99
    snprintf_res = vsnprintf(output_str, str_size, format, args2);

    if (snprintf_res >= 0 && snprintf_res >= str_size) {
        strcpy(output_str + snprintf_res + str_size - (CUTTED_STR_SIZE + 1 /* '\0' */), CUTTED_STR);

        cutted_str = true;
    } else if (snprintf_res < 0) {
        res = false;
    }

    va_end(args2);

    // Output string
    if (res) {
        res &= vs_logger_implement(output_str);
    }

    // EOL
    if (res) {
        res &= vs_logger_implement(EOL);
    }

    if (_use_heap_buffer) {
        free(output_str);
    }

    return res && !cutted_str;
}

/******************************************************************************/
bool
vs_logger_message_hex(vs_log_level_t level,
                      const char *cur_filename,
                      size_t line_num,
                      const char *prefix,
                      const void *data_buf,
                      const size_t data_size) {
    char buf[3];    // "%02X"
    unsigned char *cur_byte;
    size_t pos;
    bool res;

    ASSERT(prefix);
    ASSERT(data_buf && data_size);

    if (!vs_logger_is_loglev(level)) {
        return true;
    }

    if (!_output_preface(level, cur_filename, line_num)) {
        return false;
    }

    res = vs_logger_implement(prefix);

    if(res) {
        cur_byte = (unsigned char *)data_buf;
        for (pos = 0; pos < data_size && res; ++pos, ++cur_byte) {
            sprintf(buf, "%02X", *cur_byte);
            res = vs_logger_implement(buf);
        }
    }

    if(res){
        res = vs_logger_implement(EOL);
    }

    return res;
}
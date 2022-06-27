#pragma once

#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_


#include <stdio.h>
#include <string.h>
#include <limits.h>

#ifndef _MSC_VER
#include <stdint.h>
#include <unistd.h>
#endif

#define LOG_EXCEPT_LEVEL(EXCEPT, LEVEL, PREFIX, ...)  throw EXCEPT(std::string(LOG_MAKE(LEVEL, PREFIX, ##__VA_ARGS__)))
#define LOG_EXCEPT(EXCEPT, ...)  LOG_EXCEPT_LEVEL(EXCEPT, LOG_LEVEL_ERROR, "E:", ##__VA_ARGS__)
#define LOG_CALLSTACK(EXCEPT_TYPE, ...)  LOG_PRINT_CALLSTACK(); LOG_EXCEPT(EXCEPT_TYPE, ##__VA_ARGS__)
#define LOG_RUNTIME(...)  LOG_CALLSTACK(std::runtime_error, ##__VA_ARGS__)


#ifndef EXTERN_C
#ifdef __cplusplus
#define EXTERN_C_BEGIN \
               extern "C" {
#define EXTERN_C_END \
               }
#define EXTERN_C extern "C"
#else
#define EXTERN_C_BEGIN
#define EXTERN_C_END
#define EXTERN_C
#endif
#endif


#ifndef TO_STR
#define TO_STR2(ARG) #ARG
#define TO_STR(ARG) TO_STR2(ARG)
#endif



#ifndef ASSERT

#if defined(__cplusplus) && defined(__cpp_exceptions) && defined(__unix__)
#include <stdexcept>

// Выброс исключения на PC для тестирования нештатной ситуации
#define ASSERT_EXCEPTION(text) do { LOG_PRINT_CALLSTACK(); throw std::invalid_argument(text); } while(0)

#else

// На микроконтроллере только выход
#define ASSERT_EXCEPTION(text) do { LOG_PRINT_CALLSTACK(); LOG_ABORT(text); exit(1); } while(0)
#endif

#if defined(DEBUG)
#define ASSERT(condition)   do {\
                                if(!(condition)) { \
                                    ASSERT_EXCEPTION(TO_STR(condition) " (" __FILE__ ":" TO_STR(__LINE__) ")"); \
                                }\
                            } while(0)
#else
#define ASSERT(condition)
#endif

#endif



#ifndef VERIFY
#ifdef NDEBUG
#define VERIFY(exp) (void)(exp)
#else
#define VERIFY(exp) ASSERT(exp)
#endif
#endif


#ifndef _UNUSED
#define _UNUSED(v) ((void)(v))
#endif

#ifndef SCOPE
#ifdef UNITTEST
#define SCOPE(scope) public
#else
#define SCOPE(scope) scope
#endif
#endif


#if defined(__cplusplus)

#include <iostream>
#include <string>

#include "mcucpp/static_assert.h"
std::string BinToHex(const uint8_t * buffer, const size_t size);
std::string HexStrToBinStr(std::string & hex_str);
#endif

EXTERN_C uint8_t HexToByte(const char c);
EXTERN_C size_t HexToBin(const char * str, uint8_t * buffer, const size_t size);
EXTERN_C int HexToBinEq(const char * str, const uint8_t * buffer, const size_t size);
EXTERN_C size_t BinToHexBuffer(const uint8_t * buffer, const size_t size, char * str, const size_t str_size);


#ifndef LOG_PRINTF_FUNCTION
#define LOG_PRINTF_FUNCTION log_printf

#ifdef __GNUC__
EXTERN_C const char * log_printf(uint8_t level, char const *prefix, char const *file, int line, char const *format, ...)
__attribute__ ((format(printf, 5, 6)));
#else
EXTERN_C const char * log_printf(uint8_t level, char const *prefix, char const *file, int line, char const *format, ...);
#endif

#endif

#if defined(USE_HAL_DRIVER) || defined(_WIN32)
#define LOG_PRINT_CALLSTACK()
#else
EXTERN_C void log_print_callstack();
#define LOG_PRINT_CALLSTACK() if(utils::Logger::Instance()->GetLogLevel()>=LOG_LEVEL_DEBUG && utils::Logger::Instance()->GetPrintCallstack()){ log_print_callstack(); }
#endif

#define LOG_LEVEL_DEFAULT   0

#define LOG_LEVEL_DUMP      7
#define LOG_LEVEL_DEBUG     6
#define LOG_LEVEL_INFO      5
#define LOG_LEVEL_WARNING   4
#define LOG_LEVEL_ERROR     3
#define LOG_LEVEL_FAULT     2
#define LOG_LEVEL_ABORT     1

/**
 * Максимально возможный уровень логирования для конкретно сборки
 * Все сообщения с большим уровнем удаляются на этапе компиляции исходников
 */
#ifndef LOG_LEVEL_MAX
#ifdef NDEBUG
#define LOG_LEVEL_MAX LOG_LEVEL_INFO
#else
#ifdef USE_HAL_DRIVER
#define LOG_LEVEL_MAX LOG_LEVEL_DEBUG
#else
#define LOG_LEVEL_MAX LOG_LEVEL_DUMP
#endif
#endif
#endif

/**
 * Уровень логирования по умолчанию при запуске приложения
 * Его можно повысить до уровня не выше @ref LOG_LEVEL_MAX
 */
#ifndef LOG_LEVEL_NORMAL
#ifdef NDEBUG
#define LOG_LEVEL_NORMAL LOG_LEVEL_ERROR
#else
#define LOG_LEVEL_NORMAL LOG_LEVEL_INFO
#endif
#endif

#ifndef NDEBUG
#define LOG_MAKE(level, prefix, ...) LOG_PRINTF_FUNCTION(level, prefix, __FILE__, __LINE__, ##__VA_ARGS__)
#else
#define LOG_MAKE(level, prefix, ...) LOG_PRINTF_FUNCTION(level, prefix, NULL, 0, ##__VA_ARGS__)
#endif

#if LOG_LEVEL_MAX == LOG_LEVEL_DUMP
#define LOG_DUMP(...)   LOG_MAKE(LOG_LEVEL_DUMP, "DUMP:", ##__VA_ARGS__)
#define LOG_DEBUG(...)   LOG_MAKE(LOG_LEVEL_DEBUG, "D:", ##__VA_ARGS__)
#define LOG_INFO(...)  LOG_MAKE(LOG_LEVEL_INFO, NULL, ##__VA_ARGS__)
#define LOG_WARNING(...)  LOG_MAKE(LOG_LEVEL_WARNING, "W:", ##__VA_ARGS__)
#define LOG_ERROR(...)   LOG_MAKE(LOG_LEVEL_ERROR, "E:", ##__VA_ARGS__)
#define LOG_FAULT(...)  LOG_MAKE(LOG_LEVEL_FAULT, "F:", ##__VA_ARGS__)
#define LOG_ABORT(...)  LOG_MAKE(LOG_LEVEL_ABORT, "A:", ##__VA_ARGS__)

#elif LOG_LEVEL_MAX == LOG_LEVEL_DEBUG
#define LOG_DUMP(...)
#define LOG_DEBUG(...)   LOG_MAKE(LOG_LEVEL_DEBUG, "D:", ##__VA_ARGS__)
#define LOG_INFO(...)  LOG_MAKE(LOG_LEVEL_INFO, NULL, ##__VA_ARGS__)
#define LOG_WARNING(...)  LOG_MAKE(LOG_LEVEL_WARNING, "W:", ##__VA_ARGS__)
#define LOG_ERROR(...)   LOG_MAKE(LOG_LEVEL_ERROR, "E:", ##__VA_ARGS__)
#define LOG_FAULT(...)  LOG_MAKE(LOG_LEVEL_FAULT, "F:", ##__VA_ARGS__)
#define LOG_ABORT(...)  LOG_MAKE(LOG_LEVEL_ABORT, "A:", ##__VA_ARGS__)

#elif LOG_LEVEL_MAX == LOG_LEVEL_INFO
#define LOG_DUMP(...)
#define LOG_DEBUG(...)   
#define LOG_INFO(...)  LOG_MAKE(LOG_LEVEL_INFO, NULL, ##__VA_ARGS__)
#define LOG_WARNING(...)  LOG_MAKE(LOG_LEVEL_WARNING, "W:", ##__VA_ARGS__)
#define LOG_ERROR(...)   LOG_MAKE(LOG_LEVEL_ERROR, "E:", ##__VA_ARGS__)
#define LOG_FAULT(...)  LOG_MAKE(LOG_LEVEL_FAULT, "F:", ##__VA_ARGS__)
#define LOG_ABORT(...)  LOG_MAKE(LOG_LEVEL_ABORT, "A:", ##__VA_ARGS__)

#else
#error Define LOG_LEVEL_MAX value LOG_INFO or higher
#endif

#if defined(__cplusplus)

namespace utils {

class Logger {
public:
    typedef uint8_t LogLevelType;
    typedef void FuncCallback(void *param, LogLevelType level, const char * str, bool flush);

    inline LogLevelType GetLogLevel() {
        return m_level;
    }

    inline LogLevelType GetLogLevelNormal() {
        return LOG_LEVEL_NORMAL;
    }

    inline LogLevelType SetLogLevel(const LogLevelType level) {
        LogLevelType prev_level = m_level;
        if (level >= LOG_LEVEL_ABORT && level <= LOG_LEVEL_MAX) {
            m_level = level;
        } else {
            m_level = LOG_LEVEL_NORMAL;
        }
        return prev_level;
    }
    
    inline bool SetPrintCallstack(bool enable) {
        bool prev = m_print_callstack;
        m_print_callstack = enable;
        return prev;
    }
    inline bool GetPrintCallstack() {
        return m_print_callstack;
    }

    static void PrintfCallback(void *param, LogLevelType level, const char * str, bool flush) {
        _UNUSED(param);
        _UNUSED(level);
        fprintf(stdout, "%s", str);
        if (flush) {
            fflush(stdout);
        }
    }

    void SetCallback(FuncCallback * func, void * param) {
        m_func = func;
        m_func_param = param;
    }

    void SaveCallback(FuncCallback *&func, void * &param) {
        func = m_func;
        param = m_func_param;
    }

    void Clear();
    const char * AddString(LogLevelType level, char const *string, bool flush);
    static const char * GetLogLevelDesc(LogLevelType level);

    uint16_t GetDump(uint8_t *data, uint16_t max_size);
    uint16_t GetDumpSize();

    static inline Logger * Instance() {
        // Шаблон с определением static Logger m_instance; собирается с warning в MinGW
        if (m_instance == nullptr) {
            m_instance = new Logger();
        }
        return m_instance;
    }
private:

    Logger() {
        m_level = LOG_LEVEL_NORMAL;
#ifdef USE_HAL_DRIVER
        STATIC_ASSERT(LOG_LEVEL_MAX < LOG_LEVEL_DUMP);
        STATIC_ASSERT(LOG_LEVEL_NORMAL <= LOG_LEVEL_DEBUG);
        m_func = nullptr;
#else
        m_func = &PrintfCallback;
#endif
        m_func_param = nullptr;
        m_print_callstack = false;
    }
    Logger(const Logger&) = delete;
    const Logger& operator=(const Logger&) = delete;

    virtual ~Logger() {
    }
    static Logger * m_instance;
    LogLevelType m_level;
    FuncCallback *m_func;
    void * m_func_param;
    bool m_print_callstack;
};
}
#endif

#endif //UTILS_LOGGER_H_


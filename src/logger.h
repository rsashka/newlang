#pragma once

#ifndef UTILS_LOGGER_H_
#define UTILS_LOGGER_H_


#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <iostream>
#include <string>

#ifndef _MSC_VER
#include <stdint.h>
#include <unistd.h>
#endif

#define LOG_EXCEPT_LEVEL(EXCEPT, LEVEL, PREFIX, ...)  throw EXCEPT(std::string(LOG_MAKE(LEVEL, PREFIX, ##__VA_ARGS__)))
#define LOG_EXCEPT(EXCEPT, ...)  LOG_EXCEPT_LEVEL(EXCEPT, LOG_LEVEL_ERROR, "E:", ##__VA_ARGS__)
#define LOG_CALLSTACK(EXCEPT_TYPE, ...)  LOG_PRINT_CALLSTACK(); LOG_EXCEPT(EXCEPT_TYPE, ##__VA_ARGS__)
#define LOG_COREDUMP(...)  LOG_CALLSTACK(std::runtime_error, ##__VA_ARGS__)

#define LOG_RUNTIME(format, ...)  LOG_EXCEPT(newlang::Error, format, ##__VA_ARGS__)
#define LOG_CUSTOM_ERROR(error, format, ...)  LOG_EXCEPT_LEVEL(error, LOG_LEVEL_ERROR, "", format, ##__VA_ARGS__ )




#ifdef BUILD_UNITTEST
// Определения только для юнит-тестов
#define LOG_TEST(...)   LOG_MAKE(LOG_LEVEL_DEBUG, "T:", ##__VA_ARGS__)
#define SCOPE(scope) public
#else
// Исключаются при обычной сборке
#define LOG_TEST(...)
#define SCOPE(scope) scope
#endif



#ifndef TO_STR
#define TO_STR2(ARG) #ARG
#define TO_STR(ARG) TO_STR2(ARG)
#endif

// Use: #pragma message WARNING("My message")
#ifdef _MSC_VER
#define FILE_LINE_LINK __FILE__ "(" TO_STR(__LINE__) ") : "
#define WARNING(exp) (FILE_LINE_LINK "WARNING: " exp)
#else//__GNUC__ - may need other defines for different compilers
#define WARNING(exp) ("WARNING: " exp)
#endif

#define STATIC_ASSERT(expr) static_assert((expr), #expr)

#ifndef ASSERT

#if defined(__cplusplus) && defined(__cpp_exceptions) && defined(__unix__)
#include <stdexcept>

// Выброс исключения на PC для тестирования нештатной ситуации
#define ASSERT_EXCEPTION(text) do { LOG_PRINT_CALLSTACK(); throw std::invalid_argument(text); } while(0)

#else

// На микроконтроллере только выход
#define ASSERT_EXCEPTION(text) do { LOG_PRINT_CALLSTACK(); LOG_ABORT(text); exit(1); } while(0)
#endif

#if defined(BUILD_DEBUG)
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


std::string BinToHex(const uint8_t * buffer, const size_t size);
std::string HexStrToBinStr(std::string & hex_str);

uint8_t HexToByte(const char c);
size_t HexToBin(const char * str, uint8_t * buffer, const size_t size);
int HexToBinEq(const char * str, const uint8_t * buffer, const size_t size);
size_t BinToHexBuffer(const uint8_t * buffer, const size_t size, char * str, const size_t str_size);


#if defined(_WIN32)
#define LOG_PRINT_CALLSTACK()
#else
void log_print_callstack();
#define LOG_PRINT_CALLSTACK() if(newlang::Logger::Instance()->GetLogLevel()>=LOG_LEVEL_DEBUG && newlang::Logger::Instance()->GetPrintCallstack()){ log_print_callstack(); }
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
 * Все сообщения с бОльшим уровнем удаляются на этапе компиляции исходников
 */
#ifndef LOG_LEVEL_MAX
#ifdef NDEBUG
#define LOG_LEVEL_MAX LOG_LEVEL_INFO
#else
#define LOG_LEVEL_MAX LOG_LEVEL_DUMP
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

#define LOG_PRINTF_FUNCTION  newlang::Logger::log_printf

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


namespace newlang {

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

        static void PrintfCallback(void *, LogLevelType, const char * str, bool flush) {
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

        static std::string log_printf(uint8_t level, char const *prefix, char const *file, int line, char const *format, ...)
#ifdef __GNUC__
        __attribute__ ((format(printf, 5, 6)))
#endif
        ;

        const char * AddString(LogLevelType level, char const *string, bool flush);
        static const char * GetLogLevelDesc(LogLevelType level);

        static inline Logger * Instance() {
            // Шаблон с определением static Logger m_instance; собирается с warning в MinGW
            if (m_instance == nullptr) {
                m_instance = new Logger();
            }
            return m_instance;
        }
        //    private:

        Logger() {
            m_level = LOG_LEVEL_NORMAL;
            m_func = &PrintfCallback;
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

#endif //UTILS_LOGGER_H_


#include <stdio.h>
#include <stdarg.h>
#include <string.h>


#include <contrib/logger/logger.h>

#include <core/warning_push.h>
#include <contrib/logger/mcucpp/ring_buffer.h>
#include <core/warning_pop.h>


using namespace utils;

#ifndef LOG_BUFFER_SIZE
#ifdef USE_HAL_DRIVER

#ifdef NDEBUG
#define LOG_BUFFER_SIZE 4096
#else
#define LOG_BUFFER_SIZE 4096*2
#endif

#else
#define LOG_BUFFER_SIZE 4096*4
#endif
#endif

static Mcucpp::Containers::RingBufferPO2<LOG_BUFFER_SIZE, char> m_log_buffer;
Logger * Logger::m_instance = nullptr;

const char * Logger::AddString(LogLevelType level, const char * str, bool flush) {
    if(m_func != nullptr) {
        (*m_func)(m_func_param, level, str, flush);
    }
    if(m_log_buffer.full()) {
        m_log_buffer.pop_front();
    }
    const char *result = nullptr;
    if(m_log_buffer.empty()) {
        result = &m_log_buffer[0];
    } else {
        result = &m_log_buffer[m_log_buffer.size() - 1];
    }
    while(*str) {
        m_log_buffer.push_back(*str);
        if(m_log_buffer.full()) {
            m_log_buffer.pop_front();
        }
        str++;
    }
    return result;
}

void Logger::Clear() {
    m_log_buffer.clear();
}

const char * Logger::GetLogLevelDesc(LogLevelType level) {
    switch(level) {
        case LOG_LEVEL_DEFAULT:
            return "DEFAULT";
        case LOG_LEVEL_DUMP:
            return "DUMP";
        case LOG_LEVEL_DEBUG:
            return "DEBUG";
        case LOG_LEVEL_INFO:
            return "INFO";
        case LOG_LEVEL_WARNING:
            return "WARNING";
        case LOG_LEVEL_ERROR:
            return "ERROR";
        case LOG_LEVEL_FAULT:
            return "FAULT";
        case LOG_LEVEL_ABORT:
            return "ABORT";
    }
    return "Unknown";
}

uint16_t Logger::GetDump(uint8_t *data, uint16_t max_size) {
    size_t result = 0;
    if(data && max_size > 0) {
        while(!m_log_buffer.empty() && (result + 1) < max_size) {
            data[result] = m_log_buffer.front();
            m_log_buffer.pop_front();
            result++;
        }
        data[result] = '\0';
        result++;
    }
    return (uint16_t) result;
}

uint16_t Logger::GetDumpSize() {
    STATIC_ASSERT(LOG_BUFFER_SIZE <= 4096 * 4);
    return m_log_buffer.size();
}

#define LOG_MAX_BUFFER_SIZE 1024
static char buffer[LOG_MAX_BUFFER_SIZE + 1];

EXTERN_C const char * log_printf(uint8_t level, const char *prefix, const char *file, int line, const char *format, ...) {

    if(Logger::Instance()->GetLogLevel() < level) {
        return nullptr;
    }

    const char * result = nullptr;
    if(prefix) {
        snprintf(buffer, LOG_MAX_BUFFER_SIZE, "%s", prefix);
        result = Logger::Instance()->AddString(level, buffer, false);
    }

    if(level == LOG_LEVEL_ABORT) {
        //        utils::Time::TimeToString(utils::Time::GetTime(), buffer, LOG_MAX_BUFFER_SIZE);
        //        utils::Time::DateTime tmp;
        //        snprintf(buffer, LOG_MAX_BUFFER_SIZE, " %02d:%02d:%02d %02d/%02d/%04d ",
        //                    tmp.tm_hour, tmp.tm_min, tmp.tm_sec, tmp.tm_mday, tmp.tm_mon + 1, tmp.tm_year + 1900);
        const char * tmp = Logger::Instance()->AddString(level, buffer, false);
        if(!result) {
            result = tmp;
        }
    }

    va_list args;
    va_start(args, format);
    vsnprintf(buffer, LOG_MAX_BUFFER_SIZE, format, args);
    va_end(args);

    size_t size = strlen(buffer);
    bool nl = true;
    if(size >= 2) {
        // После сообщения заканчивающегося на \r\r не ставить перевод строки
        if(buffer[size - 2] == '\r' && buffer[size - 1] == '\r') {
            nl = false;
            buffer[size - 2] = '\0';
        }
    }

    Logger::Instance()->AddString(level, buffer, false);

    if(file && (level != LOG_LEVEL_INFO || Logger::Instance()->GetLogLevel() >= LOG_LEVEL_DEBUG)) {
        const char * file_name = strrchr(file, '/');
        snprintf(buffer, LOG_MAX_BUFFER_SIZE, " (%s:%d)%s", ((file_name && *file_name == '/') ? file_name + 1 : file), line, nl ? "\n" : "");

    } else {
        snprintf(buffer, LOG_MAX_BUFFER_SIZE, "%s", nl ? "\n" : "");
    }
    const char * tmp = Logger::Instance()->AddString(level, buffer, true);
    if(!result) {
        result = tmp;
    }
    return result;
}

EXTERN_C uint8_t HexToByte(const char c) {
    ASSERT(strchr("0123456789ABCDEFabcdef", c) != nullptr);
    if(c > '0' && c <= '9') {
        return c - 0x30;
    }
    if(c >= 'a' && c <= 'f') {
        return c - 0x61 + 10;
    }
    if(c >= 'A' && c <= 'F') {
        return c - 0x41 + 10;
    }
    return 0;
}

EXTERN_C size_t HexToBin(const char * str, uint8_t * buf, const size_t size) {
    ASSERT(str != nullptr);
    ASSERT(buf != nullptr);
    //    ASSERT((strlen(str) % 2) == 0);

    memset(buf, 0, size);

    size_t pos = 0;
    size_t count = std::min(strlen(str), size * 2);

    while(pos < count) {
        buf[pos / 2] = (uint8_t) ((HexToByte(str[pos]) << 4) | (HexToByte(str[pos + 1])));
        pos += 2;
    }
    return pos / 2;
}

EXTERN_C int HexToBinEq(const char * str, const uint8_t * buf, size_t size) {
    ASSERT(str);
    ASSERT(buf);
    //    ASSERT(strlen(str) % 2 == 0);

    size_t len = strlen(str);
    if(len / 2 != size) {
        return 0;
    }
    for (size_t pos = 0; pos < len; pos += 2) {
        uint8_t byte = (uint8_t) ((HexToByte(str[pos]) << 4) | (HexToByte(str[pos + 1])));

        if(byte != buf[pos / 2]) {
            return false;
        }
    }
    return 1;
}

static const char bin_to_hex_chars[] = "0123456789ABCDEF";

std::string BinToHex(const uint8_t * buf, const size_t size) {
    std::string result;
    result.resize(size * 2);
    //    const char hex[] = "0123456789ABCDEF";
    for (size_t i = 0; i < size; i++) {
        result[i * 2] = bin_to_hex_chars[buf[i] >> 4];
        result[i * 2 + 1] = bin_to_hex_chars[buf[i] & 0xf];
    }
    return result;
}

std::string HexStrToBinStr(std::string & hex_str) {
    std::string result;
    for (size_t pos = 0; pos < hex_str.size(); pos++) {
        if(pos != 0) {
            result.append(" ");
        }
        switch(hex_str[pos]) {
            case '0':
                result.append("0000");
                break;
            case '1':
                result.append("0001");
                break;
            case '2':
                result.append("0010");
                break;
            case '3':
                result.append("0011");
                break;
            case '4':
                result.append("0100");
                break;
            case '5':
                result.append("0101");
                break;
            case '6':
                result.append("0110");
                break;
            case '7':
                result.append("0111");
                break;
            case '8':
                result.append("1000");
                break;
            case '9':
                result.append("1001");
                break;
            case 'a':
            case 'A':
                result.append("1010");
                break;
            case 'b':
            case 'B':
                result.append("1011");
                break;
            case 'c':
            case 'C':
                result.append("1100");
                break;
            case 'd':
            case 'D':
                result.append("1101");
                break;
            case 'e':
            case 'E':
                result.append("1110");
                break;
            case 'f':
            case 'F':
                result.append("1111");
                break;
            default:
                result.append("????");
                LOG_ERROR("Non hex symbol %c", hex_str[pos]);
                break;
        }
    }
    return result;
}

EXTERN_C size_t BinToHexBuffer(const uint8_t * buf, const size_t size, char * str, const size_t str_size) {
    ASSERT(buf);
    ASSERT(str);
    if(!str || !buf || !size || !str_size) {
        return 0;
    }
    size_t max_size = std::min(size * 2, str_size - 1);
    for (size_t i = 0; i < size && (i * 2 < max_size); i++) {
        str[i * 2] = bin_to_hex_chars[buf[i] >> 4];
        str[i * 2 + 1] = bin_to_hex_chars[buf[i] & 0xf];
    }
    str[max_size] = '\0';
    return max_size;
}

#if !defined(USE_HAL_DRIVER) && !defined(_WIN32)

#include <execinfo.h>
#include <cxxabi.h>

inline char * basename(char * str) {
    if(str) {
        size_t pos = strlen(str);
        while(pos) {
            if(str[pos] == '\\' || str[pos] == '/') {
                return &str[pos + 1];
            }
            pos--;
        }
    }
    return str;
}

EXTERN_C void log_print_callstack() {
    const size_t max_depth = 5;
    size_t stack_depth;
    void *stack_addrs[max_depth];
    char **stack_strings;

    stack_depth = backtrace(stack_addrs, max_depth);
    stack_strings = backtrace_symbols(stack_addrs, stack_depth);

    utils::Logger::LogLevelType save_level = utils::Logger::Instance()->GetLogLevel();
    utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
    LOG_INFO("Call stack:");

    for (size_t i = 1; i < stack_depth; i++) {
        size_t sz = 200; // just a guess, template names will go much wider
        char *function = static_cast<char *> (malloc(sz));
        char *begin = 0, *end = 0;

        // find the parentheses and address offset surrounding the mangled name
        for (char *j = stack_strings[i]; *j; ++j) {
            if(*j == '(') {
                begin = j;
            } else if(*j == '+') {
                end = j;
            }
        }
        if(begin && end) {
            *begin++ = '\0';
            *end = '\0';
            // found our mangled name, now in [begin, end)

            int status;
            char *ret = abi::__cxa_demangle(begin, function, &sz, &status);
            if(ret) {
                // return value may be a realloc() of the input
                function = ret;
            } else {
                // demangling failed, just pretend it's a C function with no args
                strncpy(function, begin, sz);
                strncat(function, "()", sz);
                function[sz - 1] = '\0';
            }
            LOG_INFO("    %s:%s", basename(stack_strings[i]), basename(function));
        } else {
            // didn't find the mangled name, just print the whole line
            LOG_INFO("    %s", basename(stack_strings[i]));
        }
        free(function);
    }

    free(stack_strings);

    utils::Logger::Instance()->SetLogLevel(save_level);
}

#endif

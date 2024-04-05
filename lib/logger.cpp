#include "logger.h"

#include <stdarg.h>
#include <string.h>
#include <linux/limits.h>

using namespace newlang;

Logger * Logger::m_instance = nullptr;

const char * Logger::GetLogLevelDesc(LogLevelType level) {
    switch (level) {
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

std::string Logger::log_printf(uint8_t level, const char *prefix, const char *file, int line, const char *format, ...) {

    if (Logger::Instance()->GetLogLevel() < level) {
        return "";
    }

    std::string result;
    if (prefix) {
        result += prefix;
        //        snprintf(buffer, LOG_MAX_BUFFER_SIZE, "%s", prefix);
        //        result = Logger::Instance()->AddString(level, buffer, false);
    }

    //    if (level == LOG_LEVEL_ABORT) {
    //        //        Time::TimeToString(Time::GetTime(), buffer, LOG_MAX_BUFFER_SIZE);
    //        //        Time::DateTime tmp;
    //        //        snprintf(buffer, LOG_MAX_BUFFER_SIZE, " %02d:%02d:%02d %02d/%02d/%04d ",
    //        //                    tmp.tm_hour, tmp.tm_min, tmp.tm_sec, tmp.tm_mday, tmp.tm_mon + 1, tmp.tm_year + 1900);
    //        const char * tmp = Logger::Instance()->AddString(level, buffer, false);
    //        if (!result) {
    //            result = tmp;
    //        }
    //    }


    static char buffer[4096 + PATH_MAX];
    char *ptr = nullptr;
    size_t printf_size;

    {
        va_list args;
        va_start(args, format);
        printf_size = vsnprintf(nullptr, 0, format, args) + PATH_MAX;
        va_end(args);
    }


    if (printf_size > sizeof (buffer)) {
        ptr = new char[printf_size + PATH_MAX + 1];
        printf_size += (PATH_MAX + 1);
    } else {
        ptr = &buffer[0];
        printf_size = sizeof (buffer);
    }

    {
        va_list args;
        va_start(args, format);
        VERIFY(vsnprintf(ptr, printf_size, format, args) >= 0);
        va_end(args);
    }

    size_t size = strlen(ptr);
    bool nl = true;
    if (size >= 2) {
        // После сообщения заканчивающегося на \r\r не ставить перевод строки
        if (ptr[size - 2] == '\r' && ptr[size - 1] == '\r') {
            nl = false;
            ptr[size - 2] = '\0';
        }
    }

    result += ptr;

    if (file && (level != LOG_LEVEL_INFO || Logger::Instance()->GetLogLevel() >= LOG_LEVEL_DUMP)) {
        const char * file_name = strrchr(file, '/');
        snprintf(ptr, printf_size, " (%s:%d)%s", ((file_name && *file_name == '/') ? file_name + 1 : file), line, nl ? "\n" : "");

    } else {
        snprintf(ptr, printf_size, "%s", nl ? "\n" : "");
    }
    result += ptr;

    if (ptr != &buffer[0]) {
        delete[] ptr;
    }

    if (Logger::Instance()->m_func != nullptr) {
        (*Logger::Instance()->m_func)(Logger::Instance()->m_func_param, level, result.c_str(), true);
    }

    return result;
}

uint8_t HexToByte(const char c) {
    if (c >= '0' && c <= '9') {
        return c - 0x30;
    }
    if (c >= 'a' && c <= 'f') {
        return c - 0x61 + 10;
    }
    if (c >= 'A' && c <= 'F') {
        return c - 0x41 + 10;
    }
    LOG_RUNTIME("the symbol '%c' is not a hex digit!", c);
}

size_t HexToBin(const char * str, uint8_t * buf, const size_t size) {
    ASSERT(str);
    ASSERT(buf);
    ASSERT((strlen(str) % 2) == 0);

    memset(buf, 0, size);

    size_t pos = 0;
    size_t count = std::min(strlen(str), size * 2);

    while (pos < count) {
        buf[pos / 2] = (uint8_t) ((HexToByte(str[pos]) << 4) | (HexToByte(str[pos + 1])));
        pos += 2;
    }
    return pos / 2;
}

int HexToBinEq(const char * str, const uint8_t * buf, size_t size) {
    ASSERT(str);
    ASSERT(buf);
    ASSERT(strlen(str) % 2 == 0);

    size_t len = strlen(str);
    if (len / 2 != size) {
        return 0;
    }
    for (size_t pos = 0; pos < len; pos += 2) {
        uint8_t byte = (uint8_t) ((HexToByte(str[pos]) << 4) | (HexToByte(str[pos + 1])));

        if (byte != buf[pos / 2]) {
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
        if (pos != 0) {
            result.append(" ");
        }
        switch (hex_str[pos]) {
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

size_t BinToHexBuffer(const uint8_t * buf, const size_t size, char * str, const size_t str_size) {
    ASSERT(buf);
    ASSERT(str);
    if (!str || !buf || !size || !str_size) {
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

__attribute__((weak)) std::string Logger::GetStackTrace() {
    return "";
}
/*
std::string Logger::GetStackTrace() {
    std::string result("\n");
    llvm::raw_string_ostream stack(result);
    llvm::sys::PrintStackTrace(stack, 1);
    return result;
}
*/
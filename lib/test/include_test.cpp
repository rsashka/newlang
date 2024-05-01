#ifdef BUILD_UNITTEST

/*
 * Файл "include.h", точнее его обработанный препроцессором вариант include.h.i
 * включается перед транспилированным текстом и должен работать без дополнительных зависимостей.
 */

#include "include_h.i"
using namespace newlang;

// Добавить определния макросов из логгера,
// но без зависимостей и определний классов,
// так как они уже присуствуют в include.h.i
#define INCLUDE_INSIDE_JIT
#include "logger.h"

// Макросы использования в функции include_h_i_function_test
#define CHECK_TRUE(test)        if(!(test)){ LOG_ERROR("%s", #test); return false; }
#define CHECK_FALSE(test)       CHECK_TRUE(!test)
#define CHECK_ANY_THROW(test)   try { test; LOG_ERROR("%s", #test); return false; } catch(...){ }
#define CHECK_NO_THROW(test)    try { test; } catch(...){ LOG_ERROR("%s", #test); return false; }
#define CHECK_EQ(val1, val2)    CHECK_TRUE((val1) == (val2))

inline bool str_compare(const std::string_view str1, const std::string_view str2) {
    if (str1.compare(str2) != 0) {
        LOG_ERROR("'%s' != '%s'", str1.begin(), str2.begin());
        return false;
    }
    return true;
}

inline bool str_compare(const std::wstring_view str1, const std::wstring_view str2) {
    if (str1.compare(str2) != 0) {
        LOG_ERROR("\"%s\" != \"%s\"", utf8_encode(str1.begin()).c_str(), utf8_encode(str2.begin()).c_str());
        return false;
    }
    return true;
}
#define CHECK_STREQ(val1, val2)     CHECK_TRUE(str_compare(val1, val2))

extern "C" int float_compare(float val1, float val2);
extern "C" int double_compare(double val1, double val2);

#pragma message WARNING("Проблема со встроенными функциями или аргументами (возвратом)!!!")
//inline static int float_compare(float val1, float val2) {
//    return std::fabs(val1 - val2) < 1.19209e-07; //std::numeric_limits<float>::epsilon();
//}
//
//inline static int double_compare(double val1, double val2) {
//    return std::fabs(val1 - val2) < 2.22045e-16; //std::numeric_limits<double>::epsilon();
//}
#define CHECK_FLOAT_EQ(val1, val2)  CHECK_TRUE(float_compare((val1), (val2)))
#define CHECK_DOUBLE_EQ(val1, val2) CHECK_TRUE(double_compare((val1), (val2)))


#define PLAСE_ASM_COMMENT(MSG)  __asm__ volatile ( "; //SOURCE: " MSG)

// Замена макросов Google Test Framework в функции include_h_i_function_test
#define ASSERT_ANY_THROW(...)     PLAСE_ASM_COMMENT(#__VA_ARGS__); CHECK_ANY_THROW(__VA_ARGS__) 
#define ASSERT_NO_THROW(...)      PLAСE_ASM_COMMENT(#__VA_ARGS__); CHECK_NO_THROW(__VA_ARGS__) 
#define ASSERT_EQ(...)            PLAСE_ASM_COMMENT(#__VA_ARGS__); CHECK_EQ(__VA_ARGS__)
#define ASSERT_STREQ(...)         PLAСE_ASM_COMMENT(#__VA_ARGS__); CHECK_STREQ(__VA_ARGS__)

#pragma message WARNING("Не фурычит с аргументами!!!")
//#define ASSERT_FLOAT_EQ(arg1, arg2)      PLAСE_ASM_COMMENT(#arg1 #arg2); CHECK_FLOAT_EQ(arg1+1, arg2+1)
//#define ASSERT_DOUBLE_EQ(arg1, arg2)     PLAСE_ASM_COMMENT(#arg1 #arg2); CHECK_DOUBLE_EQ(arg1+1, arg2+1)
#define ASSERT_FLOAT_EQ(arg1, arg2)      PLAСE_ASM_COMMENT(#arg1 #arg2) 
#define ASSERT_DOUBLE_EQ(arg1, arg2)     PLAСE_ASM_COMMENT(#arg1 #arg2)

/*
 * Функиця запускается как в TEST(LLVM, GoogleTestFramework)
 * так и после компилиирования всего файла в JIT
 */
extern "C" bool include_h_i_function_test() {

    LOG_DEBUG("Call function include_h_i_function_test!!\n");

    Obj val;

    ObjPtr obj = Obj::CreateEmpty();
    val = (*obj);

    ASSERT_STREQ("_", obj->toString().c_str());
    ASSERT_ANY_THROW(auto _ = static_cast<bool> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int8_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint8_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int16_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint16_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int32_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint32_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int64_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint64_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<float> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<double> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<std::string> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<std::wstring> (*obj));

    ASSERT_STREQ("_", val.toString().c_str());
    ASSERT_ANY_THROW(auto _ = static_cast<bool> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<int8_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<uint8_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<int16_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<uint16_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<int32_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<uint32_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<int64_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<uint64_t> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<float> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<double> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<std::string> (val));
    ASSERT_ANY_THROW(auto _ = static_cast<std::wstring> (val));
    
    (*obj) = false;
    val = false;
    LOG_DEBUG("(*obj) = false;");

    ASSERT_EQ(false, static_cast<bool> (*obj));
    ASSERT_EQ(0, static_cast<int8_t> (*obj));
    ASSERT_EQ(0, static_cast<uint8_t> (*obj));
    ASSERT_EQ(0, static_cast<int16_t> (*obj));
    ASSERT_EQ(0, static_cast<uint16_t> (*obj));
    ASSERT_EQ(0, static_cast<int32_t> (*obj));
    ASSERT_EQ(0, static_cast<uint32_t> (*obj));
    ASSERT_EQ(0, static_cast<int64_t> (*obj));
    ASSERT_EQ(0, static_cast<uint64_t> (*obj));
    ASSERT_STREQ("0", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"0", static_cast<std::wstring> (*obj).c_str());
    ASSERT_FLOAT_EQ(0, static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(0, static_cast<double> (*obj));
    
    ASSERT_EQ(false, static_cast<bool> (val));
    ASSERT_EQ(0, static_cast<int8_t> (val));
    ASSERT_EQ(0, static_cast<uint8_t> (val));
    ASSERT_EQ(0, static_cast<int16_t> (val));
    ASSERT_EQ(0, static_cast<uint16_t> (val));
    ASSERT_EQ(0, static_cast<int32_t> (val));
    ASSERT_EQ(0, static_cast<uint32_t> (val));
    ASSERT_EQ(0, static_cast<int64_t> (val));
    ASSERT_EQ(0, static_cast<uint64_t> (val));
    ASSERT_STREQ("0", static_cast<std::string> (val).c_str());
    ASSERT_STREQ(L"0", static_cast<std::wstring> (val).c_str());
    ASSERT_FLOAT_EQ(0, static_cast<float> (val));
    ASSERT_DOUBLE_EQ(0, static_cast<double> (val));

    //    if(!double_compare(0, 0)){
//        LOG_RUNTIME("FAIL: double_compare0");
//    }
//    if(!double_compare(1, 1)){
//        LOG_RUNTIME("FAIL: double_compare1");
//    }
//    if(!float_compare(0, 0)){
//        LOG_RUNTIME("FAIL: float_compare0");
//    }
//    if(!float_compare(1, 1)){
//        LOG_RUNTIME("FAIL: float_compare1");
//    }

    CHECK_FLOAT_EQ(0, static_cast<float> (*obj));
    CHECK_DOUBLE_EQ(0, static_cast<double> (*obj));
    
    (*obj) = true;
    val = true;
    LOG_DEBUG("(*obj) = true;");

    ASSERT_EQ(true, static_cast<bool> (*obj));
    ASSERT_EQ(1, static_cast<int8_t> (*obj));
    ASSERT_EQ(1, static_cast<uint8_t> (*obj));
    ASSERT_EQ(1, static_cast<int16_t> (*obj));
    ASSERT_EQ(1, static_cast<uint16_t> (*obj));
    ASSERT_EQ(1, static_cast<int32_t> (*obj));
    ASSERT_EQ(1, static_cast<uint32_t> (*obj));
    ASSERT_EQ(1, static_cast<int64_t> (*obj));
    ASSERT_EQ(1, static_cast<uint64_t> (*obj));
    ASSERT_STREQ("1", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"1", static_cast<std::wstring> (*obj).c_str());
    ASSERT_DOUBLE_EQ(1, (double)static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(1, static_cast<double> (*obj));

    ASSERT_EQ(true, static_cast<bool> (val));
    ASSERT_EQ(1, static_cast<int8_t> (val));
    ASSERT_EQ(1, static_cast<uint8_t> (val));
    ASSERT_EQ(1, static_cast<int16_t> (val));
    ASSERT_EQ(1, static_cast<uint16_t> (val));
    ASSERT_EQ(1, static_cast<int32_t> (val));
    ASSERT_EQ(1, static_cast<uint32_t> (val));
    ASSERT_EQ(1, static_cast<int64_t> (val));
    ASSERT_EQ(1, static_cast<uint64_t> (val));
    ASSERT_STREQ("1", static_cast<std::string> (val).c_str());
    ASSERT_STREQ(L"1", static_cast<std::wstring> (val).c_str());
    ASSERT_FLOAT_EQ(1, static_cast<float> (val));
    ASSERT_DOUBLE_EQ(1, static_cast<double> (val));
    
    if(!double_compare(0, 0)){
        LOG_RUNTIME("FAIL: double_compare0");
    }
    if(!double_compare(1, 1)){
        LOG_RUNTIME("FAIL: double_compare1");
    }
    if(!float_compare(0, 0)){
        LOG_RUNTIME("FAIL: float_compare0");
    }
    if(!float_compare(1, 1)){
        LOG_RUNTIME("FAIL: float_compare1");
    }

//    if(!double_compare(static_cast<double> (*obj), 1)){
//        LOG_RUNTIME("FAIL: double_compare1");
//    }
//    if(!float_compare(static_cast<float> (*obj), 1)){
//        LOG_RUNTIME("FAIL: float_compare1");
//    }
//    CHECK_FLOAT_EQ(1, static_cast<float> (*obj));
//    CHECK_DOUBLE_EQ(1, static_cast<double> (*obj));

    (*obj) = -200;
    LOG_DEBUG("(*obj) = -200;");

    ASSERT_EQ(true, static_cast<bool> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int8_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint8_t> (*obj));
    ASSERT_EQ(-200, static_cast<int16_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint16_t> (*obj));
    ASSERT_EQ(-200, static_cast<int32_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint32_t> (*obj));
    ASSERT_EQ(-200, static_cast<int64_t> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<uint64_t> (*obj));
    ASSERT_STREQ("-200", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"-200", static_cast<std::wstring> (*obj).c_str());
    ASSERT_FLOAT_EQ(-200, static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(-200, static_cast<double> (*obj));

    (*obj) = 200;
    LOG_DEBUG("(*obj) = 200;");

    ASSERT_EQ(true, static_cast<bool> (*obj));
    ASSERT_ANY_THROW(auto _ = static_cast<int8_t> (*obj));
    ASSERT_EQ(200, static_cast<uint8_t> (*obj));
    ASSERT_EQ(200, static_cast<int16_t> (*obj));
    ASSERT_EQ(200, static_cast<uint16_t> (*obj));
    ASSERT_EQ(200, static_cast<int32_t> (*obj));
    ASSERT_EQ(200, static_cast<uint32_t> (*obj));
    ASSERT_EQ(200, static_cast<int64_t> (*obj));
    ASSERT_EQ(200, static_cast<uint64_t> (*obj));
    ASSERT_STREQ("200", static_cast<std::string> (*obj).c_str());
    ASSERT_STREQ(L"200", static_cast<std::wstring> (*obj).c_str());
    ASSERT_FLOAT_EQ(200, static_cast<float> (*obj));
    ASSERT_DOUBLE_EQ(200, static_cast<double> (*obj));

//    ObjPtr obj2 = Obj::CreateValue(123);
//    (*obj) += (*obj2);
//    ASSERT_EQ(323, static_cast<int16_t> (*obj));
//    ASSERT_EQ(323, static_cast<uint16_t> (*obj));
//    ASSERT_STREQ("323", static_cast<std::string> (*obj).c_str());
//    ASSERT_STREQ(L"323", static_cast<std::wstring> (*obj).c_str());
    
    
    return true;
}


#endif // BUILD_UNITTEST

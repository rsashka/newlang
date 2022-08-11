// Запуск программы: CTRL+F5 или меню "Отладка" > "Запуск без отладки"
// Отладка программы: F5 или меню "Отладка" > "Запустить отладку"

// Советы по началу работы 
//   1. В окне обозревателя решений можно добавлять файлы и управлять ими.
//   2. В окне Team Explorer можно подключиться к системе управления версиями.
//   3. В окне "Выходные данные" можно просматривать выходные данные сборки и другие сообщения.
//   4. В окне "Список ошибок" можно просматривать ошибки.
//   5. Последовательно выберите пункты меню "Проект" > "Добавить новый элемент", чтобы создать файлы кода, или "Проект" > "Добавить существующий элемент", чтобы добавить в проект существующие файлы кода.
//   6. Чтобы снова открыть этот проект позже, выберите пункты меню "Файл" > "Открыть" > "Проект" и выберите SLN-файл.

#include <Windows.h>
#include <codecvt>
#include <locale>
#include <string>
#include <vector>


#include "ffi.h"
#include <stdio.h>

unsigned char foo(
    unsigned int, float);

//#pragma comment(lib, "msys-2.0.dll")
//#pragma comment(lib, "libffi.dll.a")

inline std::string utf8_encode(const std::wstring wstr) {
    std::string utf8line;

    if (wstr.empty()) {
        return utf8line;
    }
    utf8line = std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(wstr.c_str());
    return utf8line;
}


std::string GetLastErrorMessage() {
    wchar_t buffer[256];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer),  NULL);
    return utf8_encode(buffer);
}
//std::string GetLastErrorMessage() {
//    char buffer[256];
//    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buffer, sizeof(buffer),
//                  NULL);
//    return buffer;
//}

int main(int argc, const char **argv) {
    // ffi_cif cif;
    // ffi_type *arg_types[2];
    // void *arg_values[2];
    // ffi_status status;

    //// Because the return value from foo() is smaller than sizeof(long), it
    //// must be passed as ffi_arg or ffi_sarg.
    // ffi_arg result;

    //// Specify the data type of each argument. Available types are defined
    //// in <ffi/ffi.h>.
    // arg_types[0] = &ffi_type_uint;
    // arg_types[1] = &ffi_type_float;

    //// Prepare the ffi_cif structure.
    // if ((status = ffi_prep_cif(&cif, FFI_DEFAULT_ABI, 2, &ffi_type_uint8, arg_types)) != FFI_OK) {
    //     // Handle the ffi_status error.
    // }

    //// Specify the values of each argument.
    // unsigned int arg1 = 42;
    // float arg2 = 5.1f;

    // arg_values[0] = &arg1;
    // arg_values[1] = &arg2;

    //// Invoke the function.
    // ffi_call(&cif, FFI_FN(foo), &result, arg_values);

     //The ffi_arg 'result' now contains the unsigned char returned from foo(),
     //which can be accessed by a typecast.
     //printf("result is %hhu\n", (unsigned char)result);

    // std::vector<std::wstring> dll = {L"msvcp140d.dll",
    //                                  L"vcruntime140d.dll",
    //                                  L"kernel32.dll",
    //                                  L"KernelBase.dll",
    //                                  L"vcruntime140_1d.dll", L"ucrtbased.dll",     L"ntdll.dll"};

    // HMODULE mod = 0;
    // mod = GetModuleHandle(nullptr);
    // if (!mod) {
    //     printf("Fail NULL module n");
    // } else {
    //     if (GetProcAddress(mod, "printf")) {
    //         printf("printf    NULL \n");
    //         return 0;
    //     }
    //     if (GetProcAddress(mod, "_printf")) {
    //         printf("___printf  NULL  \n");
    //         return 0;
    //     }
    // }

    // for (auto name: dll) {
    //     mod = GetModuleHandle(name.c_str());
    //     if (!mod) {
    //         printf("Fail module %s \n", utf8_encode(name).c_str());
    //     } else {
    //         if (GetProcAddress(mod, "printf")) {
    //             printf("printf    %s  \n", utf8_encode(name).c_str());
    //             return 0;
    //         }
    //         if (GetProcAddress(mod, "_printf")) {
    //             printf("___printf  %s  \n", utf8_encode(name).c_str());
    //             return 0;
    //         }
    //     }
    // }

    #pragma comment(lib, "libffi-7.dll")

    //#pragma comment(lib, "legacy_stdio_definitions.lib")
    //#pragma comment(lib, "legacy_stdio_wide_specifiers.lib")
    
    //#pragma comment(lib, "cygwin1.dll")
    //#pragma comment(lib, "cygffi-6.dll")
     HMODULE mod = LoadLibrary(L"msys-2.0.dll");
//     HMODULE mod = LoadLibrary(L"cygwin1.dll");
    
    // HMODULE mod = GetModuleHandle(nullptr);
    if (!mod) {
        printf("Fail load msys-2.0.dll %s\n", GetLastErrorMessage().c_str());
        return 1;
    }

    typedef void init_type();

//    init_type *init = (init_type *)GetProcAddress(mod, "cygwin_dll_init");
    init_type *init = (init_type *)GetProcAddress(mod, "msys_dll_init");
    if (!init) {
        printf("msys_dll_init not found!: %s\n", GetLastErrorMessage().c_str());
        return 1;
    }

    (*init)();

    void *ptr2 = nullptr;
    ptr2 = GetProcAddress(mod, "printf");
    if (!ptr2) {
        printf("printf not found!: %s\n", GetLastErrorMessage().c_str());
        return 1;
    }

    //void *ptr_local = GetProcAddress(nullptr, "printf");
    //if (!ptr_local) {
    //    printf("Local printf not found!: %s\n", GetLastErrorMessage().c_str());
    //    return 1;
    //}

    void *ptr = &printf;

    printf("%s  %p  %p\n", ptr == ptr2 ? "Eq: " : " NOT EQ !!!!!!: ", ptr, ptr2);

    typedef int printf_type(const char *, ...);

    printf_type *ttt = (printf_type *)ptr;
    (ttt)("Test 1\n");
    (ttt)("%s", "Test Variadic call\n");
    (ttt)("%s", "Начинаю тесты printf из msys-2.0.dll\n\n");

    printf_type *ttt2 = (printf_type *)ptr2;
    (ttt2)("Test 2 from dll\n");
    (ttt2)("%s", "Test 2 Variadic call form dll\n");
    (ttt2)("%s", "Тест_русских_символов\n");


    /*
    * libtorch под Windows собирается для использования только с нативным компилятором и не может быть использовавана с MinGW для корсскомпиляции.
    * VSCode (VSCodium) под Windows использует MinGW, поэтому остается использовать только Visual Studio
    * В Visual Studio функции stdio реализованы встроенными и у меня не получилось динамически получить адрес printf во время выполнения (включая _NO_CRT_STDIO_INLINE и legacy_stdio_definitions.lib).
    * Можно получить адрес printf в рантайме из msys-2.0.dll, но при попытки использования фунции внтури dlld возникает необрабатываемое исключение.
    * Остановился на получении адресо в функий в рантайме с GetProcAddress из cygwin1.dll, хотя её нужно инициализировать перед использованием, т.е. вызывать void cygwin_dll_init().
    * Без инициализации возникает такое же исключение как и при msys-2.0.dll (функцию инициализации msys-2.0.dll ненашел).
    * Функции из cygwin1.dll хоть и не виснут, но работают как-то странно и выводят только слова целиком. 
    * Возможно это связно с кодировкой исходников или другими настройками, но решил пока не заморачиваться.
    * 
    * 
    */

    
    //ffi_cif m_cif;
    //std::vector<ffi_type *> m_args_type;
    //std::vector<void *> m_args_ptr;

    //union VALUE {
    //    const void *ptr;
    //    //        ObjPtr obj;
    //    size_t size;
    //    int64_t integer;
    //    double number;
    //    bool boolean;
    //};
    //std::vector<VALUE> m_args_val;
    //VALUE temp;

    //m_args_type.push_back(&ffi_type_pointer);
    //temp.ptr = "%s";
    //m_args_val.push_back(temp);

    //m_args_type.push_back(&ffi_type_pointer);
    //temp.ptr = "test string LIBFFI\n";
    //m_args_val.push_back(temp);


    //for (size_t i = 0; i < m_args_val.size(); i++) {
    //    m_args_ptr.push_back((void *)&m_args_val[i]);
    //}

    //VALUE res_value;
    //ffi_type *result_ffi_type = &ffi_type_sint32;
    //
    //ffi_abi m_func_abi = FFI_DEFAULT_ABI;
    //if (ffi_prep_cif_var(&m_cif, m_func_abi, 1, static_cast<unsigned int>(m_args_type.size()), result_ffi_type, m_args_type.data()) == FFI_OK) {

    //    ffi_call(&m_cif, FFI_FN(ptr), &res_value, m_args_ptr.data());
    //    printf("Returned %Id", res_value.integer);

    //    return 0;
    //}
    return 1;
}

// The target function.
unsigned char foo(unsigned int x, float y) {
    unsigned char result = (unsigned char)(x - y);
    return result;
}
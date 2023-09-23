#pragma once
#ifndef INCLUDED_NLC_
#define INCLUDED_NLC_

#include "pch.h"

#include <warning_push.h>
#include <contrib/Lyra/include/lyra/lyra.hpp>
#include <warning_pop.h>

#include <autocomplete.h>
#include <newlang.h>


// * 30.04.2021
// * Компиляция - преобразование исходника в исполняемый файл в результет которого создается модуль динамической библиотеки.
// * В нем содержатся экспортируемые функции, которые определены в файле и предопреденные переменные.
// * Одна переменная - тектовая строка с исходным текстом самого файла
// * Вторая - экспортируемая головная функция - скомпилированная последовательность шагов головного файла (игнорируя определения функций, т.к. они собраны отдельно).
// *
// * Выполнение такого файла - загрузка интерпретатором и либо выполнение головной функции в пакетном режиме,
// * либо в диалоговом режиме повторная интерпретация исходного текста, пропуская определения функций (т.к. они уже скомпилированы заранее),
// * либо вызов других функций, введенных пользователем.
// * 
// * Выполнение обычного файла - компиляция временного файла модуля, а дальше его выполнение как предкомпилированного.
// * Для процедуры компиляции требуется - компиляция функций, компиляция головной функции и сохранение исходного текста модуля.
// * Для выполнение - диалоговый загрузчик, который либо загружает модуль, либо компилирует временный файл в модуль и только потом загружает.
// 
// 14.06.2021
// make - парсинг и компиляция функций и методов классов в отдельный модуль (nl, nlm ????)
// load - загрузка скомпилированного модуля + вызывается call для функции с именем модуля для его инициализации, если она присутствует в модуле
// exec - последовательное выполнение make, load и run для исходного текста программы (nlp ???)
// eval - парсинг и выполнение текста программы в режиме интерпретатора, т.е. вызовы (call) для функций и выражений из заранее загруженных модулей

namespace newlang {

#define NLC_FILE_HISTORY ".nlc_history"

    //    
    //#include <stdio.h>
    //#include <wchar.h>
    //#include <stdlib.h>
    //#include <locale.h>
    //
    //int main() {
    //    int r;
    //    wchar_t myChar1 = L'Ω';
    //    setlocale(LC_CTYPE, "");
    //    r = wprintf(L"char is %lc (%x)\n", myChar1, myChar1);
    //}
    //

    class NLC {
    public:

#define NLC_MODE(_) \
    _(ModeError,    0) \
    _(ModeHelp,     1)\
    _(ModeVersion,  2)\
    _(ModeInter,    3)\
    _(ModeEval,     4) \
    _(ModeExec,     5)

        //    _(COMPILE,  6)

        enum class Mode : uint8_t {
#define DEFINE_ENUM(name, value) name = value,
            NLC_MODE(DEFINE_ENUM)
#undef DEFINE_ENUM
        };

        inline const char* toString(Mode mode) {
#define DEFINE_CASE(name, _) \
  case Mode::name:     \
    return #name;

            switch (mode) {
                    NLC_MODE(DEFINE_CASE)
                default:
                    LOG_ERROR("UNKNOWN MODE %d", static_cast<int> (mode));
                    return "UNKNOWN MODE";
            }
#undef DEFINE_CASE
        }


        std::string m_path; //< Имя исполняемого файла
        Mode m_mode; //< Режим выполнения 
        std::vector<std::string> m_modules; //< Список модулей, которые загружаются перед выполнением
        std::vector<std::string> m_load_only; //< Список модулей, которые предварительно загружаются перед выполнением без инициализации
        bool m_no_default; //< Не загружать модуль default.nlp (модуль по умолчанию)
        std::string m_ifile; //< Имя входного файла (если есть)
        std::string m_ofile; //< Имя выходного файла (если есть)
        bool m_is_silent; //< Нужно ли выводит сообщения
        std::string m_output; //<  Информация для вывода, в т.ч. при ошибке о текст подсказки
        std::string m_eval; //<  Входная информация для выполнения
        Context m_ctx;
        ObjPtr m_args;
        std::map<Obj *, ObjPtr> m_local_vars; //< Локальные переменные и объекты, которые создаются интерпретатором

        utils::Logger::LogLevelType m_loglevel_save;
        utils::Logger::FuncCallback *m_log_callback_save;
        void *m_log_callback_arg_save;

        NLC() : m_ctx(RunTime::Init()) {
            m_mode = Mode::ModeError;
            m_log_callback_save = nullptr;
            m_log_callback_arg_save = nullptr;
        }

        NLC(int argc, const char** argv) : m_ctx(RunTime::Init(argc, argv)) {
            m_mode = Mode::ModeError;
            m_log_callback_save = nullptr;
            m_log_callback_arg_save = nullptr;
            ParseArgs(argc, argv);
        }

        NLC(const char * str) : NLC() {
            std::vector<std::string> split = Named::SplitString(str, " ");
            std::vector<const char *> argv;
            for (size_t i = 0; i < split.size(); i++) {
                argv.push_back(split[i].data());
            }
            ParseArgs(argv.size(), argv.data());
        }

        virtual ~NLC() {
            utils::Logger::Instance()->SetCallback(m_log_callback_save, m_log_callback_arg_save);
        }

        static void LoggerCallback(void *param, utils::Logger::LogLevelType level, const char * str, bool flush) {
            NLC *nlc = static_cast<NLC *> (param);
            if (nlc && nlc->m_is_silent) {
                return;
            }
            fprintf(stdout, "%s", str);
            if (flush) {
                fflush(stdout);
            }
            if (nlc) {
                nlc->m_output += str;
            }
        }

        bool ParseArgs(int64_t argc, const char** argv) {

            if (!argc || !argv) {
                m_output = "Bad args nullptr";
                return false;
            }

            m_args = Obj::CreateDict();
            for (int i = 0; i < argc; i++) {
                std::vector<std::string> split = Named::SplitString(argv[i], "=");
                if (split.size() > 1) {
                    m_args->push_back(Obj::CreateString(split[0]), &argv[i][split[0].size() + 1]);
                } else {
                    m_args->push_back(Obj::CreateString(argv[i]));
                }
            }


            m_mode = Mode::ModeInter;
            if (argc) {
                m_path = argv[0];
            } else {
                m_path.clear();
            }
            m_eval.clear();
            m_modules.clear();
            m_load_only.clear();
            m_no_default = false;
            m_ifile.clear();
            m_ofile.clear();
            m_output.clear();
            m_is_silent = false;

            bool is_debug = false;
            bool is_help = false;
            bool is_ver = false;
            std::string load_list;
            std::string load_only;
            std::string compile;
            std::string exec;
            std::string eval;

            utils::Logger::Instance()->SaveCallback(m_log_callback_save, m_log_callback_arg_save);
            m_loglevel_save = utils::Logger::Instance()->GetLogLevel();
            utils::Logger::Instance()->Clear();
            utils::Logger::Instance()->SetCallback(&LoggerCallback, this);

            auto cli
                    = lyra::help(is_help).description("Description!!!!!!!!!!!!!!!!!!")
                    | lyra::opt(is_ver) ["-v"] ["--version"]("Version New Lang Compiler.")
                    | lyra::opt(is_debug) ["-d"] ["--debug"]("Debug detail mode.")
                    | lyra::opt(m_is_silent) ["-s"] ["--silent"]("Silent mode without message output.")
                    | lyra::opt(m_ofile, "filename") ["-o"]["--output"] ("Output file name.")
                    | lyra::opt(load_list, "list") ["-l"] ["--load"]("List of load modules.")
                    | lyra::opt(load_only, "list") ["--load-only"]("List of load only modules (without init module after load).")
                    | lyra::opt(compile, "filename") ["-c"] ["--compile"]("Compile input file and build NLM module.")
                    | lyra::opt(exec, "filename") ["-x"] ["--exec"]("Compile and make module, load and eXecute main module function.")
                    | lyra::opt(m_ifile, "filename") ["-e"] ["--eval"]("Evaluate file in interpreter mode.")
                    | lyra::arg(m_eval, "expression") ("Evaluate expression excluding compilation.")
                    ;


            auto result = cli.parse({static_cast<int> (argc), argv});
            if (!result) {
                m_mode = Mode::ModeError;
                m_output = result.message();
                return false;
            }

            if (is_debug) {
                utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_DEBUG);
            }

            if (is_help) {
                m_mode = Mode::ModeHelp;
                std::ostringstream out;
                out << cli;
                m_output = out.str();
                return true;
            }

            if (is_ver) {
                m_mode = Mode::ModeVersion;
                m_output = "Version info";
                return true;
            }

            m_modules = Named::SplitString(load_list.c_str(), ",");
            m_load_only = Named::SplitString(load_only.c_str(), ",");

#ifdef _WIN32

#else
            int len;
            char buffer[100];
            fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
            while ((len = read(STDIN_FILENO, buffer, sizeof (buffer))) > 0) {
                //                m_eval.append(buffer, len);
            }
            fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) & ~O_NONBLOCK);
#endif
            int cnt = 0;
            cnt += !compile.empty();
            cnt += !exec.empty();
            cnt += !m_ifile.empty();
            //            cnt += !m_eval.empty();

            if (cnt > 1) {
                m_mode = Mode::ModeError;
                m_output = "Select only one mode: Compile, eXec or Eval!";
                //        } else if (!compile.empty()) {
                //            m_mode = Mode::COMPILE;
                //            m_ifile = compile;
                //        } else if (!exec.empty()) {
                //            m_mode = Mode::EXEC;
                //            m_ifile = exec;
            } else if (!m_eval.empty() || !m_ifile.empty()) {
                m_mode = Mode::ModeEval;
            } else {
                m_mode = Mode::ModeInter;
            }

            return m_mode != Mode::ModeError;
        }

        int Run() {
            try {
                //#warning EVAL
                //            ASSERT(false);
                //            for (auto &elem : m_load_only) {
                //                if (!m_ctx.m_info.global->LoadModule(AddDefaultFileExt(elem.c_str(), ".nlm").c_str(), false, &m_ctx)) {
                //                    LOG_RUNTIME("Fail load-only module '%s'", AddDefaultFileExt(elem.c_str(), ".nlm").c_str());
                //                }
                //            }
                //
                //            for (auto &elem : m_modules) {
                //                if (!m_ctx.m_info.global->LoadModule(AddDefaultFileExt(elem.c_str(), ".nlm").c_str(), true, &m_ctx)) {
                //                    LOG_RUNTIME("Fail load or init module '%s'", AddDefaultFileExt(elem.c_str(), ".nlm").c_str());
                //                }
                //            }

                if (m_mode == Mode::ModeError || m_mode == Mode::ModeVersion || m_mode == Mode::ModeHelp) {
                    LOG_INFO("%s", m_output.c_str());
                    return 0;
                    //            } else if (m_mode == Mode::COMPILE || m_mode == Mode::EXEC) {
                    //
                    //                if (m_ifile.empty()) {
                    //                    LOG_RUNTIME("Empty input file!");
                    //                    m_ifile = AddDefaultFileExt(m_ifile.c_str(), ".nlp");
                    //                }
                    //                if (m_ofile.empty()) {
                    //                    m_ofile = m_ifile;
                    //                    m_ofile = ReplaceFileExt(m_ofile.c_str(), ".nlp", ".nlm");
                    //                } else {
                    //                    m_ofile = AddDefaultFileExt(m_ofile.c_str(), ".nlm");
                    //                }
                    //
                    //                switch (m_mode) {
                    //                    case Mode::COMPILE:
                    //                        //                        if (!m_ctx.m_runtime->m_info.global->CompileModule(m_ifile.c_str(), m_ofile.c_str())) {
                    //                        LOG_RUNTIME("Compile file '%s' fail!", m_ifile.c_str());
                    //                        //                        }
                    //                        break;
                    //                    case Mode::EXEC:
                    //#warning EVAL
                    //                        ASSERT(false);
                    //                        //                        ObjPtr result = NewLang::ExecModule(m_ifile.c_str(), m_ofile.c_str(), true, &m_ctx);
                    //                        //                        m_output = result->GetValueAsString();
                    //                        break;
                    //                }
                } else if (m_mode == Mode::ModeEval) {

                    Obj *arg_ptr = nullptr;
                    ObjPtr dict = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
                    std::string source;
                    if (!m_ifile.empty()) {
                        source = ReadFile(m_ifile.c_str());
                        if (source.empty()) {
                            LOG_RUNTIME("Fail read or empty source file '%s'!", m_ifile.c_str());
                        }

                        if (!m_eval.empty()) {
                            std::vector<std::string > a = Named::SplitString(m_eval.c_str(), " ");

                            for (int i = 0; i < a.size(); i++) {
                                std::vector<std::string> split = Named::SplitString(a[i].c_str(), "=");
                                if (split.size() > 1) {
                                    dict->push_back(Obj::CreateString(split[0]), &a[i][split[0].size() + 1]);
                                } else {
                                    dict->push_back(Obj::CreateString(a[i]));
                                }
                            }
                        }
                        arg_ptr = dict.get();

                    } else {
                        source = m_eval;
                        arg_ptr = m_args.get();
                    }



                    ObjPtr result = m_ctx.ExecStr(source, arg_ptr, Context::CatchType::CATCH_AUTO);

                    if (result && m_local_vars.find(result.get()) == m_local_vars.end()) {
                        m_local_vars[result.get()] = result;
                    }

                    m_output = result->GetValueAsString();
                    m_output += "\n";

                    if (!m_ofile.empty()) {
                        std::ofstream out(m_ofile);
                        out << m_output;
                        out.close();
                    }
#ifdef _WIN32 

#else
                    fcntl(STDOUT_FILENO, F_SETFL, fcntl(STDOUT_FILENO, F_GETFL, 0) | O_NONBLOCK);
                    write(STDOUT_FILENO, m_output.c_str(), m_output.size());
#endif // _WIN32 


                } else {
                    ASSERT(m_mode == Mode::ModeInter);
                    Interative();
                }

            } catch (Return &err) {
                // Вывод информации об ошибке синтаксиса при парсинге без информации о точке вызова макроса LOG_INFO
                utils::Logger::LogLevelType save_level = utils::Logger::Instance()->GetLogLevel();
                utils::Logger::Instance()->SetLogLevel(LOG_LEVEL_INFO);
                LOG_INFO("%s", err.what());
                utils::Logger::Instance()->SetLogLevel(save_level);
                return 1;
            } catch (...) {
                return 1;
            }
            return 0;
        }

        bool IsDelimiter(wchar_t c) {
            return c == L' ' || c == L'&' || c == L'=' || c == L';' || c == L','
                    || c == L'+' || c == L'-' || c == L'*' || c == L'/'
                    || c == L'<' || c == L'>' || c == L'|' || c == L'~' || c == L'^';
        }

        bool Interative() {


            std::string output;
            std::vector<std::string> history;


            std::ifstream infile;
            infile.open(NLC_FILE_HISTORY);
            while (infile.is_open() && getline(infile, output)) {
                history.push_back(output);
            }
            infile.close();

            std::ofstream filehistory;
            filehistory.open(NLC_FILE_HISTORY, std::ios::app);


            const char* title = ">";
#ifdef _MSC_VER
            COLOR_TYPE title_color = 0;
            COLOR_TYPE predict_color = 0;
            COLOR_TYPE main_color = 0;
#else
            COLOR_TYPE title_color = "1";
            COLOR_TYPE predict_color = "80";
            COLOR_TYPE main_color = "0";
#endif

            std::wstring buff;

            // Cursor offset in buffer for moving
            int64_t offset = 0;

            color_print("Type ", predict_color);
//            color_print("help()", main_color);
//            color_print("<Enter> for help about ", predict_color);
//            color_print("NewLang", title_color);
//            color_print(" syntax and commands or ", predict_color);
            color_print("--", main_color);
            color_print("<Enter> (or ", predict_color);
            color_print("++", main_color);
            color_print("<Enter>) for exit the program.\n", predict_color);

            // Calculate title length
            int title_len = (short) strlen(title);
            bool show_all = false;

            while (1) {

                int64_t history_pos = history.size();

                while (1) {
                    // Print title with title color
                    clear_line();
                    color_print(title, title_color);
                    if (title_len) {
                        printf(" ");
                    }

                    // Get length of last word in input
                    short space_offset = 0;
                    while (buff.size() && space_offset < buff.size() && !IsDelimiter(buff[buff.size() - space_offset - 1])) {//buff[buff.size() - space_offset - 1] != L' ') {
                        space_offset += 1;
                    }

                    // Print current buffer
                    color_print(utf8_encode(buff).c_str(), main_color);

                    std::vector<std::wstring> predict;

                    predict.clear();
                    if (space_offset) {
                        size_t overflow = 5;

                        predict = m_ctx.m_runtime->m_named->SelectPredict(&buff[buff.size() - space_offset], overflow); // Не более 5 примеров продолжения

                        if (predict.size()) {
                            if (show_all) {
                                // Показать все варинанты
                                std::wstring helper;
                                for (int i = 0; i < overflow; i++) {
                                    if (i >= predict.size()) {
                                        break;
                                    }
                                    if (!helper.empty()) {
                                        helper += L" ";
                                    }
                                    helper += predict[i];
                                }
                                if (predict.size() >= overflow) {
                                    helper += L" ..."; // и есть еще
                                }
                                color_print(utf8_encode(helper.substr(space_offset)).c_str(), predict_color);
                            } else {
                                std::wstring show(predict[0]);
                                if (space_offset <= show.size()) {
                                    if (predict.size() > 1) { // Присутсвует более одного варианта
                                        show += L"...";
                                    }
                                    color_print(utf8_encode(show.substr(space_offset)).c_str(), predict_color);
                                }
                            }
                        }
                    }

                    // Move cursor to edit position
                    set_cursor_x(offset + title_len + 2);

                    // Read character from console
                    int ch = _getch();

                    //                    LOG_DEBUG("_getch() %d", ch);

                    int w_ch = 0;

                    if ((ch & 0x80) && (ch != SPECIAL_SEQ_2)) {

                        // двухбайтовые UTF8
                        if ((ch & 0xE0) == 0xC0) {
                            w_ch = ch & 0x1F;
                            w_ch <<= 6;
                            ch = _getch();
                            w_ch |= (ch & 0x3F);
                        } else {
                            LOG_ERROR("Unsupported char %d", ch);
                            w_ch = 0;
                        }
                        //                        continue;

                    } else if (is_ignore_key(ch)) { // Wait next symbol if character in ignore keys

                        continue;

                    } else if (ch == KEY_ENTER) { // Return buffer if ENTER was pressed

                        if (!buff.empty() && (history.size() == 0 || (history.size() && history[history.size() - 1].compare(utf8_encode(buff)) != 0))) {
                            history.push_back(utf8_encode(buff));
                            filehistory << utf8_encode(buff) << "\n";
                            filehistory.flush();
                        }
                        break;

                    }// Keyboard interrupt handler for Windows
#if defined(_MSC_VER)
                    else if (ch == CTRL_C) {
                        goto done;
                    }
#endif
                        // Edit buffer like backspace if BACKSPACE was pressed
                    else if (ch == KEY_BACKSPACE || ch == 8) {
                        if (buff.size() && offset) {
                            // Delete character from buffer
                            for (unsigned i = offset - 1; i < buff.size(); i++) {
                                buff[i] = buff[i + 1];
                            }
                            buff.resize(buff.size() - 1);
                            offset--;
                        }
                        // Apply prediction if TAB was pressed
                    } else if (ch == KEY_TAB) {

                        if (predict.size() == 1 || (predict.size() > 1 && show_all)) {
                            if (space_offset < predict[0].size()) {
                                buff.append(predict[0].substr(space_offset));
                            }
                            show_all = false;
                        } else {
                            show_all = true;
                        }
                        offset = buff.size();

                    }// Arrows and Delete keys handler
                    else if (
                            ch == SPECIAL_SEQ
#if defined(OS_WINDOWS)
                            || ch == SPECIAL_SEQ_2 || ch == KEY_ESC
#elif defined(OS_UNIX)
                            //&& _getch()
#endif
                            //== SPECIAL_SEQ_2
                            ) {

                        ch = _getch();
#if defined(OS_UNIX)
                        if (ch == SPECIAL_SEQ_2) {
                            ch = _getch(); // Skip next code
                        } else if (ch != SPECIAL_SEQ) {
                            continue;
                        }
#endif

                        switch (ch) {
                            case KEY_ESC:
                                buff.clear();
                                offset = 0;
                                break;
                            case KEY_HOME:
                                offset = 0;
                                break;
                            case KEY_END:
                                offset = buff.size();
                                break;
                            case KEY_LEFT:
                                offset = (offset > 0) ? offset - 1 : 0;
                                break;
                            case KEY_RIGHT:
                                // Decrease offset from the end of the buffer if left key pressed
                                offset = (offset < static_cast<int64_t> (buff.size())) ? (offset + 1) : buff.size();
                                break;
                            case KEY_UP:
                                if (!history.empty() && history_pos > 0) {
                                    clear_line();
                                    history_pos--;
                                    buff = utf8_decode(history[history_pos]);
                                }
                                offset = buff.size();
                                break;
                            case KEY_DOWN:
                                if (!history.empty() && history_pos + 1 < static_cast<int64_t> (history.size())) {
                                    clear_line();
                                    history_pos++;
                                    buff = utf8_decode(history[history_pos]);
                                }
                                offset = buff.size();
                                break;
                            case KEY_DEL: // Edit buffer like DELETE key
#if defined(OS_UNIX)
                                if (_getch() == KEY_DEL_AFTER)
#endif
                                {
                                    if (buff.size() && offset < buff.size()) {
                                        // Delete character from buffer
                                        for (unsigned i = offset; i < buff.size() - 1; i++) {
                                            buff[i] = buff[i + 1];
                                        }
                                        buff.resize(buff.size() - 1);
                                    }
                                }
                                break;
                            default:
                                break;
                        }

                    } else {
                        // Ascii char
                        w_ch = ch;
                    }

                    if (w_ch) {
                        buff.insert(offset, 1, w_ch);
                        offset++;
                    }
                }


                std::wstring result;
                if (buff.compare(L"--") == 0 || buff.compare(L"--;") == 0 || buff.compare(L"++") == 0 || buff.compare(L"++;") == 0) {
                    printf("\n");
                    break;
                } else if (!buff.empty()) {
                    try {
                        std::wstring input = buff;

                        ObjPtr res = m_ctx.ExecStr(utf8_encode(input), m_args.get(), Context::CatchType::CATCH_ALL);

                        if (res) {

                            if (m_local_vars.find(res.get()) == m_local_vars.end()) {
                                m_local_vars[res.get()] = res;
                            }

                            result = res->GetValueAsStringWide();
                        } else {
                            result = L"nullptr";
                        }

                    } catch (std::exception &err) {
                        result = utf8_decode(err.what());
                    }
                    buff.clear();
                }
                printf("\n%ls\n", result.c_str());
                show_all = false;
                offset = 0;
            }

#if defined(_MSC_VER)
done:
#endif

            filehistory.close();
            return 0;
        }

    };

}

#endif // INCLUDED_NLC_

#pragma once
#ifndef INCLUDED_ANALISIS_CPP_
#define INCLUDED_ANALISIS_CPP_

//#include "pch.h"

#include <sys/time.h>

#include "fmt/core.h"
#include "fmt/args.h"

#include "nlc-rt.h"

#include "term.h"
#include "builtin.h"
//#include <types.h>
//#include <object.h>
#include "parser.h"


namespace newlang {

    class AstAnalysis {
    protected:

        AstAnalysis(const AstAnalysis&) = delete;
        const AstAnalysis& operator=(const AstAnalysis&) = delete;
    public:

        RunTime &m_rt;
        Diag * m_diag;

        AstAnalysis(RunTime &rt, Diag* diag) : m_rt(rt), m_diag(diag) {

        }

        static std::string MakeInclude(const TermPtr &ast);

        /*
         * При раскрытии области видимости объектов и проверки корректности их имен,
         * нужно одновремно регистрировать глобальные и локальные переменные, 
         * так как nameloockup выполняется последовательно и если создавать переменные позже, 
         * то обращение может произойти к неправильным переменным 
         * (либо будет неправильное перекрытие, либо ошибка, т.к. переменная еще не создана).
         * 
         * А если регистрировать не только глобальные, но и локальные объекты, 
         * тогда в функцию анализатора нужно передавать и объект - владалец локальных переменных.
         * Точнее он уже есть, это либо глоальныей корень для переменных и функций модуля, 
         * либо глобальные объекты - функции с собственными локальными переменными.
         * m_variables
         */
        bool Analyze(TermPtr &term, TermPtr &root);
        bool RecursiveAnalyzer(TermPtr term, ScopeStack &stack);
        bool CreateOp_(TermPtr &term, ScopeStack &stack);
        bool CreateVar(TermPtr &var, TermPtr &value, ScopeStack &stack);
        bool AssignVar(TermPtr &var, TermPtr &value, ScopeStack &stack);
        bool Iterator_(TermPtr &term, ScopeStack &stack);

        TermPtr CheckField_(TermPtr &term, TermPtr &field, ScopeStack &stack);
        bool CheckIndex_(TermPtr &term, TermPtr &index, TermPtr &value, ScopeStack &stack);
        bool CheckItems_(TermPtr &term, ScopeStack &stack);

        TermPtr CheckGetValue_(TermPtr &obj, ScopeStack &stack);
        bool CheckSetValue_(TermPtr &obj, TermPtr &value, ScopeStack &stack);

        TermPtr LookupName(TermPtr &term, ScopeStack &stack);
        bool CheckNative_(TermPtr &term);

        /**
         * Функция проверки наличия ошибок при анализе AST.
         * Прерывает работу анализатора при превышении лимита или при force=true
         * @param force - Если есть ошибки- завершить работу
         */
        bool CheckError(bool result);

        bool CheckName(TermPtr &term);
        bool CheckOp(TermPtr &term);
        bool CalcType(TermPtr &term);
        bool UpcastOpType(TermPtr &op);
        bool CheckOpType(TermPtr &op, TermPtr &left, const TermPtr right);
        bool CheckReference(TermPtr &term, const TermPtr &test_ref);
        bool CheckAssignRef(TermPtr &left, TermPtr &right, ScopeStack & stack);
        bool CheckCall(TermPtr &proto, TermPtr &call, ScopeStack & stack);
        bool CheckCallArg(TermPtr &call, size_t arg_pos, ScopeStack & stack);
        void CheckDims(TermPtr &dims, ScopeStack & stack, bool allow_none, bool allow_ellipsis);
        bool CkeckRange_(TermPtr &term, ScopeStack & stack);
        bool CheckFollow_(TermPtr &term, ScopeStack & stack);
        bool CheckWhile_(TermPtr &term, ScopeStack & stack);
        bool CheckDoWhile_(TermPtr &term, ScopeStack & stack);
        bool CheckMatching_(TermPtr &term, ScopeStack & stack);
        bool CheckWith_(TermPtr &term, ScopeStack & stack);
        bool CheckTake_(TermPtr &term, ScopeStack & stack);

        bool CheckNative_(TermPtr &proto, TermPtr &term);

        bool CheckArgsType_(TermPtr proto, TermPtr value);
        bool CheckArgs_(TermPtr proto, TermPtr args);


        static bool CheckStrPrintf(const std::string_view format, TermPtr args, int start);
        static std::string MakeFormat(const std::string_view format, TermPtr args, RunTime * rt);
        static fmt::dynamic_format_arg_store<fmt::format_context> MakeFormatArgs(TermPtr args, RunTime * rt);
        //        template <typename T> static std::string ConvertToVFormat_(const std::string_view format, T args);

        /*
         * $template := "{name} {0}"; # std::format equivalent "{1} {0}"
         * $result := $template("шаблон", name = "Строка"); # result = "Строка шаблон"
         */

        template <typename T> 
        static std::string ConvertToVFormat_(const std::string_view format, T & args) {
//            if (args.m_id != TermID::DICT) {
//                LOG_RUNTIME("ConvertToFormat requires arguments!");
//            }
            std::string result;
            int pos = 0;
            while (pos < format.size()) {
                if (format[pos] == '{' && pos + 1 < format.size() && format[pos + 1] == '{') {
                    // Escaped text in doubling {{
                    result += "{{";
                    pos += 1;
                } else if (format[pos] == '}' && pos + 1 < format.size() && format[pos + 1] == '}') {
                    // Escaped text in doubling }}
                    result += "}}";
                    pos += 1;
                } else if (format[pos] == '{') {
                    pos += 1;
                    int name = pos;
                    result += '{';
                    while (pos < format.size()) {
                        if (format[pos] == '{') {
                            LOG_RUNTIME("Unexpected opening bracket '%s' at position %d!", format.begin(), pos);
                        }
                        if (format[pos] == '}') {
                            result += '}';
                            goto done;
                        }
                        if (isalpha(format[pos]) || format[pos] == '_') {
                            while (name < format.size()) {
                                if (isalnum(format[name]) || format[name] == '_') {
                                    name++;
                                } else {
                                    name -= 1;
                                    break;
                                }
                            }

                            ASSERT(name > pos);
                            std::string arg_name(&format[pos], name - pos + 1);

                            auto found = args.find(arg_name);
                            if (found == args.end()) {
                                LOG_RUNTIME("Argument name '%s' not found!", arg_name.c_str());
                            }
                            result += std::to_string(std::distance(args.begin(), found));
                            pos += (name - pos);
                        } else {
                            result += format[pos];
                        }
                        pos++;
                    }
                    LOG_RUNTIME("Closing bracket in '%s' for position %d not found!", format.begin(), name - 1);
                } else {
                    result += format[pos];
                }
done:
                pos++;
            }
            return result;
        }

        TermPtr CalcSummaryType(const TermPtr &term, ScopeStack & stack);

    protected:

    public:

    };
}


#endif //INCLUDED_ANALISIS_CPP_

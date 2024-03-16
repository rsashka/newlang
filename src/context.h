#pragma once
#ifndef INCLUDED_NEWLANG_CONTEXT_
#define INCLUDED_NEWLANG_CONTEXT_

#include "pch.h"

#include <ffi.h>

#include <term.h>
#include <object.h>
#include <module.h>
#include <runtime.h>

namespace newlang {

    /*
     * Класс контекст предназначен для хранения контекста среды выполнения при вызове функций.
     * С его помощью передаются переменные среды окружения, параметры и аргументы приложения, входные и выходные параметры функций, 
     * текущие локальные и глобальные переменныеи, создание и доступ к итераторам и т.д.
     * 
     * 
     * 
     */

    /* 
     * Выполняет модуль (файл/ast)
     */

    class Context : SCOPE(public) ScopeStack, public std::enable_shared_from_this<Context> {
        SCOPE(private) :
        RuntimePtr m_runtime;
        ObjPtr m_latter;

    public:

        Context(StorageTerm &module, RuntimePtr rt) : ScopeStack(module), m_runtime(rt) {
        }

        virtual ~Context() {
        }

        static ObjPtr Run(TermPtr ast, Context *runner);

        static ObjPtr EvalTerm(TermPtr term, Context *runner, bool rvalue = true);

        static ObjPtr StringFormat(std::string_view format, Obj &args);
        static ObjPtr StringPrintf(std::string_view format, Obj &args);

        /**
         * Метод вызывается только из NewLnag кода
         * @param runner
         * @param obj
         * @param term
         * @return 
         */
        static ObjPtr Call(Context *runner, Obj &obj, TermPtr &term);
        /**
         * Метод может быть вызван как из NewLnag кода, так и из кода на C/C++ (в реализации Obj::operator())
         * @param runner
         * @param obj
         * @param term
         * @return 
         */
        static ObjPtr Call(Context *runner, Obj &obj, Obj &args);

    protected:

        inline static RunTime * GetRT_(Context * runner) {
            return runner ? runner->m_runtime.get() : nullptr;
        }

        ObjPtr CreateNative_(TermPtr &proto, const char *module, bool lazzy, const char *mangle_name);
        static ObjPtr CallNative_(Context *runner, Obj & obj, Obj *args = nullptr);

        /**
         * Выполняет одну операцию
         * @param op
         * @param run
         * @return 
         */
        static inline ObjPtr CheckObjTerm_(TermPtr & term, Context * runner, bool rvalue = true) {
            if (!term->m_obj) {
                term->m_obj = EvalTerm(term, runner, rvalue);
            }
            return term->m_obj;
        }

        static ObjPtr CreateArgs_(ObjPtr &args, TermPtr &term, Context * runner);
        static ObjPtr CreateDict(TermPtr &term, Context * runner);
        static ObjPtr CreateRange(TermPtr &term, Context * runner);
        static ObjPtr CreateTensor(TermPtr &term, Context * runner);

        static ObjPtr SetIndexValue(TermPtr &term, ObjPtr &value, Context * runner);
        static ObjPtr SetFieldValue(TermPtr &term, ObjPtr &value, Context * runner);
        static ObjPtr GetIndexValue(TermPtr &term, ObjPtr &value, Context * runner);
        static ObjPtr GetFieldValue(TermPtr &term, ObjPtr &value, Context * runner);


        ObjPtr EvalCreate_(TermPtr & op);
        ObjPtr AssignVars_(ArrayTermType &vars, const TermPtr &r_term, bool is_pure);

        ObjPtr EvalIterator_(TermPtr & term);
        ObjPtr EvalInterrupt_(TermPtr & term);

        ObjPtr EvalOp_(TermPtr & op);

        static ObjPtr EvalOpMath_(TermPtr &op, Context * runner);
        static ObjPtr EvalOpLogical_(TermPtr &op, Context * runner);
        static ObjPtr EvalOpCompare_(TermPtr &op, Context * runner);
        static ObjPtr EvalOpBitwise_(TermPtr &op, Context * runner);
        static ObjPtr EvalRange_(TermPtr &op, Context * runner);

        static ObjPtr EvalWhile_(TermPtr & op, Context * runner);
        static ObjPtr EvalDoWhile_(TermPtr & op, Context * runner);
        static ObjPtr EvalFollow_(TermPtr & op, Context * runner);

        /**
         * Выполняет группу операций, которые записаны в стек
         * @param op
         * @param run
         * @return 
         */
        ObjPtr EvalBlock_(TermPtr &block, TermPtr proto);
        ObjPtr EvalEval_(TermPtr & op);

    };

}
#endif //INCLUDED_NEWLANG_CONTEXT_

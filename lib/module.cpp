#include "module.h"
#include "runtime.h"

using namespace newlang;

Module::Module(RuntimePtr rt, TermPtr ast) {//: Obj(ObjType::Module) {
    m_runtime = rt;

    if (ast) {
        ASSERT(rt);
        // Только создание пустых объектов без их инициалиазации.
        // Инициализация будет во время выполнения,
        // а к тому времени все объекты должны быть уже созданы
        //        for (auto &var_mod : ast->m_variables) {
        //            ASSERT(var_mod.second);
        ////            if (isStaticName(var_mod.second->m_text)) {
        ////
        ////                // Статические обекты
        ////                ASSERT(var_mod.second.expr);
        ////                ASSERT(var_mod.second.expr->m_right);
        ////                ObjPtr obj = RunTime::EvalStatic(var_mod.second.expr->m_right, var_mod.second.expr->IsPure());
        ////                insert({var_mod.first,
        ////                    {var_mod.second.proto, obj}});
        ////
        ////            } else {
        ////                //  Временные объекты не создаются
        ////                ASSERT(isLocalName(var_mod.second.proto->m_text));
        ////            }
        //
        //
        //            for (auto &var_local : var_mod.second->m_variables) {
        //
        ////                if (isStaticName(var_local.second.proto->m_text)) {
        ////
        ////                    // Статические обекты
        ////                    ASSERT(var_mod.second.expr);
        ////                    ObjPtr obj = RunTime::EvalStatic(var_mod.second.expr->m_right, var_mod.second.expr->IsPure());
        ////                    insert({var_mod.first,
        ////                        {var_mod.second.proto, obj}});
        ////
        ////                } else {
        ////                    //  Временные объекты не создаются
        ////                    ASSERT(isLocalName(var_local.second.proto->m_text));
        ////                }
        //            }
        //
        //        }
    }
    //            m_var_is_init = true;
}

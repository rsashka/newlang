//#include "pch.h"

#include <filesystem>
#include <stdbool.h>

#include "warning_push.h"
#include <torch/torch.h>
#include <ATen/ATen.h>
#include "warning_pop.h"


#include "context.h"
#include "term.h"
#include "macro.h"
#include "runtime.h"
#include "module.h"
#include "types.h"
#include "analysis.h"

using namespace newlang;

ObjType getSummaryTensorType(Obj *obj, ObjType start = ObjType::None);

Context::Context(Module &module, RunTime *rt) : m_runtime(rt), m_static(module) { //ScopeStack(module), 
    push_back({});
}

std::string Context::Dump(size_t num) {
    std::string result;
    if (!num) {
        result = "Static: ";
        //    if (m_module) {
        result += m_static.Dump();
        //    }
    }

    size_t count = 0;
    auto iter = rbegin();
    while (iter != rend()) {
        result += "Stack ";
        result += std::to_string(std::distance(iter, rend()));
        result += " [";
        result += iter->ns;
        result += "]: ";

        std::string list;
        auto iter_list = iter->vars.begin();
        while (iter_list != iter->vars.end()) {

            if (!list.empty()) {

                list += ", ";
            }

            list += iter_list->first;
            list += "=(";
            if (iter_list->second.obj) {
                list += iter_list->second.obj->toString();
            } else {
                list += "nullptr";
            }
            list += ")";
            iter_list++;
        }

        //        result += "(";
        result += list;
        result += "\n";
        iter++;

        count++;
        if (num && count >= num) {
            break;
        }
    }
    return result;
}

ObjPtr Context::CheckObjTerm_(TermPtr & term, Context * runner, bool rvalue) {
    ObjPtr temp = EvalTerm(term, runner, rvalue);
    if (term->m_obj) {
        (*term->m_obj) = *temp;
    } else {
        term->m_obj = temp;
    }
    return term->m_obj;
}

VarItem Context::CreateItem(TermPtr term, ObjPtr obj) {
    VarItem item;

    if (term->m_int_name.empty() && !isReservedName(term->m_text)) {
        LOG_RUNTIME("Empty internal name '%s'", term->m_text.c_str());
    }
    item.term = term;
    //    item.item->m_int_name = term->m_text;
    item.sync = Sync::CreateSync(term);
    if (obj) {
        obj->m_sync = item.sync.get();
    }
    item.obj = obj;
    item.term->m_obj = obj;

#ifdef BUILD_DEBUG
    item.term_check = item.term.get();
    item.obj_check = item.term->m_obj.get();
#endif        

    return item;
}

//std::multimap<std::string, DocPtr> Docs::m_docs;
//bool Context::MatchCompare(Obj &match, ObjPtr &value, MatchMode mode, Context *ctx) {
//    switch (mode) {
//        case MatchMode::MatchEqual:
//            return match.op_equal(value);
//        case MatchMode::MatchStrict:
//            return match.op_accurate(value);
//        case MatchMode::TYPE_NAME:
//            return match.op_class_test(value, ctx);
//        case MatchMode::TYPE_EQUAL:
//            return match.op_duck_test(value, false);
//
//        case MatchMode::TYPE_STRICT:
//            return match.op_duck_test(value, true);
//    }
//    LOG_RUNTIME("Unknown pattern matching type %d!", static_cast<int> (mode));
//}
//
//bool Context::MatchEstimate(Obj &match, const TermPtr &match_item, MatchMode mode, Context *ctx, Obj * args) {
//
//    ObjPtr cond = CreateRVal(ctx, match_item, args);
//
//    if (cond->is_none_type() || MatchCompare(match, cond, mode, ctx)) {
//        return true;
//    } else {
//        for (int i = 0; i < match_item->m_follow.size(); i++) {
//
//            ASSERT(match_item->m_follow[i]);
//            cond = CreateRVal(ctx, match_item->m_follow[i], args);
//
//            if (cond->is_none_type() || MatchCompare(match, cond, mode, ctx)) {
//
//                return true;
//            }
//        }
//    }
//    return false;
//}
//
//ObjPtr Context::eval_MATCHING(Context *ctx, const TermPtr &term, Obj * args, bool eval_block) {
//    /*
//     * [match] ==> { # ~> ~~> ~~~> ===>
//     *  [cond1] --> {expr};
//     *  [cond2, cond3] --> {expr};
//     *  [...] --> {expr};
//     * };
//     */
//
//    ASSERT(term->Left());
//    ASSERT(term->Right());
//
//    MatchMode mode;
//    if (term->m_text.compare("==>") == 0) {
//        mode = MatchMode::MatchEqual;
//    } else if (term->m_text.compare("===>") == 0) {
//        mode = MatchMode::MatchStrict;
//    } else if (term->m_text.compare("~>") == 0) {
//        mode = MatchMode::TYPE_NAME;
//    } else if (term->m_text.compare("~~>") == 0) {
//        mode = MatchMode::TYPE_EQUAL;
//    } else if (term->m_text.compare("~~~>") == 0) {
//        mode = MatchMode::TYPE_STRICT;
//    } else {
//        NL_PARSER(term, "Unknown pattern matching type!");
//    }
//
//    ObjPtr value = CreateRVal(ctx, term->Left(), args);
//    TermPtr list = term->Right();
//
//
//    ASSERT(list->Left());
//    ASSERT(list->Right());
//
//    ObjPtr cond = CreateRVal(ctx, list->Left(), args);
//
//    if (MatchEstimate(*cond.get(), list->Left(), mode, ctx, args)) {
//        return CreateRVal(ctx, list->Right(), args);
//    } else {
//        for (int i = 0; i < list->m_block.size(); i++) {
//
//            ASSERT(list->m_block[i]->Left());
//            ASSERT(list->m_block[i]->Right());
//
//            if (MatchEstimate(*cond.get(), list->m_block[i]->Left(), mode, ctx, args)) {
//
//                return CreateRVal(ctx, list->m_block[i]->Right(), args);
//            }
//        }
//    }
//    return Obj::CreateNone();
//}

//void Context::ItemTensorEval_(torch::Tensor &tensor, c10::IntArrayRef shape, std::vector<Index> &ind, const int64_t pos,
//        ObjPtr &obj, ObjPtr & args) {
//    ASSERT(pos < static_cast<int64_t> (ind.size()));
//    if (pos + 1 < static_cast<int64_t> (ind.size())) {
//        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
//            ItemTensorEval_(tensor, shape, ind, pos + 1, obj, args);
//        }
//    } else {
//
//        at::Scalar value;
//        ObjType type = fromTorchType(tensor.scalar_type());
//
//        for (ind[pos] = 0; ind[pos].integer() < shape[pos]; ind[pos] = ind[pos].integer() + 1) {
//
//            switch (type) {
//                case ObjType::Int8:
//                case ObjType::Char:
//                case ObjType::Byte:
//                case ObjType::Int16:
//                case ObjType::Word:
//                case ObjType::Int32:
//                case ObjType::DWord:
//                case ObjType::Int64:
//                case ObjType::DWord64:
//                    value = at::Scalar(obj->Call(this)->GetValueAsInteger()); // args
//                    tensor.index_put_(ind, value);
//                    break;
//                case ObjType::Float32:
//                case ObjType::Float64:
//                case ObjType::Single:
//                case ObjType::Double:
//                    value = at::Scalar(obj->Call(this)->GetValueAsNumber()); // args
//                    tensor.index_put_(ind, value);
//
//                    break;
//                default:
//                    ASSERT(!"Not implemented!");
//            }
//        }
//    }
//}
//
//void Context::ItemTensorEval(torch::Tensor &self, ObjPtr obj, ObjPtr args) {
//    if (self.dim() == 0) {
//
//        signed char *ptr_char = nullptr;
//        int16_t *ptr_short = nullptr;
//        int32_t *ptr_int = nullptr;
//        int64_t *ptr_long = nullptr;
//        float *ptr_float = nullptr;
//        double *ptr_double = nullptr;
//
//        switch (fromTorchType(self.scalar_type())) {
//            case ObjType::Int8:
//                ptr_char = self.data_ptr<signed char>();
//                ASSERT(ptr_char);
//                *ptr_char = static_cast<signed char> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Int16:
//                ptr_short = self.data_ptr<int16_t>();
//                ASSERT(ptr_short);
//                *ptr_short = static_cast<int16_t> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Int32:
//                ptr_int = self.data_ptr<int32_t>();
//                ASSERT(ptr_int);
//                *ptr_int = static_cast<int32_t> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Int64:
//                ptr_long = self.data_ptr<int64_t>();
//                ASSERT(ptr_long);
//                *ptr_long = static_cast<int64_t> (obj->Call(this)->GetValueAsInteger());
//                return;
//            case ObjType::Float32:
//                ptr_float = self.data_ptr<float>();
//                ASSERT(ptr_float);
//                *ptr_float = static_cast<float> (obj->Call(this)->GetValueAsNumber());
//                return;
//            case ObjType::Float64:
//                ptr_double = self.data_ptr<double>();
//                ASSERT(ptr_double);
//                *ptr_double = static_cast<double> (obj->Call(this)->GetValueAsNumber());
//                return;
//        }
//
//        ASSERT(!"Not implemented!");
//    } else {
//
//        c10::IntArrayRef shape = self.sizes(); // Кол-во эментов в каждом измерении
//        std::vector<Index> ind(shape.size(),
//                0); // Счетчик обхода всех эелемнтов тензора
//        ItemTensorEval_(self, shape, ind, 0, obj, args);
//    }
//}
//
//std::vector<int64_t> GetTensorShape(Context *ctx, TermPtr type, Obj * local_vars, bool eval_block) {
//    std::vector<int64_t> result(type->size());
//    for (int i = 0; i < type->size(); i++) {
//        ObjPtr temp = ctx->CreateRVal(ctx, type->at(i).second, local_vars);
//        if (temp->is_integer() || temp->is_bool_type()) {
//            result[i] = temp->GetValueAsInteger();
//        } else {
//            NL_PARSER(type->at(i).second, "Measurement dimension can be an integer only!");
//        }
//        if (result[i] <= 0) {
//
//            NL_PARSER(type->at(i).second, "Dimension size can be greater than zero!");
//        }
//    }
//    return result;
//}
//
//std::vector<Index> Context::MakeIndex(Context *ctx, TermPtr term, Obj * local_vars) {
//
//    // `at::indexing::TensorIndex` is used for converting C++ tensor indices such
//    // as
//    // `{None, "...", Ellipsis, 0, true, Slice(1, None, 2), torch::tensor({1,
//    // 2})}` into its equivalent `std::vector<TensorIndex>`, so that further
//    // tensor indexing operations can be performed using the supplied indices.
//    //
//    // There is one-to-one correspondence between Python and C++ tensor index
//    // types: Python                  | C++
//    // -----------------------------------------------------
//    // `None`                  | `at::indexing::None`
//    // `Ellipsis`              | `at::indexing::Ellipsis`
//    // `...`                   | `"..."`
//    // `123`                   | `123`
//    // `True` / `False`        | `true` / `false`
//    // `:`                     | `Slice()` / `Slice(None, None)`
//    // `::`                    | `Slice()` / `Slice(None, None, None)`
//    // `1:`                    | `Slice(1, None)`
//    // `1::`                   | `Slice(1, None, None)`
//    // `:3`                    | `Slice(None, 3)`
//    // `:3:`                   | `Slice(None, 3, None)`
//    // `::2`                   | `Slice(None, None, 2)`
//    // `1:3`                   | `Slice(1, 3)`
//    // `1::2`                  | `Slice(1, None, 2)`
//    // `:3:2`                  | `Slice(None, 3, 2)`
//    // `1:3:2`                 | `Slice(1, 3, 2)`
//    // `torch.tensor([1, 2])`) | `torch::tensor({1, 2})`
//
//    std::vector<Index> result;
//
//    if (!term->size()) {
//        NL_PARSER(term, "Index not found!");
//    }
//    for (int i = 0; i < term->size(); i++) {
//        if (!term->name(i).empty() || (term->at(i).second && term->at(i).second->isString())) {
//            NL_PARSER(term, "Named index not support '%d'!", i);
//        }
//        if (!term->at(i).second) {
//            NL_PARSER(term, "Empty index '%d'!", i);
//        }
//
//        if (term->at(i).second->getTermID() == TermID::ELLIPSIS) {
//            result.push_back(Index("..."));
//        } else {
//
//            ObjPtr temp = ctx->CreateRVal(ctx, term->at(i).second, local_vars);
//
//            if (temp->is_none_type()) {
//
//                result.push_back(Index(at::indexing::None));
//            } else if (temp->is_integer() || temp->is_bool_type()) {
//
//                if (temp->is_scalar()) {
//                    result.push_back(Index(temp->GetValueAsInteger()));
//                } else if (temp->m_tensor->dim() == 1) {
//                    result.push_back(Index(temp->m_tensor));
//                } else {
//                    NL_PARSER(term->at(i).second, "Extra dimensions index not support '%d'!", i);
//                }
//            } else if (temp->is_range()) {
//
//                int64_t start = temp->at("start").second->GetValueAsInteger();
//                int64_t stop = temp->at("stop").second->GetValueAsInteger();
//                int64_t step = temp->at("step").second->GetValueAsInteger();
//
//                result.push_back(Index(at::indexing::Slice(start, stop, step)));
//            } else {
//
//                NL_PARSER(term->at(i).second, "Fail tensor index '%d'!", i);
//            }
//        }
//    }
//    return result;
//}
//
//ObjPtr Context::CreateRVal(Context *ctx, TermPtr term, Obj * local_vars, bool eval_block, CatchType int_catch) {
//
//    if (!term) {
//        ASSERT(term);
//    }
//    ASSERT(local_vars);
//
//    ObjPtr result = nullptr;
//    ObjPtr temp = nullptr;
//    ObjPtr args = nullptr;
//    ObjPtr value = nullptr;
//    TermPtr field = nullptr;
//    std::string full_name;
//
//    result = Obj::CreateNone();
//    result->m_is_reference = !!term->m_ref;
//
//    int64_t val_int;
//    double val_dbl;
//    ObjType type;
//    bool has_error;
//    std::vector<int64_t> sizes;
//    at::Scalar torch_scalar;
//    switch (term->getTermID()) {
//
//            /*        case TermID::FIELD:
//                        if(module && module->HasFunc(term->GetFullName().c_str())) {
//                            // Если поле является функцией и она загружена
//                            result = Obj::CreateType(Obj::Type::FUNCTION,
//               term->GetFullName().c_str()); result->m_module = module;
//                            result->m_is_const = term->m_is_const;
//                            result->m_is_ref = term->m_is_ref;
//                            return result;
//                        }
//                        if(!result) {
//                            LOG_RUNTIME("Term '%s' not found!",
//               term->toString().c_str());
//                        }
//                        return result;
//             */
//
//        case TermID::TYPE:
//
//            result = ctx->GetTerm(term->GetFullName().c_str(), term->isRef());
//
//
//            has_error = false;
//            type = typeFromString(term->GetFullName(), ctx ? ctx->m_runtime.get() : nullptr, &has_error);
//            if (has_error) {
//                LOG_RUNTIME("Type name '%s' undefined!", term->GetFullName().c_str());
//            }
//            ASSERT(result);
//            ASSERT(result->m_var_type_fixed == type);
//
//            if (result->m_var_type_fixed == ObjType::Class) {
//                //@todo Virtual
//
//            }
//
//            // Размерность, если указана
//            result->m_dimensions = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
//            for (size_t i = 0; i < term->m_dims.size(); i++) {
//                result->m_dimensions->push_back(CreateRVal(ctx, term->m_dims[i], local_vars, eval_block, int_catch));
//            }
//
//            args = Obj::CreateDict();
//            for (int64_t i = 0; i < static_cast<int64_t> (term->size()); i++) {
//
//
//                if ((*term)[i].second->getTermID() == TermID::FILLING) {
//
//                    // Заполнение значений вызовом функции
//                    // :Type(1, 2, 3, ... rand() ... );
//
//
//                    ASSERT(!(*term)[i].second->Left());
//                    ASSERT((*term)[i].second->Right());
//
//
//                    ObjPtr expr = ctx->FindTerm((*term)[i].second->Right()->GetFullName());
//                    ASSERT(expr);
//
//                    if (!(*term)[i].second->Right()->isCall()) {
//                        LOG_RUNTIME("Operator filling supported function call only!");
//                    }
//
//                    if (i + 1 != term->size()) {
//                        LOG_RUNTIME("Function filling is supported for the last argument only!");
//                    }
//
//                    if (!result->m_dimensions || !result->m_dimensions->size()) {
//                        LOG_RUNTIME("Object has no dimensions!");
//                    }
//
//                    int64_t full_size = 1;
//                    for (int dim_index = 0; dim_index < result->m_dimensions->size(); dim_index++) {
//
//                        if (!(*result->m_dimensions)[dim_index].second->is_integer()) {
//                            LOG_RUNTIME("Dimension index for function filling support integer value only!");
//                        }
//
//                        full_size *= (*result->m_dimensions)[dim_index].second->GetValueAsInteger();
//                    }
//
//                    if (full_size <= 0) {
//                        LOG_RUNTIME("Items count error for all dimensions!");
//                    }
//
//
//                    if (expr->size()) {
//                        LOG_RUNTIME("Argument in function for filling not implemented!");
//                    }
//
//                    for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
//                        args->push_back(expr->Call(ctx));
//                    }
//
//                    break;
//
//                } else if ((*term)[i].second->getTermID() == TermID::ELLIPSIS) {
//
//                    if (!term->name(i).empty()) {
//                        LOG_RUNTIME("Named ellipsys not implemented!");
//                    }
//
//                    if ((*term)[i].second->Right()) {
//
//                        bool named = ((*term)[i].second->Left() && (*term)[i].second->Left()->getTermID() == TermID::ELLIPSIS);
//                        ObjPtr exp = CreateRVal(ctx, (*term)[i].second->Right(), eval_block);
//
//                        if (!exp->is_dictionary_type()) {
//                            LOG_RUNTIME("Expansion operator applies to dictionary only!");
//                        }
//
//
//                        for (int index = 0; index < exp->size(); index++) {
//                            if (named) {
//                                args->push_back((*exp)[index].second, exp->name(index).empty() ? "" : exp->name(index));
//                            } else {
//                                args->push_back((*exp)[index].second);
//                            }
//                        }
//
//                        continue;
//                    }
//                }
//
//                if (term->name(i).empty()) {
//                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars));
//                } else {
//                    args->push_back(CreateRVal(ctx, (*term)[i].second, local_vars), term->name(i).c_str());
//                }
//
//            }
//
//            result = result->Call(ctx, args.get());
//            ASSERT(result);
//
//            return result;
//
//
//    return nullptr;
//}
//
//ObjPtr Context::CreateClass(std::string class_name, TermPtr body, Obj * local_vars) {
//
//    ASSERT(false);
//    return nullptr;
//
//    //    ASSERT(body->getTermID() == TermID::CLASS);
//    //    ASSERT(body->m_base.size());
//    //
//    //    ObjPtr new_class = Obj::CreateBaseType(ObjType::Class);
//    //    new_class->m_var_name = class_name;
//    //    std::string constructor = MakeConstructorName(class_name);
//    //
//    //    // LOG_DEBUG("\nCreate class: '%s', constructor %s", class_name.c_str(), constructor.c_str());
//    //
//    //
//    //    if (class_name.find(":") == 0) {
//    //        class_name.erase(0, 1);
//    //    }
//    //
//    //    // Пройтись по всем базовым классам
//    //    for (int i = 0; i < body->m_base.size(); i++) {
//    //
//    //        ObjPtr base = GetTerm(body->m_base[i]->GetFullName().c_str(), false);
//    //
//    //        // LOG_DEBUG("Base %s: '%s' %d", body->m_base[i]->GetFullName().c_str(), base->toString().c_str(), (int) base->size());
//    //
//    //        bool has_error = false;
//    //        ObjType type = typeFromString(body->m_base[i]->GetFullName(), this, &has_error);
//    //        if (has_error) {
//    //            LOG_RUNTIME("Type name '%s' undefined!", body->m_base[i]->GetFullName().c_str());
//    //        }
//    //        ASSERT(base);
//    //        ASSERT(base->m_var_type_fixed == type);
//    //        ASSERT(base->m_class_name.compare(body->m_base[i]->GetFullName()) == 0);
//    //        ASSERT(!base->m_dimensions);
//    //
//    //        // Клонировать все методы (функции) базового класса с новыми именами
//    //        std::set<std::string> methods;
//    //        std::string base_constructor = MakeConstructorName(base->m_class_name);
//    //        // LOG_DEBUG("base_constructor %s", base_constructor.c_str());
//    //
//    //
//    //        std::string find_substr = base->m_class_name.substr(1);
//    //        find_substr += "::";
//    //
//    //        auto iter = m_terms->begin();
//    //        while (iter != m_terms->end()) {
//    //            if (iter->first.find(find_substr) == 0) {
//    //                if (base_constructor.compare(iter->first) != 0) {
//    //                    methods.insert(iter->first);
//    //                }
//    //            }
//    //            iter++;
//    //        }
//    //
//    //
//    //        std::string replace(base->m_class_name.substr(1));
//    //        replace += "::";
//    //
//    //        std::set<Variable<Obj>::PairType *> rollback;
//    //
//    //
//    //        try {
//    //
//    //            ObjPtr obj;
//    //
//    //            if (i == 0) {
//    //                // Только один конструктор у класса
//    //                obj = Obj::CreateFunc(constructor, &Obj::ConstructorStub_, ObjType::PureFunc);
//    //                push_back(obj, constructor); // weak_ptr
//    //                rollback.insert(&m_terms->push_back(obj, constructor));
//    //            }
//    //
//    //            for (auto &elem : methods) {
//    //
//    //                // LOG_DEBUG("Func: %s  Replace: %s", elem.c_str(), replace.c_str());
//    //
//    //                auto iter = m_terms->find(elem);
//    //                ASSERT(iter != m_terms->end());
//    //
//    //                obj = iter->second; //.lock();
//    //                ASSERT(obj);
//    //
//    //                std::string name(elem);
//    //                size_t pos = name.find(replace);
//    //
//    //                ASSERT(pos != name.npos);
//    //                ASSERT(pos == 0 || name[pos - 1] == ':');
//    //
//    //                name.erase(pos, replace.size() - 2);
//    //                name.insert(pos, class_name);
//    //
//    //                if (m_terms->find(name) == m_terms->end()) {
//    //                    // LOG_DEBUG("new name %s", name.c_str());
//    //                    push_back(obj, name); // weak_ptr
//    //                    rollback.insert(&m_terms->push_back(obj, name));
//    //                }
//    //            }
//    //
//    //            base->ClonePropTo(*new_class);
//    //
//    //
//    //            // Выполнить тело конструктора типа для создания новых полей и методов у создаваемого типа класса
//    //            bool is_pop_ns = m_runtime->m_macro->NamespacePush(class_name);
//    //            try {
//    //                for (int i = 0; i < body->m_block.size(); i++) {
//    //                    if (body->m_block[i]->IsCreate()) {
//    //                        ASSERT(body->m_block[i]->Left());
//    //                        if (body->m_block[i]->Left()->IsFunction() || body->m_block[i]->Left()->isCall()) {
//    //                            ObjPtr func = Eval(this, body->m_block[i], local_vars, true);
//    //
//    //                            if (body->m_block[i]->Right()->getTermID() == TermID::NONE) {
//    //                                func->m_var_type_current = ObjType::Virtual;
//    //                            }
//    //
//    //                        } else {
//    //
//    //                            std::string name = body->m_block[i]->Left()->getText();
//    //                            bool is_exists = (new_class->find(name) != new_class->end());
//    //
//    //                            if (body->m_block[i]->getText().compare("::=") == 0) {
//    //                                if (is_exists) {
//    //                                    LOG_RUNTIME("Dublicate property name '%s' in class '%s'!", name.c_str(), class_name.c_str());
//    //                                }
//    //                                new_class->push_back(nullptr, name);
//    //                            } else if (body->m_block[i]->getText().compare("=") == 0) {
//    //                                if (!is_exists) {
//    //                                    LOG_RUNTIME("Property name '%s' not found on base classes '%s'!", name.c_str(), class_name.c_str());
//    //                                }
//    //                            } else if (!is_exists) {
//    //                                new_class->push_back(nullptr, name);
//    //                            }
//    //
//    //                            new_class->at(name).second = CreateRVal(this, body->m_block[i]->Right(), local_vars, true);
//    //                        }
//    //                    } else {
//    //                        LOG_RUNTIME("Only create or assignment operators allowed! %s", body->m_block[i]->toString().c_str());
//    //                    }
//    //                }
//    //
//    //                if (is_pop_ns) {
//    //                    m_runtime->m_macro->NamespacePop();
//    //                }
//    //            } catch (...) {
//    //                if (is_pop_ns) {
//    //                    m_runtime->m_macro->NamespacePop();
//    //                }
//    //                throw;
//    //            }
//    //
//    //
//    //        } catch (...) {
//    //
//    //
//    //            for (auto &elem : rollback) {
//    //                // LOG_DEBUG("Rollback: '%s'", elem->first.c_str());
//    //                // remove(find(elem.first)); // weak_ptr
//    //                m_terms->remove(*elem);
//    //            }
//    //
//    //            throw;
//    //        }
//    //
//    //    }
//    //    new_class->m_var_is_init = true;
//    //
//    //    return new_class;
//}
//

/*
 * 
 * 
 */
ObjPtr Context::Execute(TermPtr term, Context *ctx) {
    Module::RegisterStaticObject(ctx->m_static, term, false);
    return Eval(term, ctx);
}

ObjPtr Context::Eval(TermPtr term, Context *ctx) {
    //    LOG_TEST("run: %s", RunTime::Escape(term->toString()).c_str());
    ObjPtr result = Obj::CreateNone();
    if ((term->m_id == TermID::SEQUENCE || term->m_id == TermID::BLOCK) && !ctx) {
        for (auto &elem : term->m_block) {
            result = Eval(elem, ctx);
        }
    } else if (term->isBlock()) {
        if (!ctx) {
            LOG_RUNTIME("Can't calculate '%s' in static mode!", term->toString().c_str());
        }
        result = ctx->EvalTryBlock_(term, ctx->m_static.m_ast->m_int_vars);
    } else {
        result = EvalTerm(term, ctx);
    }

    //    LOG_TEST("result: %s (%p)", result->toString().c_str(), result.get());

    if (ctx) {
        ctx->m_latter = result;
    } else {
        return result;
    }
    return ctx->m_latter;
}

bool Context::HasReThrow(TermPtr &block, Context &stack, Obj & obj) {
    ASSERT(isInterrupt(obj.getType()));
    ASSERT(block->m_id == TermID::SEQUENCE || block->m_id == TermID::BLOCK || block->m_id == TermID::BLOCK_PLUS
            || block->m_id == TermID::BLOCK_MINUS || block->m_id == TermID::BLOCK_TRY);

    if (obj.m_value.empty()) {
        // Non named interrupt
        if (obj.getType() == ObjType::RetPlus && (block->m_id == TermID::BLOCK_PLUS || block->m_id == TermID::BLOCK_TRY)) {
            return false;
        } else if (obj.getType() == ObjType::RetMinus && (block->m_id == TermID::BLOCK_MINUS || block->m_id == TermID::BLOCK_TRY)) {
            return false;
        }

    } else {
        // Named interrupt

        if (obj.m_value.compare("::") == 0) {
            // Global scope - always rethrow
            return true;
        }
        std::string ns = stack.MakeNamespace(0, true);
        if (ns.rfind(obj.m_value) != std::string::npos) {
            return false;
        }

    }
    return true;
}

ObjPtr Context::EvalTryBlock_(TermPtr &block, StorageTerm & storage) {
    ASSERT(block->isBlock());
    m_latter = getNoneObj();
    CtxPush scope_block(*this, block->m_id, block->m_namespace);

    std::string str;
    for (auto &elem : storage) {
        if (!str.empty()) {
            str += ", ";
        }
        str += elem.first;
        str += "->'";
        str += elem.second->m_obj ? elem.second->m_obj->toString() : "nullptr";
        str += "'";
    }

    //    LOG_TEST("Enter block: '%s' VARS: %s", block->m_namespace ? block->m_namespace->m_text.c_str() : "", str.c_str());

    try {
        size_t i = 0;
        while (i < block->m_block.size()) {

            m_latter = Eval(block->m_block[i], this);

            //            LOG_TEST("step: %d, value: %s", (int) i, m_latter->toString().c_str());

            if (isInterrupt(m_latter->getType())) {

                if (m_latter->m_value.empty()) {

                    // Не именованное прерывание -> выход из блока
                    break;

                } else if (CheckTargetScope(m_latter->m_value)) {
                    if (m_latter->getType() == ObjType::RetPlus) {
                        m_latter = m_latter->m_return_obj;

                        // Выход из блока
                        break;

                    } else if (m_latter->getType() == ObjType::RetMinus) {
                        m_latter = m_latter->m_return_obj;

                        // На начало блока
                        i = 0;
                        continue;

                    } else {
                        LOG_RUNTIME("Interrupt type '%s' not implemented!", toString(m_latter->getType()));
                    }
                } else {
                    // Именованное прерывание, но блок более низкого уровня
                    return m_latter;
                }
            }
            i++;
        }

    } catch (IntPlus &plus) {

        if (HasReThrow(block, *this, plus)) {
            throw;
        }
        m_latter = plus.m_return_obj;

    } catch (IntMinus &minus) {

        if (HasReThrow(block, *this, minus)) {
            throw;
        }
        m_latter = minus.m_return_obj;

    } catch (...) {

        throw;
    }

    if (isInterrupt(m_latter->getType())) {
        bool return_value = false;
        if (m_latter->getType() == ObjType::RetPlus && (block->m_id == TermID::BLOCK_PLUS || block->m_id == TermID::BLOCK_TRY)) {
            return_value = true;
        } else if (m_latter->getType() == ObjType::RetMinus && (block->m_id == TermID::BLOCK_MINUS || block->m_id == TermID::BLOCK_TRY)) {
            return_value = true;
        }

        if (return_value && block->m_left) {
            //            ASSERT(block->m_left->m_id == TermID::WITH);
            m_latter = m_latter->m_return_obj;
        }
    }

    return m_latter;
}

// Метод вызывается только из NewLnag кода

ObjPtr Context::Call(Context *runner, Obj &obj, TermPtr & term) {
    ObjPtr args = Obj::CreateDict();

    if (!term->m_dims) {
        obj.m_dimensions = nullptr;
    } else {
        // Размерность, если указана
        obj.m_dimensions = Obj::CreateType(ObjType::Dictionary, ObjType::Dictionary, true);
        for (size_t i = 0; i < term->m_dims->size(); i++) {
            obj.m_dimensions->push_back(EvalTerm(term->m_dims->at(i).second, runner), term->m_dims->at(i).first);
        }
    }

    for (int64_t i = 0; i < static_cast<int64_t> (term->size()); i++) {

        if ((*term)[i].second->getTermID() == TermID::FILLING) {

            // Заполнение значений вызовом функции
            // :Type(1, 2, 3, ... rand() ... );

            ASSERT(!(*term)[i].second->Left());
            ASSERT((*term)[i].second->Right());

            //            if (!(*term)[i].second->Right()->isCall()) {
            //                LOG_RUNTIME("Operator filling supported function call only!");
            //            }

            if (i + 1 != term->size()) {
                LOG_RUNTIME("Filling is supported for the last argument only!");
            }

            if (!obj.m_dimensions || !obj.m_dimensions->size()) {
                LOG_RUNTIME("Object has no dimensions!");
            }

            int64_t full_size = 1;
            for (int dim_index = 0; dim_index < obj.m_dimensions->size(); dim_index++) {

                if (!(*obj.m_dimensions)[dim_index].second->is_integer()) {
                    LOG_RUNTIME("Dimension index support integer value only!");
                }

                full_size *= (*obj.m_dimensions)[dim_index].second->GetValueAsInteger();
            }

            if (full_size <= 0) {
                LOG_RUNTIME("Items count '%ld' error for all dimensions!", full_size);
            }


            TermPtr fill_obj = (*term)[i].second->Right();
            ASSERT(fill_obj);

            if (fill_obj->size()) {
                LOG_RUNTIME("Argument in function for filling not implemented!");
            }

            for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
                args->push_back(EvalTerm(fill_obj, runner));
            }

            // Filling - last operator in the args
            break;

        } else if ((*term)[i].second->getTermID() == TermID::ELLIPSIS || (*term)[i].second->isNone()) {

            if (!term->name(i).empty()) {
                LOG_RUNTIME("Named ellipsys not implemented!");
            }

            if ((*term)[i].second->Right()) {

                bool named = ((*term)[i].second->Left() && (*term)[i].second->Left()->getTermID() == TermID::ELLIPSIS);
                ObjPtr exp = EvalTerm((*term)[i].second->Right(), runner);

                //                if (exp->is_funtion()) {
                //                    exp = exp->Call();
                //                }

                if (exp->is_dictionary_type()) {

                    for (int index = 0; index < exp->size(); index++) {
                        if (named) {
                            args->push_back((*exp)[index].second, exp->name(index).empty() ? "" : exp->name(index));
                        } else {
                            args->push_back((*exp)[index].second);
                        }
                    }

                } else if (exp->is_range()) {

                    if (named) {
                        LOG_RUNTIME("Named range expansion not applicable!");
                    }

                    ObjType type = getSummaryTensorType(exp.get());

                    if (isIntegralType(type, true) || isFloatingType(type) || type == ObjType::Rational) {

                        ObjPtr start = exp->at("start").second->Clone();
                        ObjPtr stop = exp->at("stop").second;
                        ObjPtr step = exp->at("step").second;

                        if (*step > Obj::CreateValue(0)) {
                            while (*start < stop) {
                                args->push_back(start->Clone());
                                *start += step;
                            }
                        } else if (*step < Obj::CreateValue(0)) {
                            while (*start > stop) {
                                args->push_back(start->Clone());
                                *start += step;
                            }
                        } else {
                            LOG_RUNTIME("Zero step in range expansion!");
                        }

                    } else {
                        LOG_RUNTIME("Range extension is only supported for integer, rational, or floating point types! (%s)", toString(type));
                    }

                } else {
                    LOG_RUNTIME("Expansion operator applies to dictionary or range only!");
                }

                continue;

            } else {
                // Заполнить до конца имеющимися значениями

                if (!obj.m_dimensions || !obj.m_dimensions->size()) {
                    LOG_RUNTIME("Object has no dimensions!");
                }

                //            if (!(*term)[i].second->Right()->isCall()) {
                //                LOG_RUNTIME("Operator filling supported function call only!");
                //            }

                if (i + 1 != term->size()) {
                    LOG_RUNTIME("Filling is supported for the last argument only!");
                }

                int64_t full_size = 1;
                for (int dim_index = 0; dim_index < obj.m_dimensions->size(); dim_index++) {

                    if (!(*obj.m_dimensions)[dim_index].second->is_integer()) {
                        LOG_RUNTIME("Dimension index support integer value only!");
                    }

                    full_size *= (*obj.m_dimensions)[dim_index].second->GetValueAsInteger();
                }

                if (full_size <= 0) {
                    LOG_RUNTIME("Items count '%ld' error for all dimensions!", full_size);
                }


                std::vector<ObjPtr> dup;

                for (size_t c = 0; c < args->size(); c++) {
                    dup.push_back(args->at(c).second);
                }

                if (!dup.size()) {
                    LOG_RUNTIME("Fill items not exist!");
                }


                if ((full_size - args->size()) % dup.size() && runner) {
                    if (runner->m_runtime->m_diag->m_fill_remainder) {
                        LOG_RUNTIME("The data is filled in with the remainder!");
                    } else {
                        LOG_WARNING("The data is filled in with the remainder!");
                        // runner->m_runtime->m_diag->Emit(Diag::DIAG_FILL_REMAINDER);
                    }
                }


                int64_t fill_index = 0;
                for (int64_t dim_index = args->size(); dim_index < full_size; dim_index++) {
                    args->push_back(dup[fill_index]->Clone());
                    fill_index++;
                    if (fill_index >= dup.size()) {
                        fill_index = 0;
                    }
                }
                // Filling - last operator in the args
                break;
            }
        }

        if (term->name(i).empty()) {
            args->push_back(EvalTerm((*term)[i].second, runner));
        } else {

            args->push_back(EvalTerm((*term)[i].second, runner), term->name(i).c_str());
        }
    }

    //    LOG_TEST("Call %s %s", obj.m_prototype ? obj.m_prototype->m_text.c_str() : "", args->toString().c_str());
    //    LOG_TEST("Local vars: %s", runner->Dump(2).c_str());

    return Call(runner, obj, *args.get());
}

// Метод может быть вызван как из NewLnag кода, так и из кода на C/C++ (в реализации Obj::operator())

ObjPtr Context::Call(Context *runner, Obj &obj, Obj & args) {

    if (obj.is_native()) {
        return Context::CallNative_(runner, obj, &args);
    }

    args.insert(args.begin(), std::pair<std::string, ObjPtr>("", obj.shared()));
    //    args->push_back(Obj::CreateValue(args->size()), "$#");
    //    args->push_back(args->Clone(), "$*");

    if (obj.is_string_type() || obj.is_dictionary_type()) {
        if (obj.is_string_type()) {
            if (obj.getType() == ObjType::FmtChar || obj.getType() == ObjType::FmtWide) {
                return StringPrintf(obj.GetValueAsString(), args);
            } else {
                return StringFormat(obj.GetValueAsString(), args);
            }
        } else {
            LOG_RUNTIME("Clone dict not implemented!");
        }
    }

    if (!runner) {
        LOG_RUNTIME("Call static not allowed!");
    }

    if (obj.is_function_type() || obj.is_type_name()) {

        if (obj.m_sequence) {
            /* Кол-во аргуметов у функции, их тип проверяются во время синтаксического анализа исходного текста.
             * Число позиционных агрументов должно быть не меньше, чем в прототипе функции и они идут по порядку, 
             * а именованные (во время вызова) могу быть в перемешку и идентифицируются по имени.
             * 
             * Значение по умолчанию подставляется во время анализа и вычислятся во время первого обращения?
             * 
             * Цикл перебора агрументов идет по прототипу функции.
             * Во вермя цикла связывается внутренни имена, но новые имена создаваться не могут!!!!
             * К дополнительным аргументам можно обратиться по индексу в переменной $$[0] $$[2] и т.д.
             */

            ASSERT(obj.m_prototype);
            StorageTerm storage(obj.m_prototype->m_int_vars);
            //(Context &ctx, const TermID id, const std::string_view &name) : m_ctx(ctx) {
            CtxPush stack(*runner, obj.m_sequence->m_id, obj.m_sequence->m_namespace);

            ObjPtr args_dict = Obj::CreateDict();

            int ags_count = obj.m_prototype->size();
            bool is_ellipsis = obj.m_prototype->is_variable_args();
            if (is_ellipsis) {
                ags_count--;
            }

            ASSERT(args.size());
            runner->back().vars.insert({"$0", CreateItem(Term::CreateIntName("$0", "$0"), args[0].second)});

            for (int i = 0; i < std::max(ags_count, (int) args.size() - 1); i++) {
                ObjPtr arg_value;
                std::string arg_name = ""; //runner->GetNamespace(true);
                if (i < ags_count) {
                    arg_name = "$";
                    if (obj.m_prototype->at(i).second->m_name.empty()) {
                        arg_name += obj.m_prototype->at(i).second->m_text;
                    } else {
                        arg_name += obj.m_prototype->at(i).second->m_name;
                    }
                    arg_name = NormalizeName(arg_name);
                } else {
                    if (!is_ellipsis && ags_count + 1 != args.size()) {
                        LOG_RUNTIME("Неожиданный аргумент! %s", args.toString().c_str());
                    }
                }

                if (i + 1 < args.size()) {
                    arg_value = args[i + 1].second;
                } else {
                    if (i < obj.m_prototype->size()) {
                        arg_value = Eval(obj.m_prototype->at(i).second, runner);
                    } else {
                        LOG_RUNTIME("Неожиданный аргумент по умолчанию!");
                    }
                }

                ASSERT(!runner->empty());
                if (!arg_name.empty()) {
                    if (runner->back().vars.find(arg_name) != runner->back().vars.end()) {
                        LOG_RUNTIME("Argname '%s' already exist!", arg_name.c_str());
                    }

                    std::string int_name = obj.m_prototype->at(i).second->m_int_name;
                    if (int_name.empty()) {
                        int_name = obj.m_prototype->at(i).second->m_name.empty() ? obj.m_prototype->at(i).second->m_text : obj.m_prototype->at(i).second->m_name;
                        int_name = NormalizeName(int_name);
                    }

                    runner->back().vars.insert({arg_name, CreateItem(Term::CreateIntName(arg_name, int_name), arg_value)});
                }
                std::string arg_num = "$";
                arg_num += std::to_string(i + 1);
                runner->back().vars.insert({arg_name, CreateItem(Term::CreateIntName(arg_num, arg_num), arg_value)});

                args_dict->push_back(arg_value, arg_name);
            }

            runner->back().vars.insert({"$*", CreateItem(Term::CreateIntName("$*", "$*", TermID::DICT), args_dict)});
            runner->back().vars.insert({"$#", CreateItem(Term::CreateIntName("$#", "$#"), Obj::CreateValue(args_dict->size()))});
            //            if (i >= proto->size()) {
            //                NL_PARSER(proto, "Argument pos %ld not exist!", i);
            //            }
            //            TermPtr argument = proto->at(i).second;
            //            if (argument->m_name.empty()) {
            //                // No default value
            //                args->push_back(EvalTerm((*term)[i].second, runner), argument->m_text);
            //            } else {
            //                args->push_back(EvalTerm((*term)[i].second, runner), argument->m_name);
            //            }

            return runner->Eval(obj.m_sequence, runner);

        } else {

            //            LOG_TEST("Args: %s", args.toString().c_str());

            ASSERT(std::holds_alternative<void *>(obj.m_var));
            FunctionType * func_ptr = (FunctionType *) std::get<void *>(obj.m_var);
            ASSERT(func_ptr);
            return (*func_ptr)(runner, args);
        }
    }

    ASSERT(!(obj.is_string_type() || obj.is_dictionary_type()));

    return obj.Clone();
}

ObjPtr Context::CallNative_(Context *runner, Obj &obj, Obj * args) {

    ASSERT(RunTime::m_ffi_prep_cif);
    ASSERT(RunTime::m_ffi_prep_cif_var);

    ffi_cif m_cif;
    std::vector<ffi_type *> m_args_type;
    std::vector<void *> m_args_ptr;

    union VALUE {
        const void *ptr;
        size_t size;
        int64_t integer;
        float number_f;
        double number_d;
        bool boolean;
    };

    std::vector<VALUE> m_args_val;
    VALUE temp;

    ASSERT(obj.m_var_type_current == ObjType::NativeFunc);
    ASSERT(obj.m_prototype);

    ASSERT(std::holds_alternative<void *>(obj.m_var));
    void * func_ptr = std::get<void *>(obj.m_var);

    if (!func_ptr) {
        LOG_RUNTIME("Dymanic load native address '%s'!", "NOT IMPLEMENTED");
    }
    //    NL_CHECK(func_ptr, "Fail load func name '%s' (%s) or fail load module '%s'!", m_prototype->m_text.c_str(),
    //            m_func_mangle_name.empty() ? m_prototype->m_text.c_str() : m_func_mangle_name.c_str(),
    //            m_module_name.empty() ? "none" : m_module_name.c_str());

    //    bool is_ellipsis = (obj.m_prototype->size() && (*obj.m_prototype)[obj.m_prototype->size() - 1].second->getTermID() == TermID::ELLIPSIS);
    //    size_t check_count = is_ellipsis ? obj.m_prototype->size() - 1 : obj.m_prototype->size();

    // Пропустить нулевой аргумент для нативных функций
    for (int i = 0; args && i < args->size(); i++) {

        ASSERT((*args)[i].second);
        if ((*args)[i].second->m_is_reference) {
            LOG_RUNTIME("Argument REFERENCE! %s", (*args)[i].second->toString().c_str());
        }

        size_t pind = i; // - 1; // Индекс прототипа на единицу меньше из-за пустого нулевого аргумента

        ObjType type = (*args)[i].second->getTypeAsLimit();
        switch (type) {
            case ObjType::Bool:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->asTypeString().c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_uint8);
                temp.boolean = (*args)[i].second->GetValueAsBoolean();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int8:
            case ObjType::Char:
            case ObjType::Byte:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->asTypeString().c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint8);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int16:
            case ObjType::Word:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint16);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int32:
            case ObjType::DWord:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint32);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Int64:
            case ObjType::DWord64:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_sint64);
                temp.integer = (*args)[i].second->GetValueAsInteger();
                m_args_val.push_back(temp);
                break;

            case ObjType::Float32:
            case ObjType::Single:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_float);
                temp.number_f = (*args)[i].second->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::Float64:
            case ObjType::Double:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_double);
                temp.number_d = (*args)[i].second->GetValueAsNumber();
                m_args_val.push_back(temp);
                break;

            case ObjType::StrChar:
            case ObjType::FmtChar:
            {
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    ObjType target = typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner));
                //                    if (!canCast(type, target)) {
                //                        if ((target == ObjType::Int8 || target == ObjType::Char || target == ObjType::Byte)
                //                                && isStringChar(type) && (*args)[i].second->size() == 1) {
                //
                //                            m_args_type.push_back(RunTime::m_ffi_type_sint8);
                //                            temp.integer = (*args)[i].second->m_value[0];
                //                            m_args_val.push_back(temp);
                //                            break;
                //
                //                        }
                //                        LOG_RUNTIME("Fail cast from '%s' to '%s'", toString(type), toString(target));
                //                    }
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_pointer);
                temp.ptr = (*args)[i].second->m_value.c_str();
                m_args_val.push_back(temp);
                break;
            }
            case ObjType::FmtWide:
            case ObjType::StrWide:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    ObjType target = typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner));
                //                    if (!canCast(type, target)) {
                //                        if (target == m_runtime->m_wide_char_type && isStringWide(type) && (*args)[i].second->size() == 1) {
                //
                //                            m_args_type.push_back(m_runtime->m_wide_char_type_ffi);
                //                            temp.integer = (*args)[i].second->m_string[0];
                //                            m_args_val.push_back(temp);
                //                            break;
                //
                //                        }
                //                        LOG_RUNTIME("Fail cast from '%s' to '%s'", toString(type), toString(target));
                //                    }
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_pointer);
                temp.ptr = (*args)[i].second->m_string.c_str();
                m_args_val.push_back(temp);
                break;

            case ObjType::Pointer:
                //                if (pind < check_count) {
                //                    NL_CHECK(!isDefaultType((*obj.m_prototype)[pind].second->m_type), "Undefined type arg '%s'", (*obj.m_prototype)[pind].second->toString().c_str());
                //                    NL_CHECK(canCast(type, typeFromString((*obj.m_prototype)[pind].second->m_type, GetRT_(runner))), "Fail cast from '%s' to '%s'",
                //                            (*obj.m_prototype)[pind].second->m_type->m_text.c_str(), newlang::toString(type));
                //                }
                m_args_type.push_back(RunTime::m_ffi_type_pointer);

                if (std::holds_alternative<void *>((*args)[i].second->m_var)) {
                    temp.ptr = std::get<void *>((*args)[i].second->m_var);
                    //                } else if (args[i].second->m_var_type_fixed == ObjType::None || args[i].second->m_var_type_current == ObjType::None ) {
                    //                    temp.ptr = nullptr;
                } else {
                    LOG_RUNTIME("Fail convert arg '%s' to pointer!", (*args)[i].second->toString().c_str());
                }

                m_args_val.push_back(temp);
                break;

            default:
                LOG_RUNTIME("Native arg type '%s' not implemented!", newlang::toString((*args)[i].second->getType()));
        }
        //        if (pind < check_count && (*obj.m_prototype)[pind].second->GetType()) {
        //            if ((*obj.m_prototype)[pind].second->GetType()->m_text.compare(newlang::toString(ObjType::FmtChar)) == 0) {
        //                NL_CHECK(newlang::ParsePrintfFormat(args, i), "Fail format string or type args!");
        //            }
        //        }
    }

    for (size_t i = 0; i < m_args_val.size(); i++) {
        m_args_ptr.push_back((void *) &m_args_val[i]);
    }

    NL_CHECK(!isDefaultType(obj.m_prototype->m_type), "Undefined return type '%s'", obj.m_prototype->toString().c_str());

    VALUE res_value;
    ffi_type *result_ffi_type = nullptr;

    ObjType return_type = RunTime::BaseTypeFromString(GetRT_(runner), obj.m_prototype->m_type->m_text);

    switch (return_type) {
        case ObjType::Bool:
            result_ffi_type = RunTime::m_ffi_type_uint8;
            break;

        case ObjType::Int8:
        case ObjType::Char:
        case ObjType::Byte:
            result_ffi_type = RunTime::m_ffi_type_sint8;
            break;

        case ObjType::Int16:
        case ObjType::Word:
            result_ffi_type = RunTime::m_ffi_type_sint16;
            break;

        case ObjType::Int32:
        case ObjType::DWord:
            result_ffi_type = RunTime::m_ffi_type_sint32;
            break;

        case ObjType::Int64:
        case ObjType::DWord64:
            result_ffi_type = RunTime::m_ffi_type_sint64;
            break;

        case ObjType::Float32:
        case ObjType::Single:
            result_ffi_type = RunTime::m_ffi_type_float;
            break;

        case ObjType::Float64:
        case ObjType::Double:
            result_ffi_type = RunTime::m_ffi_type_double;
            break;

        case ObjType::Pointer:
        case ObjType::StrChar:
        case ObjType::StrWide:
            result_ffi_type = RunTime::m_ffi_type_pointer;
            break;

        case ObjType::None:
            result_ffi_type = RunTime::m_ffi_type_void;
            break;

        default:
            LOG_RUNTIME("Native return type '%s' not implemented!", obj.m_prototype->m_type->asTypeString().c_str());
    }

    //    ASSERT(ctx->m_func_abi == FFI_DEFAULT_ABI); // Нужны другие типы вызовов ???
    if (RunTime::m_ffi_prep_cif(&m_cif, FFI_DEFAULT_ABI, static_cast<unsigned int> (m_args_type.size()), result_ffi_type, m_args_type.data()) == FFI_OK) {

        ASSERT(obj.m_prototype->m_type);

        RunTime::m_ffi_call(&m_cif, FFI_FN(func_ptr), &res_value, m_args_ptr.data());

        if (result_ffi_type == RunTime::m_ffi_type_void) {
            return Obj::CreateNone();
        } else if (result_ffi_type == RunTime::m_ffi_type_uint8) {
            // Возвращаемый тип может быть как Byte, так и Bool
            return Obj::CreateValue(static_cast<uint8_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint8) {
            return Obj::CreateValue(static_cast<int8_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint16) {
            return Obj::CreateValue(static_cast<int16_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint32) {
            return Obj::CreateValue(static_cast<int32_t> (res_value.integer), return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_sint64) {
            return Obj::CreateValue(res_value.integer, return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_float) {
            return Obj::CreateValue(res_value.number_f, return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_double) {
            return Obj::CreateValue(res_value.number_d, return_type);
        } else if (result_ffi_type == RunTime::m_ffi_type_pointer) {
            if (return_type == ObjType::StrChar) {
                return Obj::CreateString(reinterpret_cast<const char *> (res_value.ptr));
            } else if (return_type == ObjType::StrWide) {
                return Obj::CreateString(reinterpret_cast<const wchar_t *> (res_value.ptr));
            } else if (return_type == ObjType::Pointer) {
                ObjPtr result = Obj::CreateBaseType(return_type); //m_runtime->GetTypeFromString(GetRT_(runner), obj.m_prototype->m_type->m_text.c_str());
                result->m_var = (void *) res_value.ptr;
                result->m_var_is_init = true;
                return result;
            } else {
                LOG_RUNTIME("Error result type '%s' or not implemented!", obj.m_prototype->m_type->m_text.c_str());
            }
        } else {
            LOG_RUNTIME("Native return type '%s' not implemented!", obj.m_prototype->m_type->m_text.c_str());
        }
    }

    LOG_RUNTIME("Fail native call '%s'!", obj.toString().c_str());

    return Obj::CreateNone();
}

ObjPtr Context::CreateArgs_(TermPtr &term, Context * runner) {
    ObjPtr args = Obj::CreateDict();
    for (int i = 0; i < term->size(); i++) {
        ASSERT((*term)[i].second);
        if (term->name(i).empty()) {
            args->push_back(EvalTerm((*term)[i].second, runner));
        } else {

            args->push_back(EvalTerm((*term)[i].second, runner), term->name(i).c_str());
        }
    }
    return args;
}

ObjPtr Context::CreateDict(TermPtr &term, Context * runner) {

    ObjPtr result = CreateArgs_(term, runner);
    if (term->getTermID() == TermID::DICT) {

        result->m_var_type_fixed = ObjType::Class;
        result->m_class_name = term->m_class;
    }
    return result;
}

ObjPtr Context::CreateRange(TermPtr &term, Context * runner) {
    ObjPtr result = Obj::CreateDict();

    ASSERT(term->m_type);
    ObjType type = GetBaseTypeFromString(term->m_type->m_text);
    for (int i = 0; i < term->size(); i++) {
        ASSERT(!term->name(i).empty());
        result->push_back(EvalTerm((*term)[i].second, runner), term->name(i).c_str());
        result->back().second->toType_(type);
    }
    if (result->size() == 2) {
        result->push_back(Obj::CreateValue(1, type), "step");
    }
    result->m_var_type_current = ObjType::Range;
    result->m_var_type_fixed = ObjType::Range;
    result->m_var_is_init = true;

    return result;
}

ObjPtr Context::CreateTensor(TermPtr &term, Context * runner) {

    ASSERT(term->getTermID() == TermID::TENSOR);
    ASSERT(term->m_class.empty());

    ObjPtr result = CreateDict(term, runner);
    if (!term->m_type) {
        result->m_var_type_fixed = ObjType::None;
    } else {
        result->m_var_type_fixed = GetBaseTypeFromString(term->m_type->m_text); //, GetRT_(runner)
    }
    ObjType type = getSummaryTensorType(result.get(), result->m_var_type_fixed);

    if (type != ObjType::None) {

        std::vector<int64_t> sizes = TensorShapeFromDict(result.get());
        result->toType_(type);

        if (!sizes.empty()) {
            ASSERT(result->m_tensor);
            ASSERT(result->m_tensor->defined());
            *result->m_tensor = result->m_tensor->reshape(sizes);
        }

    } else {
        result->m_var_is_init = false;
    }
    result->m_var_type_current = type;

    return result;
}

ObjPtr Context::StringFormat(std::string_view format, Obj & args) {

    std::string conv_format = AstAnalysis::ConvertToVFormat_(format, args);

    fmt::dynamic_format_arg_store<fmt::format_context> store;
    for (int i = 0; i < args.size(); i++) {

        switch (args[i].second->getType()) {
            case ObjType::Bool:
            case ObjType::Int8:
            case ObjType::Byte:
            case ObjType::Char:
            case ObjType::Int16:
            case ObjType::Word:
            case ObjType::Int32:
            case ObjType::DWord:
            case ObjType::Int64:
            case ObjType::DWord64:
                store.push_back(args[i].second->GetValueAsInteger());
                break;

            case ObjType::Float16:
            case ObjType::Float32:
            case ObjType::Single:
            case ObjType::Float64:
            case ObjType::Double:
                store.push_back(args[i].second->GetValueAsNumber());
                break;

            default:
                store.push_back(args[i].second->GetValueAsString());
                break;
                // LOG_RUNTIME("Support type '%s' not implemented!", toString(type));
        }
    }

    return Obj::CreateString(fmt::vformat(conv_format, store));
}

ObjPtr Context::StringPrintf(std::string_view format, Obj & args) {
    static ObjPtr int_snprintf = RunTime::CreateNative("__snprintf__(buffer:Pointer, size:Int32, format:FmtChar, ... ):Int32", nullptr, false, "snprintf");
    ASSERT(int_snprintf);

    printf("%d %f\n", 100, 1.123);

    ObjPtr int_args = Obj::CreateDict();
    int_args->push_back(Obj::CreateNil());
    int_args->push_back(Obj::CreateValue(0));
    int_args->push_back(Obj::CreateString(format.begin()));
    for (int i = 1; i < args.size(); i++) {
        if ((int) args[i].second->m_var_type_current >= (int) ObjType::Bool && (int) args[i].second->m_var_type_current < (int) ObjType::Int32) {
            int_args->push_back(args[i].second->toType(ObjType::Int32));
        } else if (args[i].second->m_var_type_current == ObjType::Float16 || args[i].second->m_var_type_current == ObjType::Float32) {
            int_args->push_back(args[i].second->toType(ObjType::Double));
        } else {
            int_args->push_back(args[i]);
        }
    }


    ObjPtr str_size = int_snprintf->op_call(int_args);
    ASSERT(str_size);

    std::string buffer(str_size->GetValueAsInteger() + 1, '\0');

    int_args->at(0).second = Obj::CreateType(ObjType::Pointer, ObjType::Pointer, true);
    int_args->at(0).second->m_var = buffer.data();
    int_args->at(1).second = Obj::CreateValue(buffer.size());

    str_size = int_snprintf->op_call(int_args);
    ASSERT(str_size->GetValueAsInteger() + 1 == buffer.size());

    return Obj::CreateString(buffer);
}

ObjPtr Context::EvalTerm(TermPtr term, Context * runner, bool rvalue) {
    ASSERT(term);
    ASSERT(!term->isBlock());
    try {
        // Static calculate
        switch (term->m_id) {
            case TermID::INTEGER:
            {
                int64_t val_int = parseInteger(term->getText().c_str());
                NL_TYPECHECK(term, typeFromLimit(val_int), typeFromString(term->m_type, GetRT_(runner))); // Соответстствует ли тип значению?
                ObjPtr result = Obj::CreateValue(val_int);
                result->m_var_type_current = typeFromLimit(val_int);
                if (term->GetType() && !isDefaultType(term->GetType())) {
                    result->m_var_type_fixed = typeFromString(term->m_type, GetRT_(runner));
                    result->m_var_type_current = result->m_var_type_fixed;
                }
                return result;
            }
            case TermID::NUMBER:
            {
                double val_dbl = parseDouble(term->getText().c_str());
                NL_TYPECHECK(term, typeFromLimit(val_dbl), typeFromString(term->m_type, GetRT_(runner))); // Соответстствует ли тип значению?
                ObjPtr result = Obj::CreateValue(val_dbl);
                result->m_var_type_current = typeFromLimit(val_dbl);
                if (term->GetType() && !isDefaultType(term->GetType())) {
                    result->m_var_type_fixed = typeFromString(term->m_type, GetRT_(runner));
                    result->m_var_type_current = result->m_var_type_fixed;
                }
                return result;
            }
            case TermID::RATIONAL:
            {
                ObjPtr result = Obj::CreateRational(term->m_text);
                result->m_var_type_fixed = result->m_var_type_current;
                return result;
            }
            case TermID::STRWIDE:
            {
                ObjPtr result = Obj::CreateString(utf8_decode(term->getText()));
                ASSERT(result);
                if (term->isCall()) {
                    // Format string
                    result = Call(runner, *result, term);
                }
                return result;
            }
            case TermID::STRCHAR:
            {
                ObjPtr result = Obj::CreateString(term->getText());
                ASSERT(result);

                if (term->GetType()) {
                    result->m_var_type_current = typeFromString(term->m_type, GetRT_(runner));
                    result->m_var_type_fixed = result->m_var_type_current;
                }

                if (term->isCall()) {
                    // Format string
                    result = Call(runner, *result, term);
                }
                return result;
            }

            case TermID::TENSOR:
                return CreateTensor(term, runner);
            case TermID::DICT:
                return CreateDict(term, runner);
            case TermID::RANGE:
                return CreateRange(term, runner);
            case TermID::OP_MATH:
                return EvalOpMath_(term, runner);
            case TermID::OP_COMPARE:
                return EvalOpCompare_(term, runner);
            case TermID::OP_LOGICAL:
                return EvalOpLogical_(term, runner);
            case TermID::OP_BITWISE:
                return EvalOpBitwise_(term, runner);
            case TermID::FOLLOW:
                return EvalFollow_(term, runner);
            case TermID::NATIVE:
                return RunTime::CreateNative(term, RunTime::GetNativeAddress(nullptr, &term->m_text[1]));
            case TermID::ELLIPSIS:
                return getEllipsysObj();
            case TermID::END:
                return Obj::CreateNone();
            case TermID::NAME:
                if (term->isNone()) {
                    return Obj::CreateNone();
                }
        }

        if (!runner) {
            NL_PARSER(term, "The term type '%s' is not calculated statistically!", newlang::toString(term->getTermID()));
        }

        if (term->isCreate()) {
            return runner->EvalCreate_(term);
        } else if (term->isNamed()) {

#ifdef BUILD_DEBUG
            VarItem * item = nullptr;
            if (term->isNamed() && !isReservedName(term->m_text)) {
                item = runner->FindVarItem(term);
            }
            if (item) {
                if (item->term_check != item->term.get()) {
                    LOG_RUNTIME("item term: %s (%p != %p)", term->m_text.c_str(), item->term_check, item->term.get());
                }
                if (item->obj_check != item->obj.get()) {
                    if (item->obj_check) {
                        LOG_RUNTIME("item obj: %s (%p != %p)", term->m_text.c_str(), item->obj_check, item->obj.get());
                    } else {
                        item->obj_check = item->obj.get();
                    }
                }
            } else {
                //                LOG_TEST("VarItem '%s' - not found!", term->m_text.c_str());
            }
#endif       

            //            if (isReservedName(term->m_text)) {
            //                return Obj::CreateNil();
            //            }

            TermPtr found = runner->GetObject(term->m_int_name);
            if (!found || !found->m_obj) {
                NL_PARSER(term, "Object '%s' not calculated!", term->m_text.c_str());
            }
#ifdef BUILD_DEBUG
            if (item) {
                if (item->term_check != found.get()) {
                    LOG_RUNTIME("found term: %s (%p != %p)", term->m_text.c_str(), item->term_check, found.get());
                }
                if (item->obj_check != found->m_obj.get()) {
                    if (item->obj_check) {
                        LOG_ERROR("found obj: %s (%p != %p)", term->m_text.c_str(), item->obj_check, found->m_obj.get());
                    } else {
                        if (item->term->m_obj) {
                            if (item->obj || item->obj_check) {
                                LOG_RUNTIME("found obj: %s %p (%p != %p)", term->m_text.c_str(), item->term->m_obj.get(), item->obj.get(), item->obj_check);
                            }
                            item->obj = item->term->m_obj;
                            item->obj_check = item->term->m_obj.get();
                        } else {
                            item->obj = found->m_obj;
                            item->obj->m_sync = item->sync.get();
                            item->term->m_obj = item->obj;
                            item->obj_check = item->obj.get();
                        }
                    }
                }
            } else {
                //                LOG_TEST("VarItem '%s' - not found!", term->m_text.c_str());
            }
#endif       

            //            ASSERT(found);
            //            if (!term->m_type) {
            //                term->m_type = found->m_type;
            //            } else {
            //                ASSERT(!"Check type?????");
            //            }

            if (term->isCall()) {
                // Вызов функции или клонирование объекта
                if (term->m_is_take) {
                    return Obj::Take(*Call(runner, *found->m_obj, term));
                } else {
                    return Call(runner, *found->m_obj, term);
                }
            } else if (rvalue && term->m_right) {

                if (term->m_right->m_id == TermID::INDEX) {
                    return GetIndexValue(term, found->m_obj, runner);
                } else if (term->m_right->m_id == TermID::FIELD) {
                    return GetFieldValue(term, found->m_obj, runner);
                } else {
                    NL_PARSER(term, "Eval type '%s' not implemented!", toString(term->m_right->m_id));
                }

            } else if (term->m_is_take) {
                ASSERT(found);
                ASSERT(found->m_obj);
                return Obj::Take(*found->m_obj);
            } else {
                ASSERT(found);
                return found->m_obj;
            }

        } else {

            switch (term->m_id) {

                case TermID::ITERATOR:
                    return runner->EvalIterator_(term);
                case TermID::EVAL:
                    return runner->EvalEval_(term);
                case TermID::WHILE:
                    return runner->EvalWhile_(term);
                case TermID::DOWHILE:
                    return runner->EvalDoWhile_(term);
                case TermID::TAKE:
                    return runner->EvalTake_(term);

                case TermID::INT_PLUS:
                case TermID::INT_MINUS:
                case TermID::INT_REPEAT:
                    return runner->EvalInterrupt_(term);

            }

            //        if (term->GetType() && !isDefaultType(term->GetType())) {
            //            term->m_obj->m_var_type_fixed = typeFromString(term->m_type, m_runtime.get());
            //            term->m_obj->m_var_type_current = term->m_obj->m_var_type_fixed;
            //        }
        }

        LOG_RUNTIME("EvalTerm_ for type '%s'(%s) not implemented!", newlang::toString(term->m_id), term->m_text.c_str());

    } catch (std::exception & ex) {

        NL_PARSER(term, "%s", ex.what());
    }

    NL_PARSER(term, "The term type '%s' is not calculated statistically!", newlang::toString(term->getTermID()));
}

ObjPtr Context::EvalTake_(TermPtr & op) {
    ASSERT(op->size());
    CheckObjTerm_(op->at(0).second, this);
    ObjPtr args;
    if (op->isCall()) {
        args = CreateArgs_(op, this);
    } else {
        args = Obj::CreateDict();
        args->push_back(op->at(0).second->m_obj);
    }

    m_latter = Obj::Take(*args);

    return m_latter;
}

ObjPtr Context::CreateNative_(TermPtr &proto, const char *module, bool lazzy, const char *mangle_name) {
    ObjPtr result = m_runtime->CreateNative(proto, module, lazzy, mangle_name);
    result->m_ctx = this;

    return result;
}

TermPtr Context::GetObject(const std::string_view int_name) {

    //    std::string int_name;
    //    if (isInternalName(int_name)) {
    //    }
    //    int_name = NormalizeName(int_name);
    //    ASSERT();
    //
    //    std::string int_name;
    //    
    //    if (term->isNone()) {
    //        term->m_int_name = "_";
    //    }
    if (int_name.empty()) {
        LOG_RUNTIME("Has no internal name! AST analysis required!");
    }

    TermPtr result = nullptr;
    if (int_name.compare("_") == 0) {
        result = Term::CreateNone();
    } else if (int_name.compare("$^") == 0) {
        result = Term::CreateNone();
        result->m_obj = Obj::CreateNone();
        if (m_latter) {
            *result->m_obj = *m_latter;
        }
    } else {
        //        auto found = find(int_name.begin());
        //        if(found != end()){
        //            result = found->second.item;
        //        }
        result = FindInternalName(int_name);
        //        if (!result && m_rt && (isGlobalScope(int_name) || isTypeName(int_name))) {
        //            result = m_rt->GlobFindProto(int_name);
        //        }
    }
    if (!result) {
        //#ifdef BUILD_UNITTEST
        //            if (term->m_text.compare("__STAT_RUNTIME_UNITTEST__") == 0) {
        //                ASSERT(term->isCall());
        //                ASSERT(term->size() == 2);
        //                return Obj::CreateValue(m_rt->__STAT_RUNTIME_UNITTEST__(
        //                        parseInteger(term->at(0).second->m_text.c_str()),
        //                        parseInteger(term->at(1).second->m_text.c_str())));
        //            }
        //#endif
        //        LOG_RUNTIME("Object with internal name '%s' not found!", int_name.begin());
    }

    return result;
}

VarItem * Context::FindVarItem(const std::string_view name) {
    auto iter = rbegin();
    while (iter != rend()) {
        if (iter->vars.find(name.begin()) != iter->vars.end()) {
            return &iter->vars.find(name.begin())->second;
        }
        iter++;
    }
    auto found = m_static.find(name.begin());
    if (found != m_static.end()) {
        return &found->second;
    }
    return nullptr;
}

TermPtr Context::FindInternalName(const std::string_view int_name) {

    VarItem * item = FindVarItem(int_name);
    //    int_name = NormalizeName(int_name);

    //    StorageTerm & stor = getStorage_();
    //    if (&stor != &m_static && stor.find(int_name.begin()) != stor.end()) {
    //        return stor.find(int_name.begin())->second;
    //    }

    //    auto iter = rbegin();
    //    while (iter != rend()) {
    //        auto found = iter->vars.find(int_name.begin());
    //        if (found != iter->vars.end()) {
    //            return found->second.item;
    //        }
    //        iter++;
    //    }
    //
    //    auto found = m_static.find(int_name.begin());
    //    if (found != m_static.end()) {
    //        return found->second.item;
    //    }

    if (item) {
        return item->term;
    }

    if (m_runtime) {
        GlobItem * found = m_runtime->NameFind(int_name);
        if (found) {
            //            if (std::holds_alternative<ObjWeak>(found->obj)) {
            //                return std::get<ObjWeak>(found->obj).lock();
            //            } else if (std::holds_alternative<std::vector < ObjWeak >> (found->obj)) {
            //                return std::get<std::vector < ObjWeak >> (found->obj)[0].lock();
            //            }
            if (std::holds_alternative<TermWeak>(found->term)) {
                return std::get<TermWeak>(found->term).lock();
            } else if (std::holds_alternative<std::vector < TermWeak >> (found->term)) {

                return std::get<std::vector < TermWeak >> (found->term)[0].lock();
            }
        }
    }
    return nullptr;
}

ObjPtr Context::EvalCreate_(TermPtr & op) {
    ASSERT(op);
    ASSERT(op->isCreate());
    ASSERT(op->m_left);

    if (op->m_left->isCall()) {
        // Functions
        // a() :=  %func
        // a() :=  {  }     func
        // && a() :=  {  }  async func     val :=  *a();  **( val := *a()){   },[...] {  };   -> val :=  await  a() ??????????????????????
        // a() :=  %() {  } coro  co_yeld, co_return,  co_wait  ????????????????
        m_latter = EvalCreateAsFunc_(op);
    } else {
        // Variables
        if (op->m_right->m_id == TermID::ELLIPSIS) {
            // a, b, c :=  ... dict
            m_latter = EvalCreateAsEllipsis_(op);
        } else if (op->m_right->m_id == TermID::FILLING) {
            // a, b, c :=  ... dict ...      FILLING
            m_latter = EvalCreateAsFilling_(op);
        } else {
            // a, b, c :=  value
            // a, b :=  b, a    swap

            m_latter = EvalCreateAsValue_(op);
        }
    }
    return m_latter;


    //    ArrayTermType l_vars = op->m_left->CreateArrayFromList();
    //
    //    bool is_ellipsis = false;
    //    TermPtr var_found;
    //    for (auto &elem : l_vars) {
    //
    //        if (elem->getTermID() == TermID::ELLIPSIS) {
    //
    //            //@todo добавить поддержку многоточия с левой стороный оператора присвоения
    //            // NL_PARSER(elem, "Ellipsis on the left side in assignment not implemented!");
    //
    //            //  Игнорировать несколько объектов
    //            elem->m_obj = getEllipsysObj();
    //            if (is_ellipsis) {
    //                NL_PARSER(elem, "Multiple ellipsis on the left side of the assignment!");
    //            }
    //            is_ellipsis = true;
    //
    //        } else if (elem->isNone()) {
    //
    //            //  Игнорировать один объект
    //            elem->m_obj = getNoneObj();
    //
    //        } else {
    //
    //            var_found = GetObject(elem->m_int_name);
    //
    //            if (op->isCreateOnce() && var_found && var_found->m_obj) {
    //                NL_PARSER(elem, "Object '%s' already exist", elem->m_text.c_str());
    //            } else if (op->getTermID() == TermID::ASSIGN && !(var_found && var_found->m_obj)) {
    //                NL_PARSER(elem, "Object '%s' not exist!", elem->m_text.c_str());
    //            }
    //
    //            if (var_found) {
    //                if (var_found->isCall() && var_found->m_obj) {
    //                    NL_PARSER(elem, "The function cannot be overridden! '%s'", var_found->toString().c_str());
    //                }
    //                elem = var_found;
    //                //                elem->m_int_name = var_found->m_int_name;
    //                //                elem->m_obj = var_found->m_obj;
    //                //                LOG_TEST("0: %s = %s (%p)", elem->m_text.c_str(), elem->m_obj ? elem->m_obj->toString().c_str() : "nullptr", elem->m_obj.get());
    //            }
    //        }
    //    }
    //
    //    m_latter = AssignVars_(l_vars, op->m_right, op->isPure());
    //    for (auto &elem : l_vars) {
    //
    //        //        LOG_TEST("4: %s = %s (%p)", elem->m_text.c_str(), elem->m_obj ? elem->m_obj->toString().c_str() : "nullptr", elem->m_obj.get());
    //
    //        if (isGlobalScope(elem->m_int_name)) {
    //            m_runtime->NameRegister(op->isCreateOnce(), elem->m_int_name.c_str(), elem); //, elem->m_obj);
    //        }
    //    }
    //    return m_latter;
}

ObjPtr Context::EvalCreateAsFunc_(TermPtr & op) {

    ASSERT(op->isCreate());

    ArrayTermType l_vars;
    EvalLeftVars_(l_vars, op);

    ASSERT(l_vars.size() == 1);
    ASSERT(l_vars[0]->isCall());

    ASSERT(op->m_right);
    if (l_vars[0]->m_ref) {
        // && a() :=  {  }  async func     val :=  *a();  **( val := *a()){   },[...] {  };   -> val :=  await  a() ??????????????????????
        NL_PARSER(op, "Create Async function not implemented!");
    } else if (op->m_right->getTermID() == TermID::COROUTINE) {
        // a() :=  %() {  } coro  co_yeld, co_return,  co_wait  ????????????????
        NL_PARSER(op, "Create coroutine not implemented!");

    } else if (op->m_right->isBlock()) {
        // a() :=  {  }     func
        l_vars[0]->m_obj = m_runtime->CreateFunction(l_vars[0], op->m_right);

    } else if (op->m_right->getTermID() == TermID::NATIVE) {
        // a() :=  %func ...
        l_vars[0]->m_obj = m_runtime->CreateNative(l_vars[0], nullptr, false, op->m_right->m_text.substr(1).c_str());

    } else {

        NL_PARSER(op, "The type of the function being created is not recognized!");
    }

    return l_vars[0]->m_obj;
}

ObjPtr Context::EvalCreateAsEllipsis_(TermPtr & op) {
    // a, b, c :=  ... dict

    //    NL_PARSER(op, "Create ellipsis not implemented!");

    // При раскрытии словаря присвоить значение можно как одному, так и сразу нескольким терминам: 
    // var1, var2, _ = ... func(); Первый и второй элементы записывается в var1 и var2, 
    // а все остальные игнорируются (если они есть)
    // var1, var2 = ... func(); Если функция вернула словарь с двумя элементами, 
    // то их значения записываются в var1 и var2. Если в словаре было больше двух элементов, 
    // то первый записывается в var1, а все оставшиеся элементы в var2. !!!!!!!!!!!!!
    // item, dict = ... dict; Первый элементы записывается в item и удаялется из dict 

    // _, var1, ..., var2 = ... func(); 
    // Первый элемент словаря игнорируется, второй записывается в var1, а последний в var2.

    //@todo добавить поддержку многоточия с левой стороный оператора присвоения

    ArrayTermType l_vars;
    EvalLeftVars_(l_vars, op);

    bool is_named = !!op->m_right->m_left;
    if (is_named) {
        NL_PARSER(op->m_right, "Named ELLIPSIS NOT implemented!");
        ASSERT(op->m_right->m_left->getTermID() == TermID::ELLIPSIS);
    }

    bool is_last = false;
    ObjPtr right_obj = CheckObjTerm_(op->m_right->m_right, this);
    for (size_t i = 0; i < l_vars.size(); i++) {
        if (l_vars[i]->isNone()) {
            // Ignore position
            continue;
        }
        if (i + 1 == l_vars.size()) {
            is_last = true;
        }

        LOG_TEST("1: %s = %s (%p)", l_vars[i]->m_text.c_str(), l_vars[i]->m_obj ? l_vars[i]->m_obj->toString().c_str() : "nullptr", l_vars[i]->m_obj.get());

        if (i < right_obj->size()) {
            if (is_last) {
                if (l_vars[i]->m_int_name.compare(op->m_right->m_right->m_int_name) == 0) {
                    // Остаток элементов присваивается тому же словарю
                    l_vars[i]->m_obj = right_obj;
                } else {
                    l_vars[i]->m_obj = right_obj->Clone();
                }
                l_vars[i]->m_obj->erase(0, l_vars.size() - 1);
            } else {
                *l_vars[i]->m_obj = *(*right_obj)[i].second;
            }
        } else {
            if (is_last) {
                if (l_vars[i]->m_int_name.compare(op->m_right->m_right->m_int_name) == 0) {
                    // Остаток элементов присваивается тому же словарю
                    l_vars[i]->m_obj = right_obj;
                } else {
                    l_vars[i]->m_obj = right_obj->Clone();
                }
                l_vars[i]->m_obj->erase(0, 0);
            } else {
                *l_vars[i]->m_obj = *Obj::CreateNone();
            }
        }
        LOG_TEST("2: %s = %s (%p)", l_vars[i]->m_text.c_str(), l_vars[i]->m_obj ? l_vars[i]->m_obj->toString().c_str() : "nullptr", l_vars[i]->m_obj.get());
    }
    m_latter = l_vars[l_vars.size() - 1]->m_obj;
    return m_latter;
}

ObjPtr Context::EvalCreateAsFilling_(TermPtr & op) {
    // a, b, c :=  ... func() ...

    NL_PARSER(op, "Create filling not implemented!");
}

ObjPtr Context::EvalCreateAsValue_(TermPtr & op) {
    // a, b, c :=  value
    // a, b :=  b, a
    ArrayTermType l_vars;
    EvalLeftVars_(l_vars, op);

    ArrayTermType r_vars = op->m_right->CreateArrayFromList();
    if (r_vars.empty()) {
        // Delete vars
        NL_PARSER(op, "Delete var NOT implemented!");
        //            m_latter = getNoneObj();

    } else if (r_vars.size() == 1) {

        ObjPtr temp = Eval(r_vars[0], this);

        //        LOG_TEST("temp: %s = %s (%p)", r_vars[0]->m_text.c_str(), r_vars[0]->m_obj ? r_vars[0]->m_obj->toString().c_str() : "nullptr", r_vars[0]->m_obj.get());

        for (auto &elem : l_vars) {
            if (elem->getTermID() == TermID::ELLIPSIS) {
                NL_PARSER(elem, "Ellipses unexpected!");
            } else if (elem->isNone()) {
                NL_PARSER(elem, "None var unexpected!");
            } else if (elem->m_right) {

                if (elem->m_right->m_id == TermID::INDEX) {
                    SetIndexValue(elem, temp, this);
                } else if (elem->m_right->m_id == TermID::FIELD) {
                    SetFieldValue(elem, temp, this);
                } else {
                    if (elem->m_obj) {
                        *elem->m_obj = *temp;
                    } else {
                        elem->m_obj = temp;
                    }
                    //                        NL_PARSER(elem, "Eval type '%s' not implemented!", toString(elem->m_right->m_id));
                }

            } else {

                if (elem->m_obj) {
                    *elem->m_obj = *temp;
                } else {
                    elem->m_obj = temp;
                    if (elem->m_type) {

                        ObjType proto_type = typeFromString(elem->m_type, m_runtime);
                        //                        if (!canCast(proto_type, temp->getType())) {
                        //                            NL_MESSAGE(LOG_LEVEL_INFO, fill_obj, "Fail cast type '%s' to '%s' type!", fill_obj->m_type->m_text.c_str(), call->m_type->m_text.c_str());
                        //                            return false;
                        //                        }


                        elem->m_obj->m_var_type_current = proto_type;
                        elem->m_obj->m_var_type_fixed = proto_type;
                    }
                }

#ifdef BUILD_DEBUG
                VarItem * item = nullptr;
                if (elem->isNamed() && !isReservedName(elem->m_text)) {
                    item = FindVarItem(elem);
                }
                if (item) {
                    if (item->term_check != item->term.get()) {
                        LOG_RUNTIME("elem term: %s (%p != %p)", elem->m_text.c_str(), item->term_check, item->term.get());
                    }
                    if (item->obj_check != item->obj.get()) {
                        if (item->obj_check) {
                            LOG_RUNTIME("elem obj: %s (%p != %p)", elem->m_text.c_str(), item->obj_check, item->obj.get());
                        } else {
                            item->obj = elem->m_obj;
                            item->obj->m_sync = item->sync.get();
                            item->term->m_obj = item->obj;
                            item->obj_check = item->obj.get();
                        }
                    } else if (!item->obj_check) {
                        item->obj = elem->m_obj;
                        item->obj->m_sync = item->sync.get();
                        item->term->m_obj = item->obj;
                        item->obj_check = item->obj.get();
                    }
                } else {
                    LOG_TEST("VarItem '%s' - not found!", elem->m_text.c_str());
                }
#endif       
            }
        }

    } else {
        // Что присваиваем (правая часть выражения)
        // @todo В будущем можно будет сделать сахар для обмена значениями при одинаковом кол-ве объектов у оператора присваивания
        // a, b = b, a;   a, b, c = c, b, a; и т.д.

        //        if (l_vars.size() != r_vars.size()) {

        NL_PARSER(op, "Fail count right values! Expected one or %d.", (int) l_vars.size());
        //        }
        //        for (size_t i = 0; i < l_vars.size(); i++) {
        //            if (l_vars[i]->getTermID() == TermID::ELLIPSIS) {
        //                NL_PARSER(l_vars[i], "Ellipses unexpected!");
        //            } else if (l_vars[i]->isNone()) {
        //                NL_PARSER(l_vars[i], "None var unexpected!");
        //            } else {
        //
        //                m_latter = Run(r_vars[i], this);
        //                l_vars[i]->m_obj = m_latter;
        //            }
    }

    return l_vars[l_vars.size() - 1]->m_obj;
}

void Context::EvalLeftVars_(ArrayTermType &vars, const TermPtr & op) {

    vars = op->m_left->CreateArrayFromList();

    bool is_ellipsis = false;
    TermPtr var_found = nullptr;
    for (auto &elem : vars) {

        if (elem->getTermID() == TermID::ELLIPSIS) {

            //@todo добавить поддержку многоточия с левой стороный оператора присвоения
            // NL_PARSER(elem, "Ellipsis on the left side in assignment not implemented!");

            //  Игнорировать несколько объектов
            elem->m_obj = getEllipsysObj();
            if (is_ellipsis) {
                NL_PARSER(elem, "Multiple ellipsis on the left side of the assignment!");
            }
            is_ellipsis = true;

        } else if (elem->isNone()) {

            //  Игнорировать один объект
            elem->m_obj = getNoneObj();

        } else {

            var_found = GetObject(elem->m_int_name);

            if (op->isCreateOnce() && var_found && var_found->m_obj) {
                NL_PARSER(elem, "Object '%s' already exist", elem->m_text.c_str());
            } else if (op->getTermID() == TermID::ASSIGN && !(var_found && var_found->m_obj)) {
                NL_PARSER(elem, "Object '%s' not exist!", elem->m_text.c_str());
            }

            if (var_found && !op->isCreateForce()) {
                //                if (elem->isCall() && elem->m_obj) {
                //                    NL_PARSER(elem, "Recreate function not implemented!");
                //                }
                //                if (elem->m_obj) {
                //                    *elem->m_obj = *var_found->m_obj;
                //                } else {
                elem->m_obj = var_found->m_obj;
                //                }

            } else {
                ASSERT(!empty());
                if (back().vars.find(elem->m_int_name) != back().vars.end()) {
                    NL_PARSER(elem, "Local object '%s' already exist", elem->m_text.c_str());
                }

                //                VarItem item;
                //                item.item = op->m_left;
                //                item.sync = Context::CreateSync(op->m_left);
                //                item.obj = nullptr;
                back().vars.insert({elem->m_int_name, Context::CreateItem(op->m_left, nullptr)});

            }

            if (isGlobalScope(elem->m_int_name)) {

                m_runtime->NameRegister(op->isCreateOnce(), elem->m_int_name.c_str(), elem);
            }
        }
    }
}

//ObjPtr Context::AssignVars_(ArrayTermType &l_vars, const TermPtr &r_term, bool is_pure) {
//
//    ASSERT(l_vars.size());
//    if (r_term->getTermID() == TermID::NATIVE) {
//
//        ASSERT(l_vars.size() == 1);
//        //        ASSERT(vars[0]->m_obj);
//        //        ASSERT(r_term->m_obj);
//
//        l_vars[0]->m_obj = m_runtime->CreateNative(l_vars[0], nullptr, false, r_term->m_text.substr(1).c_str());
//        //        vars[0]->m_obj = r_term->m_obj;
//        m_latter = l_vars[0]->m_obj;
//
//    } else if (r_term->getTermID() == TermID::ELLIPSIS) {
//        // При раскрытии словаря присвоить значение можно как одному, так и сразу нескольким терминам: 
//        // var1, var2, _ = ... func(); Первый и второй элементы записывается в var1 и var2, 
//        // а все остальные игнорируются (если они есть)
//        // var1, var2 = ... func(); Если функция вернула словарь с двумя элементами, 
//        // то их значения записываются в var1 и var2. Если в словаре было больше двух элементов, 
//        // то первый записывается в var1, а все оставшиеся элементы в var2. !!!!!!!!!!!!!
//        // item, dict = ... dict; Первый элементы записывается в item и удаялется из dict 
//
//        // _, var1, ..., var2 = ... func(); 
//        // Первый элемент словаря игнорируется, второй записывается в var1, а последний в var2.
//
//        //@todo добавить поддержку многоточия с левой стороный оператора присвоения
//
//        bool is_named = !!r_term->m_left;
//        if (is_named) {
//            NL_PARSER(r_term, "Named ELLIPSIS NOT implemented!");
//            ASSERT(r_term->m_left->getTermID() == TermID::ELLIPSIS);
//        }
//
//        bool is_last = false;
//        ObjPtr right_obj = CheckObjTerm_(r_term->m_right, this);
//        for (size_t i = 0; i < l_vars.size(); i++) {
//            if (l_vars[i]->isNone()) {
//                // Ignore position
//                continue;
//            }
//            if (i + 1 == l_vars.size()) {
//                is_last = true;
//            }
//
//            //            LOG_TEST("1: %s = %s (%p)", l_vars[i]->m_text.c_str(), l_vars[i]->m_obj ? l_vars[i]->m_obj->toString().c_str() : "nullptr", l_vars[i]->m_obj.get());
//
//            if (i < right_obj->size()) {
//                if (is_last) {
//                    if (l_vars[i]->m_int_name.compare(r_term->m_right->m_int_name) == 0) {
//                        // Остаток элементов присваивается тому же словарю
//                        l_vars[i]->m_obj = right_obj;
//                    } else {
//                        l_vars[i]->m_obj = right_obj->Clone();
//                    }
//                    l_vars[i]->m_obj->erase(0, l_vars.size() - 1);
//                } else {
//                    l_vars[i]->m_obj = (*right_obj)[i].second;
//                }
//            } else {
//                if (is_last) {
//                    if (l_vars[i]->m_int_name.compare(r_term->m_right->m_int_name) == 0) {
//                        // Остаток элементов присваивается тому же словарю
//                        l_vars[i]->m_obj = right_obj;
//                    } else {
//                        l_vars[i]->m_obj = right_obj->Clone();
//                    }
//                    l_vars[i]->m_obj->erase(0, 0);
//                } else {
//                    l_vars[i]->m_obj = Obj::CreateNone();
//                }
//            }
//            //            LOG_TEST("2: %s = %s (%p)", l_vars[i]->m_text.c_str(), l_vars[i]->m_obj ? l_vars[i]->m_obj->toString().c_str() : "nullptr", l_vars[i]->m_obj.get());
//        }
//        m_latter = l_vars[l_vars.size() - 1]->m_obj;
//
//    } else if (r_term->getTermID() == TermID::FILLING) {
//        // Заполнение переменных значениями последовательным вызовом фукнции?
//
//        //@todo добавить поддержку многоточия с левой стороный оператора присвоения
//        NL_PARSER(r_term, "FILLING NOT implemented!");
//
//    } else if (r_term->isBlock()) {
//
//        if (l_vars.size() > 1) {
//            //@todo добавить поддержку присвоения сразу нескольким функциям
//            NL_PARSER(r_term, "Multiple function assignment not implemented!");
//        }
//        ASSERT(l_vars.size() == 1);
//
//        if (!l_vars[0]->isCall()) {
//            NL_PARSER(l_vars[0], "Function name expected!");
//        }
//
//        ASSERT(!l_vars[0]->m_right);
//
//        l_vars[0]->m_obj = m_runtime->CreateFunction(l_vars[0], r_term);
//        m_latter = l_vars[0]->m_obj;
//
//    } else {
//
//        ArrayTermType r_vars = r_term->CreateArrayFromList();
//        if (r_vars.empty()) {
//            // Delete vars
//            NL_PARSER(r_term, "NOT implemented!");
//            //            m_latter = getNoneObj();
//
//        } else if (r_vars.size() == 1) {
//
//            m_latter = Run(r_vars[0], this);
//            for (auto &elem : l_vars) {
//                if (elem->getTermID() == TermID::ELLIPSIS) {
//                    NL_PARSER(elem, "Ellipses unexpected!");
//                } else if (elem->isNone()) {
//                    NL_PARSER(elem, "None var unexpected!");
//                } else if (elem->m_right) {
//
//                    if (elem->m_right->m_id == TermID::INDEX) {
//                        SetIndexValue(elem, m_latter, this);
//                    } else if (elem->m_right->m_id == TermID::FIELD) {
//                        SetFieldValue(elem, m_latter, this);
//                    } else {
//                        elem->m_obj = m_latter;
//                        //                        NL_PARSER(elem, "Eval type '%s' not implemented!", toString(elem->m_right->m_id));
//                    }
//
//                } else {
//                    elem->m_obj = m_latter;
//                }
//            }
//
//        } else {
//            // Что присваиваем (правая часть выражения)
//            // @todo В будущем можно будет сделать сахар для обмена значениями при одинаковом кол-ве объектов у оператора присваивания
//            // a, b = b, a;   a, b, c = c, b, a; и т.д.
//
//            if (l_vars.size() != r_vars.size()) {
//                NL_PARSER(r_term, "Fail count right values! Expected one or %d.", (int) l_vars.size());
//            }
//            for (size_t i = 0; i < l_vars.size(); i++) {
//                if (l_vars[i]->getTermID() == TermID::ELLIPSIS) {
//                    NL_PARSER(l_vars[i], "Ellipses unexpected!");
//                } else if (l_vars[i]->isNone()) {
//                    NL_PARSER(l_vars[i], "None var unexpected!");
//                } else {
//
//                    m_latter = Run(r_vars[i], this);
//                    l_vars[i]->m_obj = m_latter;
//                }
//            }
//        }
//    }
//
//    return m_latter;
//}

ObjPtr Context::SetIndexValue(TermPtr &term, ObjPtr &value, Context * runner) {

    ASSERT(term->m_right);

    TermPtr index = term->m_right;
    ASSERT(index->size());

    switch (term->m_id) {
        case TermID::DICT:
        case TermID::STRCHAR:
        case TermID::STRWIDE:
            if (index->size() > 1) {
                NL_PARSER(index, "Strings and dictionaries do not support multiple dimensions!");
            }
        case TermID::TENSOR:
            break;

        case TermID::NAME:
        case TermID::LOCAL:
        case TermID::TYPE:
        case TermID::ARGS:
        case TermID::ARGUMENT:
        case TermID::STATIC:
            break;

        default:
            NL_PARSER(term, "Term type '%s' not indexable!", toString(term->m_id));
    }

    ObjPtr obj = CheckObjTerm_(term, runner, false);
    ASSERT(obj);
    ObjPtr ind = CheckObjTerm_(index->at(0).second, runner);
    ASSERT(ind);

    if (isStringChar(obj->getType())) {

        if (canCast(value->getType(), ObjType::Int8)) {
            obj->index_set_({ind->GetValueAsInteger()}, value);
        } else {
            NL_PARSER(term, "Fail cast from '%s' to ':Int8'", toString(value->getType()));
        }
        return value;

    } else if (isStringWide(obj->getType())) {

        if (canCast(value->getType(), runner->m_runtime->m_wide_char_type)) {
            obj->index_set_({ind->GetValueAsInteger()}, value);
        } else {

            NL_PARSER(term, "Fail cast from '%s' to '%s'", toString(value->getType()), newlang::toString(runner->m_runtime->m_wide_char_type));
        }
        return value;
    }
    NL_PARSER(term, "Index type '%s' not implemented!", toString(obj->getType()));
}

ObjPtr Context::GetIndexValue(TermPtr &term, ObjPtr &obj, Context * runner) {
    ASSERT(term->m_right);

    TermPtr index = term->m_right;
    ASSERT(index->size());

    switch (term->m_id) {
        case TermID::DICT:
        case TermID::STRCHAR:
        case TermID::STRWIDE:
            if (index->size() > 1) {
                NL_PARSER(index, "Strings and dictionaries do not support multiple dimensions!");
            }
        case TermID::TENSOR:
            break;

        case TermID::NAME:
        case TermID::LOCAL:
        case TermID::TYPE:
        case TermID::ARGS:
        case TermID::ARGUMENT:
        case TermID::STATIC:
            break;

        default:
            NL_PARSER(term, "Term type '%s' not indexable!", toString(term->m_id));
    }

    ObjPtr ind = CheckObjTerm_(index->at(0).second, runner);
    ASSERT(ind);

    if (isIndexingType(obj->m_var_type_current, obj->m_var_type_fixed)) {
        return obj->index_get({ind->GetValueAsInteger()});
    }

    NL_PARSER(term, "Index type '%s' not implemented!", toString(obj->getType()));
}

ObjPtr Context::GetFieldValue(TermPtr &term, ObjPtr &value, Context * runner) {

    NL_PARSER(term, "GetFieldValue not implemented!");
}

ObjPtr Context::SetFieldValue(TermPtr &term, ObjPtr &value, Context * runner) {

    NL_PARSER(term, "SetFieldValue not implemented!");
}

/*
 * 
 * 
 * 
 */

ObjPtr Context::EvalOpLogical_(TermPtr & op, Context * runner) {
    ASSERT(op);
    if (op->m_id == TermID::OP_MATH || op->m_id == TermID::OP_LOGICAL) {
        //        if(op->m_text.compare(==))
    }

    NL_PARSER(op, "EvalOpOther '%s' not implemented!", op->m_text.c_str());
}

ObjPtr Context::EvalOpMath_(TermPtr & op, Context * runner) {
    ASSERT(op);
    ASSERT(op->m_id == TermID::OP_MATH);
    ASSERT(op->m_left);
    ASSERT(op->m_right);

    op->m_left->m_obj = EvalTerm(op->m_left, runner);
    op->m_right->m_obj = EvalTerm(op->m_right, runner);

    ObjPtr result = op->m_left->m_obj;
    if (op->m_text.rfind("=") == op->m_text.size() - 2) {
        result = result->Clone();
    }

    if (op->m_text.find("+") == 0) {
        LOG_TEST_DUMP("before: %s + %s", result->GetValueAsString().c_str(), op->m_right->m_obj->GetValueAsString().c_str());
        (*result) += op->m_right->m_obj;
        LOG_TEST_DUMP("after: %s + %s", result->GetValueAsString().c_str(), op->m_right->m_obj->GetValueAsString().c_str());
    } else if (op->m_text.find("-") == 0) {
        (*result) -= op->m_right->m_obj;
    } else if (op->m_text.find("*") == 0) {
        (*result) *= op->m_right->m_obj;
    } else if (op->m_text.find("//") == 0) {
        result->op_div_ceil_(op->m_right->m_obj);
    } else if (op->m_text.find("/") == 0) {
        (*result) /= op->m_right->m_obj;
    } else if (op->m_text.find("%") == 0) {
        (*result) %= op->m_right->m_obj;
    } else {

        NL_PARSER(op, "Math operator '%s' not implemented!", op->m_text.c_str());
    }
    return result;
}

ObjPtr Context::EvalOpCompare_(TermPtr & op, Context * runner) {
    ASSERT(op);
    ASSERT(op->m_id == TermID::OP_COMPARE);
    ASSERT(op->m_left);
    ASSERT(op->m_right);
    CheckObjTerm_(op->m_left, runner);
    CheckObjTerm_(op->m_right, runner);


    LOG_TEST("Compare: %s  %s  %s", op->m_left->m_obj->toString().c_str(), op->m_text.c_str(), op->m_right->m_obj->toString().c_str());

    ObjPtr result;
    if (op->m_text.compare("==") == 0) {
        result = Obj::CreateBool(op->m_left->m_obj->op_equal(op->m_right->m_obj));
    } else if (op->m_text.compare("!=") == 0) {
        result = Obj::CreateBool(!op->m_left->m_obj->op_equal(op->m_right->m_obj));
    } else if (op->m_text.compare("<") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) < op->m_right->m_obj);
    } else if (op->m_text.compare("<=") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) <= op->m_right->m_obj);
    } else if (op->m_text.compare(">") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) > op->m_right->m_obj);
    } else if (op->m_text.compare(">=") == 0) {
        result = Obj::CreateBool((*op->m_left->m_obj) >= op->m_right->m_obj);
    } else if (op->m_text.compare("===") == 0) {
        result = Obj::CreateBool(op->m_left->m_obj->op_accurate(op->m_right->m_obj));
    } else if (op->m_text.compare("!==") == 0) {
        result = Obj::CreateBool(!op->m_left->m_obj->op_accurate(op->m_right->m_obj));
    } else {

        NL_PARSER(op, "Compare operator '%s' not implemented!", op->m_text.c_str());
    }
    return result;
}

ObjPtr Context::EvalWhile_(TermPtr & term) {
    ASSERT(term->Left());
    ASSERT(term->Right());

    ObjPtr cond = Eval(term->Left(), this);
    if (!cond->GetValueAsBoolean() && !term->m_follow.empty()) {
        // else
        ASSERT(term->m_follow.size() == 1);
        ASSERT(term->m_follow[0]->Left()->m_id == TermID::ELLIPSIS);
        m_latter = Eval(term->m_follow[0]->Right(), this);
    } else {
        size_t i = 0;
        while (cond->GetValueAsBoolean()) {

            m_latter = Eval(term->Right(), this);

            //            LOG_TEST("while: %d, value: %s", (int) i, m_latter->toString().c_str());

            if (isInterrupt(m_latter->getType())) {

                if (m_latter->m_value.empty()) {

                    // Не именованное прерывание -> выход из блока
                    break;

                } else if (CheckTargetScope(m_latter->m_value)) {
                    if (m_latter->getType() == ObjType::RetPlus) {
                        m_latter = m_latter->m_return_obj;

                        // Выход из блока
                        break;

                    } else if (m_latter->getType() == ObjType::RetMinus) {
                        m_latter = m_latter->m_return_obj;

                        // На начало блока
                        i = 0;
                        goto while_continue;

                    } else {

                        LOG_RUNTIME("Interrupt type '%s' not implemented!", toString(m_latter->getType()));
                    }
                }
            }
            i++;

while_continue:
            cond = Eval(term->Left(), this);
        }
    }
    return m_latter;
}

ObjPtr Context::EvalDoWhile_(TermPtr & term) {
    ASSERT(term);
    ASSERT(term->Left());
    ASSERT(term->Right());

    ObjPtr cond;
    size_t i = 0;
    do {

        m_latter = Eval(term->Left(), this);

        LOG_TEST("dowhile: %d, value: %s", (int) i, m_latter->toString().c_str());

        if (isInterrupt(m_latter->getType())) {

            if (m_latter->m_value.empty()) {

                // Не именованное прерывание -> выход из блока
                break;

            } else if (CheckTargetScope(m_latter->m_value)) {
                if (m_latter->getType() == ObjType::RetPlus) {
                    m_latter = m_latter->m_return_obj;

                    // Выход из блока
                    break;

                } else if (m_latter->getType() == ObjType::RetMinus) {
                    m_latter = m_latter->m_return_obj;

                    // На начало блока
                    i = 0;
                    goto dowhile_continue;

                } else {
                    LOG_RUNTIME("Interrupt type '%s' not implemented!", toString(m_latter->getType()));
                }
            }
        }
        i++;

dowhile_continue:

        cond = Eval(term->Right(), this);

    } while (cond->GetValueAsBoolean());

    return m_latter;
}

ObjPtr Context::EvalFollow_(TermPtr & term, Context * runner) {
    /*
     * [cond] --> {expr};
     * [cond] --> {expr}, [...] --> {else};
     * 
     */

    for (int64_t i = 0; i < static_cast<int64_t> (term->m_follow.size()); i++) {

        ASSERT(term->m_follow[i]);
        ASSERT(term->m_follow[i]->Left());

        bool condition = false;
        if (term->m_follow[i]->Left()->getTermID() == TermID::ELLIPSIS) {
            ASSERT(i + 1 == term->m_follow.size());
            condition = true;
        } else {
            ObjPtr cond = Eval(term->m_follow[i]->Left(), runner);
            condition = cond->GetValueAsBoolean();
        }

        if (condition) {

            return Eval(term->m_follow[i]->Right(), runner);
        }
    }
    return Obj::CreateNone();
}

ObjPtr Context::EvalEval_(TermPtr & term) {

    if (!m_runtime->m_eval_enable) {
        NL_PARSER(term, "The eval operator cannot be used! Use flag: --nlc-eval-enable");
    }

    /*
     * subprocess.run(args, *, stdin=None, input=None, stdout=None, stderr=None, 
     * capture_output=False, shell=False, cwd=None, timeout=None, check=False, 
     * encoding=None, errors=None, text=None, env=None, universal_newlines=None, **other_popen_kwargs)            
     */
    /* class subprocess.Popen(args, bufsize = -1, executable = None, 
     * stdin = None, stdout = None, stderr = None, 
     * preexec_fn = None, close_fds = True, shell = False, cwd = None, 
     * env = None, universal_newlines = None, startupinfo = None, 
     * creationflags = 0, restore_signals = True, start_new_session = False, 
     * pass_fds = (), *, group = None, extra_groups = None, user = None, umask = -1
     * , encoding = None, errors = None, text = None, 
     * pipesize = -1, process_group = None)¶
     */


    // we use std::array rather than a C-style array to get all the
    // C++ array convenience functions
    std::array<char, 128> buffer;
    std::string result;

    // popen() receives the command and parameter "r" for read,
    // since we want to read from a stream.
    // by using unique_ptr, the pipe object is automatically cleaned
    // from memory once we've read all the data from the pipe.
    std::unique_ptr<FILE, decltype(&pclose) > pipe(popen(term->getText().c_str(), "r"), pclose);

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {

        result += buffer.data();
    }

    return Obj::CreateString(result);
}

///*
// * Создание итератора
// * ?, ?(), ?("Фильтр"), ?(func), ?(func, args...)
// * 
// * Перебор элементов итератора
// * !, !(), !(0), !(3), !(-3)
// * 
// * dict! и dict!(0) эквивалентны
// * dict! -> 1,  dict! -> 2, dict! -> 3, dict! -> 4, dict! -> 5, dict! -> :IteratorEnd
// * 
// * Различия отрицательного размера возвращаемого словаря для итератора
// * dict!(-1) -> (1,),  ...  dict!(-1) -> (5,),  dict!(-1) -> (:IteratorEnd,),  
// * dict!(1) -> (1,),  ...  dict!(1) -> (5,),  dict!(1) -> (,),  
// * dict!(-3) -> (1, 2, 3,),  dict!(-3) -> (4, 5, :IteratorEnd,)
// * dict!(3) -> (1, 2, 3,), dict!(3) -> (4, 5,)
// * 
// * Операторы ?! и !? эквивалентны и возвращают текущие данные без перемещения указателя итератора.
// * 
// * Оператор ?? создает итератор и сразу его выполняет, возвращая все значения 
// * в виде элементов словаря, т.е. аналог последовательности ?(LINQ); !(:Int64.__max__);
// * 
// * Оператор !! - сбрасывает итератор в начальное состояние и возвращает первый элемент
// */
//

ObjPtr Context::EvalIterator_(TermPtr & op) {
    ASSERT(op);
    ASSERT(op->m_left);

    ObjPtr object;
    if (isReservedName(op->m_left->m_text)) {
        if (op->m_left->m_text.compare("$") == 0) {

            object = Obj::CreateDict();
            for (auto &scope : * this) {
                for (auto &elem : scope.vars) {
                    ASSERT(elem.second.term);
                    object->push_back(Obj::CreateString(elem.second.term->m_text));
                }
            }
            return object;

            //        } else if (op->m_left->m_text.compare("@") == 0) {
            //
            //            object = Obj::CreateDict();
            //            if (m_runtime && m_runtime->m_macro) {
            //                for (auto &elem : *(m_runtime->m_macro)) {
            //                    object->push_back(Obj::CreateString(elem.first));
            //                }
            //            }
            //            return object;

        } else if (op->m_left->m_text.compare("$^") == 0) {
            return m_latter;
        } else if (op->m_left->m_text.compare("@::") == 0) {

            object = Obj::CreateDict();
            for (auto &elem : m_static) {
                object->push_back(Obj::CreateString(elem.first));
            }
            return object;
            //        } else if (op->m_left->m_text.compare("%") == 0) {
            //        } else if (op->m_left->m_text.compare("$$") == 0) {
            //        } else if (op->m_left->m_text.compare("@$") == 0) {
            //        }
        } else {
            NL_PARSER(op, "Eval iterator for reserved word '%s' not implemented!", op->m_left->m_text.c_str());
        }
    } else {
        object = EvalTerm(op->m_left, this);
    }

    ObjPtr args = CreateArgs_(op, this);

    //    args->insert(args->begin(), {
    //        "", object
    //    });

    if (op->m_text.compare("?") == 0) {

        return object->IteratorMake(args.get());

    } else if (op->m_text.compare("!") == 0) {

        ASSERT(!args->size() && "Argument processing not implemented");
        return object->IteratorNext(0);

    } else if (op->m_text.compare("!!") == 0) {

        ASSERT(!args->size() && "Argument processing not implemented");
        object->IteratorReset();
        return object->IteratorData();

    } else if (op->m_text.compare("?!") == 0 || op->m_text.compare("!?") == 0) {

        return object->IteratorData();

    } else if (op->m_text.compare("??") == 0) {

        ObjPtr result;
        int64_t val_int = std::numeric_limits<int64_t>::max();
        if (args->empty() || (args->size() == 1 && args->at(0).second->is_integer())) {
            result = object->IteratorMake(nullptr, false);
            if (args->size()) {
                val_int = args->at(0).second->GetValueAsInteger();
            }
        } else if (args->size() == 1 && args->at(0).second->is_string_type()) {
            result = object->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
        } else if (args->size() == 2 && args->at(0).second->is_string_type() && args->at(1).second->is_integer()) {
            result = object->IteratorMake(args->at(0).second->GetValueAsString().c_str(), false);
            val_int = args->at(1).second->GetValueAsInteger();
        } else {

            LOG_RUNTIME("Iterator`s args '%s' not allowed!", args->toString().c_str());
        }
        return result->IteratorNext(val_int);

    }
    NL_PARSER(op, "Eval iterator '%s' not implemented!", op->m_text.c_str());
}

ObjPtr Context::EvalOpBitwise_(TermPtr & op, Context * runner) {

    ASSERT(op);
    NL_PARSER(op, "Bitwise operator '%s' not implemented!", op->m_text.c_str());
}

////ObjPtr Context::EvalOp_(TermPtr & op) {
////    ASSERT(op);
////    ObjPtr result;
////    //    switch (obj->getTermID()) {
////    //        case TermID::INT_PLUS:
////    //            result = Obj::CreateString(obj->m_namespace ? obj->m_namespace->m_text : "");
////    //            result->m_var_type = ObjType::RetPlus;
////    //            break;
////    //        case TermID::INT_MINUS:
////    //            result = Obj::CreateString(obj->m_namespace ? obj->m_namespace->m_text : "");
////    //            result->m_var_type = ObjType::RetMinus;
////    //            break;
////    //        case TermID::INT_REPEAT:
////    //            result = Obj::CreateString(obj->m_namespace ? obj->m_namespace->m_text : "");
////    //            result->m_var_type = ObjType::RetRepeat;
////    //            break;
////    //        default:
////    //            NL_PARSER(term, "EvalOp for type '%s' not implemented!", toString(obj->getTermID()));
////    //    }
////    return result;
////}
//
//
////VERIFY(CreateMacro("@@ exit(code) @@ ::= @@ :: ++ @$code ++ @@"));
////VERIFY(CreateMacro("@@ abort() @@ ::= @@ :: -- @@"));

ObjPtr Context::EvalInterrupt_(TermPtr & term) {
    ASSERT(term);
    ObjType type;
    switch (term->m_id) {
        case TermID::INT_PLUS:
            type = ObjType::RetPlus;
            break;
        case TermID::INT_MINUS:
            type = ObjType::RetMinus;
            break;
        case TermID::INT_REPEAT:
            type = ObjType::RetRepeat;
            break;
        default:
            NL_PARSER(term, "Term '%s' is not interrupt!", toString(term->m_id));
    }
    ObjPtr result = Obj::CreateType(type, type, true);
    if (term->Right()) {
        result->m_return_obj = EvalTerm(term->Right(), this);
    } else {
        result->m_return_obj = Obj::CreateNone();
    }
    if (!term->m_namespace || term->m_namespace->m_text.compare("::") == 0) {
        switch (type) {
            case ObjType::RetPlus:
                throw IntPlus(result);
            case ObjType::RetMinus:
                throw IntMinus(result);
            case ObjType::RetRepeat:
                NL_PARSER(term, "Type RetRepeat not implemented!");
            default:
                NL_PARSER(term, "Type '%s' is not interrupt!", toString(type));
        }
    }
    result->m_value = term->m_namespace->m_text;

    if (!result->m_value.empty() && result->m_value.rfind("::") != result->m_value.size() - 2) {
        result->m_value += "::";
    }

    return result;
}


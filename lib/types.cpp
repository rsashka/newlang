
#include "types.h"
#include "term.h"
#include "object.h"

using namespace newlang;

ObjPtr newlang::ObjCreateInteger(int64_t value) {
    return Obj::CreateValue(value);
}

ObjPtr newlang::ObjCreateNumber(double value) {
    return Obj::CreateValue(value);
}

ObjPtr newlang::ObjCreateRational(const Rational &value) {
    return Obj::CreateRational(value);
}

ObjPtr newlang::ObjCreateString(const std::string_view str) {
    return Obj::CreateString(str);
}

ObjPtr newlang::ObjCreateWString(const std::wstring_view str) {
    return Obj::CreateString(str);
}

ObjPtr newlang::ObjCreatePointer(void * ptr) {
    return Obj::CreatePointer(ptr);
}

std::unique_ptr<Sync> Sync::CreateSync(const TermPtr &term) {
    ASSERT(term);
    if (term->m_ref) {
        RefType ref_type = RefTypeFromString(term->m_ref->m_text);
        if (ref_type != RefType::None) {
            return std::make_unique<Sync>(ref_type);
        }
    }
    return std::unique_ptr<Sync>(nullptr);
}

/*
 * 
 * 
 */
VarGuard::VarGuard(const VarGuard *copy, const std::chrono::milliseconds & timeout_duration)
: m_var(copy ? copy->m_var : nullptr), m_edit_mode(copy ? copy->m_edit_mode : false) {
    if (!m_var) {
        LOG_RUNTIME("Fail take!");
    }
    if (m_var->sync) {
        m_is_locked = m_var->sync->SyncLock(m_edit_mode, timeout_duration);
    }
}

VarGuard::VarGuard(const VarData *var, bool edit_mode, const std::chrono::milliseconds & timeout_duration)
: m_var(const_cast<VarData *> (var)), m_edit_mode(edit_mode) {
    if (!m_var) {
        LOG_RUNTIME("Fail take!");
    }
    if (m_var->sync) {
        m_is_locked = m_var->sync->SyncLock(edit_mode, timeout_duration);
    }
}

VarGuard::~VarGuard() {
    if (m_var && m_var->sync && m_is_locked) {
        m_var->sync->SyncUnLock();
    }
}

/*
 * 
 * 
 */
VarData VarData::Copy(const std::chrono::milliseconds & timeout_duration) const {
    if (owner) {
        return VarData(owner); // copy shared_ptr
    } else if (is_taked()) {
        return VarData(VarGuard(std::get<std::unique_ptr < VarGuard >> (data).get(), timeout_duration));
    } else if (std::holds_alternative<std::monostate>(data)) {
        return VarData(std::monostate());
    } else if (std::holds_alternative<std::shared_ptr < VarData >> (data)) {
        return VarData(std::get<std::shared_ptr < VarData >> (data));
    } else if (std::holds_alternative<std::weak_ptr < VarData >> (data)) {
        return VarData(std::get<std::weak_ptr < VarData >> (data));
    } else if (std::holds_alternative<ObjPtr>(data)) {
        return VarData(std::get<ObjPtr>(data));
    } else if (std::holds_alternative<int64_t>(data)) {
        return VarData(std::get<int64_t>(data));
    } else if (std::holds_alternative<double>(data)) {
        return VarData(std::get<double>(data));
    } else if (std::holds_alternative<std::string>(data)) {
        return VarData(std::get<std::string>(data));
    } else if (std::holds_alternative<std::wstring>(data)) {
        return VarData(std::get<std::wstring>(data));
    } else if (std::holds_alternative<void *>(data)) {
        return VarData(std::get<void *>(data));
    } else if (std::holds_alternative<Rational>(data)) {
        return VarData(std::get<Rational>(data));
    } else {
        LOG_RUNTIME("Fail copy index '%zu'!", data.index());
    }
}

VarData VarData::Ref() const {
    if (owner) {
        return VarData(std::weak_ptr<VarData>(owner));
    } else if (std::holds_alternative<std::shared_ptr < VarData >> (data)) {
        return VarData(std::weak_ptr<VarData>(std::get<std::shared_ptr < VarData >> (data)));
    } else if (std::holds_alternative<std::weak_ptr < VarData >> (data)) {
        return VarData(std::get<std::weak_ptr < VarData >> (data));
    } else if (is_taked()) {
        return std::get<std::unique_ptr < VarGuard >> (data)->m_var->Ref();
    } else {
        LOG_RUNTIME("Fail logic Ref()!");
    }
}

std::unique_ptr<VarGuard> VarData::MakeTake(bool edit_mode, const std::chrono::milliseconds & timeout_duration) const {
    if (owner) {
        return std::make_unique<VarGuard>(this, edit_mode, timeout_duration);
    } else if (std::holds_alternative<std::shared_ptr < VarData >> (data)) {
        return std::make_unique<VarGuard>(std::get<std::shared_ptr < VarData >> (data).get(), edit_mode, timeout_duration);
    } else if (std::holds_alternative<std::weak_ptr < VarData >> (data)) {
        return std::make_unique<VarGuard>(std::get<std::weak_ptr < VarData >> (data).lock().get(), edit_mode, timeout_duration);
    } else {
        LOG_RUNTIME("Fail logic Take()!");
    }
}

void VarData::set(const ObjPtr & new_value, bool edit_mode, const std::chrono::milliseconds & timeout_duration) {
    ASSERT(new_value);
    std::unique_ptr<VarGuard> self_taken;
    VarGuard * var_tacken = is_taked() ? std::get<std::unique_ptr < VarGuard >> (data).get() : nullptr;
    if (!var_tacken) {
        self_taken = MakeTake(edit_mode, timeout_duration);
        var_tacken = self_taken.get();
    }
    if (!var_tacken || !var_tacken->m_var) {
        LOG_RUNTIME("No data borrowed!");
    }

    if (std::holds_alternative<ObjPtr>(var_tacken->m_var->data)) {
        std::get<ObjPtr>(var_tacken->m_var->data)->SetValue_(new_value);
    } else if (std::holds_alternative<int64_t>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = static_cast<int64_t> (*new_value);
    } else if (std::holds_alternative<double>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = static_cast<double> (*new_value);
    } else if (std::holds_alternative<std::string>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = static_cast<std::string> (*new_value);
    } else if (std::holds_alternative<std::wstring>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = static_cast<std::wstring> (*new_value);
    } else if (std::holds_alternative<void *>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = static_cast<void *> (*new_value);
    } else if (std::holds_alternative<Rational>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = *new_value->GetValueAsRational();
    } else if (std::holds_alternative<std::monostate>(var_tacken->m_var->data)) {
        LOG_RUNTIME("Object not initialized!");
    } else {
        LOG_RUNTIME("Fail logic set object or incompatible data type index '%zu'!", var_tacken->m_var->data.index());
    }
}

template <typename T>
typename std::enable_if<!std::is_same<T, ObjPtr>::value, void>::type
VarData::set(const T & new_value, bool edit_mode, const std::chrono::milliseconds & timeout_duration) {
    std::unique_ptr<VarGuard> self_taken;
    VarGuard * var_tacken = is_taked() ? std::get<std::unique_ptr < VarGuard >> (data).get() : nullptr;
    if (!var_tacken) {
        self_taken = MakeTake(edit_mode, timeout_duration);
        var_tacken = self_taken.get();
    }
    if (!var_tacken || !var_tacken->m_var) {
        LOG_RUNTIME("No data borrowed!");
    }
    if (std::holds_alternative<T>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = new_value;
    } else if (std::holds_alternative<ObjPtr>(var_tacken->m_var->data)) {
        std::get<ObjPtr>(var_tacken->m_var->data)->SetValue_(new_value);
    } else if (std::holds_alternative<std::monostate>(var_tacken->m_var->data)) {
        LOG_RUNTIME("Object not initialized!");
    } else {
        LOG_RUNTIME("Fail logic set object or incompatible data type index '%zu'!", var_tacken->m_var->data.index());
    }
}

VarData & VarData::__iadd__(VarData &self, const VarData &other) {
    std::unique_ptr<VarGuard> self_taken;
    VarGuard * var_tacken = self.is_taked() ? std::get<std::unique_ptr < VarGuard >> (self.data).get() : nullptr;
    if (!var_tacken) {
        self_taken = self.MakeTake(true, Sync::SyncWithoutWait);
        var_tacken = self_taken.get();
    }
    if (!var_tacken || !var_tacken->m_var) {
        LOG_RUNTIME("No data borrowed!");
    }
    if (!var_tacken->m_edit_mode) {
        LOG_RUNTIME("Take not edit mode!");
    }

    if (std::holds_alternative<int64_t>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = (std::get<int64_t>(var_tacken->m_var->data) + other.get<int64_t>());
    } else if (std::holds_alternative<double>(var_tacken->m_var->data)) {
        var_tacken->m_var->data = (std::get<double>(var_tacken->m_var->data) + other.get<double>());
    } else if (std::holds_alternative<std::string>(var_tacken->m_var->data)) {
        std::get<std::string>(var_tacken->m_var->data) += other.get<std::string>();
    } else if (std::holds_alternative<std::wstring>(var_tacken->m_var->data)) {
        std::get<std::wstring>(var_tacken->m_var->data) += other.get<std::wstring>();
    } else if (std::holds_alternative<Rational>(var_tacken->m_var->data)) {
        std::get<Rational>(var_tacken->m_var->data) += other.get<Rational>();
    } else if (std::holds_alternative<ObjPtr>(var_tacken->m_var->data)) {
        *std::get<ObjPtr>(var_tacken->m_var->data) += other.get<ObjPtr>();
    } else if (std::holds_alternative<std::monostate>(var_tacken->m_var->data)) {
        LOG_RUNTIME("Object not initialized!");
    } else {
        LOG_RUNTIME("Fail logic set object or incompatible data type index '%zu'!", var_tacken->m_var->data.index());
    }
    return self;
}

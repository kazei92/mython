#include "comparators.h"
#include "object.h"
#include "object_holder.h"

#include <functional>
#include <optional>
#include <sstream>

using namespace std;

namespace Runtime {

template <typename T>
std::optional<bool> EqualValue(ObjectHolder lhs, ObjectHolder rhs) {
    auto lhs_val = lhs.TryAs<T>();
    auto rhs_val = rhs.TryAs<T>();
    if (lhs_val && rhs_val) {
        return std::optional(lhs_val->GetValue() == rhs_val->GetValue());
    }
    return std::optional<bool>();
}

template <typename T>
std::optional<bool> LessValue(ObjectHolder lhs, ObjectHolder rhs) {
    auto lhs_val = lhs.TryAs<T>();
    auto rhs_val = rhs.TryAs<T>();
    if (lhs_val && rhs_val) {
        return std::optional(lhs_val->GetValue() < rhs_val->GetValue());
    }
    return std::optional<bool>();
}


bool Equal(ObjectHolder lhs, ObjectHolder rhs) {
    auto option = EqualValue<Runtime::String>(lhs, rhs);
    if (option.has_value()) { return option.value(); }

    option = EqualValue<Runtime::Number>(lhs, rhs);
    if (option.has_value()) { return option.value(); }
    
    option = EqualValue<Runtime::Bool>(lhs, rhs);
    if (option.has_value()) { return option.value(); }
    
    return false;
}



bool Less(ObjectHolder lhs, ObjectHolder rhs) {
    auto option = LessValue<Runtime::String>(lhs, rhs);
    if (option.has_value()) {
        return option.value();
    }

    option = LessValue<Runtime::Number>(lhs, rhs);
    if (option.has_value()) {
        return option.value();
    }

    option = LessValue<Runtime::Bool>(lhs, rhs);
    if (option.has_value()) {
        return option.value();
    }
    return false;
}


} /* namespace Runtime */

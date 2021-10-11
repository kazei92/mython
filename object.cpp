#include "object.h"
#include "statement.h"

#include <sstream>
#include <string_view>

using namespace std;



namespace Runtime {

void PrintClosure(const Closure& closure) {
    for (const auto& item : closure) {
        std::cout << item.first << std::endl;
    }
    std::cout << "------------------------" << std::endl;
}

void ClassInstance::Print(std::ostream& os) { 
    if (!HasMethod("__str__", 0)) {
        os << this;
    } 
    else {
        Call("__str__", {}).Get()->Print(os);
    }
}
bool ClassInstance::HasMethod(const std::string& method, size_t argument_count) const {
    bool result = false;
    for (const auto& class_method : _class_.class_methods) {
        if (class_method.name == method && class_method.formal_params.size() == argument_count) {
            result = true;
        }
    }
    return result;
}

const Closure& ClassInstance::Fields() const { return fields; }
Closure& ClassInstance::Fields() { return fields; }
ClassInstance::ClassInstance(const Class& cls) : _class_(cls) {
    fields["self"] = ObjectHolder::Share(*this);
}

ClassInstance::ClassInstance (ClassInstance&& other) : fields(std::move(other.fields)), _class_(std::move(other._class_)) {
    fields["self"] = ObjectHolder::Share(*this);
}

ObjectHolder ClassInstance::Call(const std::string& method, const std::vector<ObjectHolder>& actual_args) {
    const Runtime::Method * method_of_class = _class_.GetMethod(method);

    if (method_of_class->formal_params.size() != actual_args.size()) {
        throw std::runtime_error("not all arguments provided");
    }

    for (size_t i = 0; i < method_of_class->formal_params.size(); ++i){
        fields[method_of_class->formal_params[i]] = actual_args[i];
    }

    return method_of_class->body.get()->Execute(fields);
}


Class::Class(std::string name, std::vector<Method> methods, const Class* parent) : 
class_name(name), class_methods(std::move(methods)), class_parent(parent) {}
const Method* Class::GetMethod(const std::string& name) const {
    for (const Method& method : class_methods) {
        if (method.name == name) {
            return &method;
        }
    }
    if (class_parent != nullptr) {
        return class_parent->GetMethod(name);
    }

    return nullptr;
}
void Class::Print(ostream& os) { os << GetName(); }
const std::string& Class::GetName() const { return class_name; }


void Bool::Print(std::ostream& os) {
    if (GetValue()) { os << "True"; } 
    else { os << "False"; }
}

} /* namespace Runtime */

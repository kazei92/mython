#include "statement.h"
#include "object.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace Ast {

using Runtime::Closure;

ObjectHolder Assignment::Execute(Closure& closure) {
  ObjectHolder statement_result = right_value.get()->Execute(closure);
  closure[var_name] = statement_result;
  return closure[var_name];
}

Assignment::Assignment(std::string var, std::unique_ptr<Statement> rv) : var_name(var), right_value(std::move(rv)) {
}

VariableValue::VariableValue(std::string var_name) { dotted_ids.push_back(std::move(var_name)); }
VariableValue::VariableValue(std::vector<std::string> dotted_ids) : dotted_ids(std::move(dotted_ids)) {}

ObjectHolder VariableValue::Execute(Closure& closure) {
  try {
    if (dotted_ids.size() == 2) {
      return closure.at(dotted_ids[0]).TryAs<Runtime::ClassInstance>()->Fields().at(dotted_ids[1]);
  }
  return closure.at(dotted_ids[0]);
  
  } catch (std::out_of_range) {
    throw std::runtime_error("variable is not defined");
  }
}

unique_ptr<Print> Print::Variable(std::string var) {
  auto statement = std::make_unique<VariableValue>(var);
  return std::make_unique<Print>(std::move(statement));
}

Print::Print(unique_ptr<Statement> argument) { args.push_back(std::move(argument)); }
Print::Print(vector<unique_ptr<Statement>> args) : args(std::move(args)) {}

ObjectHolder Print::Execute(Closure& closure) {
  if (args.size() == 1) {
    auto object = args[0].get()->Execute(closure);
    if (object.Get() == nullptr) { *output << "None";} 
    else { object.Get()->Print(*output); }
    
  } else {
    for (size_t i = 0; i < args.size(); ++i) {
      auto object = args[i].get()->Execute(closure);
      if (object.Get() == nullptr) { *output << "None"; } 
      else { object.Get()->Print(*output); }
      if (i != args.size()-1) { *output << ' '; }
    }
  }
  *output << '\n';
  return ObjectHolder();
}

ostream* Print::output = &cout;

void Print::SetOutputStream(ostream& output_stream) {
  output = &output_stream;
}

MethodCall::MethodCall(
  std::unique_ptr<Statement> object, std::string method, std::vector<std::unique_ptr<Statement>> args) :
  object(std::move(object)), method(std::move(method)), args(std::move(args)) {}

ObjectHolder MethodCall::Execute(Closure& closure) {
  auto instance = object.get()->Execute(closure).TryAs<Runtime::ClassInstance>();
  std::vector<ObjectHolder> actual_args;
  for (const auto& statement : args) {
    actual_args.push_back(statement.get()->Execute(closure));
  }
  return instance->Call(method, actual_args);
}

ObjectHolder Stringify::Execute(Closure& closure) {
  auto object = argument.get()->Execute(closure);
  std::ostringstream ss;
  object.Get()->Print(ss);
  return ObjectHolder::Own(Runtime::String{ss.str()});
}

ObjectHolder Add::Execute(Closure& closure) {
  ObjectHolder lhs_res = lhs.get()->Execute(closure);
  ObjectHolder rhs_res = rhs.get()->Execute(closure);

  auto lhs_obj = lhs_res.TryAs<Runtime::ClassInstance>();
  if (lhs_obj && lhs_obj->HasMethod("__add__", 1)) {
    ObjectHolder var = rhs.get()->Execute(lhs_obj->Fields());
    return lhs_obj->Call("__add__", {var});
  }
  
  Runtime::Number * lhs_num = lhs_res.TryAs<Runtime::Number>();
  Runtime::Number * rhs_num = rhs_res.TryAs<Runtime::Number>();
  if (lhs_num && rhs_num) {
    auto new_value = lhs_num->GetValue() + rhs_num->GetValue();
    return ObjectHolder::Own(Runtime::Number{new_value});
  }

  Runtime::String * lhs_str = lhs_res.TryAs<Runtime::String>();
  Runtime::String * rhs_str = rhs_res.TryAs<Runtime::String>();
  if (lhs_str && rhs_str) {
    auto new_value = lhs_str->GetValue() + rhs_str->GetValue();
    return ObjectHolder::Own(Runtime::String{new_value});
  }

  throw std::runtime_error("invalid arguments");

}

ObjectHolder Sub::Execute(Closure& closure) {
  ObjectHolder lhs_res = lhs.get()->Execute(closure);
  ObjectHolder rhs_res = rhs.get()->Execute(closure);
  Runtime::Number * lhs_num = lhs_res.TryAs<Runtime::Number>();
  Runtime::Number * rhs_num = rhs_res.TryAs<Runtime::Number>();
  if (lhs_num && rhs_num) {
    auto new_value = lhs_num->GetValue() - rhs_num->GetValue();
    return ObjectHolder::Own(Runtime::Number{new_value});
  }
  throw std::runtime_error("invalid arguments");
}

ObjectHolder Mult::Execute(Runtime::Closure& closure) {
  ObjectHolder lhs_res = lhs.get()->Execute(closure);
  ObjectHolder rhs_res = rhs.get()->Execute(closure);
  Runtime::Number * lhs_num = lhs_res.TryAs<Runtime::Number>();
  Runtime::Number * rhs_num = rhs_res.TryAs<Runtime::Number>();
  if (lhs_num && rhs_num) {
    auto new_value = lhs_num->GetValue() * rhs_num->GetValue();
    return ObjectHolder::Own(Runtime::Number{new_value});
  }
  throw std::runtime_error("invalid arguments");
}

ObjectHolder Div::Execute(Runtime::Closure& closure) {
  ObjectHolder lhs_res = lhs.get()->Execute(closure);
  ObjectHolder rhs_res = rhs.get()->Execute(closure);
  Runtime::Number * lhs_num = lhs_res.TryAs<Runtime::Number>();
  Runtime::Number * rhs_num = rhs_res.TryAs<Runtime::Number>();
  if (lhs_num && rhs_num) {
    auto new_value = lhs_num->GetValue() / rhs_num->GetValue();
    return ObjectHolder::Own(Runtime::Number{new_value});
  }
  throw std::runtime_error("invalid arguments");
}

ObjectHolder Compound::Execute(Closure& closure) {
  // ObjectHolder res;
  // std::string class_name;
  // for (const auto& statement : statements) {
  //   res = statement.get()->Execute(closure);
  //   class_name = typeid(*statement.get()).name();
  //   if (class_name == "class Ast::IfElse" && res.Get() != nullptr) { return res; }
  //   if (class_name == "class Ast::Return") { return res; }
  //   if (class_name == "class Ast::Compound") { return res; }
  // }
  // return ObjectHolder::None();
    for (auto& statement : statements) {
    auto ret = statement->Execute(closure);

    bool check_for_return = false
      || dynamic_cast<Return*>(statement.get())
      || dynamic_cast<IfElse*>(statement.get())
      || dynamic_cast<Compound*>(statement.get());

    if (check_for_return && ret.Get()) {
      return ret;
    }
  }
  return ObjectHolder::None();
}

ObjectHolder Return::Execute(Closure& closure) {
  return statement.get()->Execute(closure);
}

ClassDefinition::ClassDefinition(ObjectHolder class_) : cls(class_), class_name(cls.TryAs<Runtime::Class>()->GetName()) {}

ObjectHolder ClassDefinition::Execute(Runtime::Closure& closure) {
  closure[class_name] = cls;
  return closure[class_name];
}

FieldAssignment::FieldAssignment(VariableValue object, std::string field_name, std::unique_ptr<Statement> rv)
: object(object), field_name(std::move(field_name)), right_value(std::move(rv))
{
}

ObjectHolder FieldAssignment::Execute(Runtime::Closure& closure) {
  Runtime::Closure& object_fields = object.Execute(closure).TryAs<Runtime::ClassInstance>()->Fields();
  ObjectHolder right = right_value.get()->Execute(closure);
  object_fields[field_name] = right;
  return object_fields[field_name];


}

IfElse::IfElse(
  std::unique_ptr<Statement> condition,
  std::unique_ptr<Statement> if_body,
  std::unique_ptr<Statement> else_body
) : condition(std::move(condition)), if_body(std::move(if_body)), else_body(std::move(else_body))
{
}

ObjectHolder IfElse::Execute(Runtime::Closure& closure) {
  if (Runtime::IsTrue(condition->Execute(closure))) {
    return if_body.get()->Execute(closure);
  } else if (else_body) {
    return else_body->Execute(closure);
  }
  return ObjectHolder::None();
}

ObjectHolder Or::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
    Runtime::Bool(Runtime::IsTrue(lhs->Execute(closure)) || Runtime::IsTrue(rhs->Execute(closure)))
  );
}

ObjectHolder And::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
    Runtime::Bool(Runtime::IsTrue(lhs->Execute(closure)) && Runtime::IsTrue(rhs->Execute(closure)))
  );
}

ObjectHolder Not::Execute(Runtime::Closure& closure) {
  return ObjectHolder::Own(
    Runtime::Bool(!Runtime::IsTrue(argument->Execute(closure)))
  );
}


Comparison::Comparison(Comparator cmp, unique_ptr<Statement> lhs, unique_ptr<Statement> rhs
) : left(std::move(lhs)), right(std::move(rhs)), comparator(cmp) {}

ObjectHolder Comparison::Execute(Runtime::Closure& closure) {
  ObjectHolder left_value = left.get()->Execute(closure);
  ObjectHolder right_value = right.get()->Execute(closure);
  bool res = comparator(left_value, right_value);
  
  return ObjectHolder::Own(Runtime::Bool(res));
}

NewInstance::NewInstance(const Runtime::Class& class_, std::vector<std::unique_ptr<Statement>> args
) : _class_(class_), args(std::move(args)) {}

NewInstance::NewInstance(const Runtime::Class& class_) : NewInstance(class_, {}) {
}

ObjectHolder NewInstance::Execute(Runtime::Closure& closure) {
  Runtime::ClassInstance new_instance(_class_);
  if (new_instance.HasMethod("__init__", args.size())) {
    std::vector<ObjectHolder> actual_args;
    for (const auto& statement : args) {
      actual_args.push_back(statement.get()->Execute(closure));
    }
    new_instance.Call("__init__", std::move(actual_args));
  }
  ObjectHolder holder = ObjectHolder::Own(std::move(new_instance));
  return holder;
}


} /* namespace Ast */

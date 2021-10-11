#include "object_holder.h"
#include "object.h"

namespace Runtime {

ObjectHolder ObjectHolder::Share(Object& object) {
  return ObjectHolder(std::shared_ptr<Object>(&object, [](auto*) { /* do nothing */ }));
}

ObjectHolder ObjectHolder::None() {
  return ObjectHolder();
}

Object& ObjectHolder::operator *() {
  return *Get();
}

const Object& ObjectHolder::operator *() const {
  return *Get();
}

Object* ObjectHolder::operator ->() {
  return Get();
}

const Object* ObjectHolder::operator ->() const {
  return Get();
}

Object* ObjectHolder::Get() {
  return data.get();
}

const Object* ObjectHolder::Get() const {
  return data.get();
}

ObjectHolder::operator bool() const {
  return Get();
}

bool IsTrue(ObjectHolder object) {
  if (!object.Get()) {
    return false;
  }
  if (auto bool_value = object.TryAs<Bool>()) {
    return bool_value->GetValue();
  }
  if (auto nubmer_value = object.TryAs<Number>()) {
    return nubmer_value->GetValue() != 0;
  }
  if (auto string_value = object.TryAs<String>()) {
    return string_value->GetValue() != "";
  }
  if (auto object_value = object.TryAs<ClassInstance>()) {
    return true;
  }
  return false;
}

}

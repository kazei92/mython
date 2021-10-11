#include "object_holder.h"
#include "object.h"

#include "test_runner.h"

#include <sstream>

using namespace std;

namespace Runtime {

class Logger : public Object {
public:
  static int instance_count;

  explicit Logger(int value = 0) : id(value) {
    ++instance_count;
  }

  Logger(const Logger& rhs) : id(rhs.id)
  {
    ++instance_count;
  }

  Logger(Logger&& rhs) : id(rhs.id)
  {
    ++instance_count;
  }

  int GetId() const { return id; }

  ~Logger() {
    --instance_count;
  }

  void Print(ostream& os) override {
    os << id;
  }

private:
  int id;
};

int Logger::instance_count = 0;

void TestNonowning() {
  ASSERT_EQUAL(Logger::instance_count, 0);
  Logger logger(784);
  {
    auto oh = ObjectHolder::Share(logger);
    ASSERT(oh);
  }
  ASSERT_EQUAL(Logger::instance_count, 1);

  auto oh = ObjectHolder::Share(logger);
  ASSERT(oh);
  ASSERT(oh.Get() == &logger);

  ostringstream os;
  oh->Print(os);
  ASSERT_EQUAL(os.str(), "784");
}

void TestOwning() {
  ASSERT_EQUAL(Logger::instance_count, 0);
  {
    auto oh = ObjectHolder::Own(Logger());
    ASSERT(oh);
    ASSERT_EQUAL(Logger::instance_count, 1);
  }
  ASSERT_EQUAL(Logger::instance_count, 0);

  auto oh = ObjectHolder::Own(Logger(312));
  ASSERT(oh);
  ASSERT_EQUAL(Logger::instance_count, 1);

  ostringstream os;
  oh->Print(os);
  ASSERT_EQUAL(os.str(), "312");

}

void TestMove() {
  {
    ASSERT_EQUAL(Logger::instance_count, 0);
    Logger logger;

    auto one = ObjectHolder::Share(logger);
    ObjectHolder two = std::move(one);

    ASSERT_EQUAL(Logger::instance_count, 1);
    ASSERT(two.Get() == &logger);
  }
  {
    ASSERT_EQUAL(Logger::instance_count, 0);
    auto one = ObjectHolder::Own(Logger());
    ASSERT_EQUAL(Logger::instance_count, 1);
    Object* stored = one.Get();
    ObjectHolder two = std::move(one);
    ASSERT_EQUAL(Logger::instance_count, 1);

    ASSERT(two.Get() == stored);
    ASSERT(!one);
  }
}

void TestNullptr() {
  ObjectHolder oh;
  ASSERT(!oh);
  ASSERT(!oh.Get());
}

void RunObjectHolderTests(TestRunner& tr) {
  RUN_TEST(tr, Runtime::TestNonowning);
  RUN_TEST(tr, Runtime::TestOwning);
  RUN_TEST(tr, Runtime::TestMove);
  RUN_TEST(tr, Runtime::TestNullptr);
}

} /* namespace Runtime */

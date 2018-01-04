#include "structures/static_list.hpp"
#include "test/test.hpp"

namespace _tests {
class StaticListTest : public test::TestCase {
public:
    StaticListTest() : test::TestCase("Static List Test") {};

    virtual void run_test() override {
        test("Constructors");
        StaticList<int, 10> a = StaticList<int, 10>();
        assert(a.size() == 0);
        assert(a.empty());

        StaticList<int, 10> b = StaticList<int, 10>();

        test("Emptying a List");
        b.clear();
        assert(b.size() == 0);
        assert(b.empty());

        test("Front pushing a list");
        b.push_front(1);
        assert(b.size() == 1);
        assert(!b.empty());
        assert(b.front() == 1);
        assert(b.back() == 1);

        b.push_front(2);
        assert(b.size() == 2);
        assert(b.front() == 2);
        assert(b.back() == 1);

        test("Front popping a list");
        assert(b.pop_front() == 2);
        assert(b.size() == 1);
        assert(b.front() == 1);
        assert(b.back() == 1);

        assert(b.pop_front() == 1);
        assert(b.empty());

        test("Back pushing a list");
        b.push_back(1);
        assert(b.size() == 1);
        assert(!b.empty());
        assert(b.front() == 1);
        assert(b.back() == 1);

        b.push_back(2);
        assert(b.size() == 2);
        assert(b.front() == 1);
        assert(b.back() == 2);

        test("Back popping a list");
        assert(b.pop_back() == 2);
        assert(b.size() == 1);
        assert(b.front() == 1);
        assert(b.back() == 1);

        assert(b.pop_back() == 1);
        assert(b.empty());
        assert(b.size() == 0);

        test("Emplace");
        uint8_t constructs = 0;
        class TestClass {
        public:
            int val;
            TestClass() {};
            TestClass(int i, uint8_t& const_count) : val(i) {
                const_count ++;
            }
        };

        StaticList<TestClass, 10> c;
        c.emplace_back(1, constructs);
        c.emplace_front(2, constructs);

        assert(!c.empty());
        assert(constructs == 2);
    }
};

test::AddTestCase<StaticListTest> staticListTest;
}

#include "structures/list.hpp"
#include "test/test.hpp"

namespace _tests {
class ListTest : public test::TestCase {
public:
    ListTest() : test::TestCase("List Test") {};

    virtual void run_test() override {
        test("Constructors");
        list<int> a = list<int>();
        assert(a.size() == 0);
        assert(a.empty());

        list<int> b = list<int>(5);
        assert(b.size() == 5);
        assert(b.front() == 0);
        assert(!b.empty());

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
        b.pop_front();
        assert(b.size() == 1);
        assert(b.front() == 1);
        assert(b.back() == 1);

        b.pop_front();
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
        b.pop_back();
        assert(b.size() == 1);
        assert(b.front() == 1);
        assert(b.back() == 1);

        b.pop_back();
        assert(b.empty());

        test("Iterating through a list");
        b.push_front(1);
        b.push_front(2);
        b.push_front(3);
        b.push_front(4);
        b.push_front(5);

        uint8_t seeking = 5;
        for(int e : b) {
            assert(e == (seeking --));
        }

        test("Emplace");
        uint8_t constructs = 0;
        class TestClass {
        public:
            int val;
            TestClass(int i, uint8_t& const_count) : val(i) {
                const_count ++;
            }
        };

        list<TestClass> c;
        c.emplace_back(1, constructs);
        c.emplace_front(2, constructs);

        seeking = 2;
        for(TestClass e : c) {
            assert(e.val == (seeking --));
        }

        c.clear();
        assert(c.empty());
        assert(constructs == 2);
    }
};

test::AddTestCase<ListTest> listTest;
}

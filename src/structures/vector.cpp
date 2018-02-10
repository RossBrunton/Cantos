#include "main/common.hpp"
#include "test/test.hpp"

namespace _tests {
class VectorTest : public test::TestCase {
public:
    VectorTest() : test::TestCase("Vector Test") {};

    virtual void run_test() override {
        test("Constructors");
        vector<int> a = vector<int>();
        assert(a.size() == 0);
        assert(a.empty());

        vector<int> b = vector<int>(5);
        assert(b.size() == 5);
        assert(b.front() == 0);
        assert(!b.empty());

        test("Emptying a vector");
        b.clear();
        assert(b.size() == 0);
        assert(b.empty());

        test("Back pushing a vector");
        b.push_back(1);
        assert(b.size() == 1);
        assert(!b.empty());
        assert(b.front() == 1);
        assert(b.back() == 1);

        b.push_back(2);
        assert(b.size() == 2);
        assert(b.front() == 1);
        assert(b.back() == 2);

        test("Back popping a vector");
        b.push_back(2);
        b.push_back(3);
        b.pop_back();
        assert(b.size() == 3);
        assert(b.front() == 1);
        assert(b.back() == 2);

        b.pop_back();
        b.pop_back();
        b.pop_back();
        assert(b.empty());
        assert(b.size() == 0);

        test("Iterating through a vector");
        b.push_back(1);
        b.push_back(2);
        b.push_back(3);
        b.push_back(4);
        b.push_back(5);

        uint8_t seeking = 0;
        for(int e : b) {
            assert(e == (++ seeking));
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

        vector<TestClass> c;
        c.emplace_back(1, constructs);
        assert(c[0].val == 1);

        c.clear();
        assert(c.empty());
        assert(constructs == 1);
    }
};

test::AddTestCase<VectorTest> vectorTest;
}

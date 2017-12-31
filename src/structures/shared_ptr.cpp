#include "structures/shared_ptr.hpp"
#include "test/test.hpp"

namespace _tests {
class SharedPtrTest : public test::TestCase {
public:
    SharedPtrTest() : test::TestCase("shared_ptr Test") {};

    void run_test() override {
        test("Constructing");
        shared_ptr<int> ap = make_shared<int>(7);

        assert(ap);
        assert(*ap == 7);

        test("Moving a pointer");
        shared_ptr<int> bp = shared_ptr<int>(move(ap));
        assert(bp);
        assert(!ap);
        assert(*bp == 7);

        ap = move(bp);
        assert(ap);
        assert(!bp);
        assert(*ap == 7);

        test("Resetting a pointer");
        int *b = new int(8);
        ap.reset(b);
        assert(*ap == 8);

        int *a = new int(7);
        bp.reset(a);
        assert(*bp == 7);

        test("Swapping pointers");
        ap.swap(bp);
        assert(*bp == 8);
        assert(*ap == 7);

        bp = nullptr;
        ap.swap(bp);
        assert(!ap);
        assert(*bp == 7);

        ap.swap(bp);
        assert(!bp);
        assert(*ap == 7);

        test("make_unique");
        bp = make_shared<int>(9);
        assert(*bp == 9);

        test("Shared assignment");
        uint8_t constructs = 0;
        uint8_t destructs = 0;
        class TestClass {
        public:
            int val;
            uint8_t &destructs;
            TestClass(int i, uint8_t &constructs, uint8_t &destructs) : val(i), destructs(destructs) {
                constructs ++;
            }
            ~TestClass() {
                destructs ++;
            }
        };

        shared_ptr<TestClass> cp = make_shared<TestClass>(2, constructs, destructs);
        assert(cp);
        assert(constructs == 1);
        assert(destructs == 0);
        assert(cp->val == 2);

        shared_ptr<TestClass> dp = shared_ptr<TestClass>(cp);
        assert(dp);
        assert(cp);
        assert(constructs == 1);
        assert(destructs == 0);
        assert(dp->val == 2);

        cp = nullptr;
        assert(dp);
        assert(!cp);
        assert(constructs == 1);
        assert(destructs == 0);
        assert(dp->val == 2);

        cp = dp;
        assert(dp);
        assert(constructs == 1);
        assert(destructs == 0);
        assert(cp->val == 2);

        cp = nullptr;
        dp = nullptr;
        assert(constructs == 1);
        assert(destructs == 1);
    }
};

test::AddTestCase<SharedPtrTest> sharedPtrTest;
}

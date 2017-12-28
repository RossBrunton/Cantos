#include "structures/unique_ptr.hpp"
#include "test/test.hpp"

TEST("Unique_ptr Test", UniquePtrTest, {
    test("Constructing");
    int *a = new int(7);
    int *b = new int(8);

    unique_ptr<int> ap = unique_ptr<int>(a);

    assert(ap);
    assert(*ap == 7);

    test("Moving a pointer");
    unique_ptr<int> bp = unique_ptr<int>(move(ap));
    assert(bp);
    assert(!ap);
    assert(*bp == 7);

    ap = move(bp);
    assert(ap);
    assert(!bp);
    assert(*ap == 7);

    test("Releasing a pointer");
    assert(ap.release() == a);
    assert(!ap);

    test("Resetting a pointer");
    ap.reset(b);
    assert(*ap == 8);

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
    bp = make_unique<int>(9);
    assert(*bp == 9);
})

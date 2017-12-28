#ifndef _HPP_STRUCT_LIST_
#define _HPP_STRUCT_LIST_

#include <stddef.h>

#include "main/cpp.hpp"
#include "structures/unique_ptr.hpp"
#include "structures/iterator.hpp"

/** Implements a linked list over a specified type, allowing dynamically sized storage
 *
 * This functions similarly to std::list, except it doesn't support custom deleters. Elements can be appended to either
 *  the front of the back of the list, and iteraters can traverse from the front to the back.
 *
 * This is implemented as a singly-linked list, so appending an element to the back of this list will run in `O(n)`
 *  time. Besides this and list::clear (which also runs in `O(n)` time), all operations run in `O(1)` time.
 *
 * Adding or removing elements is not thread safe.
 *
 * When an object is removed from the list, their default deconstructor is called, and any references/pointers to them
 *  are invalid. Deleting an element does not invalidate any references/pointers to other elements or the list itself.
 */
template<class T> class list {
private:
    struct Entry {
        unique_ptr<Entry> next;
        T object;

        Entry(T obj) : object(obj) {};
    };

    unique_ptr<Entry> head = nullptr;
    Entry *tail = nullptr;
    size_t count = 0;

public:
    /** An iterator for lists
     *
     * See list::begin for more details.
     */
    class Iterator {
    private:
        Entry* entry;

    public:
        using iterator_type = Entry;
        using iterator_category = iterator::forward_iterator_tag;
        using value_type = const T;
        using difference_type = size_t;
        using reference = T&;
        using pointer = T *;

        explicit Iterator(Entry* e) : entry(e) {}

        reference operator*() const {return entry->object;}
        pointer operator->() const {return &(entry->object);}
        Iterator& operator++() {
            entry = entry->next.get();
            return *this;
        }
        Iterator operator++(int) {return Iterator(this++);}
        bool operator==(Iterator& other) const {
            return other.entry == this.entry;
        }
        bool operator!=(Iterator& other) const {
            return other.entry != entry;
        }
    };

    /** Constructs a new, empty list */
    list() {};
    /** Constructs a list of `count` default-constructed elements
     *
     * @param count The number of default constructed elements to create
     */
    list(size_t count);

    /** Returns a reference to the frontmost element of the list
     *
     * @return The front element
     */
    T& front() const {
        return head->object;
    }

    /** Returns a reference to the backmost element of the list
     *
     * @return The back element
     */
    T& back() const {
        return tail->object;
    }

    /** Returns true iff the list has no elements
     *
     * @return Whether the list is empty
     */
    bool empty() const {
        return !head;
    }

    /** Returns the number of items in the list
     *
     * @return How many items are in this list
     */
    size_t size() const {
        return count;
    }

    /** Deletes all items in the list
     *
     * After this call, the list will be empty.
     */
    void clear();

    /** Push an item to the front of the list
     *
     * The element will be copied.
     *
     * @param value The item to add
     */
    void push_front(const T& value);
    /** Push an item to the front of the list
     *
     * The element will be moved.
     *
     * @param value The item to add
     */
    void push_front(T&& value);
    /** Remove the front item from the list
     *
     * The item will be deleted, and the new front will be the item following it.
     */
    void pop_front();
    /** Construct a new item at the front of the list
     *
     * @param args The arguments for the newly created item's constructor
     */
    template<class... Args> void emplace_front(Args&&... args);

    /** Push an item to the back of the list
     *
     * The element will be copied.
     *
     * @param value The item to add
     */
    void push_back(const T& value);
    /** Push an item to the back of the list
     *
     * The element will be moved.
     *
     * @param value The item to add
     */
    void push_back(T&& value);
    /** Remove the back item from the list
     *
     * The item will be deleted, and the new back will be the item preceding it.
     */
    void pop_back();
    /** Construct a new item at the back of the list
     *
     * @param args The arguments for the newly created item's constructor
     */
    template<class... Args> void emplace_back(Args&&... args);

    /** Returns an iterator to the first element of the list
     *
     * This iterator will start at the front of the list, and advance through all the items in order.
     *
     * Modifying an item is allowed, as is appending a new item (whether this item gets iterated through is undefined).
     *  Removing an item other than the current item is also allowed, and it will not be iterated through if it has not
     *  been seen yet. Removing the current item is not allowed.
     *
     * @return An iterater through the list's contents
     */
    Iterator begin();
    /** Returns a sentinel value that represents the end of an iteration
     *
     * Do not attempt to use this as an iterated value.
     *
     * @return A null iterator
     */
    Iterator end();
};

#include "structures/list.tpp"
#endif
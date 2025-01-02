#pragma once

#include <iterator>

class CounterIterator {
public:
    CounterIterator(const int value = 0) : mValue(value) { }

    CounterIterator& operator++() {
        ++mValue;
        return *this;
    }

    CounterIterator operator++(int) {
        CounterIterator tmp(*this);
        operator++();
        return tmp;
    }

    bool operator==(const CounterIterator& rhs) const {
        return mValue == rhs.mValue;
    }

    bool operator!=(const CounterIterator& rhs) const {
        return mValue != rhs.mValue;
    }

    int operator*() const {
        return mValue;
    }

    using difference_type = int;
    using value_type = int;
    using pointer = int*;
    using reference = int&;

private:
    int mValue;
};

// Copyright 2006-2015 Tobias Sargeant (tobias.sargeant@gmail.com).
//
// This file is part of the Carve CSG Library (http://carve-csg.com/)
//
// Permission is hereby granted, free of charge, to any person
// obtaining a copy of this software and associated documentation
// files (the "Software"), to deal in the Software without
// restriction, including without limitation the rights to use, copy,
// modify, merge, publish, distribute, sublicense, and/or sell copies
// of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
// BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.


#pragma once

#include <algorithm>
#include <array>
#include <vector>
#include <cassert>

namespace carve
{

template <typename set_t> class set_insert_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void>
{

protected:
    set_t* set;

public:
    set_insert_iterator(set_t& s) : set(&s)
    {
    }

    set_insert_iterator& operator=(typename set_t::const_reference value)
    {
        set->insert(value);
        return *this;
    }

    set_insert_iterator& operator*()
    {
        return *this;
    }
    set_insert_iterator& operator++()
    {
        return *this;
    }
    set_insert_iterator& operator++(int)
    {
        return *this;
    }
};

template <typename set_t> inline set_insert_iterator<set_t> set_inserter(set_t& s)
{
    return set_insert_iterator<set_t>(s);
}

template <typename T, int SizeOnStack = 4> class small_vector_on_stack
{
public:
    using value_type = T;

private:
    std::array<T, SizeOnStack> stackStorage;
    std::vector<T>* heapStorage;
    size_t _size;

public:
    class iter
    {
    private:
        size_t idx;
        small_vector_on_stack* vec;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = T*;
        using reference = T&;
        using iterator_category = std::random_access_iterator_tag;

        iter() : idx(0), vec(nullptr)
        {
        }
        iter(size_t idx, small_vector_on_stack* vec) : idx(idx), vec(vec)
        {
        }

        iter operator++()
        {
            ++idx;
            return *this;
        }

        iter operator+(difference_type diff)
        {
            idx += diff;
            return *this;
        }

        iter operator--()
        {
            --idx;
            return *this;
        }

        iter operator-(difference_type diff)
        {
            idx -= diff;
            return *this;
        }

        size_t index() const
        {
            return idx;
        }

        bool operator==(const iter& other) const
        {
            return idx == other.idx;
        }

        bool operator!=(const iter& other) const
        {
            return idx != other.idx;
        }

        bool operator<(const iter& other) const
        {
            return idx < other.idx;
        }

        bool operator>(const iter& other) const
        {
            return idx > other.idx;
        }

        T& operator*() const
        {
            return vec->operator[](idx);
        }

        static difference_type distance(const iter& a, const iter& b)
        {
            return (difference_type)b.idx - (difference_type)a.idx;
        }

        difference_type operator -(const iter& other) const
        {
            return distance(*this, other);
        }
    };

    class const_iter
    {
    private:
        size_t idx;
        const small_vector_on_stack* vec;

    public:
        using difference_type = std::ptrdiff_t;
        using value_type = T;
        using pointer = const T*;
        using reference = const T&;
        using iterator_category = std::random_access_iterator_tag;

        const_iter(size_t idx, const small_vector_on_stack* vec) : idx(idx), vec(vec)
        {
        }
        const_iter() : idx(0), vec(nullptr)
        {
        }

        const_iter operator++()
        {
            ++idx;
            return *this;
        }

        const_iter operator+(difference_type diff)
        {
            idx += diff;
            return *this;
        }

        const_iter operator--()
        {
            --idx;
            return *this;
        }

        const_iter operator-(difference_type diff)
        {
            idx -= diff;
            return *this;
        }

        size_t index() const
        {
            return idx;
        }

        bool operator==(const const_iter& other) const
        {
            return idx == other.idx;
        }

        bool operator!=(const const_iter& other) const
        {
            return idx != other.idx;
        }

        bool operator<(const const_iter& other) const
        {
            return idx < other.idx;
        }

        bool operator>(const const_iter& other) const
        {
            return idx > other.idx;
        }

        const T& operator*() const
        {
            return vec->operator[](idx);
        }

        static difference_type distance(const const_iter& a, const const_iter& b)
        {
            return (difference_type)b.idx - (difference_type)a.idx;
        }

        difference_type operator -(const const_iter& other) const
        {
            return distance(*this, other);
        }
    };

public:
    using iterator = iter;
    using const_iterator = const_iter;
    using size_type = size_t;

    small_vector_on_stack() : stackStorage(), heapStorage(nullptr), _size(0)
    {
    }

    small_vector_on_stack(size_t size) : stackStorage(), heapStorage(nullptr), _size(size)
    {
        if (size >= SizeOnStack)
        {
            heapStorage = new std::vector<T>(size);
            _size = 0;
        }
    }

    ~small_vector_on_stack()
    {
        delete heapStorage;
        heapStorage = nullptr;
        _size = 0;
    }

    void move_from(small_vector_on_stack<T, SizeOnStack>& other)
    {
        if (other.heapStorage == nullptr)
        {
            _size = other._size;
            stackStorage = other.stackStorage;
            heapStorage = nullptr;
        }
        else
        {
            _size = 0;
            stackStorage = std::array<T, SizeOnStack>();
            heapStorage = other.heapStorage;
        }

        other._size = 0;
        other.heapStorage = nullptr;
    }

    void push_back(const T& value)
    {
        if (heapStorage == nullptr)
        {
            if (_size != SizeOnStack)
            {
                // on the stack
                stackStorage[_size++] = value;
            }
            else
            {
                // move to heap
                heapStorage = new std::vector<T>();
                heapStorage->reserve(SizeOnStack + 2);
                heapStorage->insert(heapStorage->begin(), stackStorage.begin(), stackStorage.end());
                heapStorage->emplace_back(value);
                _size = 0;
            }
        }
        else
        {
            // already on heap
            heapStorage->emplace_back(value);
        }
    }

    T& operator[](size_t idx)
    {
        assert(heapStorage == nullptr ? idx < _size : true);
        return heapStorage == nullptr ? stackStorage[idx] : heapStorage->operator[](idx);
    }

    const T& operator[](size_t idx) const
    {
        assert(heapStorage == nullptr ? idx < _size : true);
        return heapStorage == nullptr ? stackStorage[idx] : heapStorage->operator[](idx);
    }

    void resize(size_t newSize, T fillValue = T())
    {
        if (heapStorage == nullptr)
        {
            if (newSize > SizeOnStack)
            {
                // move to heap
                heapStorage = new std::vector<T>();
                heapStorage->reserve(newSize);

                size_t oldSize = _size;
                size_t i = 0;
                for (; i < oldSize; ++i)
                {
                    heapStorage->push_back(stackStorage[i]);
                }

                heapStorage->resize(newSize, fillValue);
                _size = 0;
            }
            else
            {
                // stay on stack, set size and values
                size_t oldSize = _size;
                for (size_t i = oldSize; i < newSize; ++i)
                {
                    stackStorage[i] = fillValue;
                }
                _size = newSize;
            }
        }
        else
        {
            // just resize the vector
            heapStorage->resize(newSize, fillValue);
        }
    }

    template <typename _Iter> void insert(_Iter begin, _Iter end)
    {
        for (_Iter it = begin; it != end; ++it)
        {
            push_back(*it);
        }
    }

    void remove(const T& value)
    {
        iter it = find(value);
        if (it != end())
        {
            erase(it);
        }
    }

    iter find(const T& value)
    {
        for (iter it = begin(); it != end(); ++it)
        {
            if (*it == value)
            {
                return it;
            }
        }

        return end();
    }

    const_iter find(const T& value) const
    {
        for (const_iter it = cbegin(); it != cend(); ++it)
        {
            if (*it == value)
            {
                return it;
            }
        }

        return cend();
    }

    void reverse()
    {
        if (heapStorage == nullptr)
        {
            size_t start = 0;
            size_t end = _size;

            while (start < end)
            {
                std::swap(stackStorage[start], stackStorage[end]);
                ++start;
                --end;
            }
        }
        else
        {
            std::reverse(heapStorage->begin(), heapStorage->end());
        }
    }

    size_t size() const
    {
        return heapStorage == nullptr ? _size : heapStorage->size();
    }

    bool empty() const
    {
        return _size == 0;
    }

    void clear()
    {
        if (heapStorage == nullptr)
        {
            _size = 0;
        }
        else
        {
            heapStorage->clear();
        }
    }

    iter erase(iter at)
    {
        size_t idx = at.index();
        if (heapStorage == nullptr)
        {
            for (size_t i = idx; i < _size - 1; ++i)
            {
                stackStorage[i] = stackStorage[i + 1];
            }
            --_size;
        }
        else
        {
            for (size_t i = idx; i < heapStorage->size() - 1; ++i)
            {
                heapStorage[i] = heapStorage[i + 1];
            }
            heapStorage->pop_back();
        }

        return at;
    }

    T& front()
    {
        return stackStorage[0];
    }

    const T& front() const
    {
        return stackStorage[0];
    }

    T& back()
    {
        if (heapStorage == nullptr)
        {
            return stackStorage[_size - 1];
        }
        else
        {
            return heapStorage->back();
        }
    }

    const T& back() const
    {
        if (heapStorage == nullptr)
        {
            return stackStorage[_size - 1];
        }
        else
        {
            return heapStorage->back();
        }
    }

    iter begin()
    {
        return iter(0, this);
    }

    iter end()
    {
        return iter(size(), this);
    }

    const_iter begin() const
    {
        return const_iter(0, this);
    }

    const_iter end() const
    {
        return const_iter(size(), this);
    }

    const_iter cbegin() const
    {
        return const_iter(0, this);
    }

    const_iter cend() const
    {
        return const_iter(size(), this);
    }
};

} // namespace carve

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

#include <vector>
#include <array>

namespace carve {

  template<typename set_t>
  class set_insert_iterator : public std::iterator<std::output_iterator_tag, void, void, void, void> {

  protected:
    set_t *set;
  public:

    set_insert_iterator(set_t &s) : set(&s) {
    }

    set_insert_iterator &
    operator=(typename set_t::const_reference value) {
      set->insert(value);
      return *this;
    }

    set_insert_iterator &operator*() { return *this; }
    set_insert_iterator &operator++() { return *this; }
    set_insert_iterator &operator++(int) { return *this; }
  };

  template<typename set_t>
  inline set_insert_iterator<set_t>
  set_inserter(set_t &s) {
    return set_insert_iterator<set_t>(s);
  }

  template <typename T, int SizeOnStack = 4>
  class small_vector_on_stack {
  private:
    std::array<T, SizeOnStack> stackStorage;
    std::vector<T>* heapStorage;
    size_t _size;
  public:
    class iter {
    private:
      size_t idx;
      small_vector_on_stack* vec;
    public:
      iter(size_t idx, small_vector_on_stack* vec) : idx(idx), vec(vec) { }

      iter operator++() {
        ++idx;
        return *this;
      }

      bool operator==(const iter& other) const {
        return idx == other.idx;
      }

      bool operator!=(const iter& other) const {
        return idx != other.idx;
      }

      T& operator*() {
        return vec->operator[](idx);
      }
    };

    class const_iter {
    private:
      size_t idx;
      const small_vector_on_stack* vec;

    public:
      const_iter(size_t idx, const small_vector_on_stack* vec) : idx(idx), vec(vec) { }
    
      const_iter operator++() {
        ++idx;
        return *this;
      }
    
      bool operator==(const const_iter& other) const {
        return idx == other.idx;
      }
    
      bool operator!=(const const_iter& other) const {
        return idx != other.idx;
      }
    
      const T& operator*() {
        return vec->operator[](idx);
      }
    };

  public:
    using iterator = iter;
    using const_iterator = const_iter;

    small_vector_on_stack() : stackStorage(), heapStorage(nullptr), _size(0) { }
    ~small_vector_on_stack() {
      delete heapStorage;
    }

    void push_back(const T& value)
    {
      if (heapStorage == nullptr) {
        if (_size != SizeOnStack) {
          // on the stack
          stackStorage[_size++] = value;
        }
        else {
          // move to heap
          heapStorage = new std::vector<T>();
          heapStorage->reserve(SizeOnStack + 2);
          heapStorage->insert(heapStorage->begin(), stackStorage.begin(), stackStorage.end());
          heapStorage->emplace_back(value);
          _size = 0;
        }
      }
      else {
        // already on heap
        heapStorage->emplace_back(value);
      }
    }

    T& operator[](size_t idx) {
      return heapStorage == nullptr ? stackStorage[idx] : heapStorage->operator[](idx);
    }

    const T& operator[](size_t idx) const {
      return heapStorage == nullptr ? stackStorage[idx] : heapStorage->operator[](idx);
    }

    size_t size() const {
      return heapStorage == nullptr ? _size : heapStorage->size();
    }

    bool empty() const {
      return _size == 0;
    }

    void clear() {
      if (heapStorage == nullptr) {
        _size = 0;
      }
      else {
        heapStorage->clear();
      }
    }

    T& front() {
      return stackStorage[0];
    }

    iter begin() {
      return iterator(0, this);
    }

    iter end() {
      return iterator(size(), this);
    }

    iter cbegin() {
      return const_iterator(0, this);
    }

    iter cend() {
      return const_iterator(size(), this);
    }
  };
}

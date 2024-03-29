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

#include <execution>

namespace carve
{
namespace util
{
struct min_functor
{
    template <typename T> const T& operator()(const T& a, const T& b) const
    {
        return std::min(a, b);
    }
};
struct max_functor
{
    template <typename T> const T& operator()(const T& a, const T& b) const
    {
        return std::max(a, b);
    }
};

class pcg_random
{
private:
    uint64_t seed;
    uint64_t state;

    uint32_t rand_internal()
    {
        uint64_t oldstate = state;
        state = oldstate * 6364136223846793005ULL + seed;
        uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
        uint32_t rot = (uint32_t)(oldstate >> 59u);
        return (xorshifted >> rot) | (xorshifted << (32 - rot));
    }

public:
    pcg_random(uint64_t seed, uint64_t state = 0) : seed((seed << 1) | 1), state(state)
    {
        rand_internal();
        rand_internal();
    }
    uint32_t u32()
    {
        return rand_internal();
    }
    uint64_t u64()
    {
        return (((uint64_t)rand_internal()) << 32) | ((uint64_t)rand_internal());
    }
    double f64_01()
    {
        return (double)u64() / (double)std::numeric_limits<uint64_t>::max();
    }
    double f64(double min, double max)
    {
        return min + (max - min) * f64_01();
    }
    double normal_distribution(double mean = 0.0, double variance = 0.5)
    {
        constexpr double epsilon = 1e-14;
        constexpr double twoPI = 3.1415926535897932384626 * 2.0;
        double u1, u2;
        do
        {
            u1 = f64_01();
            u2 = f64_01();
        } while (u1 < epsilon || u2 < epsilon);

        return sqrt(-2.0 * log(u1)) * cos(u2 * twoPI);
    }
};

template <typename T>
class RangeIterator
{
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = T;
    using pointer = T*;
    using difference_type = T;
    using reference = T&;

    RangeIterator() : _index(T(0))
    {
    }

    RangeIterator(T index) : _index(index)
    {
    }

    RangeIterator& operator++()
    {
        ++_index;
        return *this;
    }

    RangeIterator& operator+=(T i)
    {
        _index += i;
        return *this;
    }

    T operator-(const RangeIterator& other) const
    {
        return _index - other._index;
    }

    bool operator!=(const RangeIterator& other)
    {
        return _index != other._index;
    }

    T operator*()
    {
        return _index;
    }

private:
    T _index;
};

template<typename Enumerable, typename Func>
inline void forEachParallel(Enumerable& enumerable, Func func)
{
#if defined(CARVE_MULTITHREADING) && defined(NDEBUG)
    std::for_each(std::execution::par_unseq, enumerable.begin(), enumerable.end(), func);
#else
    for (auto&& element : enumerable)
    {
        func(element);
    }
#endif
}

template <typename TIndex, typename Func>
inline void forEachParallel(TIndex firstIndex, TIndex numElements, TIndex batchSize, Func func)
{
#if defined(CARVE_MULTITHREADING) && defined(NDEBUG)
    TIndex numBatches = numElements / batchSize;
    if (numElements % batchSize != 0)
    {
        ++numBatches;
    }

    RangeIterator<TIndex> begin(0);
    RangeIterator<TIndex> end(numBatches);

    std::for_each(std::execution::par_unseq, begin, end,
        [batchSize, firstIndex, numElements, func](TIndex batchIndex)
        {
            TIndex iStart = firstIndex + batchIndex * batchSize;
            TIndex iEnd = iStart + batchSize;
            if (iEnd > firstIndex + numElements)
            {
                iEnd = firstIndex + numElements;
            }
            for (TIndex i = iStart; i < iEnd; ++i)
            {
                func(i);
            }
        }
    );
#else 
    for (TIndex i = firstIndex; i < firstIndex + numElements; ++i)
    {
        func(i);
    }
#endif
}

} // namespace util
} // namespace carve

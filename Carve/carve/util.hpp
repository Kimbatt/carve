// Begin License:
// Copyright (C) 2006-2011 Tobias Sargeant (tobias.sargeant@gmail.com).
// All rights reserved.
//
// This file is part of the Carve CSG Library (http://carve-csg.com/)
//
// This file may be used under the terms of the GNU General Public
// License version 2.0 as published by the Free Software Foundation
// and appearing in the file LICENSE.GPL2 included in the packaging of
// this file.
//
// This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
// INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE.
// End:


#pragma once

namespace carve {
  namespace util {
    struct min_functor {
      template<typename T>
      const T &operator()(const T &a, const T &b) const { return std::min(a, b); }
    };
    struct max_functor {
      template<typename T>
      const T &operator()(const T &a, const T &b) const { return std::max(a, b); }
    };
    class pcg_random {
    private:
      uint64_t seed;
      uint64_t state;

      uint32_t rand_internal() {
          uint64_t oldstate = state;
          state = oldstate * 6364136223846793005ULL + seed;
          uint32_t xorshifted = (uint32_t)(((oldstate >> 18u) ^ oldstate) >> 27u);
          uint32_t rot = (uint32_t)(oldstate >> 59u);
          return (xorshifted >> rot) | (xorshifted << (32 - rot));
      }
    public:
      pcg_random(uint64_t seed, uint64_t state = 0) : seed((seed << 1) | 1), state(state) {
        rand_internal();
        rand_internal();
      }
      uint32_t u32() {
        return rand_internal();
      }
      uint64_t u64() {
        return (((uint64_t)rand_internal()) << 32) | ((uint64_t)rand_internal());
      }
      double f64_01() {
        return (double)u64() / (double)std::numeric_limits<uint64_t>::max();
      }
      double f64(double min, double max) {
        return min + (max - min) * f64_01();
      }
    };
  }
}

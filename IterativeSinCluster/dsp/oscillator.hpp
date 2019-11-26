// (c) 2019 Takamitsu Endo
//
// This file is part of IterativeSinCluster.
//
// IterativeSinCluster is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// IterativeSinCluster is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with IterativeSinCluster.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "constants.hpp"
#include "somemath.hpp"

#include <array>
#include <numeric>

namespace SomeDSP {

// Mostly uniform gain range.
// - double : freq > 0.25Hz.
// - float  : freq > 8Hz. Huge bump around 1Hz.
template<typename Sample, size_t size> struct BiquadOscN {
  std::array<Sample, size> frequency{};
  std::array<Sample, size> gain{};
  std::array<Sample, size> u1{};
  std::array<Sample, size> u0{};
  std::array<Sample, size> k{};

  void setup(Sample sampleRate)
  {
    u1.fill(0);
    for (size_t i = 0; i < size; ++i) {
      auto omega = Sample(twopi) * frequency[i] / sampleRate;
      u0[i] = -sin(omega);
      k[i] = 2 * cos(omega);
    }
  }

  Sample process()
  {
    Sample sum = 0;
    Sample out;
    for (size_t i = 0; i < size; ++i) {
      out = k[i] * u1[i] - u0[i];
      u0[i] = u1[i];
      u1[i] = out;
      sum += gain[i] * out;
    }
    return sum / size;
  }
};

} // namespace SomeDSP
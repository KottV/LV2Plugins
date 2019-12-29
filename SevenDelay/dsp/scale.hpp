// (c) 2019 Takamitsu Endo
//
// This file is part of SevenDelay.
//
// SevenDelay is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SevenDelay is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SevenDelay.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include <cmath>

// This scale makes sure to round values that are close to 0 to be false.
template<typename T> class BoolScale {
public:
  BoolScale() {}
  void set() {}
  bool map(T input) const { return input > T(0.5); }
  bool reverseMap(T input) const { return input <= T(0.5); }
  T invmap(bool input) const { return T(input); }
  T getMin() { return T(false); }
  T getMax() { return T(true); }
};

// Maps a value in [0, 1] to [min, max].
// min /= max.
template<typename T> class LinearScale {
public:
  LinearScale(T min, T max) { set(min, max); }

  void set(T min, T max)
  {
    this->min = min;
    this->max = max;
    scale = max - min;
  }

  T map(T input) const
  {
    T value = input * scale + min;
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  T reverseMap(T input) const { return map(T(1.0) - input); }

  T invmap(T input) const
  {
    T value = (input - min) / scale;
    if (value < T(0.0)) return T(0.0);
    if (value > T(1.0)) return T(1.0);
    return value;
  }

  T getMin() { return min; }
  T getMax() { return max; }

protected:
  T scale;
  T min;
  T max;
};

// min /= max. power > 0.
template<typename T> class SPolyScale {
public:
  SPolyScale(T min, T max, T power = T(2.0)) { set(min, max, power); }

  void set(T min, T max, T power)
  {
    this->min = min;
    this->max = max;
    this->power = power;
    powerInv = T(1.0) / power;
    scale = max - min;
  }

  T map(T input) const
  {
    if (input < T(0.0)) return min;
    if (input > T(1.0)) return max;
    T value = input <= T(0.5) ? T(0.5) * pow(T(2.0) * input, power)
                              : T(1.0) - T(0.5) * pow(T(2.0) - T(2.0) * input, power);
    return value * scale + min;
  }

  T reverseMap(T input) const { return map(T(1.0) - input); }

  T invmap(T input) const
  {
    if (input < min) return T(0.0);
    if (input > max) return T(1.0);
    T value = (input - min) / scale;
    return value <= T(0.5) ? T(0.5) * pow(T(2.0) * value, powerInv)
                           : T(1.0) - T(0.5) * pow(T(2.0) - T(2.0) * value, powerInv);
  }

  T getMin() { return min; }
  T getMax() { return max; }

protected:
  T scale;
  T min;
  T max;
  T power;
  T powerInv;
};

// map(inValue) == outValue.
// min > max, inValue > 0, outValue > min.
template<typename T> class LogScale {
public:
  LogScale(T min, T max, T inValue = T(0.5), T outValue = T(0.1))
  {
    set(min, max, inValue, outValue);
  }

  void set(T min, T max, T inValue, T outValue)
  {
    this->min = min;
    this->max = max;

    scale = fabs(max - min);
    expo = log(fabs(outValue - min) / scale) / log(inValue);
    expoInv = 1.0 / expo;
  }

  T map(T input) const
  {
    if (input < T(0.0)) return min;
    if (input > T(1.0)) return max;
    T value = pow(input, expo) * scale + min;
    return value;
  }

  T reverseMap(T input) const { return map(T(1.0) - input); }

  T invmap(T input) const
  {
    if (input < min) return T(0.0);
    if (input > max) return T(1.0);
    T value = pow((input - min) / scale, expoInv);
    return value;
  }

  T getMin() { return min; }
  T getMax() { return max; }

public:
  T scale;
  T expo;
  T expoInv;
  T min;
  T max;
};

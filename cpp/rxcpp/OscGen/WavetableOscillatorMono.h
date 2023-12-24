#pragma once

/*
 * /////////
 * // Clover
 *
 * Audio processing algorithms and DAG with feedback loops that do not break
 * acyclicity.
 *
 * Copyright (C) 2023 Rob W. Albus
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version. This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details. You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <memory>
#include <vector>

#include "OscillatorIndexCalculator.h"

std::vector<float> Sine(int size) {
  std::vector<float> wavetable;
  wavetable.reserve(size);
  float size_ = static_cast<float>(size);
  for (int i = 0; i < size; i++) {
    float i_ = static_cast<float>(i);
    wavetable.emplace_back(static_cast<float>(sin((i_ / size_) * M_PI * 2.)));
  }
  
  return wavetable;
}

struct WavetableOscillatorMono {

  WavetableOscillatorMono(float sampleRateHz, std::shared_ptr<std::vector<float>> wt)
      : calculator(sampleRateHz, wt->size()), wavetable_(wt) {}

  void phase(float phase) { calculator.phase(phase); }
  float phase() { return calculator.phase(); }

  void phaseOffset(float offset) { calculator.phaseOffset(offset); }
  float phaseOffset() { return calculator.phaseOffset(); }

  void freq(float freq) { calculator.freq(freq); }
  float freq() { return calculator.freq(); }

  void period(float periodSamples) { calculator.period(periodSamples); }
  float period() { return calculator.period(); }

  int size() { return calculator.size(); }
  float sampleRate() { return calculator.sampleRate(); }

  float process() {
    OscillatorIndexCalculatorResult index = calculator.process();
    this->processed = std::lerp<float>(
        (*wavetable_)[index.indexA],
        (*wavetable_)[index.indexB],
        index.lerpAmount
    );
    return this->processed;
  }

private:
  OscillatorIndexCalculator calculator;
  std::shared_ptr<std::vector<float>> wavetable_;
public:
  float processed;
  const float &last() const { return processed; }
};

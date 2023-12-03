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

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

struct OscillatorIndexCalculatorResult {
  int indexA;
  int indexB;
  float lerpAmount;
};


struct OscillatorIndexCalculator {

  OscillatorIndexCalculator(float sampleRateHz, int wavetableSize = 0)
      : // clang-format off
      freq_(0.f),
      freqReciprocal_(0.f),
      readIndex_(0.f),
      readIndexIncrementSamples_(0.f),
      phaseOffsetPercent_(0.f),
      phaseOffsetSamples_(0.f)
        // clang-format on
  {
    sampleRate(sampleRateHz);
    size(wavetableSize);
  }

  void freq(float freqHz) {
    freqHz = normalizeFreq(freqHz);
    if (freq_ == freqHz)
      return;

    freq_ = freqHz;
    freqReciprocal_ = freq_ == 0.f ? 0.f : 1.f / freq_;

    calculateReadIndexIncrement();
  }

  float freq() { return freq_; }

  void phase(float phasePercent) {
    phasePercent = normalizePhase(phasePercent);
    readIndex_ =
        normalizeReadIndex(wavetableSize_ * phasePercent + phaseOffsetSamples_);
  }

  float phase() {
    float deOffsetIndex = readIndex_ - phaseOffsetSamples_;

    if (deOffsetIndex < 0.f)
      deOffsetIndex += wavetableSize_;

    return deOffsetIndex * wavetableSizeResciprocal_;
  }

  void phaseOffset(float offsetPercent) {
    readIndex_ -= phaseOffsetSamples_;

    phaseOffsetPercent_ = normalizePhase(offsetPercent);
    phaseOffsetSamples_ = wavetableSize_ * phaseOffsetPercent_;

    readIndex_ += phaseOffsetSamples_;

    if (readIndex_ < 0.f)
      readIndex_ += wavetableSize_;
  }

  float phaseOffset() { return phaseOffsetPercent_; }

  OscillatorIndexCalculatorResult process() {
    this->processed.indexA = static_cast<int>(readIndex_);
    this->processed.indexB =
        static_cast<int>(normalizeReadIndex(this->processed.indexA + 1));
    this->processed.lerpAmount = getFractionalComponent(readIndex_);
    readIndex_ = normalizeReadIndex(readIndex_ + readIndexIncrementSamples_);

    return this->processed;
  }

  void period(float periodSamples) { freq(sampleRate_ / periodSamples); }

  float period() { return sampleRate_ * freqReciprocal_; }

  float sampleRate() { return sampleRate_; }
  int size() { return wavetableSize_; }
  void size(int sizeSamples) {
    sizeSamples = normalizeSize(sizeSamples);
    wavetableSize_ = static_cast<float>(sizeSamples);
    wavetableSizeResciprocal_ =
        wavetableSize_ == 0.f ? 0.f : 1.f / wavetableSize_;
    calculateReadIndexIncrement();
  }

private:
  void sampleRate(float rateHz) {
    sampleRate_ = normalizeSampleRate(rateHz);
    sampleRateReciprocal_ = sampleRate_ == 0.f ? 0.f : 1.f / sampleRate_;
  }

  float normalizePhase(float phase) { return getFractionalComponent(phase); }

  float getFractionalComponent(float num) {
    num = num - static_cast<int>(num);
    if (num < 0.f)
      num += 1.f;
    return num;
  }

  float normalizeFreq(float freq) {
    if (freq < 0.f)
      freq *= -1.f;
    return std::clamp(freq, 0.f, nyquistThreshold());
  }

  float nyquistThreshold() { return sampleRate_ * 0.5f; }

  float normalizeSampleRate(float rateHz) { return std::max(0.f, rateHz); }

  float normalizeReadIndex(float index) {
    while (index >= wavetableSize_) {
      index -= wavetableSize_;
    }

    return index;
  }

  void calculateReadIndexIncrement() {
    readIndexIncrementSamples_ = freq_ * wavetableSize_ * sampleRateReciprocal_;
  }

  int normalizeSize(int size) { return std::max(0, size); }

  float freq_;
  float freqReciprocal_;
  float readIndex_;
  float readIndexIncrementSamples_;

  float phaseOffsetPercent_;
  float phaseOffsetSamples_;

  float sampleRate_;
  float sampleRateReciprocal_;

  float wavetableSize_;
  float wavetableSizeResciprocal_;



public:
  OscillatorIndexCalculatorResult processed;
  const OscillatorIndexCalculatorResult &last() const { return processed; }
};


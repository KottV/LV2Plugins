// Original by:
// DISTRHO Plugin Framework (DPF)
// Copyright (C) 2012-2015 Filipe Coelho <falktx@falktx.com>
//
// Modified by:
// (c) 2019 Takamitsu Endo
//
// This file is part of SyncSawSynth.
//
// SyncSawSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// SyncSawSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with SyncSawSynth.  If not, see <https://www.gnu.org/licenses/>.

#include <utility>

#include "DistrhoPlugin.hpp"
#include "dsp/dspcore.hpp"

START_NAMESPACE_DISTRHO

class SyncSawSynth : public Plugin {
public:
  // Plugin(nParameters, nPrograms, nStates).
  SyncSawSynth() : Plugin(ParameterID::ID_ENUM_LENGTH, 1, 0)
  {
    sampleRateChanged(getSampleRate());
    lastNoteId.reserve(dsp.maxVoice + 1);
    alreadyRecievedNote.reserve(dsp.maxVoice);
  }

protected:
  /* Information */
  const char *getLabel() const override { return "SyncSawSynth"; }
  const char *getDescription() const override
  {
    return "A synthesizer equipped with up to 10th order PTR sawtooth oscillator.";
  }
  const char *getMaker() const override { return "Uhhyou"; }
  const char *getHomePage() const override { return "https://example.com"; }
  const char *getLicense() const override { return "GPLv3"; }
  uint32_t getVersion() const override
  {
    return d_version(MAJOR_VERSION, MINOR_VERSION, PATCH_VERSION);
  }
  int64_t getUniqueId() const override { return d_cconst('u', 's', 'y', 'n'); }

  void initParameter(uint32_t index, Parameter &parameter) override
  {
    dsp.param.initParameter(index, parameter);

    switch (index) {
      case ParameterID::bypass:
        parameter.designation = kParameterDesignationBypass;
        break;
    }

    parameter.symbol = parameter.name;
  }

  float getParameterValue(uint32_t index) const override
  {
    return dsp.param.getParameterValue(index);
  }

  void setParameterValue(uint32_t index, float value) override
  {
    dsp.param.setParameterValue(index, value);
  }

  void initProgramName(uint32_t index, String &programName) override
  {
    switch (index) {
      case 0:
        programName = "Default";
        break;

        // Add program here.
    }
  }

  void loadProgram(uint32_t index) override
  {
    switch (index) {
      case 0:
        break;

        // Add program here.
    }
  }

  void sampleRateChanged(double newSampleRate) { dsp.setup(newSampleRate); }
  void activate() { dsp.startup(); }
  void deactivate() { dsp.reset(); }

  void handleMidi(const MidiEvent ev)
  {
    if (ev.size != 3) return;

    // Copied from nekobi code.
    switch (ev.data[0] & 0xf0) {
      // Note off.
      case 0x80: {
        auto it = std::find_if(
          lastNoteId.begin(), lastNoteId.end(),
          [&](const std::pair<uint8_t, uint32_t> &p) { return p.first == ev.data[1]; });
        if (it == std::end(lastNoteId)) break;
        dsp.noteOff(it->second);
        lastNoteId.erase(it);
      } break;

      // Note on. data[1]: note number, data[2] velocity.
      case 0x90:
        if (ev.data[2] > 0) {
          for (const auto &noteNo : alreadyRecievedNote) {
            if (noteNo == ev.data[1]) goto NoteAlreadyPressed;
          }
          dsp.noteOn(noteId, ev.data[1], 0.0f, ev.data[2] / float(INT8_MAX));
          lastNoteId.push_back(std::pair<uint8_t, uint32_t>(ev.data[1], noteId));
          alreadyRecievedNote.push_back(ev.data[1]);
          noteId += 1;
        }
      NoteAlreadyPressed:
        break;

      // Pitch bend. Center is 8192 (0x2000).
      case 0xe0:
        dsp.param.value[ParameterID::pitchBend]->setFromRaw(
          ((uint16_t(ev.data[2]) << 7) + ev.data[1]) / 16384.0f);
        break;

      default:
        break;
    }
  }

  void run(
    const float **,
    float **outputs,
    uint32_t frames,
    const MidiEvent *midiEvents,
    uint32_t midiEventCount) override
  {
    if (outputs == nullptr) return;
    if (dsp.param.value[ParameterID::bypass]->getRaw()) return;

    const auto timePos = getTimePosition();
    if (!wasPlaying && timePos.playing) dsp.startup();
    wasPlaying = timePos.playing;

    for (size_t i = 0; i < midiEventCount; ++i) handleMidi(midiEvents[i]);
    alreadyRecievedNote.resize(0);

    dsp.setParameters();
    dsp.process(frames, outputs[0], outputs[1]);
  }

private:
  DSPCore dsp;
  bool wasPlaying = false;
  uint32_t noteId = 0;
  std::vector<std::pair<uint8_t, uint32_t>> lastNoteId;
  std::vector<uint8_t> alreadyRecievedNote;

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SyncSawSynth)
};

Plugin *createPlugin() { return new SyncSawSynth(); }

END_NAMESPACE_DISTRHO

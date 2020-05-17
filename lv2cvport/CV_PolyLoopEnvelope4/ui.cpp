// Original by:
// DISTRHO Plugin Framework (DPF)
// Copyright (C) 2012-2015 Filipe Coelho <falktx@falktx.com>
//
// Modified by:
// (c) 2020 Takamitsu Endo
//
// This file is part of CV_PolyLoopEnvelope4.
//
// CV_PolyLoopEnvelope4 is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// CV_PolyLoopEnvelope4 is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with CV_PolyLoopEnvelope4.  If not, see <https://www.gnu.org/licenses/>.

#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <unordered_map>
#include <vector>

#include "../../common/ui.hpp"
#include "parameter.hpp"

#include "../../common/gui/TinosBoldItalic.hpp"
#include "../../common/gui/barbox.hpp"
#include "../../common/gui/button.hpp"
#include "../../common/gui/checkbox.hpp"
#include "../../common/gui/knob.hpp"
#include "../../common/gui/label.hpp"
#include "../../common/gui/optionmenu.hpp"
#include "../../common/gui/rotaryknob.hpp"
#include "../../common/gui/tabview.hpp"
#include "../../common/gui/textview.hpp"
#include "../../common/gui/vslider.hpp"
#include "gui/envelopeview.hpp"

START_NAMESPACE_DISTRHO

constexpr float uiTextSize = 14.0f;
constexpr float midTextSize = 16.0f;
constexpr float pluginNameTextSize = 22.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 80.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr uint32_t defaultWidth = uint32_t(6 * knobX + 30);
constexpr uint32_t defaultHeight = uint32_t(12 * labelY + 30);

enum tabIndex { tabMain, tabPadSynth, tabInfo };

class CV_PolyLoopEnvelope4UI : public PluginUI {
protected:
  void parameterChanged(uint32_t index, float value) override
  {
    updateUI(index, param.parameterChanged(index, value));
  }

  void updateUI(uint32_t id, float normalized)
  {
    auto vWidget = valueWidget.find(id);
    if (vWidget != valueWidget.end()) {
      vWidget->second->setValue(normalized);
      repaint();
    }
  }

  void updateValue(uint32_t id, float normalized) override
  {
    if (id >= ParameterID::ID_ENUM_LENGTH) return;
    setParameterValue(id, param.updateValue(id, normalized));
    repaint();
    // dumpParameter(); // Used to make preset. There may be better way to do this.
  }

  void updateState(std::string /* key */, std::string /* value */)
  {
    // setState(key.c_str(), value.c_str());
  }

  void programLoaded(uint32_t index) override
  {
    param.loadProgram(index);

    for (auto &vPair : valueWidget) {
      if (vPair.second->id >= ParameterID::ID_ENUM_LENGTH) continue;
      vPair.second->setValue(param.value[vPair.second->id]->getNormalized());
    }

    repaint();
  }

  void stateChanged(const char * /* key */, const char * /* value */)
  {
    // This method is required by DPF.
  }

  void onNanoDisplay() override
  {
    envelopeView->update(param);

    beginPath();
    rect(0, 0, getWidth(), getHeight());
    fillColor(colorBack);
    fill();
  }

private:
  GlobalParameter param;

  Color colorBack{255, 255, 255};
  Color colorFore{0, 0, 0};
  Color colorInactive{237, 237, 237};
  Color colorBlue{11, 164, 241};
  Color colorGreen{19, 193, 54};
  Color colorOrange{252, 192, 79};
  Color colorRed{252, 128, 128};

  FontId fontId = -1;

  std::shared_ptr<EnvelopeView> envelopeView;
  std::vector<std::shared_ptr<Widget>> widget;
  std::unordered_map<int, std::shared_ptr<ValueWidget>> valueWidget;

  void dumpParameter()
  {
    std::cout << "{\n";
    for (const auto &value : param.value)
      std::cout << "\"" << value->getName()
                << "\": " << std::to_string(value->getNormalized()) << ",\n";
    std::cout << "}" << std::endl;
  }

  std::shared_ptr<CheckBox>
  addCheckbox(float left, float top, float width, const char *title, uint32_t id)
  {
    auto checkbox = std::make_shared<CheckBox>(this, this, title, fontId);
    checkbox->id = id;
    checkbox->setSize(width, labelHeight);
    checkbox->setAbsolutePos(left, top);
    checkbox->setForegroundColor(colorFore);
    checkbox->setHighlightColor(colorBlue);
    checkbox->setTextSize(uiTextSize);
    valueWidget.emplace(std::make_pair(id, checkbox));
    return checkbox;
  }

  std::shared_ptr<Label> addLabel(
    int left,
    int top,
    float width,
    std::string name,
    int textAlign = ALIGN_CENTER | ALIGN_MIDDLE)
  {
    auto label = std::make_shared<Label>(this, name, fontId);
    label->setSize(width, labelHeight);
    label->setAbsolutePos(left, top);
    label->setForegroundColor(colorFore);
    label->drawBorder = false;
    label->setTextSize(uiTextSize);
    label->setTextAlign(textAlign);
    widget.push_back(label);
    return label;
  };

  std::shared_ptr<Label> addGroupLabel(int left, int top, float width, const char *name)
  {
    auto label = std::make_shared<Label>(this, name, fontId);
    label->setSize(width, labelHeight);
    label->setAbsolutePos(left, top);
    label->setForegroundColor(colorFore);
    label->drawBorder = true;
    label->setBorderWidth(2.0f);
    label->setTextSize(midTextSize);
    widget.push_back(label);
    return label;
  };

  template<typename Scale>
  std::shared_ptr<TextKnob<Scale>> addTextKnob(
    float left,
    float top,
    float width,
    Color highlightColor,
    uint32_t id,
    Scale &scale,
    bool isDecibel = false,
    uint32_t precision = 0,
    int32_t offset = 0)
  {
    auto knob = std::make_shared<TextKnob<Scale>>(this, this, fontId, scale, isDecibel);
    knob->id = id;
    knob->setSize(width, labelHeight);
    knob->setAbsolutePos(left, top);
    knob->setForegroundColor(colorFore);
    knob->setHighlightColor(highlightColor);
    auto defaultValue = param.value[id]->getDefaultNormalized();
    knob->setDefaultValue(defaultValue);
    knob->setValue(defaultValue);
    knob->setPrecision(precision);
    knob->offset = offset;
    knob->setTextSize(uiTextSize);
    valueWidget.emplace(std::make_pair(id, knob));
    return knob;
  }

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CV_PolyLoopEnvelope4UI)

public:
  CV_PolyLoopEnvelope4UI() : PluginUI(defaultWidth, defaultHeight)
  {
    setGeometryConstraints(defaultWidth, defaultHeight, true, true);

    fontId = createFontFromMemory(
      "sans", (unsigned char *)(TinosBoldItalic::TinosBoldItalicData),
      TinosBoldItalic::TinosBoldItalicDataSize, false);

    using ID = ParameterID::ID;

    const auto top0 = 15.0f;
    const auto left0 = 15.0f;

    const auto top1 = top0 + labelY;
    const auto left1 = left0 + knobX;

    const int labelAlign = ALIGN_LEFT | ALIGN_MIDDLE;

    addLabel(left0, top0, 2 * knobX, "CV_PolyLoopEnvelope4");

    addLabel(left0, top1, knobX, "Gain", labelAlign);
    addTextKnob(left1, top1, knobX, colorBlue, ID::gain, Scales::level, false, 4);

    addLabel(left0, top1 + 1 * labelY, knobX, "Loop Start", labelAlign);
    addTextKnob(
      left1, top1 + 1 * labelY, knobX, colorBlue, ID::loopStart, Scales::section);

    addLabel(left0, top1 + 2 * labelY, knobX, "Loop End", labelAlign);
    addTextKnob(left1, top1 + 2 * labelY, knobX, colorBlue, ID::loopEnd, Scales::section);

    addLabel(left0, top1 + 3 * labelY, knobX, "Rate", labelAlign);
    addTextKnob(
      left1, top1 + 3 * labelY, knobX, colorBlue, ID::rate, Scales::rate, false, 2);

    addLabel(left0, top1 + 4 * labelY, knobX, "Slide [s]", labelAlign);
    addTextKnob(
      left1, top1 + 4 * labelY, knobX, colorBlue, ID::rateSlideTime,
      Scales::rateSlideTime, false, 5);

    addCheckbox(
      left0, top1 + 5 * labelY, 2 * knobX, "Rate Key Follow", ID::rateKeyFollow);

    envelopeView = std::make_shared<EnvelopeView>(this, fontId);
    envelopeView->setSize(4 * knobX - 4 * margin, 7 * labelY - 2 * margin);
    envelopeView->setAbsolutePos(left1 + knobX + 4 * margin, top0);

    constexpr size_t nEnvelopeSection = 4;

    const auto leftMatrix0 = left0;
    const std::array<float, nEnvelopeSection> leftMatrix = {
      left0 + 1.0f * knobX,
      left0 + 2.0f * knobX,
      left0 + 3.0f * knobX,
      left0 + 4.0f * knobX,
    };
    const auto leftMatrixRelease = left0 + 5.0f * knobX;

    const auto topMatrix0 = top1 + 6 * labelY;
    const auto topMatrix1 = topMatrix0 + labelY;
    const auto topMatrix2 = topMatrix1 + labelY;
    const auto topMatrix3 = topMatrix2 + labelY;
    const auto topMatrix4 = topMatrix3 + labelY;

    addLabel(leftMatrix0, topMatrix1, knobX, "Decay [s]");
    addLabel(leftMatrix0, topMatrix2, knobX, "Hold [s]");
    addLabel(leftMatrix0, topMatrix3, knobX, "Level");
    addLabel(leftMatrix0, topMatrix4, knobX, "Curve");

    std::string sectionLabel("Section ");
    for (size_t idx = 0; idx < nEnvelopeSection; ++idx) {
      auto indexStr = std::to_string(idx);
      addLabel(leftMatrix[idx], topMatrix0, knobX, sectionLabel + indexStr);
      addTextKnob(
        leftMatrix[idx], topMatrix1, knobX, colorBlue, ID::s0DecayTime + idx,
        Scales::decay, false, 4);
      addTextKnob(
        leftMatrix[idx], topMatrix2, knobX, colorBlue, ID::s0HoldTime + idx,
        Scales::decay, false, 4);
      addTextKnob(
        leftMatrix[idx], topMatrix3, knobX, colorBlue, ID::s0Level + idx, Scales::level,
        false, 4);
      addTextKnob(
        leftMatrix[idx], topMatrix4, knobX, colorBlue, ID::s0Curve + idx, Scales::curve,
        false, 4);
    }

    addLabel(leftMatrixRelease, topMatrix0, knobX, "Release");
    addTextKnob(
      leftMatrixRelease, topMatrix1, knobX, colorBlue, ID::releaseTime, Scales::decay,
      false, 4);
    auto knob = addTextKnob(
      leftMatrixRelease, topMatrix4, knobX, colorBlue, ID::releaseCurve, Scales::curve,
      false, 4);
  }
};

UI *createUI() { return new CV_PolyLoopEnvelope4UI(); }

END_NAMESPACE_DISTRHO
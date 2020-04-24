// Original by:
// DISTRHO Plugin Framework (DPF)
// Copyright (C) 2012-2015 Filipe Coelho <falktx@falktx.com>
//
// Modified by:
// (c) 2019-2020 Takamitsu Endo
//
// This file is part of LightPadSynth.
//
// LightPadSynth is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// LightPadSynth is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with LightPadSynth.  If not, see <https://www.gnu.org/licenses/>.

#include <iostream>
#include <memory>
#include <sstream>
#include <tuple>
#include <vector>

#include "../common/ui.hpp"
#include "parameter.hpp"

#include "../common/gui/TinosBoldItalic.hpp"
#include "../common/gui/barbox.hpp"
#include "../common/gui/button.hpp"
#include "../common/gui/checkbox.hpp"
#include "../common/gui/knob.hpp"
#include "../common/gui/label.hpp"
#include "../common/gui/optionmenu.hpp"
#include "../common/gui/rotaryknob.hpp"
#include "../common/gui/tabview.hpp"
#include "../common/gui/textview.hpp"
#include "../common/gui/vslider.hpp"

START_NAMESPACE_DISTRHO

constexpr float uiTextSize = 14.0f;
constexpr float midTextSize = 16.0f;
constexpr float pluginNameTextSize = 22.0f;
constexpr float margin = 5.0f;
constexpr float labelHeight = 20.0f;
constexpr float labelY = 30.0f;
constexpr float knobWidth = 50.0f;
constexpr float knobHeight = 40.0f;
constexpr float knobX = 60.0f; // With margin.
constexpr float knobY = knobHeight + labelY;
constexpr float barboxWidth = 12.0f * knobX;
constexpr float barboxHeight = 2.0f * knobY;
constexpr float barboxY = barboxHeight + 2.0f * margin;
constexpr float checkboxWidth = 60.0f;
constexpr float splashHeight = 40.0f;
constexpr uint32_t defaultWidth
  = uint32_t(barboxWidth + labelY + 2 * knobX + 12 * margin + 40);
constexpr uint32_t defaultHeight = uint32_t(40 + labelY + 4 * barboxY + 9 * margin);

enum tabIndex { tabMain, tabPadSynth, tabInfo };

class LightPadSynthUI : public PluginUI {
protected:
  void parameterChanged(uint32_t index, float value) override
  {
    updateUI(index, param.parameterChanged(index, value));
  }

  void updateUI(uint32_t id, float normalized)
  {
    for (auto &vWidget : valueWidget) {
      if (vWidget->id != id) continue;
      vWidget->setValue(normalized);
      repaint();
      return;
    }

    for (auto &aWidget : arrayWidget) {
      auto &idVec = aWidget->id;
      auto iter = std::find(idVec.begin(), idVec.end(), id);
      if (iter != idVec.end()) {
        aWidget->setValueAt(std::distance(idVec.begin(), iter), normalized);
        repaint();
        return;
      }
    }
  }

  void updateValue(uint32_t id, float normalized) override
  {
    if (id >= ParameterID::ID_ENUM_LENGTH) return;
    setParameterValue(id, param.updateValue(id, normalized));
    repaint();
    // dumpParameter(); // Used to make preset. There may be better way to do this.
  }

  void updateState(std::string key, std::string value)
  {
    setState(key.c_str(), value.c_str());
  }

  void programLoaded(uint32_t index) override
  {
    param.loadProgram(index);

    for (auto &vWidget : valueWidget) {
      if (vWidget->id >= ParameterID::ID_ENUM_LENGTH) continue;
      vWidget->setValue(param.value[vWidget->id]->getNormalized());
    }

    for (auto &aWidget : arrayWidget) {
      for (size_t idx = 0; idx < aWidget->id.size(); ++idx) {
        if (aWidget->id[idx] >= ParameterID::ID_ENUM_LENGTH) continue;
        aWidget->setValueAt(idx, param.value[aWidget->id[idx]]->getNormalized());
      }
    }

    repaint();
  }

  void stateChanged(const char * /* key */, const char * /* value */)
  {
    // This method is required by DPF.
  }

  void onNanoDisplay() override
  {
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

  std::vector<std::shared_ptr<Widget>> widget;
  std::vector<std::shared_ptr<ValueWidget>> valueWidget;
  std::vector<std::shared_ptr<ArrayWidget>> arrayWidget;
  std::vector<std::shared_ptr<StateWidget>> stateWidget;

  void dumpParameter()
  {
    std::cout << "{\n";
    for (const auto &value : param.value)
      std::cout << "\"" << value->getName()
                << "\": " << std::to_string(value->getNormalized()) << ",\n";
    std::cout << "}" << std::endl;
  }

  std::shared_ptr<BarBox> addBarBox(
    float left, float top, float width, float height, uint32_t id0, size_t nElement)
  {
    std::vector<uint32_t> id(nElement);
    for (size_t i = 0; i < id.size(); ++i) id[i] = id0 + i;
    std::vector<double> value(id.size());
    for (size_t i = 0; i < value.size(); ++i)
      value[i] = param.value[id[i]]->getDefaultNormalized();
    std::vector<double> defaultValue(value);

    auto barBox = std::make_shared<BarBox>(this, this, id, value, defaultValue, fontId);
    barBox->setSize(width, height);
    barBox->setAbsolutePos(left, top);
    barBox->setBorderColor(colorFore);
    barBox->setValueColor(colorBlue);
    arrayWidget.push_back(barBox);
    return barBox;
  }

  std::shared_ptr<ToggleButton>
  addButton(float left, float top, float width, const char *title, uint32_t id)
  {
    auto button = std::make_shared<ToggleButton>(this, this, title, fontId);
    button->id = id;
    button->setSize(width, labelHeight);
    button->setAbsolutePos(left, top);
    button->setForegroundColor(colorFore);
    button->setHighlightColor(colorOrange);
    button->setTextSize(midTextSize);
    valueWidget.push_back(button);
    return button;
  }

  std::shared_ptr<StateButton> addStateButton(
    float left,
    float top,
    float width,
    std::string title,
    std::string key,
    std::string value)
  {
    auto button = std::make_shared<StateButton>(this, this, title, key, value, fontId);
    button->setSize(width, 2.0f * labelHeight);
    button->setAbsolutePos(left, top);
    button->setForegroundColor(colorFore);
    button->setHighlightColor(colorRed);
    button->setTextSize(midTextSize);
    stateWidget.push_back(button);
    return button;
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
    valueWidget.push_back(checkbox);
    return checkbox;
  }

  std::shared_ptr<Label> addLabel(int left, int top, float width, const char *name)
  {
    auto label = std::make_shared<Label>(this, name, fontId);
    label->setSize(width, labelHeight);
    label->setAbsolutePos(left, top);
    label->setForegroundColor(colorFore);
    label->drawBorder = false;
    label->setTextSize(uiTextSize);
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

  std::shared_ptr<VLabel>
  addGroupVerticalLabel(int left, int top, float width, const char *name)
  {
    auto label = std::make_shared<VLabel>(this, name, fontId);
    label->setSize(width, labelHeight);
    label->setAbsolutePos(left, top);
    label->setForegroundColor(colorFore);
    label->drawBorder = false;
    label->setBorderWidth(2.0f);
    label->setTextSize(midTextSize);
    widget.push_back(label);
    return label;
  };

  enum class LabelPosition {
    top,
    left,
    bottom,
    right,
  };

  auto addKnob(
    float left,
    float top,
    float width,
    Color highlightColor,
    const char *name,
    uint32_t id,
    LabelPosition labelPosition = LabelPosition::bottom)
  {
    auto height = width - 2.0f * margin;

    auto knob = std::make_shared<Knob>(this, this);
    knob->id = id;
    knob->setSize(width - 2.0f * margin, height);
    knob->setAbsolutePos(left + margin, top + margin);
    knob->setHighlightColor(highlightColor);
    if (id < param.value.size()) {
      auto defaultValue = param.value[id]->getDefaultNormalized();
      knob->setDefaultValue(defaultValue);
      knob->setValue(defaultValue);
    }
    valueWidget.push_back(knob);

    auto label = addKnobLabel(left, top, width, height, name, labelPosition);
    return std::make_tuple(knob, label);
  }

  template<typename Scale>
  auto addNumberKnob(
    float left,
    float top,
    float width,
    Color highlightColor,
    const char *name,
    uint32_t id,
    Scale &scale,
    LabelPosition labelPosition = LabelPosition::bottom)
  {
    auto height = width - 2.0f * margin;

    auto knob = std::make_shared<NumberKnob<Scale>>(this, this, fontId, scale);
    knob->id = id;
    knob->setSize(width - 2.0f * margin, height);
    knob->setAbsolutePos(left + margin, top + margin);
    knob->setHighlightColor(highlightColor);
    auto defaultValue = param.value[id]->getDefaultNormalized();
    knob->setDefaultValue(defaultValue);
    knob->setValue(defaultValue);
    valueWidget.push_back(knob);

    auto label = addKnobLabel(left, top, width, height, name, labelPosition);
    return std::make_tuple(knob, label);
  }

  auto addRotaryKnob(
    float left,
    float top,
    float width,
    Color highlightColor,
    const char *name,
    uint32_t id,
    LabelPosition labelPosition = LabelPosition::bottom)
  {
    auto height = width - 2.0f * margin;

    auto knob = std::make_shared<RotaryKnob>(this, this);
    knob->id = id;
    knob->setSize(width - 2.0f * margin, height);
    knob->setAbsolutePos(left + margin, top + margin);
    knob->setHighlightColor(highlightColor);
    auto defaultValue = param.value[id]->getDefaultNormalized();
    knob->setDefaultValue(defaultValue);
    knob->setValue(defaultValue);
    valueWidget.push_back(knob);

    auto label = addKnobLabel(left, top, width, height, name, labelPosition);
    return std::make_tuple(knob, label);
  }

  std::shared_ptr<Label> addKnobLabel(
    float left,
    float top,
    float width,
    float height,
    const char *name,
    LabelPosition labelPosition)
  {
    switch (labelPosition) {
      default:
      case LabelPosition::bottom:
        top = top + height;
        height = 30.0f;
        break;

      case LabelPosition::right:
        height = width;
        left = left + width + 10.0;
        width *= 2.0f;
        break;
    }

    auto label = std::make_shared<Label>(this, name, fontId);
    label->setSize(width, height);
    label->setAbsolutePos(left, top);
    label->setForegroundColor(colorFore);
    label->setTextSize(uiTextSize);
    if (labelPosition == LabelPosition::right)
      label->setTextAlign(ALIGN_LEFT | ALIGN_MIDDLE);
    widget.push_back(label);
    return label;
  }

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
    valueWidget.push_back(knob);
    return knob;
  }

  std::shared_ptr<OptionMenu> addOptionMenu(
    float left,
    float top,
    float width,
    uint32_t id,
    const std::vector<std::string> &items)
  {
    auto menu = std::make_shared<OptionMenu>(this, this, items, fontId);
    menu->id = id;
    menu->setSize(width, labelHeight);
    menu->setAbsolutePos(left, top);
    menu->setDefaultValue(param.value[id]->getDefaultNormalized());
    menu->setForegroundColor(colorFore);
    menu->setHighlightColor(colorBlue);
    menu->setTextSize(uiTextSize);
    valueWidget.push_back(menu);
    return menu;
  }

  auto addVSlider(float left, float top, Color valueColor, const char *name, uint32_t id)
  {
    // width, height = 100, 270.

    const auto width = 70.0f;
    const auto sliderHeight = 230.0f;

    auto slider = std::make_shared<VSlider>(this, this);
    slider->id = id;
    slider->setSize(width, sliderHeight);
    slider->setAbsolutePos(left, top);
    auto defaultValue = param.value[id]->getDefaultNormalized();
    slider->setDefaultValue(defaultValue);
    slider->setValue(defaultValue);
    slider->setHighlightColor(valueColor);
    slider->setValueColor(valueColor);
    slider->setBorderColor(colorFore);
    valueWidget.push_back(slider);

    top += sliderHeight + 10.0;

    auto label = std::make_shared<Label>(this, name, fontId);
    label->setSize(width, labelHeight);
    label->setAbsolutePos(left, top);
    label->setForegroundColor(colorFore);
    label->setTextSize(uiTextSize);
    widget.push_back(label);

    return std::make_tuple(slider, label);
  }

  auto addTextView(float left, float top, float width, float height, std::string text)
  {
    auto view = std::make_shared<TextView>(this, text, fontId);
    view->setSize(width, height);
    view->setAbsolutePos(left, top);
    return view;
  }

  auto addTextTableView(
    float left, float top, float width, float height, std::string text, float cellWidth)
  {
    auto view = std::make_shared<TextTableView>(this, text, cellWidth, fontId);
    view->setSize(width, height);
    view->setAbsolutePos(left, top);
    return view;
  }

  DISTRHO_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LightPadSynthUI)

public:
  LightPadSynthUI() : PluginUI(defaultWidth, defaultHeight)
  {
    setGeometryConstraints(defaultWidth, defaultHeight, true, true);

    fontId = createFontFromMemory(
      "sans", (unsigned char *)(TinosBoldItalic::TinosBoldItalicData),
      TinosBoldItalic::TinosBoldItalicDataSize, false);

    using ID = ParameterID::ID;

    const auto top0 = 20.0f;
    const auto left0 = 20.0f;

    // TabView test.
    const auto tabTop0 = top0;
    const auto tabLeft0 = left0;

    std::vector<std::string> tabs = {"Main", "Wavetable", "Information"};
    const auto tabWidth = defaultWidth - 40.0f;
    const auto tabHeight = labelY + 4.0f * barboxY + 9.0f * margin;
    auto tabview = std::make_shared<TabView>(
      this, tabs, fontId, labelY, tabLeft0, tabTop0, tabWidth, tabHeight);
    tabview->setHighlightColor(colorBlue);
    widget.push_back(tabview);

    const auto tabInsideTop0 = tabTop0 + labelY + 4.0f * margin;
    const auto tabInsideLeft0 = tabLeft0 + 4.0f * margin;

    // Gain.
    const auto gainTop = tabInsideTop0;
    const auto gainLeft = tabInsideLeft0 + labelY;
    tabview->addWidget(tabMain, addGroupLabel(gainLeft, gainTop, 6.0f * knobX, "Gain"));
    const auto gainKnobTop = gainTop + labelY;

    tabview->addWidget(
      tabMain, addKnob(gainLeft, gainKnobTop, knobWidth, colorBlue, "Gain", ID::gain));
    tabview->addWidget(
      tabMain,
      addKnob(
        gainLeft + 1.0f * knobX, gainKnobTop, knobWidth, colorBlue, "A", ID::gainA));
    tabview->addWidget(
      tabMain,
      addKnob(
        gainLeft + 2.0f * knobX, gainKnobTop, knobWidth, colorBlue, "D", ID::gainD));
    tabview->addWidget(
      tabMain,
      addKnob(
        gainLeft + 3.0f * knobX, gainKnobTop, knobWidth, colorBlue, "S", ID::gainS));
    tabview->addWidget(
      tabMain,
      addKnob(
        gainLeft + 4.0f * knobX, gainKnobTop, knobWidth, colorBlue, "R", ID::gainR));
    tabview->addWidget(
      tabMain,
      addKnob(
        gainLeft + 5.0f * knobX, gainKnobTop, knobWidth, colorBlue, "Curve",
        ID::gainCurve));

    // Filter.
    const auto filterTop = gainTop;
    const auto filterLeft = gainLeft + 6.0f * knobX + 2.0f * margin;
    tabview->addWidget(
      tabMain, addGroupLabel(filterLeft, filterTop, 8.0f * knobX, "Filter"));

    const auto filterTop0 = filterTop + labelY;
    tabview->addWidget(
      tabMain,
      addKnob(filterLeft, filterTop0, knobWidth, colorBlue, "Cutoff", ID::filterCutoff));
    tabview->addWidget(
      tabMain,
      addKnob(
        filterLeft + 1.0f * knobX, filterTop0, knobWidth, colorBlue, "Resonance",
        ID::filterResonance));

    tabview->addWidget(
      tabMain,
      addKnob(
        filterLeft + 2.0f * knobX, filterTop0, knobWidth, colorBlue, "A", ID::filterA));
    tabview->addWidget(
      tabMain,
      addKnob(
        filterLeft + 3.0f * knobX, filterTop0, knobWidth, colorBlue, "D", ID::filterD));
    tabview->addWidget(
      tabMain,
      addKnob(
        filterLeft + 4.0f * knobX, filterTop0, knobWidth, colorBlue, "S", ID::filterS));
    tabview->addWidget(
      tabMain,
      addKnob(
        filterLeft + 5.0f * knobX, filterTop0, knobWidth, colorBlue, "R", ID::filterR));
    tabview->addWidget(
      tabMain,
      addKnob(
        filterLeft + 6.0f * knobX, filterTop0, knobWidth, colorBlue, "Amount",
        ID::filterAmount));
    tabview->addWidget(
      tabMain,
      addKnob(
        filterLeft + 7.0f * knobX, filterTop0, knobWidth, colorBlue, "KeyFollow",
        ID::filterKeyFollow));

    // Unison.
    const auto unisonTop = tabInsideTop0 + labelY + knobY;
    const auto unisonLeft = gainLeft;
    tabview->addWidget(
      tabMain, addGroupLabel(unisonLeft, unisonTop, 7.0f * knobX, "Unison"));
    const auto unisonKnobTop = unisonTop + labelY;
    tabview->addWidget(
      tabMain, addLabel(unisonLeft, unisonKnobTop, knobWidth, "nUnison"));
    tabview->addWidget(
      tabMain,
      addTextKnob(
        unisonLeft, unisonKnobTop + labelHeight + margin, knobWidth, colorBlue,
        ID::nUnison, Scales::nUnison, false, 0, 1));
    tabview->addWidget(
      tabMain,
      addKnob(
        unisonLeft + knobX, unisonKnobTop, knobWidth, colorBlue, "Detune",
        ID::unisonDetune));
    tabview->addWidget(
      tabMain,
      addKnob(
        unisonLeft + 2.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "GainRnd",
        ID::unisonGainRandom));
    tabview->addWidget(
      tabMain,
      addKnob(
        unisonLeft + 3.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "Phase",
        ID::unisonPhase));
    tabview->addWidget(
      tabMain,
      addKnob(
        unisonLeft + 4.0f * knobX, unisonKnobTop, knobWidth, colorBlue, "Spread",
        ID::unisonPan));
    tabview->addWidget(
      tabMain,
      addCheckbox(
        unisonLeft + 1.0f * knobX - 1.5f * margin, unisonKnobTop + knobY, knobWidth,
        "Random Detune", ID::unisonDetuneRandom));

    std::vector<std::string> nVoiceOptions
      = {"16 Voices", "32 Voices", "48 Voices",  "64 Voices",
         "80 Voices", "96 Voices", "112 Voices", "128 Voices"};
    tabview->addWidget(
      tabMain,
      addOptionMenu(
        unisonLeft + 3.0f * knobX - 1.0f * margin, unisonKnobTop + knobY, checkboxWidth,
        ID::nVoice, nVoiceOptions));

    std::vector<std::string> unisonPanTypeOptions{
      "Alternate L-R", "Alternate M-S", "Ascend L -> R", "Ascend R -> L", "High on Mid",
      "High on Side",  "Random",        "Rotate L",      "Rotate R",      "Shuffle"};
    tabview->addWidget(
      tabMain,
      addLabel(
        unisonLeft + 5.0f * knobX, unisonKnobTop, 2.0f * knobWidth, "Spread Type"));
    tabview->addWidget(
      tabMain,
      addOptionMenu(
        unisonLeft + 5.0f * knobX, unisonKnobTop + labelY, 2.0f * knobWidth,
        ID::unisonPanType, unisonPanTypeOptions));

    // Phase.
    const auto phaseTop = unisonTop;
    const auto phaseLeft = unisonLeft + 7.0f * knobX + 2.0f * margin;
    tabview->addWidget(
      tabMain, addGroupLabel(phaseLeft, phaseTop, 2.0f * knobX, "Phase"));
    const auto phaseKnobTop = phaseTop + labelY;

    tabview->addWidget(
      tabMain,
      addKnob(
        phaseLeft, phaseKnobTop, knobWidth, colorBlue, "Phase", ID::oscInitialPhase));

    const auto phaseLeft1 = phaseLeft + knobX;
    tabview->addWidget(
      tabMain,
      addCheckbox(phaseLeft1, phaseKnobTop, knobWidth, "Reset", ID::oscPhaseReset));
    tabview->addWidget(
      tabMain,
      addCheckbox(
        phaseLeft1, phaseKnobTop + labelY, knobWidth, "Random", ID::oscPhaseRandom));

    // Misc.
    const auto miscTop = phaseTop;
    const auto miscLeft = phaseLeft + 2.0f * knobX + 2.0f * margin;
    tabview->addWidget(
      tabMain, addGroupLabel(miscLeft, miscTop, 1.0f * knobX + 2.0f * margin, "Misc."));

    const auto miscTop0 = miscTop + labelY;
    const auto miscLeft0 = miscLeft + 2.0f * margin;
    tabview->addWidget(
      tabMain,
      addKnob(miscLeft0, miscTop0, knobWidth, colorBlue, "Smooth", ID::smoothness));

    // Tuning.
    const auto tuningTop = unisonTop;
    const auto tuningLeft = miscLeft + 1.0f * knobX + 4.0f * margin;
    tabview->addWidget(
      tabMain,
      addGroupLabel(tuningLeft, tuningTop, 4.0f * knobX - 6.0 * margin, "Tuning"));

    const auto tuningLabelWidth = knobX - 2.0f * margin;
    const auto tuningLeft1 = tuningLeft;
    const auto tuningLeft2 = tuningLeft1 + tuningLabelWidth;

    const auto tuningTop1 = tuningTop + labelY;
    tabview->addWidget(
      tabMain, addLabel(tuningLeft1, tuningTop1, tuningLabelWidth, "Octave"));
    tabview->addWidget(
      tabMain,
      addTextKnob(
        tuningLeft2, tuningTop1, knobWidth, colorBlue, ID::oscOctave, Scales::oscOctave,
        false, 0, -12));

    const auto tuningTop2 = tuningTop1 + labelY;
    tabview->addWidget(
      tabMain, addLabel(tuningLeft1, tuningTop2, tuningLabelWidth, "Semi"));
    tabview->addWidget(
      tabMain,
      addTextKnob(
        tuningLeft2, tuningTop2, knobWidth, colorBlue, ID::oscSemi, Scales::oscSemi,
        false, 0, -120));

    const auto tuningTop3 = tuningTop2 + labelY;
    tabview->addWidget(
      tabMain, addLabel(tuningLeft1, tuningTop3, tuningLabelWidth, "Milli"));
    auto knobOscMilli = addTextKnob(
      tuningLeft2, tuningTop3, knobWidth, colorBlue, ID::oscMilli, Scales::oscMilli,
      false, 0, -1000);
    knobOscMilli->sensitivity = 0.001f;
    knobOscMilli->lowSensitivity = 0.00025f;
    tabview->addWidget(tabMain, knobOscMilli);

    const auto tuningLeft3 = tuningLeft + 2.0f * knobX - 2.0f * margin;
    const auto tuningLeft4 = tuningLeft3 + tuningLabelWidth;

    const auto tuningTop4 = tuningTop + 1.0f * labelY;
    tabview->addWidget(
      tabMain, addLabel(tuningLeft3, tuningTop4, tuningLabelWidth, "ET"));
    tabview->addWidget(
      tabMain,
      addTextKnob(
        tuningLeft4, tuningTop4, knobWidth, colorBlue, ID::equalTemperament,
        Scales::equalTemperament, false, 0, 1));

    const auto tuningTop5 = tuningTop4 + labelY;
    tabview->addWidget(
      tabMain, addLabel(tuningLeft3, tuningTop5, tuningLabelWidth, "A4 [Hz]"));
    tabview->addWidget(
      tabMain,
      addTextKnob(
        tuningLeft4, tuningTop5, knobWidth, colorBlue, ID::pitchA4Hz, Scales::pitchA4Hz,
        false, 0, 100));

    // Delay.
    const auto delayTop = unisonTop + 2.0f * labelY + knobY;
    const auto delayLeft = gainLeft;
    tabview->addWidget(
      tabMain, addGroupLabel(delayLeft, delayTop, 5.0f * knobX, "Delay"));
    const auto delayKnobTop = delayTop + labelY;

    tabview->addWidget(
      tabMain,
      addKnob(delayLeft, delayKnobTop, knobWidth, colorBlue, "Mix", ID::delayMix));
    tabview->addWidget(
      tabMain,
      addKnob(
        delayLeft + 1.0f * knobX, delayKnobTop, knobWidth, colorBlue, "Feedback",
        ID::delayFeedback));
    tabview->addWidget(
      tabMain,
      addKnob(
        delayLeft + 2.0f * knobX, delayKnobTop, knobWidth, colorBlue, "Attack",
        ID::delayAttack));

    const auto delayDetuneLeft = delayLeft + 3.0f * knobX;
    tabview->addWidget(
      tabMain, addLabel(delayDetuneLeft, delayKnobTop, tuningLabelWidth, "Semi"));
    tabview->addWidget(
      tabMain,
      addTextKnob(
        delayDetuneLeft + knobX, delayKnobTop, knobWidth, colorBlue, ID::delayDetuneSemi,
        Scales::delayDetuneSemi, false, 0, -120));

    tabview->addWidget(
      tabMain,
      addLabel(delayDetuneLeft, delayKnobTop + labelY, tuningLabelWidth, "Milli"));
    auto knobDelayMilli = addTextKnob(
      delayDetuneLeft + knobX, delayKnobTop + labelY, knobWidth, colorBlue,
      ID::delayDetuneMilli, Scales::oscMilli, false, 0, -1000);
    knobDelayMilli->sensitivity = 0.001f;
    knobDelayMilli->lowSensitivity = 0.00025f;
    tabview->addWidget(tabMain, knobDelayMilli);

    // LFO.
    const auto lfoTop = delayTop;
    const auto lfoLeft = delayLeft + 5.0f * knobX + 2.0f * margin;
    tabview->addWidget(
      tabMain, addGroupLabel(lfoLeft, lfoTop, 9.0f * knobX, "Delay LFO"));
    const auto lfoKnobTop = lfoTop + labelY;

    const auto lfoLeft1 = lfoLeft;
    const auto lfoTempoTop = lfoKnobTop;
    auto knobLfoTempoNumerator = addTextKnob(
      lfoLeft1, lfoTempoTop, knobWidth, colorBlue, ID::lfoTempoNumerator,
      Scales::lfoTempoNumerator, false, 0, 1);
    knobLfoTempoNumerator->sensitivity = 0.001;
    knobLfoTempoNumerator->lowSensitivity = 0.00025;
    tabview->addWidget(tabMain, knobLfoTempoNumerator);

    auto knobLfoTempoDenominator = addTextKnob(
      lfoLeft1, lfoTempoTop + labelHeight + 1.0f, knobWidth, colorBlue,
      ID::lfoTempoDenominator, Scales::lfoTempoDenominator, false, 0, 1);
    knobLfoTempoDenominator->sensitivity = 0.001;
    knobLfoTempoNumerator->lowSensitivity = 0.00025;
    tabview->addWidget(tabMain, knobLfoTempoDenominator);

    tabview->addWidget(
      tabMain,
      addLabel(
        lfoLeft1, lfoTempoTop + labelHeight + labelY - margin, knobWidth, "Tempo"));

    tabview->addWidget(
      tabMain,
      addKnob(
        lfoLeft1 + 1.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Multiply",
        ID::lfoFrequencyMultiplier));
    tabview->addWidget(
      tabMain,
      addKnob(
        lfoLeft1 + 2.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Amount",
        ID::lfoDelayAmount));
    tabview->addWidget(
      tabMain,
      addKnob(
        lfoLeft1 + 3.0f * knobX, lfoKnobTop, knobWidth, colorBlue, "Lowpass",
        ID::lfoLowpass));

    const auto lfoLeft2 = lfoLeft + 5.0f * knobX;

    std::vector<std::string> lfoWavetableTypeOptions{"Step", "Linear", "Cubic"};
    tabview->addWidget(
      tabMain,
      addLabel(lfoLeft2 + 0.375f * knobX, lfoKnobTop, knobWidth * 1.5f, "Interpolation"));
    tabview->addWidget(
      tabMain,
      addOptionMenu(
        lfoLeft2 + 0.375f * knobX, lfoKnobTop + labelY, knobWidth * 1.5f,
        ID::lfoWavetableType, lfoWavetableTypeOptions));

    tabview->addWidget(
      tabMain,
      addStateButton(
        lfoLeft2 + 2.0f * knobX, lfoKnobTop + 2.0f * margin, 2.0f * knobX, "Refresh LFO",
        "lfo", "N/A"));

    // LFO wavetable.
    const auto lfoWaveTop = lfoKnobTop + knobY + 0.5f * labelY;
    const auto lfoWaveLeft = tabInsideLeft0;
    const auto lfoBarboxHeight = barboxHeight + 3.5f * labelY + 3.0f * margin;
    tabview->addWidget(
      tabMain,
      addGroupVerticalLabel(lfoWaveLeft, lfoWaveTop, lfoBarboxHeight, "LFO Wave"));
    auto barboxLfoWavetable = addBarBox(
      lfoWaveLeft + labelY, lfoWaveTop, barboxWidth + 2.0f * knobX + 4.0f * margin,
      lfoBarboxHeight, ID::lfoWavetable0, nLFOWavetable);
    barboxLfoWavetable->drawCenterLine = true;
    tabview->addWidget(tabMain, barboxLfoWavetable);

    // Wavetable pitch.
    const auto tablePitchTop = tabInsideTop0;
    const auto tablePitchLeft0 = tabInsideLeft0;
    const auto tablePitchLeft1 = tablePitchLeft0 + knobX;
    tabview->addWidget(
      tabPadSynth, addGroupLabel(tablePitchLeft0, tablePitchTop, 2.0f * knobX, "Pitch"));

    tabview->addWidget(
      tabPadSynth,
      addLabel(tablePitchLeft0, tablePitchTop + labelY, knobX, "Base Freq."));
    tabview->addWidget(
      tabPadSynth,
      addTextKnob(
        tablePitchLeft1, tablePitchTop + labelY, knobX, colorBlue, ID::tableBaseFrequency,
        Scales::tableBaseFrequency, false, 2));

    tabview->addWidget(
      tabPadSynth,
      addLabel(tablePitchLeft0, tablePitchTop + 2.0f * labelY, knobX, "Multiply"));
    tabview->addWidget(
      tabPadSynth,
      addTextKnob(
        tablePitchLeft1, tablePitchTop + 2.0f * labelY, knobX, colorBlue,
        ID::overtonePitchMultiply, Scales::overtonePitchMultiply, false, 4));

    tabview->addWidget(
      tabPadSynth,
      addLabel(tablePitchLeft0, tablePitchTop + 3.0f * labelY, knobX, "Modulo"));
    tabview->addWidget(
      tabPadSynth,
      addTextKnob(
        tablePitchLeft1, tablePitchTop + 3.0f * labelY, knobX, colorBlue,
        ID::overtonePitchModulo, Scales::overtonePitchModulo, false, 4));

    const auto tableSpectrumTop = tablePitchTop + 5.0f * labelY;
    const auto tableSpectrumLeft0 = tablePitchLeft0;
    const auto tableSpectrumLeft1 = tablePitchLeft1;
    tabview->addWidget(
      tabPadSynth,
      addGroupLabel(tableSpectrumLeft0, tableSpectrumTop, 2.0f * knobX, "Spectrum"));

    tabview->addWidget(
      tabPadSynth,
      addLabel(tableSpectrumLeft0, tableSpectrumTop + labelY, knobX, "Expand"));
    tabview->addWidget(
      tabPadSynth,
      addTextKnob(
        tableSpectrumLeft1, tableSpectrumTop + labelY, knobX, colorBlue,
        ID::spectrumExpand, Scales::spectrumExpand, false, 4));

    tabview->addWidget(
      tabPadSynth,
      addLabel(tableSpectrumLeft0, tableSpectrumTop + 2.0f * labelY, knobX, "Shift"));
    auto knobSpectrumShift = addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + 2.0f * labelY, knobX, colorBlue,
      ID::spectrumShift, Scales::spectrumShift, false, 0, -spectrumSize);
    knobSpectrumShift->sensitivity = 1.0f / spectrumSize;
    knobSpectrumShift->lowSensitivity = 0.08f / spectrumSize;
    tabview->addWidget(tabPadSynth, knobSpectrumShift);

    tabview->addWidget(
      tabPadSynth,
      addLabel(tableSpectrumLeft0, tableSpectrumTop + 3.0 * labelY, knobX, "Comb"));
    auto knobProfileComb = addTextKnob(
      tableSpectrumLeft1, tableSpectrumTop + 3.0 * labelY, knobX, colorBlue,
      ID::profileComb, Scales::profileComb);
    knobProfileComb->sensitivity = 0.002;
    knobProfileComb->lowSensitivity = 0.0002;
    tabview->addWidget(tabPadSynth, knobProfileComb);

    tabview->addWidget(
      tabPadSynth,
      addLabel(tableSpectrumLeft0, tableSpectrumTop + 4.0 * labelY, knobX, "Shape"));
    tabview->addWidget(
      tabPadSynth,
      addTextKnob(
        tableSpectrumLeft1, tableSpectrumTop + 4.0 * labelY, knobX, colorBlue,
        ID::profileShape, Scales::profileShape, false, 4, 0));

    const auto tablePhaseTop = tableSpectrumTop + 6.0f * labelY;
    const auto tablePhaseLeft0 = tablePitchLeft0;
    tabview->addWidget(
      tabPadSynth, addGroupLabel(tablePhaseLeft0, tablePhaseTop, 2.0f * knobX, "Phase"));
    tabview->addWidget(
      tabPadSynth,
      addCheckbox(
        tablePhaseLeft0, tablePhaseTop + labelY, checkboxWidth, "UniformPhase",
        ID::uniformPhaseProfile));

    // Wavetable random.
    const auto tableRandomTop = tablePhaseTop + 2.0f * labelY;
    const auto tableRandomLeft0 = tablePitchLeft0;
    const auto tableRandomLeft1 = tablePitchLeft1;
    tabview->addWidget(
      tabPadSynth,
      addGroupLabel(tableRandomLeft0, tableRandomTop, 2.0f * knobX, "Random"));

    tabview->addWidget(
      tabPadSynth, addLabel(tableRandomLeft0, tableRandomTop + labelY, knobX, "Seed"));
    tabview->addWidget(
      tabPadSynth,
      addTextKnob(
        tableRandomLeft1, tableRandomTop + labelY, knobX, colorBlue, ID::padSynthSeed,
        Scales::seed));

    // Wavetable modifier.
    const auto tableModifierTop = tableRandomTop + 2.0f * labelY;
    const auto tableModifierLeft0 = tablePitchLeft0;
    const auto tableModifierLeft1 = tablePitchLeft1;
    tabview->addWidget(
      tabPadSynth,
      addGroupLabel(tableModifierLeft0, tableModifierTop, 2.0f * knobX, "Modifier"));

    const auto tableModifierTop0 = tableModifierTop + labelY;
    tabview->addWidget(
      tabPadSynth,
      addKnob(
        tableModifierLeft0, tableModifierTop0, knobWidth, colorBlue, "Gain^",
        ID::overtoneGainPower));
    tabview->addWidget(
      tabPadSynth,
      addKnob(
        tableModifierLeft1, tableModifierTop0, knobWidth, colorBlue, "Width*",
        ID::overtoneWidthMultiply));

    // Refresh button.
    const auto refreshTop = tabTop0 + tabHeight - 2.0f * labelY;
    const auto refreshLeft = tabInsideLeft0;
    tabview->addWidget(
      tabPadSynth,
      addStateButton(
        refreshLeft, refreshTop, 2.0f * knobX, "Refresh Table", "padsynth", "N/A"));

    // Overtone Gain.
    const auto otGainTop = tabInsideTop0;
    const auto otGainLeft = tabInsideLeft0 + 2.0f * knobX + 4.0f * margin;
    tabview->addWidget(
      tabPadSynth, addGroupVerticalLabel(otGainLeft, otGainTop, barboxHeight, "Gain"));

    const auto otGainLeft0 = otGainLeft + labelY;
    tabview->addWidget(
      tabPadSynth,
      addBarBox(
        otGainLeft0, otGainTop, barboxWidth, barboxHeight, ID::overtoneGain0, nOvertone));

    // Overtone Width.
    const auto otWidthTop = otGainTop + barboxY + margin;
    const auto otWidthLeft = otGainLeft;
    tabview->addWidget(
      tabPadSynth, addGroupVerticalLabel(otWidthLeft, otWidthTop, barboxHeight, "Width"));

    const auto otWidthLeft0 = otWidthLeft + labelY;
    tabview->addWidget(
      tabPadSynth,
      addBarBox(
        otWidthLeft0, otWidthTop, barboxWidth, barboxHeight, ID::overtoneWidth0,
        nOvertone));

    // Overtone Pitch.
    const auto otPitchTop = otWidthTop + barboxY + margin;
    const auto otPitchLeft = otGainLeft;
    tabview->addWidget(
      tabPadSynth, addGroupVerticalLabel(otPitchLeft, otPitchTop, barboxHeight, "Pitch"));

    const auto otPitchLeft0 = otPitchLeft + labelY;
    tabview->addWidget(
      tabPadSynth,
      addBarBox(
        otPitchLeft0, otPitchTop, barboxWidth, barboxHeight, ID::overtonePitch0,
        nOvertone));

    // Overtone Phase.
    const auto otPhaseTop = otPitchTop + barboxY + margin;
    const auto otPhaseLeft = otGainLeft;
    tabview->addWidget(
      tabPadSynth, addGroupVerticalLabel(otPhaseLeft, otPhaseTop, barboxHeight, "Phase"));

    const auto otPhaseLeft0 = otPhaseLeft + labelY;
    tabview->addWidget(
      tabPadSynth,
      addBarBox(
        otPhaseLeft0, otPhaseTop, barboxWidth, barboxHeight, ID::overtonePhase0,
        nOvertone));

    auto textKnobControl = R"(- Knob -
Shift + Left Drag|Fine Adjustment
Ctrl + Left Click|Reset to Default)";
    tabview->addWidget(
      tabInfo,
      addTextTableView(
        tabInsideLeft0, tabInsideTop0, 400.0f, 400.0f, textKnobControl, 150.0f));

    auto textNumberControl = R"(- Number -
Shares same controls with knob, and:
Right Click|Flip Minimum and Maximum)";
    tabview->addWidget(
      tabInfo,
      addTextTableView(
        tabInsideLeft0, tabInsideTop0 + 80.0f, 400.0f, 400.0f, textNumberControl,
        150.0f));

    auto textOvertoneControl = R"(- Overtone & LFO Wave -
Ctrl + Left Click|Reset to Default
Right Drag|Draw Line
D|Reset to Default
Shift + D|Toggle Min/Mid/Max
E|Emphasize Low
Shift + E|Emphasize High
F|Low-pass Filter
Shift + F|High-pass Filter
I|Invert Value
Shift + I|Invert Value (Minimum to 0)
N|Normalize
Shift + N|Normalize (Minimum to 0)
P|Permute
R|Randomize
Shift + R|Sparse Randomize
S|Sort Decending Order
Shift + S|Sort Ascending Order
T|Subtle Randomize
, (Comma)|Rotate Back
. (Period)|Rotate Forward
1|Decrease
2-9|Decrease 2n-9n)";
    tabview->addWidget(
      tabInfo,
      addTextTableView(
        tabInsideLeft0, tabInsideTop0 + 160.0f, 400.0f, 400.0f, textOvertoneControl,
        150.0f));

    const auto tabInfoLeft1 = tabInsideLeft0 + tabWidth / 2.0f;

    auto textRefreshNotice = R"(Wavetables do not refresh automatically.
Press following button to apply changes.
- `Refresh LFO` at center-left in Main tab.
- `Refresh Table` at bottom-left in Wavetable tab.)";
    tabview->addWidget(
      tabInfo,
      addTextView(tabInfoLeft1, tabInsideTop0, 400.0f, 400.0f, textRefreshNotice));

    const auto tabInfoBottom = tabInsideTop0 + tabHeight - labelY;
    std::stringstream ssPluginName;
    ssPluginName << "LightPadSynth " << std::to_string(MAJOR_VERSION) << "."
                 << std::to_string(MINOR_VERSION) << "." << std::to_string(PATCH_VERSION);
    auto pluginNameTextView = addTextView(
      tabInfoLeft1, tabInfoBottom - 140.0f, 400.0f, 400.0f, ssPluginName.str());
    pluginNameTextView->textSize = 36.0f;
    tabview->addWidget(tabInfo, pluginNameTextView);

    tabview->addWidget(
      tabInfo,
      addTextView(
        tabInfoLeft1, tabInfoBottom - 100.0f, 400.0f, 400.0f,
        "© 2020 Takamitsu Endo (ryukau@gmail.com)\n\nHave a nice day!"));

    tabview->refreshTab();
  }
};

UI *createUI() { return new LightPadSynthUI(); }

END_NAMESPACE_DISTRHO
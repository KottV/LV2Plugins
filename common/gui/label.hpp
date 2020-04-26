// (c) 2019-2020 Takamitsu Endo
//
// This file is part of Uhhyou Plugins.
//
// Uhhyou Plugins is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Uhhyou Plugins is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Uhhyou Plugins.  If not, see <https://www.gnu.org/licenses/>.

#pragma once

#include "Widget.hpp"

#include "../dsp/constants.hpp"

#include <string>

class Label : public NanoWidget {
public:
  bool drawBorder = false;
  bool drawUnderline = false;

  explicit Label(NanoWidget *group, std::string labelText, FontId fontId)
    : NanoWidget(group), labelText(labelText), fontId(fontId)
  {
  }

  void onNanoDisplay() override
  {
    resetTransform();
    translate(getAbsoluteX(), getAbsoluteY());

    const auto width = getWidth();
    const auto height = getHeight();

    // Text.
    if (labelText.size() == 0) return;
    fontFaceId(fontId);
    fontSize(textSize);
    textAlign(align);

    float labelX = (align & ALIGN_LEFT) ? 0 : (align & ALIGN_RIGHT) ? width : width / 2;
    float labelY = height / 2;
    if (drawBorder) {
      beginPath();
      moveTo(0, height / 2);
      lineTo(width, height / 2);
      strokeColor(colorFore);
      strokeWidth(borderWidth);
      stroke();

      auto textBackMargin = 10.0f;
      Rectangle<float> textBack;
      textBounds(labelX, labelY, labelText.c_str(), NULL, textBack);
      beginPath();
      rect(
        textBack.getX() - textBackMargin, textBack.getY(),
        textBack.getWidth() + 2 * textBackMargin, textBack.getHeight());
      fillColor(colorBack);
      fill();
    }

    fillColor(colorFore);
    text(labelX, labelY, labelText.c_str(), nullptr);
  }

  void setForegroundColor(Color color) { colorFore = color; }
  void setBackgroundColor(Color color) { colorBack = color; }
  void setBorderWidth(float width) { borderWidth = width < 0.0f ? 0.0f : width; }
  void setTextAlign(int align) { this->align = align; }
  void setTextSize(float size) { textSize = size < 0.0f ? 0.0f : size; }

protected:
  Color colorFore{0, 0, 0};
  Color colorBack{0xff, 0xff, 0xff};

  std::string labelText;
  FontId fontId = -1;
  int align = ALIGN_CENTER | ALIGN_MIDDLE;
  float borderWidth = 1.0f;
  float textSize = 18.0f;
};

class VLabel : public Label {
public:
  explicit VLabel(NanoWidget *group, const char *labelText, FontId fontId)
    : Label(group, labelText, fontId)
  {
  }

  void onNanoDisplay() override
  {
    const auto width = getWidth();
    const auto height = getHeight();

    resetTransform();
    translate(getAbsoluteX(), getAbsoluteY() + width);
    rotate(-SomeDSP::pi / 2.0f);

    // Text.
    if (labelText.size() == 0) return;
    fontFaceId(fontId);
    fontSize(textSize);
    textAlign(align);

    float labelX = (align & ALIGN_LEFT) ? 0 : (align & ALIGN_RIGHT) ? width : width / 2;
    float labelY = height / 2;
    if (drawBorder) {
      beginPath();
      moveTo(0, height / 2);
      lineTo(width, height / 2);
      strokeColor(colorFore);
      strokeWidth(borderWidth);
      stroke();

      auto textBackMargin = 10.0f;
      Rectangle<float> textBack;
      textBounds(labelX, labelY, labelText.c_str(), NULL, textBack);
      beginPath();
      rect(
        textBack.getX() - textBackMargin, textBack.getY(),
        textBack.getWidth() + 2 * textBackMargin, textBack.getHeight());
      fillColor(colorBack);
      fill();
    }

    fillColor(colorFore);
    text(labelX, labelY, labelText.c_str(), nullptr);
  }
};

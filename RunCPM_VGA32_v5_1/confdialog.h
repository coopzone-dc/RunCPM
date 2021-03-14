 /*
  Created by Fabrizio Di Vittorio (fdivitto2013@gmail.com) - www.fabgl.com
  Copyright (c) 2019-2020 Fabrizio Di Vittorio.
  All rights reserved.

  This file is part of FabGL Library.

  FabGL is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  FabGL is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with FabGL.  If not, see <http://www.gnu.org/licenses/>.
 */


#pragma once

#include <Preferences.h>

#include "fabui.h"
#include "uistyle.h"

Preferences preferences;


#define TERMVERSION_MAJ 1
#define TERMVERSION_MIN 3


constexpr int       BOOTINFO_DISABLED     = 0;
constexpr int       BOOTINFO_ENABLED      = 1;
constexpr int       BOOTINFO_TEMPDISABLED = 2;





struct ConfDialogApp : public uiApp {

  Rect              frameRect;
  int               progToInstall;

  uiFrame *         frame;
  uiComboBox *      termComboBox;
  uiComboBox *      kbdComboBox;
  uiColorComboBox * bgColorComboBox;
  uiColorComboBox * fgColorComboBox;
  uiCheckBox *      infoCheckBox;

  void init() {

    setStyle(&dialogStyle);

    rootWindow()->frameProps().fillBackground = false;

    frame = new uiFrame(rootWindow(), "Terminal Configuration", UIWINDOW_PARENTCENTER, Size(380, 275), true, STYLE_FRAME);
    frameRect = frame->rect(fabgl::uiOrigin::Screen);

    frame->frameProps().resizeable        = false;
    frame->frameProps().moveable          = false;
    frame->frameProps().hasCloseButton    = false;
    frame->frameProps().hasMaximizeButton = false;
    frame->frameProps().hasMinimizeButton = false;

    // ESC : exit without save
    // F10 : save and exit
    frame->onKeyUp = [&](uiKeyEventInfo key) {
      if (key.VK == VirtualKey::VK_ESCAPE)
        quit(0);
      if (key.VK == VirtualKey::VK_F10) {
        saveProps();
        quit(0);
      }
    };

    int y = 24;

    // little help
    new uiLabel(frame, "RunCPM for VGA32 by coopzone-dc & Guido Lehwalder", Point(52, y), Size(0, 0), true, STYLE_LABELHELP);
    new uiLabel(frame, "Press TAB key to move between fields", Point(100, y +24), Size(0, 0), true, STYLE_LABELHELP);
    new uiLabel(frame, "Outside this dialog press CTRL-ALT-F12 to reset settings", Point(52, y + 36), Size(0, 0), true, STYLE_LABELHELP);


    y += 56;

    // select terminal emulation combobox
    new uiLabel(frame, "Terminal Type", Point(10,  y), Size(0, 0), true, STYLE_LABEL);
    termComboBox = new uiComboBox(frame, Point(10, y + 12), Size(85, 20), 80, true, STYLE_COMBOBOX);
    termComboBox->items().append(SupportedTerminals::names(), SupportedTerminals::count());
    termComboBox->selectItem((int)getTermType());

    // select keyboard layout
    new uiLabel(frame, "Keyboard Layout", Point(110, y), Size(0, 0), true, STYLE_LABEL);
    kbdComboBox = new uiComboBox(frame, Point(110, y + 12), Size(75, 20), 70, true, STYLE_COMBOBOX);
    kbdComboBox->items().append(SupportedLayouts::names(), SupportedLayouts::count());
    kbdComboBox->selectItem(getKbdLayoutIndex());

    // background color
    new uiLabel(frame, "Background Color", Point(200,  y), Size(0, 0), true, STYLE_LABEL);
    bgColorComboBox = new uiColorComboBox(frame, Point(200, y + 12), Size(75, 20), 70, true, STYLE_COMBOBOX);
    bgColorComboBox->selectColor(getBGColor());

    // foreground color
    new uiLabel(frame, "Foreground Color", Point(290,  y), Size(0, 0), true, STYLE_LABEL);
    fgColorComboBox = new uiColorComboBox(frame, Point(290, y + 12), Size(75, 20), 70, true, STYLE_COMBOBOX);
    fgColorComboBox->selectColor(getFGColor());

    y += 48;

    // show boot info
    new uiLabel(frame, "Show Boot Info", Point(10, y), Size(0, 0), true, STYLE_LABEL);
    infoCheckBox = new uiCheckBox(frame, Point(80, y - 2), Size(16, 16), uiCheckBoxKind::CheckBox, true, STYLE_CHECKBOX);
    infoCheckBox->setChecked(getBootInfo() == BOOTINFO_ENABLED);


    y += 48;

    // exit without save button
    auto exitNoSaveButton = new uiButton(frame, "Quit [ESC]", Point(10, y), Size(90, 20), uiButtonKind::Button, true, STYLE_BUTTON);
    exitNoSaveButton->onClick = [&]() {
      quit(0);
    };

    // exit with save button
    auto exitSaveButton = new uiButton(frame, "Save & Quit [F10]", Point(110, y), Size(90, 20), uiButtonKind::Button, true, STYLE_BUTTON);
    exitSaveButton->onClick = [&]() {
      saveProps();
      quit(0);
    };



    setActiveWindow(frame);
    setFocusedWindow(exitNoSaveButton);

  }


  void saveProps() {
    preferences.putInt("TermType", termComboBox->selectedItem());
    preferences.putInt("KbdLayout", kbdComboBox->selectedItem());
    preferences.putInt("BGColor", (int)bgColorComboBox->selectedColor());
    preferences.putInt("FGColor", (int)fgColorComboBox->selectedColor());
    preferences.putInt("BootInfo", infoCheckBox->checked() ? BOOTINFO_ENABLED : BOOTINFO_DISABLED);

    loadConfiguration();
  }


  ~ConfDialogApp() {
    // this is required, becasue the terminal may not cover the entire screen
    canvas()->reset();
    canvas()->setBrushColor(getBGColor());
    canvas()->fillRectangle(frameRect);
  }


  static TermType getTermType() {
    return (TermType) preferences.getInt("TermType", 7);    // default 7 = ANSILegacy
  }

  static int getKbdLayoutIndex() {
    return preferences.getInt("KbdLayout", 3);              // default 3 = "US"
  }

  static Color getBGColor() {
    return (Color) preferences.getInt("BGColor", (int)Color::Black);
  }

  static Color getFGColor() {
    return (Color) preferences.getInt("FGColor", (int)Color::BrightGreen);
  }

  static int getBootInfo() {
    return preferences.getInt("BootInfo", BOOTINFO_ENABLED);
  }

  // if version in preferences doesn't match, reset preferences
  static void checkVersion() {
    if (preferences.getInt("VerMaj", 0) != TERMVERSION_MAJ || preferences.getInt("VerMin", 0) != TERMVERSION_MIN) {
      preferences.clear();
      preferences.putInt("VerMaj", TERMVERSION_MAJ);
      preferences.putInt("VerMin", TERMVERSION_MIN);
    }
  }


  static void loadConfiguration() {
    Terminal.setTerminalType(getTermType());
    Terminal.keyboard()->setLayout(SupportedLayouts::layouts()[getKbdLayoutIndex()]);
    Terminal.connectLocally();      // to use Terminal.read(), available(), etc..
    Terminal.setBackgroundColor(getBGColor());
    Terminal.setForegroundColor(getFGColor());
  }

};

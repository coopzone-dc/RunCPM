#include "globals.h"

#include <SPI.h>

#include <SdFat.h>  // One SD library to rule them all - Greinman SdFat from Library Manager

// =========================================================================================
// TTGO Hardware Definition
// =========================================================================================
// Board definitions go into the "hardware" folder
// Choose/change a file from there

#include "hardware/esp32/ttgo_vga32_esp32.h"

// =========================================================================================

// Delays for LED blinking
#define sDELAY 50
#define DELAY 100

// =========================================================================================
// FabGL Init
// =========================================================================================
// Define this to use the VGA screen/keyboard as a terminal
#define FABGL true

#ifdef FABGL
#include "fabgl.h"
fabgl::VGA16Controller DisplayController;
fabgl::PS2Controller     PS2Controller;
fabgl::Terminal          Terminal;
#include "confdialog.h"
#endif

// =========================================================================================

#include "abstraction_arduino.h"

// Serial port speed
#define SERIALSPD 115200

// PUN: device configuration
#ifdef USE_PUN
File pun_dev;
int pun_open = FALSE;
#endif

// LST: device configuration
#ifdef USE_LST
File lst_dev;
int lst_open = FALSE;
#endif

// =========================================================================================
// RunCPM includes
// =========================================================================================

#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "host.h"
#include "cpm.h"
#ifdef CCP_INTERNAL
#include "ccp.h"
#endif

// =========================================================================================
// void setup start
// =========================================================================================

void setup(void) {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

// =========================================================================================
// FabGL Displaycontroller IF part
// =========================================================================================

#ifdef FABGL
  preferences.begin("RunCPM", false);
  PS2Controller.begin(PS2Preset::KeyboardPort0);

// Options for non-default pins:
// Use GPIO 22-21 for red, GPIO 19-18 for green, GPIO 5-4 for blue, GPIO 23 for HSync and GPIO 15 for VSync
// DisplayController.begin(GPIO_NUM_22,GPIO_NUM_21,GPIO_NUM_19,GPIO_NUM_18,GPIO_NUM_5,GPIO_NUM_17,GPIO_NUM_23,GPIO_NUM_16);

  DisplayController.begin(); //default
  DisplayController.setResolution(VGA_640x480_60Hz);

  Terminal.begin(&DisplayController);
  // Terminal.connectLocally();      // to use Terminal.read(), available(), etc..

  ConfDialogApp::loadConfiguration();
  
  Terminal.enableCursor(true);

  if (ConfDialogApp::getBootInfo() == BOOTINFO_ENABLED) {
    Terminal.clear();
    //Terminal.write("Boot-Info\r\n");
    Terminal.write("____________________________________________\r\n");
    Terminal.printf("Screen Size        :  %d x %d\r\n", DisplayController.getScreenWidth(), DisplayController.getScreenHeight());
    Terminal.printf("Terminal Size      :   %d x %d\r\n", Terminal.getColumns(), Terminal.getRows());
    Terminal.printf("Keyboard Layout    :   %s\r\n", PS2Controller.keyboard()->isKeyboardAvailable() ? SupportedLayouts::names()[ConfDialogApp::getKbdLayoutIndex()] : "No Keyboard");
    Terminal.printf("Terminal Type      :   %s\r\n", SupportedTerminals::names()[(int)ConfDialogApp::getTermType()]);
    Terminal.printf("Free Memory        :   %d bytes\r\n", heap_caps_get_free_size(MALLOC_CAP_32BIT));
    Terminal.printf("Version            :   %d.%d\r\n", TERMVERSION_MAJ, TERMVERSION_MIN);
    Terminal.write("____________________________________________\r\n");
    Terminal.write("Press BREAK  [Ctrl+PAUSE]    to reboot\r\n");
    Terminal.write("Press F12 to change terminal configuration\r\n");
//    Terminal.write("    and CTRL-ALT-F12 to reset settings\r\n");
  } else if (ConfDialogApp::getBootInfo() == BOOTINFO_TEMPDISABLED) {
    preferences.putInt("BootInfo", BOOTINFO_ENABLED);
  }  

  Terminal.onVirtualKey = [&](VirtualKey * vk, bool keyDown) {
    if (*vk == VirtualKey::VK_F12) {
      if (!keyDown) {
        // releasing F12 key to open configuration dialog
        Terminal.deactivate();
        auto dlgApp = new ConfDialogApp;
        dlgApp->run(&DisplayController);
        delete dlgApp;
        Terminal.keyboard()->emptyVirtualKeyQueue();
        Terminal.activate();
      } else {
        // pressing CTRL + ALT + F12, reset parameters and reboot
        if ((Terminal.keyboard()->isVKDown(VirtualKey::VK_LCTRL) || Terminal.keyboard()->isVKDown(VirtualKey::VK_RCTRL)) &&
            (Terminal.keyboard()->isVKDown(VirtualKey::VK_LALT) || Terminal.keyboard()->isVKDown(VirtualKey::VK_RALT))) {
          Terminal.write("\r\nReset of terminal settings...");
          preferences.clear();
          delay(2000);
          Terminal.write("\r\nRebooting...");
          delay(2000);
          ESP.restart();
        }
      }
      *vk = VirtualKey::VK_NONE;
    }

    if (*vk == VirtualKey::VK_BREAK) {
      if (!keyDown) {
        // releasing BREAK key to reboot
        Terminal.write("\r\n");
        Terminal.write("\r\n______________________________");
        Terminal.write("\r\nReboot requested via BREAK-Key");
        Terminal.write("\r\nRebooting in 2 seconds...");
        Terminal.write("\r\n______________________________");
        delay(2000);
        ESP.restart();
      } 
      *vk = VirtualKey::VK_NONE;
    }

  };
  
// =========================================================================================

#else
  Serial.begin(SERIALSPD);
  while (!Serial) {  // Wait until serial is connected
    digitalWrite(LED, HIGH^LEDinv);
    delay(sDELAY);
    digitalWrite(LED, LOW^LEDinv);
    delay(sDELAY);
  }
#endif
// =========================================================================================

#ifdef DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

// _clrscr();
_puts("____________________________________________\r\n");
_puts("  CP/M 2.2 Emulator v" VERSION " by Marcelo Dantas \r\n");
_puts("  using FabGL Terminal   by @fdivitto                     \r\n");
// _puts("Arduino read/write support by Krzysztof Klis\r\n");
// _puts("      Built " __DATE__ " - " __TIME__ "\r\n");
_puts("____________________________________________\r\n");
_puts("CCP   : " CCPname "   CCP Address [0x");
_puthex16(CCPaddr);
_puts("]");
_puts("\r\nBOARD : ");
_puts(BOARD);
_puts("\r\n");

#if defined board_esp32
  _puts("INIT  : SPI-Bus [14,2,12,13]  ");
  SPI.begin(SPIINIT);
  _puts("  ->  [ Done ]\r\n");
  if (SD.begin(SDINIT)) {
  _puts("        MicroSD Card at ");
  _puts(SDMHZ_TXT);
  _puts("Mhz ");
  _puts("  ->  [ Done ]\r\n");
#endif

_puts("____________________________________________\r\n");
      
    if (VersionCCP >= 0x10 || SD.exists(CCPname)) {
      while (true) {
        _puts(CCPHEAD);
        _PatchCPM();
  Status = 0;
#ifndef CCP_INTERNAL
        if (!_RamLoad((char *)CCPname, CCPaddr)) {
          _puts("Unable to load the CCP.\r\nCPU halted.\r\n");
          break;
        }
        Z80reset();
        SET_LOW_REGISTER(BC, _RamRead(DSKByte));
        PC = CCPaddr;
        Z80run();
#else
        _ccp();
#endif
        if (Status == 1)
          break;
#ifdef USE_PUN
        if (pun_dev)
          _sys_fflush(pun_dev);
#endif
#ifdef USE_LST
        if (lst_dev)
          _sys_fflush(lst_dev);
#endif
      }
    } else {
      _puts("Unable to load CP/M CCP.\r\nCPU halted.\r\n");
    }
  } else {
    _puts("Unable to initialize SD card.\r\nCPU halted.\r\n");
  }
}


// =========================================================================================
// void loop
// =========================================================================================
void loop(void) {
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY);
  digitalWrite(LED, HIGH^LEDinv);
  delay(DELAY);
  digitalWrite(LED, LOW^LEDinv);
  delay(DELAY * 4);
}

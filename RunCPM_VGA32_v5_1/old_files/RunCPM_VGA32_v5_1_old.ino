#include "globals.h"

#include <SPI.h>

#ifdef ARDUINO_TEENSY41
  #include <SdFat-beta.h>
#else
  #include <SdFat.h>  // One SD library to rule them all - Greinman SdFat from Library Manager
#endif

// Board definitions go into the "hardware" folder
// Choose/change a file from there
#include "hardware/esp32/ttgo_vga32_esp32.h"
#define SS 13. //Bug fix, you have to define this because later it is used by SD.init. This was missed out on the master branch.
#define LED 5
#define SDINIT 14,2,12,SS// VGA32 MOSI 12 may not reload properly with sd card in slot

// Delays for LED blinking
#define sDELAY 50
#define DELAY 100

//Define this to use the VGA screen/keyboard as a terminal
#define FABGL true

#ifdef FABGL
#include "fabgl.h"
fabgl::VGA16Controller DisplayController;
fabgl::PS2Controller     PS2Controller;
fabgl::Terminal          Terminal;
#include "confdialog.h"
#endif

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

#include "ram.h"
#include "console.h"
#include "cpu.h"
#include "disk.h"
#include "host.h"
#include "cpm.h"
#ifdef CCP_INTERNAL
#include "ccp.h"
#endif

void setup(void) {
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

#ifdef FABGL
  preferences.begin("RunCPM", false);
  PS2Controller.begin(PS2Preset::KeyboardPort0);

// Options for non-defaukt pins, Use GPIO 22-21 for red, GPIO 19-18 for green, GPIO 5-4 for blue, GPIO 23 for HSync and GPIO 15 for VSync
//  DisplayController.begin(GPIO_NUM_22,GPIO_NUM_21,GPIO_NUM_19,GPIO_NUM_18,GPIO_NUM_5,GPIO_NUM_17,GPIO_NUM_23,GPIO_NUM_16);
  DisplayController.begin(); //default
  DisplayController.setResolution(VGA_640x480_60Hz);

  Terminal.begin(&DisplayController);
  Terminal.connectLocally();      // to use Terminal.read(), available(), etc..

  ConfDialogApp::loadConfiguration();

  Terminal.enableCursor(true);

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
  };
#else
  Serial.begin(SERIALSPD);
  while (!Serial) {  // Wait until serial is connected
    digitalWrite(LED, HIGH^LEDinv);
    delay(sDELAY);
    digitalWrite(LED, LOW^LEDinv);
    delay(sDELAY);
  }
#endif  

#ifdef DEBUGLOG
  _sys_deletefile((uint8 *)LogName);
#endif

  _clrscr();
  _puts("CP/M 2.2 Emulator v" VERSION " by Marcelo Dantas\r\n");
  _puts("Arduino read/write support by Krzysztof Klis\r\n");
  _puts("      Build " __DATE__ " - " __TIME__ "\r\n");
  _puts("--------------------------------------------\r\n");
  _puts("CCP  : " CCPname "    CCP Address: 0x");
  _puthex16(CCPaddr);
  _puts("\r\nBOARD: ");
  _puts(BOARD);
  _puts("\r\n");

#if defined board_agcm4
  _puts("Initializing Grand Central SD card.\r\n");
  if (SD.cardBegin(SDINIT, SD_SCK_MHZ(50))) {

    if (!SD.fsBegin()) {
      _puts("\nFile System initialization failed.\n");
      return;
    }
#elif defined board_teensy40 
  _puts("Initializing Teensy 4.0 SD card.\r\n");
  if (SD.begin(SDINIT, SD_SCK_MHZ(25))) {
#elif defined board_esp32
  _puts("Initializing TTGO VGA32 SD card.\r\n");
  SPI.begin(SDINIT);
  if (SD.begin(SS, SD_SCK_MHZ(SDMHZ))) {
#else
  _puts("Initializing SD card.\r\n");
  _puts("--------------------------------------------\r\n");
  if (SD.begin(SDINIT)) {
#endif
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
        SET_LOW_REGISTER(BC, _RamRead(0x0004));
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

#ifndef ESP32_H
#define ESP32_H

SdFat SD;
//#define SDINIT 14,2,12,13 // TTGO_T1,LOLIN32_Pro
//#define SDINIT 14,16,17,13 // TTGO_T1,LOLIN32_Pro
//#define SDINIT 18=clk,19=miso,23=mosi,5=cs // ePaper,ESP32_DevKit
#define SDMHZ 25 // TTGO_T1,LOLIN32_Pro=25 ePaper,ESP32_DevKit=20
#define LED 34 // TTGO_T1=22 LOLIN32_Pro=5(inverted) DOIT_Esp32=2 ESP32-PICO-KIT=no led
#define LEDinv 1
#define BOARD "VGA32 (clone)"
#define board_esp32
#define board_digital_io

uint8 esp32bdos(uint16 dmaaddr) {
	return(0x00);
}

#endif

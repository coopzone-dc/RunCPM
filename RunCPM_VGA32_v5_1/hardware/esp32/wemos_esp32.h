#ifndef ESP32_H
#define ESP32_H

// SPI_DRIVER_SELECT must be set to 0 in SdFat/SdFatConfig.h

SdFat SD;
#define SPIINIT 18,19,23,5
#define SDINIT SS, SD_SCK_MHZ(SDMHZ)
#define SDMHZ 19
#define LED 2
#define LEDinv 1
#define BOARD "WeMOS D1 R32 ESP-WROOM-32"
#define board_esp32
#define board_digital_io

uint8 esp32bdos(uint16 dmaaddr) {
	return(0x00);
}

#endif
#include "stm32f7xx_hal.h"

#define FLASH_IP_SIZE 		4
#define FLASH_MAC_SIZE 		6
#define FLASH_IP_ADDRESS 	0x08010000	//FLASH_SECTOR_2

#define FLASH_MX_SIZE 		24
#define FLASH_MX_ADDRESS 	0x08018000	//FLASH_SECTOR_3

uint8_t FLASH_IPADDR[FLASH_IP_SIZE];
uint8_t FLASH_MACADDR[FLASH_MAC_SIZE];
uint16_t FLASH_SETTINGS [FLASH_MX_SIZE];

void MX_FLASH_DefaultIP(void) {
	FLASH_IPADDR[0] = 192;
	FLASH_IPADDR[1] = 168;
	FLASH_IPADDR[2] = 31;
	FLASH_IPADDR[3] = 222;
	
	FLASH_MACADDR[0] = 0xAA;
	FLASH_MACADDR[1] = 0x07;
	FLASH_MACADDR[2] = 0x05;
	FLASH_MACADDR[3] = 0x19;
	FLASH_MACADDR[2] = 0x85;
	FLASH_MACADDR[3] = 0xAA;
}

void MX_FLASH_SaveIP(void) {
	HAL_FLASH_Unlock();
	FLASH_Erase_Sector(FLASH_SECTOR_2, VOLTAGE_RANGE_3);
	for (uint8_t i=0; i<FLASH_IP_SIZE; i++) HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, FLASH_IP_ADDRESS+i, FLASH_IPADDR[i]);
	HAL_FLASH_Lock();
}

void MX_FLASH_LoadIP(void) {
	MX_FLASH_DefaultIP();
	if (*(uint32_t*)(FLASH_IP_ADDRESS) == 0xFFFFFFFF) MX_FLASH_SaveIP();
	else for (uint8_t i=0; i<FLASH_IP_SIZE; i++) FLASH_IPADDR[i] = *(uint8_t*)(FLASH_IP_ADDRESS+i);
}

void MX_FLASH_DefaultSettings(void) {
	FLASH_SETTINGS[0] = 200;			//	Measuring Frequency
	FLASH_SETTINGS[1] = 7000; 		//	Nakal Maximum Value
	FLASH_SETTINGS[2] = 5000; 		//	Nakal Manual Value
	FLASH_SETTINGS[3] = 10000; 		//	Anode Coefficient
	
	FLASH_SETTINGS[4] = 30000; 		//	Coefficient U+
	FLASH_SETTINGS[5] = 30000; 		//	Coefficient U-
	FLASH_SETTINGS[6] = 104; 			//	Coefficient Ia
	FLASH_SETTINGS[7] = 256; 			//	Coefficient Iz
	
	FLASH_SETTINGS[8] = 150; 			//	Minimum Voltage
	FLASH_SETTINGS[9] = 350; 			//	Maximum Voltage
	FLASH_SETTINGS[10] = 60; 			//	Charging Time
	FLASH_SETTINGS[11] = 30; 			//	Discharge Time
	
	FLASH_SETTINGS[12] = 50; 			//	Warning Temperature
	FLASH_SETTINGS[13] = 75; 			//	Error Temperature
	FLASH_SETTINGS[14] = 6000; 		//	PWM Pulse
	FLASH_SETTINGS[15] = 12000; 	//	PWM Period
	
	FLASH_SETTINGS[16] = 50; 			//	KV Minimum
	FLASH_SETTINGS[17] = 100; 		//	KV Maximum
	FLASH_SETTINGS[18] = 50; 			//	MA Minimum
	FLASH_SETTINGS[19] = 100; 		//	MA Maximum
	FLASH_SETTINGS[20] = 50; 			//	MS Minimum
	FLASH_SETTINGS[21] = 100; 		//	MS Maximum
	
	FLASH_SETTINGS[22] = 30; 			//	Pause
	FLASH_SETTINGS[23] = 5; 			//	Prepare
}

void MX_FLASH_SaveSettings(void) {
	HAL_FLASH_Unlock(); 
	FLASH_Erase_Sector(FLASH_SECTOR_3, VOLTAGE_RANGE_3);
	for (uint8_t i=0; i<FLASH_MX_SIZE; i++) HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, FLASH_MX_ADDRESS+i*2, FLASH_SETTINGS[i]);
	HAL_FLASH_Lock();
}


void MX_FLASH_LoadSettings(void) {
	MX_FLASH_DefaultSettings();
	if (*(uint32_t*)(FLASH_MX_ADDRESS) == 0xFFFFFFFF) MX_FLASH_SaveSettings();
	else for (uint8_t i=0; i<FLASH_MX_SIZE; i++) FLASH_SETTINGS[i] = *(uint16_t*)(FLASH_MX_ADDRESS+i*2);
}

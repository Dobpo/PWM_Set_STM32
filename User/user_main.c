#include "user_main.h"

/* ------------------------------------------------------ */

extern DAC_HandleTypeDef hdac;

extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim8;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

extern ADC_HandleTypeDef hadc1;
extern ADC_HandleTypeDef hadc2;
extern ADC_HandleTypeDef hadc3;
extern DMA_HandleTypeDef hdma_adc1;
extern DMA_HandleTypeDef hdma_adc2;
extern DMA_HandleTypeDef hdma_adc3;

/* ------------------------------------------------------ */

extern void MX_FLASH_DefaultIP(void);
extern void MX_FLASH_SaveIP(void);
extern void MX_FLASH_LoadIP(void);
extern void MX_FLASH_DefaultSettings(void);
extern void MX_FLASH_SaveSettings(void);
extern void MX_FLASH_LoadSettings(void);

extern void MX_ENC28J60_Init(void);
extern void tcp_server_reply(uint8_t *buf, uint16_t dlen);
extern uint16_t tcp_fill_data (uint8_t *buf, const uint8_t *s, uint16_t len);
extern uint16_t enc28j60PacketReceive (uint16_t maxlen, uint8_t* packet);
extern uint16_t packetloop_icmp_tcp(uint8_t *buf, uint16_t plen);

/* ------------------------------------------------------ */

void USER_MAIN_StartupInit(void);
void USER_MAIN_StartupError(uint8_t errCode);
void USER_MAIN_CalculateValues(void);

void USER_MAIN_ENC26J60_CheckPackets(void);
void USER_MAIN_ENC26J60_SendReply(uint16_t len);
uint8_t USER_MAIN_ENC26J60_Checksum(uint8_t *array, uint16_t len, uint8_t start);

void USER_MAIN_ENC26J60_COM_Error(void);
void USER_MAIN_ENC26J60_COM_Unknown(void);
void USER_MAIN_ENC26J60_COM_ChangeIP(void);
void USER_MAIN_ENC26J60_COM_Power(void);
void USER_MAIN_ENC26J60_COM_Triac(void);
void USER_MAIN_ENC26J60_COM_Check(void);
void USER_MAIN_ENC26J60_COM_Load(void);
void USER_MAIN_ENC26J60_COM_Save(void);
void USER_MAIN_ENC26J60_COM_Reset(void);
void USER_MAIN_ENC26J60_COM_Prepare(void);
void USER_MAIN_ENC26J60_COM_Xray(void);
void USER_MAIN_ENC26J60_COM_Cancel(void);

/* ------------------------------------------------------ */

#define FLASH_IP_SIZE 			4
extern uint8_t FLASH_IPADDR[FLASH_IP_SIZE];

#define FLASH_MX_SIZE 			24
extern uint16_t FLASH_SETTINGS [FLASH_MX_SIZE];

#define TCP_DATA_START 			54
#define TCP_BUFFER_SIZE 		1500
uint8_t TCP_READ_BUFFER [TCP_BUFFER_SIZE];
uint8_t TCP_WRITE_BUFFER [TCP_BUFFER_SIZE];

#define ADC1_BUFFER_SIZE 		7
uint16_t ADC1_TMP_VALUES [ADC1_BUFFER_SIZE];
uint16_t ADC1_DMA_VALUES [ADC1_BUFFER_SIZE];
uint16_t ADC1_DMA_BUFFER [ADC1_BUFFER_SIZE*ADC1_BUFFER_SIZE];

#define ADC2_BUFFER_SIZE		14000
#define ADC3_BUFFER_SIZE 		56000
uint16_t ADC2_DMA_BUFFER [ADC2_BUFFER_SIZE];
uint16_t ADC3_DMA_BUFFER [ADC3_BUFFER_SIZE];

uint32_t MODE_COUNT, MODE_PULSE;
uint16_t MODE_PERIOD, MODE_NAKAL, MODE_KV;
/* ------------------------------------------------------ */

uint32_t FLAG_STATUS, FLAG_ERRORS;
uint8_t LED_STEP, LED_COUNT, DATA_COUNT;// It was some idea to add //PREPARE_COUNT;

/* ------------------------------------------------------ */

void USER_MAIN_StartupInit(void) {	
	SIGNAL_SD_RESET; osDelay(33); if (SIGNAL_SD_READ) USER_MAIN_StartupError(1);
	SIGNAL_SD_SET; osDelay(33); if (!SIGNAL_SD_READ) USER_MAIN_StartupError(2);
		
	LEDS_ON_1; osDelay(250);

  TIM_CCxChannelCmd(htim11.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE); __HAL_TIM_MOE_ENABLE(&htim11);
	TIM11->ARR = 0xFFFF; TIM11->CCR1 = 0xFFFF; __HAL_TIM_ENABLE(&htim11); osDelay(33);
	if (!SIGNAL_PWM_READ) USER_MAIN_StartupError(3); __HAL_TIM_DISABLE(&htim11);
	TIM11->ARR = 0xFFFF; TIM11->CCR1 = 0x0000; __HAL_TIM_ENABLE(&htim11); osDelay(33);
	if (SIGNAL_PWM_READ) USER_MAIN_StartupError(4); __HAL_TIM_DISABLE(&htim11);
	
	LEDS_ON_2; osDelay(250);
	
	HAL_DAC_SetValue(&hdac,DAC_CHANNEL_1,DAC_ALIGN_12B_R,0); HAL_DAC_Start(&hdac,DAC_CHANNEL_1); osDelay(33);
	HAL_DAC_SetValue(&hdac,DAC_CHANNEL_2,DAC_ALIGN_12B_R,0); HAL_DAC_Start(&hdac,DAC_CHANNEL_2); osDelay(33);
	
	LEDS_ON_3; osDelay(250);
		
	HAL_ADC_Start_DMA(&hadc1, (uint32_t*)ADC1_DMA_BUFFER, ADC1_BUFFER_SIZE); 	
	__HAL_DMA_DISABLE_IT(&hdma_adc1,DMA_IT_HT); __HAL_DMA_DISABLE_IT(&hdma_adc1,DMA_IT_TC);	
	__HAL_DMA_DISABLE_IT(&hdma_adc1,DMA_IT_TE); __HAL_DMA_DISABLE_IT(&hdma_adc1,DMA_IT_DME); 
	__HAL_DMA_DISABLE_IT(&hdma_adc1,DMA_IT_FE); osDelay(33); 
	HAL_ADC_Start_DMA(&hadc2, (uint32_t*)ADC2_DMA_BUFFER, ADC2_BUFFER_SIZE); 
	__HAL_DMA_DISABLE_IT(&hdma_adc2,DMA_IT_HT); __HAL_DMA_DISABLE_IT(&hdma_adc2,DMA_IT_TC);	
	__HAL_DMA_DISABLE_IT(&hdma_adc2,DMA_IT_TE); __HAL_DMA_DISABLE_IT(&hdma_adc2,DMA_IT_DME); 
	__HAL_DMA_DISABLE_IT(&hdma_adc2,DMA_IT_FE); osDelay(33); 
	HAL_ADC_Start_DMA(&hadc3, (uint32_t*)ADC3_DMA_BUFFER, ADC3_BUFFER_SIZE); 	
	__HAL_DMA_DISABLE_IT(&hdma_adc3,DMA_IT_HT); __HAL_DMA_DISABLE_IT(&hdma_adc3,DMA_IT_TC);	
	__HAL_DMA_DISABLE_IT(&hdma_adc3,DMA_IT_TE); __HAL_DMA_DISABLE_IT(&hdma_adc3,DMA_IT_DME); 
	__HAL_DMA_DISABLE_IT(&hdma_adc3,DMA_IT_FE); osDelay(33);
		
	MX_FLASH_LoadSettings(); MX_FLASH_LoadIP(); MX_ENC28J60_Init(); osDelay(250);
	LEDS_OFF_ALL; POWER_OFF; DATA_COUNT = 0x00;
}

void USER_MAIN_StartupError(uint8_t errCode) {
	SIGNAL_SD_RESET; __HAL_TIM_DISABLE(&htim11);
	while (1) { LEDS_TOGGLE_ALL; osDelay(250); }
}

void USER_MAIN_CalculateValues(void) {	
	DATA_COUNT++;
	//if(PREPARE_COUNT != 0) PREPARE_COUNT++; //Forgot for what!
	//if(PREPARE_COUNT == 20) Cancel prc = 0; //Forgot for what!
	if (DATA_COUNT == 50) { DATA_COUNT = 0x00; POWER_OFF; }
	for (uint8_t idx=0; idx<ADC1_BUFFER_SIZE; idx++) {
		ADC1_TMP_VALUES[idx] = 0;
		for (uint8_t cnt=0; cnt<ADC1_BUFFER_SIZE; cnt++) ADC1_TMP_VALUES[idx] += ADC1_DMA_BUFFER[ADC1_BUFFER_SIZE*cnt+idx];
		ADC1_DMA_VALUES[idx] = ADC1_TMP_VALUES[idx] / ADC1_BUFFER_SIZE;
	} 
}

/* ------------------------------------------------------ */

void USER_MAIN_ENC26J60_CheckPackets(void) {
	
	LED_COUNT++;
	if (LED_COUNT == 0xFF) {
		LED_STEP++; LEDS_OFF_ALL;
		switch (LED_STEP){
			case 1: LEDS_ON_1; break;
			case 2: LEDS_ON_2; break;
			case 3: LEDS_ON_3; LED_STEP = 0; break;
			default: break;
		} LED_COUNT = 0;
	}
	
	uint16_t PACKLEN;
	PACKLEN = enc28j60PacketReceive(TCP_BUFFER_SIZE, TCP_READ_BUFFER);
	if (PACKLEN == 0) return;
	PACKLEN = packetloop_icmp_tcp(TCP_READ_BUFFER, PACKLEN);
	if (PACKLEN == 0) return; DATA_COUNT = 0x00;
	if (TCP_READ_BUFFER[TCP_DATA_START+1] == USER_MAIN_ENC26J60_Checksum(TCP_READ_BUFFER, PACKLEN, TCP_DATA_START)){
		TCP_WRITE_BUFFER[0] = TCP_READ_BUFFER[TCP_DATA_START];
		switch (TCP_READ_BUFFER[TCP_DATA_START]){
			case 0x0F: USER_MAIN_ENC26J60_COM_ChangeIP(); break;
			case 0x1F: USER_MAIN_ENC26J60_COM_Check(); break;
			case 0x2F: USER_MAIN_ENC26J60_COM_Load(); break;
			case 0x3F: USER_MAIN_ENC26J60_COM_Save(); break;
			case 0x4F: USER_MAIN_ENC26J60_COM_Reset(); break;
			case 0x5F: USER_MAIN_ENC26J60_COM_Power(); break;
			case 0x6F: USER_MAIN_ENC26J60_COM_Triac(); break;
			case 0x7F: USER_MAIN_ENC26J60_COM_Prepare(); break;
			case 0x8F: USER_MAIN_ENC26J60_COM_Xray(); break;
			case 0x9F: USER_MAIN_ENC26J60_COM_Cancel(); break;
			default: USER_MAIN_ENC26J60_COM_Unknown(); break;
		}
	} else USER_MAIN_ENC26J60_COM_Error();
}

uint8_t USER_MAIN_ENC26J60_Checksum(uint8_t *array, uint16_t len, uint8_t start) {
	uint8_t tmpValue = array[start];
	for (uint16_t idx=start+2; idx<start+len; idx++) tmpValue = tmpValue ^ array[idx];
	tmpValue += 0x80; return tmpValue;
}

void USER_MAIN_ENC26J60_SendReply(uint16_t len) {
	TCP_WRITE_BUFFER[1] = USER_MAIN_ENC26J60_Checksum(TCP_WRITE_BUFFER, len, 0); 
	tcp_server_reply(TCP_READ_BUFFER, tcp_fill_data(TCP_READ_BUFFER, TCP_WRITE_BUFFER, len));
}


/* ------------------------------------------------------ */

void USER_MAIN_ENC26J60_COM_ChangeIP(void) {
	TCP_WRITE_BUFFER[2] = FLASH_IPADDR[0]; TCP_WRITE_BUFFER[3] = FLASH_IPADDR[1];
	TCP_WRITE_BUFFER[4] = FLASH_IPADDR[2]; TCP_WRITE_BUFFER[5] = FLASH_IPADDR[3];
	USER_MAIN_ENC26J60_SendReply(6);
}

void USER_MAIN_ENC26J60_COM_Check(void) {
	if (TCP_READ_BUFFER[TCP_DATA_START+2] == 0x01) { POWER_OFF; }
	for (uint8_t i=0;i<4;i++) {
		TCP_WRITE_BUFFER[5-i]	= (FLAG_ERRORS >> i*8) & 0xFF;
		TCP_WRITE_BUFFER[9-i] = (FLAG_STATUS >> i*8) & 0xFF;
	}	
	for (uint8_t i=0;i<ADC1_BUFFER_SIZE;i++) {
		TCP_WRITE_BUFFER[i*2+10] = (ADC1_DMA_VALUES[i] >> 8) & 0xFF; 
		TCP_WRITE_BUFFER[i*2+11] = (ADC1_DMA_VALUES[i] >> 0) & 0xFF;
	}	
	USER_MAIN_ENC26J60_SendReply(24);
}

void USER_MAIN_ENC26J60_COM_Load(void) {
	for (uint8_t i=0;i<FLASH_MX_SIZE;i++) {	
		TCP_WRITE_BUFFER[i*2+2] = (FLASH_SETTINGS[i] >> 8) & 0xFF;	
		TCP_WRITE_BUFFER[i*2+3] = (FLASH_SETTINGS[i] >> 0) & 0xFF;
	}	
	USER_MAIN_ENC26J60_SendReply(50);
}
	
void USER_MAIN_ENC26J60_COM_Save(void) {
	for (uint8_t i=0;i<FLASH_MX_SIZE;i++) {
		FLASH_SETTINGS[i] = (TCP_READ_BUFFER[TCP_DATA_START+i*2+2] << 8) + TCP_READ_BUFFER[TCP_DATA_START+i*2+3];
	}	MX_FLASH_SaveSettings();
	USER_MAIN_ENC26J60_COM_Load();
}

void USER_MAIN_ENC26J60_COM_Reset(void) {
	MX_FLASH_DefaultSettings();	MX_FLASH_SaveSettings();
	USER_MAIN_ENC26J60_COM_Load();
}

void USER_MAIN_ENC26J60_COM_Power(void) {
	TCP_WRITE_BUFFER[2] = TCP_READ_BUFFER[TCP_DATA_START+2]; 
	if (TCP_WRITE_BUFFER[2] == 0x01) { POWER_ON; } else { POWER_OFF; }
	USER_MAIN_ENC26J60_SendReply(3);
}
	
void USER_MAIN_ENC26J60_COM_Triac(void) {
	TCP_WRITE_BUFFER[2] = TCP_READ_BUFFER[TCP_DATA_START+2]; 
	if (TCP_WRITE_BUFFER[2] == 0x01) { TRIAC_ON; } else { TRIAC_OFF; }	
	USER_MAIN_ENC26J60_SendReply(3);
}
	
void USER_MAIN_ENC26J60_COM_Prepare(void) {
	TCP_WRITE_BUFFER[2] = TCP_READ_BUFFER[TCP_DATA_START+2];
	/*PREPARE_COUNT = 1;*/
	MODE_PULSE = FLASH_SETTINGS[14] * 216 / 1000;
	MODE_PERIOD = FLASH_SETTINGS[15] * 216 / 1000;
	
	MODE_NAKAL = (TCP_READ_BUFFER[TCP_DATA_START+2] << 8) + TCP_READ_BUFFER[TCP_DATA_START+3];
	MODE_KV = (TCP_READ_BUFFER[TCP_DATA_START+4] << 8) + TCP_READ_BUFFER[TCP_DATA_START+5];
	MODE_COUNT = ((TCP_READ_BUFFER[TCP_DATA_START+6] << 8) + TCP_READ_BUFFER[TCP_DATA_START+7]) * 1000000 / FLASH_SETTINGS[16];
	//PB8->Tim10_CH1 - Config & Swich ON PWM (Anode glow)
	//Tim10 must be config by CubeMX in tim.c or here by data from PC
	HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
	
	//PB13->Pin Set -  Anode rotatuin ON
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);
	//PA4->DAC_OUT1 - Anode U
	//PA5->DAC_OUT2 - Anode I
	
	
	//PB9->TIm11_CH1 - Main PWN Config
	USER_MAIN_ENC26J60_SendReply(3);
}
	
void USER_MAIN_ENC26J60_COM_Xray(void){
	TCP_WRITE_BUFFER[2] = TCP_READ_BUFFER[TCP_DATA_START+2];  
	/*count = 0*/
	//PB9->TIm11_CH1 - Main PWN ON
	//PC0_3->ADC3_In10_13	
	USER_MAIN_ENC26J60_SendReply(3);
}
	
void USER_MAIN_ENC26J60_COM_Cancel(void){
	TCP_WRITE_BUFFER[2] = TCP_READ_BUFFER[TCP_DATA_START+2];
  //PB8->Tim10_CH1 - Swich OFF PWM
	HAL_TIM_PWM_Stop(&htim10, TIM_CHANNEL_1);
	//PB13->Pin Reset -  Anode rotatuin OFF
	HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_RESET);
	USER_MAIN_ENC26J60_SendReply(3);
}

void USER_MAIN_ENC26J60_COM_Unknown(void){
}

void USER_MAIN_ENC26J60_COM_Error(void){
}		
/* ------------------------------------------------------ */

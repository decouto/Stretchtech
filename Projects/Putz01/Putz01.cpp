// Putz01.cpp -- application module definition
//
#include "Putz01.h"

extern "C" {
#include "Board_Buttons.h"
#include "Board_LED.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_sai.h"
#include "stm32f4xx_hal_i2s.h"
#include <string.h>
//#include "cmsis_os.h"
}

void Putz01Assert(uint8_t* file, uint32_t line) {
	printf("FAILED ASSERT in: file %s at line %d\r\n", file, line);
	while(1);
}

void DumpHWds (uint16_t *p, uint16_t len, char *note) {
	if (note) printf (note);
	for (uint16_t i = 0; i != len; i++) {
		if (i && !(i & 15)) puts ("\r");
		printf ("%04x ", p[i]); 
	}
	if (len) puts ("\r");
}

#if 0

//volatile int32_t ITM_RxBuffer;

int stdin_getchar(void) {
	return ITM_ReceiveChar();
}

void stdout_putchar(int c) {
	ITM_SendChar(c);
}

#else

int stdin_getchar(void) {
	uint8_t data[2] = {0};
	HAL_StatusTypeDef st = HAL_UART_Receive(&huart3, data, 1, HAL_MAX_DELAY);
	return data[0];
}

void stdout_putchar(int c) {
	HAL_StatusTypeDef st = HAL_UART_Transmit(&huart3, (uint8_t*)&c, 1, HAL_MAX_DELAY);
	while (st != HAL_OK);
}

#endif


bool Putz01Init (void) {
	puts ("\r\n\nPutz01Init() Enter\r");
	Buttons_Initialize();
	LED_Initialize();
	puts ("Putz01Init() Leave\r");
	return true;
}

bool Putz01Run(void) {
	puts ("Putz01Run() Enter\r");
	memset(&I2S2BufCtl,0,sizeof(I2S2BufCtl)); 
	memset(I2S2Buf,0,sizeof(I2S2Buf)); 
	memset(&I2S3BufCtl,0,sizeof(I2S3BufCtl)); 
	memset(I2S2Buf,0,sizeof(I2S3Buf)); 
	Putz01I2S2Start();
	while (1) {
//		HAL_Delay(1000);
//		for (int i = 0; i < (1<<20); i++); 
		Putz01HandleButtons();
		Putz01I2S2Proc();
		Putz01I2S3Proc();
	};
//	return true;
}

bool Putz01HandleButtons (void) {
	int i = 1;
	while (Buttons_GetState()) {
		if (i) puts ("Putz01HandleButtons() Looping\r");
		i = 0;
	}
	{ static bool onOff = false;
		LED_SetOut(onOff ? 2 : 1);
		onOff = !onOff;
	}
	return true;
}

I2sBufCtl_t I2S2BufCtl;
uint16_t I2S2Buf[I2S2BUFSZ];
bool Putz01I2S2Start (void) {
	puts ("Putz01I2S2Start() Enter\r");
	HAL_StatusTypeDef sts = HAL_I2S_Receive_DMA(&hi2s2, I2S2Buf, I2S2BUFSZ);
	PUTZ_ASSERT(sts==HAL_OK);
	return true;
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	I2S2BufCtl.nRecv++;
}
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	I2S2BufCtl.nRecv++;
}

void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s) {
	I2S2BufCtl.nError++;
}

bool Putz01I2S2Proc (void) {
	char strbuf[32];
	if (I2S2BufCtl.nRecvProc != I2S2BufCtl.nRecv) {
		if (I2S2BufCtl.nErrorProc != I2S2BufCtl.nError) {
				I2S2BufCtl.nErrorProc++;
		}
		if (!(I2S2BufCtl.nRecvProc & 0x000e)) {
			if (I2S2BufCtl.nRecvProc & 1) {
				sprintf (strbuf, "I2S Rx%d.5 Er%d ", I2S2BufCtl.nRecvProc/2, I2S2BufCtl.nErrorProc);
				DumpHWds (I2S2Buf+(I2S2BUFSZ/2),8,strbuf);
			} else {
				sprintf (strbuf, "I2S Rx%d.0 Er%d ", I2S2BufCtl.nRecvProc/2, I2S2BufCtl.nErrorProc);
				DumpHWds (I2S2Buf,8,strbuf);
			}
		}
		I2S2BufCtl.nRecvProc++;
	} else if (I2S2BufCtl.nErrorProc != I2S2BufCtl.nError) {
		I2S2BufCtl.nErrorProc++;
		printf ("I2S Er%d\r\n", I2S2BufCtl.nErrorProc);
	}
	return true;
}

I2sBufCtl_t I2S3BufCtl;
uint16_t I2S3Buf[I2S3BUFSZ];
bool Putz01I3S2Start (void) {
	return true;
}
bool Putz01I2S3Proc (void) {
	return true;
}


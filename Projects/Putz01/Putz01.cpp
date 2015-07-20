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

#define SWAPHWDS(x)	((((x)&0x0000ffff)<<16)|(((x)&0xffff0000)>>16))
void DumpFWds (uint32_t *p, uint16_t len, char *note) {
	if (note) printf (note);
	for (uint16_t i = 0; i != len; i++) {
		if (i && !(i & 15)) puts ("\r");
		printf ("%08x ", SWAPHWDS(p[i])); 
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
	memset(I2S3Buf,0,sizeof(I2S3Buf)); 
	Putz01I2S2Start();
	Putz01I2S3Start();
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

#define PORTA_IDR	0x40020010
#define PORTB_IDR	0x40020410
#define PA04I (*((uint32_t*)PORTA_IDR)&(1<<04))
#define PB12I (*((uint32_t*)PORTB_IDR)&(1<<12))
#define BVAL 0
void WaitForI2sWs(I2S_HandleTypeDef *h) {
	int bHigh = BVAL;
	if (h == &hi2s2) {
		printf ("WaitForIs2Ws (PB12) transition to %s\r\n", bHigh ? "HIGH" : "LOW");
		while (!PB12I);					// Wait while low
		while (PB12I);					// Wait while high
		if (bHigh) while (!PB12I);	// Wait while low
	} else if (h == &hi2s3) {
		printf ("WaitForIs2Ws (PA04) transition to %s\r\n", bHigh ? "HIGH" : "LOW");
		while (!PA04I);					// Wait while low
		while (PA04I);					// Wait while high
		if (bHigh) while (!PA04I);	// Wait while low
	} else {
		PUTZ_ASSERT(false);
	}
	
}

I2sBufCtl_t I2S2BufCtl;
I2sData_t I2S2Buf[I2S2BUFSZ];
bool Putz01I2S2Start (void) {
	puts ("Putz01I2S2Start() Enter\r");
	HAL_StatusTypeDef sts = HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)I2S2Buf, I2S2BUFSZ*sizeof(I2sData_t)/sizeof(uint16_t));
	PUTZ_ASSERT(sts==HAL_OK);
	return true;
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	HAL_I2S_RxCpltCallback(hi2s);
}
void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	if (hi2s == &hi2s2) {
		I2S2BufCtl.nRecv++;
	} else if (hi2s == &hi2s3) {
			I2S3BufCtl.nRecv++;
	} else {
		PUTZ_ASSERT(false);
	}
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
				sprintf (strbuf, "I2S2 Rx%06d.5 Er%d ", I2S2BufCtl.nRecvProc/2, I2S2BufCtl.nErrorProc);
				DumpFWds ((uint32_t*)&I2S2Buf[I2S2BUFSZ/2],8,strbuf);
			} else {
				sprintf (strbuf, "I2S2 Rx%06d.0 Er%d ", I2S2BufCtl.nRecvProc/2, I2S2BufCtl.nErrorProc);
				DumpFWds ((uint32_t*)&I2S2Buf[0],8,strbuf);
			}
		}
		I2S2BufCtl.nRecvProc++;
	} else if (I2S2BufCtl.nErrorProc != I2S2BufCtl.nError) {
		I2S2BufCtl.nErrorProc++;
		printf ("I2S Er%04d\r\n", I2S2BufCtl.nErrorProc);
	}
	return true;
}

I2sBufCtl_t I2S3BufCtl;
I2sData_t I2S3Buf[I2S3BUFSZ];
bool Putz01I2S3Start (void) {
	puts ("Putz01I2S3Start() Enter\r");
	HAL_StatusTypeDef sts = HAL_I2S_Receive_DMA(&hi2s3, (uint16_t*)I2S3Buf, I2S3BUFSZ*sizeof(I2sData_t)/sizeof(uint16_t));
	PUTZ_ASSERT(sts==HAL_OK);
	return true;
}
bool Putz01I2S3Proc (void) {
	char strbuf[32];
	if (I2S3BufCtl.nRecvProc != I2S3BufCtl.nRecv) {
		if (I2S3BufCtl.nErrorProc != I2S3BufCtl.nError) {
				I2S3BufCtl.nErrorProc++;
		}
		if (!(I2S3BufCtl.nRecvProc & 0x000e)) {
			if (I2S3BufCtl.nRecvProc & 1) {
				sprintf (strbuf, "I2S3 Rx%06d.5 Er%d ", I2S3BufCtl.nRecvProc/2, I2S3BufCtl.nErrorProc);
				DumpFWds ((uint32_t*)&I2S3Buf[I2S3BUFSZ/2],8,strbuf);
			} else {
				sprintf (strbuf, "I2S3 Rx%06d.0 Er%d ", I2S3BufCtl.nRecvProc/2, I2S3BufCtl.nErrorProc);
				DumpFWds ((uint32_t*)&I2S3Buf[0],8,strbuf);
			}
		}
		I2S3BufCtl.nRecvProc++;
	} else if (I2S3BufCtl.nErrorProc != I2S3BufCtl.nError) {
		I2S3BufCtl.nErrorProc++;
		printf ("I2S Er%04d\r\n", I2S3BufCtl.nErrorProc);
	}
	return true;
}



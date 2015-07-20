// Putz01.h -- Application Interface
//

#include <ctype.h>
#include <stdint.h>
#include <stdbool.h>
#include "stm32f4xx_hal_dma.h"
#include "stm32f4xx_hal_i2s.h"
#include "stm32f4xx_hal_usart.h"
#include "stm32f4xx_hal_uart.h"

#define PUTZ_ASSERT(x)	if(!(x))Putz01Assert((uint8_t *)__FILE__, __LINE__)

#define I2S2BUFSZ	4096
#define I2S3BUFSZ	4096

typedef int32_t I2sData_t;

typedef struct I2sBufCtl {
	volatile uint32_t nRecv;
	volatile uint32_t nRecvProc;
	volatile uint32_t nError;
	volatile uint32_t nErrorProc;
} I2sBufCtl_t;

#ifdef __cplusplus 
extern "C" {
#endif

// Public function prototypes	
bool Putz01Init (void);
bool Putz01Run(void);
bool Putz01HandleButtons (void);
bool Putz01I2S2Proc (void);
bool Putz01I2S3Proc (void);
bool Putz01I2S2Start (void);
bool Putz01I2S3Start (void);
void Putz01Assert(uint8_t* file, uint32_t line);
int stdin_getchar(void);
void stdout_putchar(int c);
void WaitForI2sWs(I2S_HandleTypeDef *h);
extern void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s);
extern void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s);
extern void HAL_I2S_ErrorCallback(I2S_HandleTypeDef *hi2s);

// PUblic data structures
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern I2S_HandleTypeDef hi2s2;
extern I2S_HandleTypeDef hi2s3;
extern DMA_HandleTypeDef hdma_spi2_rx;
extern DMA_HandleTypeDef hdma_spi3_rx;
extern volatile int32_t ITM_RxBuffer;
extern I2sData_t I2S2Buf[I2S2BUFSZ];
extern I2sData_t I2S3Buf[I2S3BUFSZ];
extern I2sBufCtl_t I2S2BufCtl;
extern I2sBufCtl_t I2S3BufCtl;

#ifdef __cplusplus 
}
#endif

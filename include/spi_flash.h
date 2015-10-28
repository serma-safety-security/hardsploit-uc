/**
  ******************************************************************************
  * @file    stm32_eval_spi_flash.h
  * @author  MCD Application Team
  * @version V4.3.0
  * @date    10/15/2010
  * @brief   This file contains all the functions prototypes for the stm32_eval_spi_flash
  *          firmware driver.
  ******************************************************************************
  * @copy
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2010 STMicroelectronics</center></h2>
  */ 

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SPI_FLASH_H
#define __SPI_FLASH_H
#include "stm32f2xx_hal.h"
#include "usbd_def.h"
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/


#define  FLASH_WriteAddress     0x000000
#define  FLASH_ReadAddress      0x000000




#define sFLASH_SPI                       SPI2


 void sFLASH_LowLevel_DeInit(void);
 void sFLASH_LowLevel_Init(void);



/**
  * @brief  M25P SPI Flash supported commands
  */  


 #define sFLASH_CMD_WRITE          0x02  /*!< Write to Memory instruction */
#define sFLASH_CMD_WRSR           0x01  /*!< Write Status Register instruction */
#define sFLASH_CMD_WREN           0x06  /*!< Write enable instruction */
#define sFLASH_CMD_READ           0x03  /*!< Read from Memory instruction */
#define sFLASH_CMD_RDSR           0x05  /*!< Read Status Register instruction  */

#define sFLASH_CMD_RDID           0xAB  /*!< Read memory ID  */

#define sFLASH_WIP_FLAG           0x01  /*!< Write In Progress (WIP) flag */

#define sFLASH_DUMMY_BYTE         0xA5
#define sFLASH_SPI_PAGESIZE       0x100  //256


  
/**
  * @}
  */ 
  
/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Macros
  * @{
  */
/**
  * @brief  Select sFLASH: Chip Select pin low
  */


//#define sFLASH_CS_LOW() do { GPIOA->BRR = GPIO_Pin_4; } while (0)
//#define sFLASH_CS_HIGH() do { GPIOA->BSRR = GPIO_Pin_4; } while (0)



//#define sFLASH_CS_LOW()       GPIO_ResetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
/**
  * @brief  Deselect sFLASH: Chip Select pin high
  */
//#define sFLASH_CS_HIGH()      GPIO_SetBits(sFLASH_CS_GPIO_PORT, sFLASH_CS_PIN)
/**
  * @}
  */ 
  


/** @defgroup STM32_EVAL_SPI_FLASH_Exported_Functions
  * @{
  */
/**
  * @brief  High layer functions
  */
void sFLASH_DeInit(void);
void sFLASH_Init(void);
uint8_t sFLASH_READID(void);
uint8_t sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_WriteBuffer(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite);
void sFLASH_ReadBuffer(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void sFLASH_StartReadSequence(uint32_t ReadAddr);
void sFLASH_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead);
void  sFLASH_ERASE(void);

/**
  * @brief  Low layer functions
  */
uint8_t sFLASH_ReadByte(void);
uint8_t sFLASH_SendByte(uint8_t byte);
uint16_t sFLASH_SendHalfWord(uint16_t HalfWord);
void sFLASH_WriteEnable(void);
void sFLASH_WaitForWriteEnd(void);
void sFLASH_CS_LOW(void);
void sFLASH_CS_HIGH(void);
void fpgaCommand(uint8_t* transmitBuffer, uint8_t* receiveBuffer,uint16_t NumByteToRead);
void fpgaData(uint8_t* transmitBuffer, uint8_t* receiveBuffer,uint16_t NumByteToRead);
#ifdef __cplusplus
}
#endif

#endif /* __STM32_EVAL_SPI_FLASH_H */
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */ 

/**
  * @}
  */  

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

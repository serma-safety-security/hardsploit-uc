////////////////////////////////////////////////////////
//  Hardsploit uC - By Opale Security				  //
//  www.opale-security.com || www.hardsploit.io		  //
//  License: GNU General Public License v3			  //
//  License URI: http://www.gnu.org/licenses/gpl.txt  //
////////////////////////////////////////////////////////

#include "stm32f2xx_hal.h"
#include "spi_flash.h"
#include "spi.h"
#include "stm32f2xx_hal_spi.h"

extern void Delay(__IO uint32_t nCount);
extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

extern uint8_t UserTxBufferFS[8192];
extern USBD_HandleTypeDef  *hUsbDevice_0;
/**
  * @brief  DeInitializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_DeInit(void)
{
  sFLASH_LowLevel_DeInit();
}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_Init(void)
{
}

uint8_t  sFLASH_READ_STATUT(void){
	unsigned char temp[2] = {0x24, 0x24};
	unsigned char readDeviceID[5] = {sFLASH_CMD_RDSR, 0x00};
	sFLASH_CS_LOW();
 	HAL_SPI_TransmitReceive(&hspi2, *(&readDeviceID),*(&temp), 2, 5000);
	sFLASH_CS_HIGH();
	Delay(1000);
	return temp[1];
}


void  sFLASH_WRITE_ENABLE(void){
	unsigned char command[1] = {sFLASH_CMD_WREN};
	sFLASH_CS_LOW();
 	HAL_SPI_Transmit(&hspi2, *(&command),1, 5000);
	sFLASH_CS_HIGH();
	Delay(1000);


}
void  sFLASH_ERASE_BULK(void){
	unsigned char eraseCommand[1] = {0xC7};

	sFLASH_WRITE_ENABLE();

	sFLASH_CS_LOW();
 	HAL_SPI_Transmit(&hspi2, *(&eraseCommand),1, 5000);
	sFLASH_CS_HIGH();

	  /*!< Wait the end of Flash writing */
	 sFLASH_WaitForWriteEnd();

}
void  sFLASH_ERASE(void){
	unsigned char eraseCommandSector1[] = {0xD8,0x00,0x00,0x00}; //Sector erase command + erase first sector
	unsigned char eraseCommandSector2[] = {0xD8,0x01,0x00,0x00}; //Sector erase command + erase second sector

	sFLASH_WRITE_ENABLE();


//4Mb flash memory
//With 512kb per sector
//We need to erase sector 1 and 2   at 0x00  and 0x10000

	sFLASH_CS_LOW();
 	HAL_SPI_Transmit(&hspi2, *(&eraseCommandSector1),4, 5000);
	sFLASH_CS_HIGH();

	  /*!< Wait the end of Flash writing */
	 sFLASH_WaitForWriteEnd();



	sFLASH_CS_LOW();
	HAL_SPI_Transmit(&hspi2, *(&eraseCommandSector2),4, 5000);
	sFLASH_CS_HIGH();

	/*!< Wait the end of Flash writing */
	sFLASH_WaitForWriteEnd();

}


uint8_t  sFLASH_READID(void){
	unsigned char temp[5] = {0x24, 0x24, 0x24, 0x24,0x24};
	unsigned char readDeviceID[5] = {sFLASH_CMD_RDID, 0x00, 0x00, 0x00,0x00};
	sFLASH_CS_LOW();
 	HAL_SPI_TransmitReceive(&hspi2, *(&readDeviceID),*(&temp), 5, 5000);
	sFLASH_CS_HIGH();
	return temp[4];
}


/**
  * @brief  Writes more than one byte to the FLASH with a single WRITE cycle 
  *         (Page WRITE sequence).
  * @note   The number of byte can't exceed the FLASH page size.
  * @param  pBuffer: pointer to the buffer  containing the data to be written
  *         to the FLASH.
  * @param  WriteAddr: FLASH's internal address to write to.
  * @param  NumByteToWrite: number of bytes to write to the FLASH, must be equal
  *         or less than "sFLASH_PAGESIZE" value.
  * @retval None
  */
uint8_t sFLASH_WritePage(uint8_t* pBuffer, uint32_t WriteAddr, uint16_t NumByteToWrite)
{
		unsigned char statut =0x00;
	  unsigned char write[4] = {sFLASH_CMD_WRITE, (WriteAddr & 0xFF0000) >> 16, (WriteAddr& 0xFF00) >> 8, WriteAddr & 0xFF};


	  if (NumByteToWrite >256 ){
		  NumByteToWrite =256;
	  }
	  /*!< Enable the write access to the FLASH */
      sFLASH_WRITE_ENABLE();
     statut = sFLASH_READ_STATUT();


	  sFLASH_CS_LOW();

	  HAL_SPI_Transmit(&hspi2, *(&write), 4, 5000);
	  HAL_SPI_Transmit(&hspi2,pBuffer, NumByteToWrite, 5000);


	  /*!< Deselect the FLASH: Chip Select high */
	  sFLASH_CS_HIGH();
	  Delay(1000);

	   /*!< Wait the end of Flash writing */
    sFLASH_WaitForWriteEnd();

  return statut;
}







void sFLASH_Read(uint8_t* pBuffer, uint32_t ReadAddr, uint16_t NumByteToRead)
{

  unsigned char read[4] = {sFLASH_CMD_READ, (ReadAddr & 0xFF0000) >> 16, (ReadAddr& 0xFF00) >> 8, ReadAddr & 0xFF};

  sFLASH_CS_LOW();
  HAL_SPI_Transmit(&hspi2, *(&read), 4, 5000);
  HAL_SPI_Receive(&hspi2,pBuffer, NumByteToRead, 5000);
  sFLASH_CS_HIGH();
  Delay(1000);
}


/**
  * @brief  Polls the status of the Write In Progress (WIP) flag in the FLASH's
  *         status register and loop until write opertaion has completed.
  * @param  None
  * @retval None
  */
void sFLASH_WaitForWriteEnd(void)
{
  /*!< Loop as long as the memory is busy with a write cycle */
  while ((sFLASH_READ_STATUT() & sFLASH_WIP_FLAG)); /* Write in progress */

}


void sFLASH_LowLevel_DeInit(void)
{

}

/**
  * @brief  Initializes the peripherals used by the SPI FLASH driver.
  * @param  None
  * @retval None
  */
void sFLASH_LowLevel_Init(void)
{

}

void sFLASH_CS_LOW(){
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,0);
}
void sFLASH_CS_HIGH(){
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,1);
}

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/

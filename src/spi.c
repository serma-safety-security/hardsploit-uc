////////////////////////////////////////////////////////
//  Hardsploit uC - By Opale Security				  //
//  www.opale-security.com || www.hardsploit.io		  //
//  License: GNU General Public License v3			  //
//  License URI: http://www.gnu.org/licenses/gpl.txt  //
////////////////////////////////////////////////////////

#include "spi.h"
#include "gpio.h"
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
SPI_HandleTypeDef hspi3;

static DMA_HandleTypeDef hdma_tx_SPI_ToFPGA;
static DMA_HandleTypeDef hdma_rx_SPI_ToFPGA;

static DMA_HandleTypeDef hdma_tx_SPI_FromFPGA;
static DMA_HandleTypeDef hdma_rx_SPI_FromFPGA;


/* Definition for SPIx's DMA */
#define SPI1_TX_DMA_STREAM               DMA2_Stream3
#define SPI1_RX_DMA_STREAM               DMA2_Stream2
#define SPI1_TX_DMA_CHANNEL              DMA_CHANNEL_3
#define SPI1_RX_DMA_CHANNEL              DMA_CHANNEL_3

/* Definition for SPIx's NVIC */
#define SPI1_DMA_TX_IRQn                 DMA2_Stream3_IRQn
#define SPI1_DMA_RX_IRQn                 DMA2_Stream2_IRQn
#define SPI1_DMA_TX_IRQHandler           DMA2_Stream3_IRQHandler
#define SPI1_DMA_RX_IRQHandler           DMA2_Stream2_IRQHandler



/* Definition for SPIx's DMA */
#define SPI3_TX_DMA_STREAM               DMA1_Stream5
#define SPI3_RX_DMA_STREAM               DMA1_Stream2
#define SPI3_TX_DMA_CHANNEL              DMA_CHANNEL_0
#define SPI3_RX_DMA_CHANNEL              DMA_CHANNEL_0

/* Definition for SPIx's NVIC */
#define SPI3_DMA_TX_IRQn                 DMA1_Stream5_IRQn
#define SPI3_DMA_RX_IRQn                 DMA1_Stream2_IRQn
#define SPI3_DMA_TX_IRQHandler           DMA1_Stream5_IRQHandler
#define SPI3_DMA_RX_IRQHandler           DMA1_Stream2_IRQHandler






/* SPI1 init function */
void MX_SPI1_Init(void)
{

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_SLAVE;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi1.Init.CRCPolynomial= 1;
  HAL_SPI_Init(&hspi1);

}
/* SPI2 init function */
void MX_SPI2_Init(void)
{

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi2.Init.CRCPolynomial= 1;

  HAL_SPI_Init(&hspi2);

}
/* SPI3 init function */
void MX_SPI3_Init(void)
{

  hspi3.Instance = SPI3;
  hspi3.Init.Mode = SPI_MODE_MASTER;
  hspi3.Init.Direction = SPI_DIRECTION_2LINES;
  hspi3.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi3.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi3.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi3.Init.NSS = SPI_NSS_SOFT;
  hspi3.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi3.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi3.Init.TIMode = SPI_TIMODE_DISABLED;
  hspi3.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLED;
  hspi3.Init.CRCPolynomial= 1;
  HAL_SPI_Init(&hspi3);

}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance==SPI1)
  {
    __SPI1_CLK_ENABLE();
    __DMA2_CLK_ENABLE();
    /**SPI1 GPIO Configuration    
    PA4     ------> SPI1_NSS 20
    PA5     ------> SPI1_SCK  21
    PA6     ------> SPI1_MISO  22
    PA7     ------> SPI1_MOSI  23
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);


    //spi1 NSS
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	  /* Enable and set EXTI Line0 Interrupt to the lowest priority */
	  HAL_NVIC_SetPriority(EXTI4_IRQn, 2, 0);
	  HAL_NVIC_EnableIRQ(EXTI4_IRQn);






    /*##-3- Configure the DMA streams ##########################################*/
        /* Configure the DMA handler for Transmission process */
  hdma_tx_SPI_ToFPGA.Instance                 = SPI1_TX_DMA_STREAM;
    hdma_tx_SPI_ToFPGA.Init.Channel             = SPI1_TX_DMA_CHANNEL;
    hdma_tx_SPI_ToFPGA.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
    hdma_tx_SPI_ToFPGA.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
    hdma_tx_SPI_ToFPGA.Init.MemBurst            = DMA_MBURST_INC4;
    hdma_tx_SPI_ToFPGA.Init.PeriphBurst         = DMA_PBURST_INC4;
    hdma_tx_SPI_ToFPGA.Init.Direction           = DMA_MEMORY_TO_PERIPH;
    hdma_tx_SPI_ToFPGA.Init.PeriphInc           = DMA_PINC_DISABLE;
    hdma_tx_SPI_ToFPGA.Init.MemInc              = DMA_MINC_ENABLE;
    hdma_tx_SPI_ToFPGA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_tx_SPI_ToFPGA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
    hdma_tx_SPI_ToFPGA.Init.Mode                = DMA_NORMAL;
    hdma_tx_SPI_ToFPGA.Init.Priority            = DMA_PRIORITY_LOW;

	HAL_DMA_Init(&hdma_tx_SPI_ToFPGA);

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(hspi,hdmatx ,hdma_tx_SPI_ToFPGA);

	/* Configure the DMA handler for Transmission process */
	hdma_rx_SPI_ToFPGA.Instance                 = SPI1_RX_DMA_STREAM;

	hdma_rx_SPI_ToFPGA.Init.Channel             = SPI1_RX_DMA_CHANNEL;
	hdma_rx_SPI_ToFPGA.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	hdma_rx_SPI_ToFPGA.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_rx_SPI_ToFPGA.Init.MemBurst            = DMA_MBURST_INC4;
	hdma_rx_SPI_ToFPGA.Init.PeriphBurst         = DMA_PBURST_INC4;
	hdma_rx_SPI_ToFPGA.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_rx_SPI_ToFPGA.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_rx_SPI_ToFPGA.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_rx_SPI_ToFPGA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_rx_SPI_ToFPGA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_rx_SPI_ToFPGA.Init.Mode                = DMA_NORMAL;
	hdma_rx_SPI_ToFPGA.Init.Priority            = DMA_PRIORITY_HIGH;

	HAL_DMA_Init(&hdma_rx_SPI_ToFPGA);

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(hspi, hdmarx, hdma_rx_SPI_ToFPGA);

	/*##-4- Configure the NVIC for DMA #########################################*/
	/* NVIC configuration for DMA transfer complete interrupt (SPI1_TX) */
	HAL_NVIC_SetPriority(SPI1_DMA_TX_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(SPI1_DMA_TX_IRQn);

	/* NVIC configuration for DMA transfer complete interrupt (SPI1_RX) */
	HAL_NVIC_SetPriority(SPI1_DMA_RX_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(SPI1_DMA_RX_IRQn);

  /* USER CODE BEGIN SPI1_MspInit 1 */

  /* USER CODE END SPI1_MspInit 1 */
  }
  else if(hspi->Instance==SPI2)  //fpga
  {
  /* USER CODE BEGIN SPI2_MspInit 0 */

  /* USER CODE END SPI2_MspInit 0 */
    /* Peripheral clock enable */
    __SPI2_CLK_ENABLE();
    /* Enable DMA1 clock */

    /**SPI2 GPIO Configuration    
    PC2     ------> SPI2_MISO
    PC3     ------> SPI2_MOSI
    PB12     ------> SPI2_NSS // Soft
    PB10     ------> SPI2_SCK
    */
    GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF5_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    //spi2 memoire fpga CS
     GPIO_InitStruct.Pin = GPIO_PIN_12;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	HAL_GPIO_WritePin(GPIOB ,GPIO_PIN_12,1);

  /* USER CODE BEGIN SPI2_MspInit 1 */

  /* USER CODE END SPI2_MspInit 1 */
  }
  else if(hspi->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspInit 0 */

  /* USER CODE END SPI3_MspInit 0 */
    /* Peripheral clock enable */
    __SPI3_CLK_ENABLE();
    __DMA1_CLK_ENABLE();
    /**SPI3 GPIO Configuration    
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF6_SPI3;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);


    //spi3 command   CS
     GPIO_InitStruct.Pin = GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	 HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,1);



    /*##-3- Configure the DMA streams ##########################################*/
        /* Configure the DMA handler for Transmission process */
  hdma_tx_SPI_FromFPGA.Instance                 = SPI3_TX_DMA_STREAM;
  hdma_tx_SPI_FromFPGA.Init.Channel             = SPI3_TX_DMA_CHANNEL;
  hdma_tx_SPI_FromFPGA.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
  hdma_tx_SPI_FromFPGA.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
  hdma_tx_SPI_FromFPGA.Init.MemBurst            = DMA_MBURST_INC4;
  hdma_tx_SPI_FromFPGA.Init.PeriphBurst         = DMA_PBURST_INC4;
  hdma_tx_SPI_FromFPGA.Init.Direction           = DMA_MEMORY_TO_PERIPH;
  hdma_tx_SPI_FromFPGA.Init.PeriphInc           = DMA_PINC_DISABLE;
  hdma_tx_SPI_FromFPGA.Init.MemInc              = DMA_MINC_ENABLE;
  hdma_tx_SPI_FromFPGA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
  hdma_tx_SPI_FromFPGA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
  hdma_tx_SPI_FromFPGA.Init.Mode                = DMA_NORMAL;
  hdma_tx_SPI_FromFPGA.Init.Priority            = DMA_PRIORITY_LOW;

	HAL_DMA_Init(&hdma_tx_SPI_FromFPGA);

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(hspi,hdmatx ,hdma_tx_SPI_FromFPGA);

	/* Configure the DMA handler for Transmission process */
	hdma_rx_SPI_FromFPGA.Instance                 = SPI3_RX_DMA_STREAM;

	hdma_rx_SPI_FromFPGA.Init.Channel             = SPI3_RX_DMA_CHANNEL;
	hdma_rx_SPI_FromFPGA.Init.FIFOMode            = DMA_FIFOMODE_DISABLE;
	hdma_rx_SPI_FromFPGA.Init.FIFOThreshold       = DMA_FIFO_THRESHOLD_FULL;
	hdma_rx_SPI_FromFPGA.Init.MemBurst            = DMA_MBURST_INC4;
	hdma_rx_SPI_FromFPGA.Init.PeriphBurst         = DMA_PBURST_INC4;
	hdma_rx_SPI_FromFPGA.Init.Direction           = DMA_PERIPH_TO_MEMORY;
	hdma_rx_SPI_FromFPGA.Init.PeriphInc           = DMA_PINC_DISABLE;
	hdma_rx_SPI_FromFPGA.Init.MemInc              = DMA_MINC_ENABLE;
	hdma_rx_SPI_FromFPGA.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
	hdma_rx_SPI_FromFPGA.Init.MemDataAlignment    = DMA_MDATAALIGN_BYTE;
	hdma_rx_SPI_FromFPGA.Init.Mode                = DMA_NORMAL;
	hdma_rx_SPI_FromFPGA.Init.Priority            = DMA_PRIORITY_HIGH;

	HAL_DMA_Init(&hdma_rx_SPI_FromFPGA);

	/* Associate the initialized DMA handle to the the SPI handle */
	__HAL_LINKDMA(hspi, hdmarx, hdma_rx_SPI_FromFPGA);

	/*##-4- Configure the NVIC for DMA #########################################*/
	/* NVIC configuration for DMA transfer complete interrupt (SPI1_TX) */
	HAL_NVIC_SetPriority(SPI3_DMA_TX_IRQn, 0, 1);
	HAL_NVIC_EnableIRQ(SPI3_DMA_TX_IRQn);

	/* NVIC configuration for DMA transfer complete interrupt (SPI1_RX) */
	HAL_NVIC_SetPriority(SPI3_DMA_RX_IRQn, 0, 0);
	HAL_NVIC_EnableIRQ(SPI3_DMA_RX_IRQn);




  /* USER CODE BEGIN SPI3_MspInit 1 */

  /* USER CODE END SPI3_MspInit 1 */
  }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{

  if(hspi->Instance==SPI1)
  {
	  //Disable IRQ
	 	  HAL_NVIC_DisableIRQ(EXTI4_IRQn);

	 	  //clear interupt
	 	  HAL_NVIC_ClearPendingIRQ(EXTI4_IRQn);

	 	  //Disable DMA
	 	  HAL_DMA_DeInit(&hdma_tx_SPI_ToFPGA);
	 	  HAL_DMA_DeInit(&hdma_rx_SPI_ToFPGA);

	 	  HAL_NVIC_DisableIRQ(SPI1_DMA_TX_IRQn);
	 	  HAL_NVIC_DisableIRQ(SPI1_DMA_RX_IRQn);
	      __SPI1_CLK_DISABLE();

	     /**SPI1 GPIO Configuration
	     PA4     ------> SPI1_NSS
	     PA5     ------> SPI1_SCK
	     PA6     ------> SPI1_MISO
	     PA7     ------> SPI1_MOSI
	     */
	     HAL_GPIO_DeInit(GPIOA, GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7);
  }
  else if(hspi->Instance==SPI2)//fpga
  {
  /* USER CODE BEGIN SPI2_MspDeInit 0 */

  /* USER CODE END SPI2_MspDeInit 0 */
    /* Peripheral clock disable */
   // __SPI2_CLK_DISABLE();
  
    /**SPI2 GPIO Configuration    
    PC2     ------> SPI2_MISO
    PC3     ------> SPI2_MOSI
    PB12     ------> SPI2_NSS
    PB10     ------> SPI2_SCK
    */

    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_2|GPIO_PIN_3);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);



    /*##-3- Disable the DMA Streams ############################################*/
        /* De-Initialize the DMA Stream associate to transmission process */
        //HAL_DMA_DeInit(&hdma_tx);
        /* De-Initialize the DMA Stream associate to reception process */
      //  HAL_DMA_DeInit(&hdma_rx);

        /*##-4- Disable the NVIC for DMA ###########################################*/
       // HAL_NVIC_DisableIRQ(SPIx_DMA_TX_IRQn);
       // HAL_NVIC_DisableIRQ(SPIx_DMA_RX_IRQn);


  /* USER CODE BEGIN SPI2_MspDeInit 1 */

  /* USER CODE END SPI2_MspDeInit 1 */
  }
  else if(hspi->Instance==SPI3)
  {
  /* USER CODE BEGIN SPI3_MspDeInit 0 */

  /* USER CODE END SPI3_MspDeInit 0 */
    /* Peripheral clock disable */
    __SPI3_CLK_DISABLE();
  
    /**SPI3 GPIO Configuration    
    PC10     ------> SPI3_SCK
    PC11     ------> SPI3_MISO
    PC12     ------> SPI3_MOSI 
    */
    HAL_GPIO_DeInit(GPIOC, GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_9);

  /* USER CODE BEGIN SPI3_MspDeInit 1 */

  /* USER CODE END SPI3_MspDeInit 1 */
  }
} 




/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

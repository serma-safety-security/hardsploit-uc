////////////////////////////////////////////////////////
//  Hardsploit uC - By Opale Security				  //
//  www.opale-security.com || www.hardsploit.io		  //
//  License: GNU General Public License v3			  //
//  License URI: http://www.gnu.org/licenses/gpl.txt  //
////////////////////////////////////////////////////////


/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_hal.h"
#include "spi.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "usbd_raw.h"


/* Private variables ---------------------------------------------------------*/
#define COUNTOF(__BUFFER__)   (sizeof(__BUFFER__) / sizeof(*(__BUFFER__)))
extern uint8_t receiveRawDataFromFPGABySPI[8192];



extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;


extern u_char  Data_buffer_Transmit_InProgress ;
extern u_char  Data_buffer_Transmit_Start_Transmit ;
extern uint16_t Data_buffer_Transmit_length ;
extern uint8_t Data_buffer_Transmit[SIZE_FRAME] ;

extern u_char startProcessCommand ;
extern   uint8_t receiveRawDataFromFPGABySPI[SIZE_FRAME] ;

extern uint8_t rx_raw_Frame;
extern uint16_t rx_raw_FrameLen;

extern  u_char mustValidCommandAfterSend;
extern  u_char Sent_FPGA_DATA_inProgress ;
extern u_char fpgaProgrammingMode   ;

extern uint8_t UserTxBufferFS[8192];

uint16_t size_temp = 0 ;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);


void Delay(__IO uint32_t nCount)
{

  while(nCount--)
  {
  }

}

int main(void)
{
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  //SLAVE uC  FPGA MASTER
  MX_SPI1_Init();


  //Master uC FPGA SLAVE
  MX_SPI3_Init();


	//Nce FPGA   disable at start
  	HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,1);

	//nconfig on release reconfigure fpga but here disable
	HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,0);


    /* LED of uC are off */
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,0);




	MX_USB_DEVICE_Init();
	Delay(5000);


	//Red led set on at start
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,1);

	while (1){


	}
}

uint8_t sendUSB(uint8_t * buffer, uint16_t size){
	//if transmit is in progress wait for
	//while( Data_buffer_Transmit_InProgress == 1) ;
	if (Data_buffer_Transmit_InProgress == 0){
		buffer[0] = LOBYTE(size);  //Set the true lenght of trame in the header (first two byte)
		buffer[1] = HIBYTE(size);

		Data_buffer_Transmit_InProgress = 1;
		USBD_LL_Transmit(&hUsbDeviceFS,RAW_EPIN_ADDR,buffer,size);
		return 1;
	}else{
		return 0;

	}
	//End of processing FPGA DATA
	//while (Data_buffer_Transmit_InProgress == 1 );

}
void processFPGAData(){
	//Indicate that a buffer is full
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,1);

	//Transmit to USB ( length of data acquire by DMA)   4 + (8187 -sizeDMA)
	size_temp = 8186 - __HAL_DMA_GET_COUNTER( hspi1.hdmarx )+4;

	Sent_FPGA_DATA_inProgress = 1;
	sendUSB(receiveRawDataFromFPGABySPI,size_temp);

}

void endOfSendFPGADATA(){
	//prepare DMA for the next receiving
	//Offset + 4   lenght & command type  4093 * 2 = 8186  max for 16bits transmition must have a buffer size at multiple of 2
	HAL_SPI_Receive_DMA(&hspi1, *(&receiveRawDataFromFPGABySPI) + 4, 8186 );

	//Indicate that a buffer is free
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_6,0);

}

void  HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	//END OF SPI
	if (GPIO_Pin == GPIO_PIN_4){
		//If SPI close communication close DMA transfert
		 HAL_DMA_Abort(hspi1.hdmarx);
		 processFPGAData();
	}
}

//If DMA TRANSFERT IS FINISH ( ONLY WHEN EGAL TO SIZE OF BUFFER)
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi){
	//DMA COMPLETED FOR SPI1 (RECEIVE RAW DATA FROM FPGA)
	if (hspi->Instance == SPI1 ){
			processFPGAData();
	}
}


//If DMA TRANSFERT IS FINISH OF SPI3 (COMMAND SPI)
//void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi){
void HAL_SPI_TxCpltCallback(SPI_HandleTypeDef *hspi){
	if (hspi->Instance == SPI3 ){
		HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,1); //Release SPI3 because end of transmition

		endProcessCommandAllowReceiveAgain();
	}
}


/** System Clock Configuration
*/
void SystemClock_Config(void)
{

  RCC_OscInitTypeDef RCC_OscInitStruct;
  RCC_ClkInitTypeDef RCC_ClkInitStruct;

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;

  //16Mhz
  RCC_OscInitStruct.PLL.PLLM = 13;
  RCC_OscInitStruct.PLL.PLLN = 195;

  //25Mhz
  // RCC_OscInitStruct.PLL.PLLM = 20;
  //  RCC_OscInitStruct.PLL.PLLN = 192;



  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 5;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_SYSCLK|RCC_CLOCKTYPE_PCLK1
                              |RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
  HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3);


}

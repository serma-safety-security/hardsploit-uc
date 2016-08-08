////////////////////////////////////////////////////////
//  Hardsploit uC - By Opale Security				  //
//  www.opale-security.com || www.hardsploit.io		  //
//  License: GNU General Public License v3			  //
//  License URI: http://www.gnu.org/licenses/gpl.txt  //
////////////////////////////////////////////////////////

#include "usbd_raw.h"
#include "usbd_desc.h"
#include "usbd_ctlreq.h"
#include "usbd_raw.h"
#include "spi_flash.h"
unsigned char versionNumber[] ={"HW:V1.00 SW:V1.0.3"};  //Size 18

static uint8_t  USBD_RAW_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx);

static uint8_t  USBD_RAW_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx);

static uint8_t  USBD_RAW_Setup (USBD_HandleTypeDef *pdev, 
                                USBD_SetupReqTypedef *req);

static uint8_t  *USBD_RAW_GetCfgDesc (uint16_t *length);

static uint8_t  *USBD_RAW_GetDeviceQualifierDesc (uint16_t *length);

static uint8_t  USBD_RAW_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_RAW_DataOut (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_RAW_EP0_RxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_RAW_EP0_TxReady (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_RAW_SOF (USBD_HandleTypeDef *pdev);

static uint8_t  USBD_RAW_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

static uint8_t  USBD_RAW_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum);

extern SPI_HandleTypeDef hspi1;
extern SPI_HandleTypeDef hspi2;
extern SPI_HandleTypeDef hspi3;

typedef enum {
  LED1   = 0,
  LED2   = 1,
  LOOPBACK=2,
  ERASE_FIRMWARE = 3,
  WRITE_PAGE_FIRMWARE = 4,
  READ_PAGE_FIRMWARE = 5,
  READ_ID_FLASH = 6,
  START_FPGA = 7,
  STOP_FPGA = 8,
  FPGA_COMMAND = 9,
  FPGA_DATA = 10,
  STOP_FPGA_DATA = 11,
  START_FPGA_DATA = 12,
  GET_VERSION_NUMBER = 14,
  COMMAND_NOT_FOUND   = 0xFFFF
}VCP_COMMAND;

extern USBD_HandleTypeDef hUsbDeviceFS;


uint8_t rx_raw_packet[RAW_EP_SIZE];
uint8_t rx_raw_Frame[SIZE_FRAME];
uint16_t rx_raw_FrameLen=0;


 u_char usbRxProcessInProgress  = 0 ;
 u_char usbTxProcessInProgress  = 0 ;
 u_char fpgaProgrammingMode  = 0 ;

 u_char  Data_buffer_Transmit_InProgress = 0 ;
 u_char  Data_buffer_Transmit_Start_Transmit = 0 ;
 uint8_t Data_buffer_Transmit[SIZE_FRAME] ;
 uint16_t Data_buffer_Transmit_length = 0  ;

 u_char mustValidCommandAfterSend = 0  ;

 u_char Sent_FPGA_DATA_inProgress = 0 ;
 uint8_t receiveRawDataFromFPGABySPI[SIZE_FRAME] ;

 u_char startProcessCommand  = 0 ;


/**
  * @brief  USBD_RAW_DataIn
* 	Data received on non-control Int endpoint  (transmit data to PC are received)
  *         handle data IN Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_RAW_DataIn (USBD_HandleTypeDef *pdev, uint8_t epnum)
{
	Data_buffer_Transmit_InProgress = 0 ;
	if(Sent_FPGA_DATA_inProgress == 1){
		//rearm for the next receive from fpga
		endOfSendFPGADATA();
		Sent_FPGA_DATA_inProgress = 0;
	}

	if( mustValidCommandAfterSend == 1 ){
		endProcessCommandAllowReceiveAgain();
	}
	return USBD_OK;
}


/**
  * @brief  USBD_RAW_DataOut
  *			Data received on non-control Out endpoint (receive a packet from pc)
  *         handle data OUT Stage
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_RAW_DataOut (USBD_HandleTypeDef *pdev,   uint8_t epnum)
{

	uint16_t size = USBD_LL_GetRxDataSize(pdev,epnum);

	//Before add, check if not buffer overflow
	if((rx_raw_FrameLen+size) >  SIZE_FRAME){
	  rx_raw_FrameLen = 0; //if buffer overflow restart at 0
	}
	//Copy packet at the end of Frame buffer
	memcpy (rx_raw_Frame+rx_raw_FrameLen,rx_raw_packet,size);
	rx_raw_FrameLen +=size;

	//last packet because packet lenght <  packet max  process trame
	if(size <  RAW_EP_SIZE){
	  //process command in the main loop to decrease the heap
	 // startProcessCommand = 1;
		processCommand();
	}else{
		// initiate next USB packet transfer,we don't have received full trame
	 USBD_LL_PrepareReceive(&hUsbDeviceFS,RAW_EPOUT_ADDR,rx_raw_packet,RAW_EP_SIZE);
	}
    return USBD_OK;
}


void endProcessCommandAllowReceiveAgain(){
	 rx_raw_FrameLen=0;
	 // initiate next USB packet transfer, to append to existing data in buffer
	 USBD_LL_PrepareReceive(&hUsbDeviceFS,RAW_EPOUT_ADDR,rx_raw_packet,RAW_EP_SIZE);
 }


 uint16_t  lowHighByteToInt(uint8_t lByte,uint8_t hByte){
 	return ((uint16_t)(hByte << 8) + lByte)  ;
 }

void processCommand(){
	uint32_t lengthTxData = 0;
	uint32_t addr = 0;
	uint32_t i;
	uint32_t temp;
	//At minimun resent length of trame and command name (minimun header)
	//Data_buffer_Transmit[0] = 0; //Low byte of length  set in  sendAndWaitIfNotReadyAndValidCommand before send
	//Data_buffer_Transmit[1] = 0; // High byte of length set in  sendAndWaitIfNotReadyAndValidCommand before send
	Data_buffer_Transmit[2] = rx_raw_Frame[2]; //at minimum, answer is Low byte of command
	Data_buffer_Transmit[3] = rx_raw_Frame[3]; //at minimum, answer is Hight byte of command

switch (lowHighByteToInt(rx_raw_Frame[2],rx_raw_Frame[3])  ){
		case FPGA_COMMAND :
			receiveRawDataFromFPGABySPI[2] = LOBYTE(FPGA_DATA);
			receiveRawDataFromFPGABySPI[3] = HIBYTE(FPGA_DATA);
			HAL_SPI_Receive_DMA(&hspi1, *(&receiveRawDataFromFPGABySPI) + 4, 8186 ); //Offset + 4   lenght & command type
			lengthTxData = 	lowHighByteToInt(rx_raw_Frame[0],rx_raw_Frame[1]); //resend same length to receive the answer of SPI COMMAND

			//Cs low to start spi
			HAL_GPIO_WritePin(GPIOC,GPIO_PIN_9,0);
  		    HAL_SPI_Transmit_DMA(&hspi3,*(&rx_raw_Frame)+4, lengthTxData-4);

		  break;

		  case LOOPBACK :

 			//Copy frame in tx buffer to send the same thing just a loopback for testing purpose
 			memcpy (Data_buffer_Transmit,rx_raw_Frame,rx_raw_FrameLen);

 			mustValidCommandAfterSend = 1;
 			sendUSB(Data_buffer_Transmit,rx_raw_FrameLen);
 			//endProcessCommandAllowReceiveAgain();
 			//sendAndWaitIfNotReady(rx_raw_FrameLen);

	 	 break;
		 case LED1 :
				Data_buffer_Transmit[4] = rx_raw_Frame[4];  //resend value of led
		 		lengthTxData = 4+1;  //increment length because send a value of led


		 		HAL_GPIO_WritePin(GPIOA,GPIO_PIN_9,rx_raw_Frame[4] & 1 ); //Change value of led

		 		//sendAndWaitIfNotReadyAndValidCommand(lengthTxData);
		 		//If you don't want answer just
		 		endProcessCommandAllowReceiveAgain();
		 break;
		 case LED2 :
				Data_buffer_Transmit[4] = rx_raw_Frame[4] ; //resend value of led
				lengthTxData = 4 + 1;  //increment length because send a value of led



				HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,rx_raw_Frame[4] & 1 );//Change value of led
				//sendAndWaitIfNotReadyAndValidCommand(lengthTxData);
				//If you don't want answer just
				endProcessCommandAllowReceiveAgain();
		 break;
		 	case START_FPGA :

		 		HAL_SPI_DeInit(&hspi2);

				//Nce FPGA   enable
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,0);

				//short pulse nconfig on release reconfigure fpga
				//HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,0);
				Delay(1000);
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,1);

				Delay(5000);

				//Restart SPI1 to receive data from FPGA
				__SPI1_RELEASE_RESET();
				MX_SPI1_Init();

				endProcessCommandAllowReceiveAgain();

		 	break;
		 	case STOP_FPGA :
		 		//Stop SPI1  ( receive data from FPGA)
		 		//To avoid some problem, when reset fpga (upload) 100ns pulse generate fake edge and shift (1bit) the reveived data
		 		__SPI1_FORCE_RESET();

		 		//HAL_SPI_MspDeInit(&hspi1);
		 		//Nce FPGA   disable
				HAL_GPIO_WritePin(GPIOD,GPIO_PIN_2,1);
				//nconfig on release reconfigure fpga
				HAL_GPIO_WritePin(GPIOC,GPIO_PIN_0,0);

				Delay(5000); //wait to be sure that the fpga was stopped

				MX_SPI2_Init();
				sFLASH_READID(); //Must read ID to wake up the memory
				Delay(5000);
				//HAL_SPI_MspInit(&hspi1);
				endProcessCommandAllowReceiveAgain();

		 	break;

		 	case ERASE_FIRMWARE :
		 		Data_buffer_Transmit[4] = 1;
		 		lengthTxData =   4 + 1;  //increment length because send a value (1=ok)
		 		//sFLASH_ERASE_BULK(); //slow replaced by just erasing 2 sectors
		 		sFLASH_ERASE();
		 		mustValidCommandAfterSend = 1;
		 		sendUSB(Data_buffer_Transmit,lengthTxData);

		 	break;
		 	case READ_PAGE_FIRMWARE :

		 		if (rx_raw_Frame[6] <= 31) {//Just check number of page if not less than 31 do nothing
		 			lengthTxData =   (uint32_t)rx_raw_Frame[6] * 256;

		 			Data_buffer_Transmit[4] = rx_raw_Frame[4]; //low byte Number of the first page
		 			Data_buffer_Transmit[5] = rx_raw_Frame[5]; //high byte Number of the first page
		 			Data_buffer_Transmit[6] = rx_raw_Frame[6]; //Nb of page

		 			addr=lowHighByteToInt(rx_raw_Frame[4],rx_raw_Frame[5])*256;
					sFLASH_Read(*(&Data_buffer_Transmit)+7,addr,lengthTxData);

					mustValidCommandAfterSend = 1;
					sendUSB(Data_buffer_Transmit,lengthTxData+7);

		 		}else{
		 			Data_buffer_Transmit[2] = LOBYTE(COMMAND_NOT_FOUND);
		 			Data_buffer_Transmit[3] = HIBYTE(COMMAND_NOT_FOUND);
		 			mustValidCommandAfterSend = 1;
		 			sendUSB(Data_buffer_Transmit,4);
		 		}

		 	break;
		 	case WRITE_PAGE_FIRMWARE :
		 		//Data_buffer_Transmit[4] = rx_raw_Frame[4]; //low byte Number of the first page
		 		//Data_buffer_Transmit[5] = rx_raw_Frame[5]; //high byte Number of the first page
		 		//Data_buffer_Transmit[6] = rx_raw_Frame[6]; //Nb of page  (31 max)

	 			addr=lowHighByteToInt(rx_raw_Frame[4],rx_raw_Frame[5])*256;
		 		for (int i=0;i< rx_raw_Frame[6];i++){
		 			sFLASH_WritePage(*(&rx_raw_Frame)+7+i*256,addr+i*256,256);
		 		}

		 		endProcessCommandAllowReceiveAgain();

		 	break;

		 	case GET_VERSION_NUMBER :
					lengthTxData = 18;
					memcpy(&Data_buffer_Transmit[4],&versionNumber,lengthTxData);
					mustValidCommandAfterSend = 1;
					sendUSB(Data_buffer_Transmit,lengthTxData+4);
			break;


		 	default:
				//mustValidCommandAfterSend = 1;
				//sendAndWaitIfNotReady(lengthTxData);
				//If you don't want answer just
				endProcessCommandAllowReceiveAgain();


	}
}

void sendAndWaitIfNotReady(uint16_t size){

	//if transmit is in progress wait for
	while( (Data_buffer_Transmit_Start_Transmit == 1)  || (Data_buffer_Transmit_InProgress == 1 ));
		Data_buffer_Transmit[0] = LOBYTE(size);  //Set the true lenght of trame in the header (first two byte)
		Data_buffer_Transmit[1] = HIBYTE(size);
		Data_buffer_Transmit_length = size;
		Data_buffer_Transmit_Start_Transmit = 1;
}


/**
  * @brief  USBD_RAW_SOF
  *         handle SOF event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_RAW_SOF (USBD_HandleTypeDef *pdev)
{

//Start of frame


  return USBD_OK;
}






/**
  * @}
  */ 

/** @defgroup USBD_RAW_Private_Variables
  * @{
  */ 

USBD_ClassTypeDef  USBD_RAW_ClassDriver = 
{
  USBD_RAW_Init,
  USBD_RAW_DeInit,
  USBD_RAW_Setup,
  USBD_RAW_EP0_TxReady,  
  USBD_RAW_EP0_RxReady,
  USBD_RAW_DataIn,
  USBD_RAW_DataOut,
  USBD_RAW_SOF,
  USBD_RAW_IsoINIncomplete,
  USBD_RAW_IsoOutIncomplete,      
  USBD_RAW_GetCfgDesc,
  USBD_RAW_GetCfgDesc, 
  USBD_RAW_GetCfgDesc,
  USBD_RAW_GetDeviceQualifierDesc,
};

#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
/* USB TEMPLATE device Configuration Descriptor */
static uint8_t USBD_RAW_CfgDesc[USB_RAW_CONFIG_DESC_SIZ] =
{
  0x09, /* bLength: Configuation Descriptor size */
  USB_DESC_TYPE_OTHER_SPEED_CONFIGURATION, /* bDescriptorType: Configuration */
  USB_RAW_CONFIG_DESC_SIZ,
  /* wTotalLength: Bytes returned */
  0x00,
  0x01,         /*bNumInterfaces: 1 interface*/
  0x01,         /*bConfigurationValue: Configuration value*/
  0x02,         /*iConfiguration: Index of string descriptor describing the configuration*/
  0xC0,         /*bmAttributes: bus powered and Supprts Remote Wakeup */
  0x32,         /*MaxPower 100 mA: this current is used for detecting Vbus*/
  /* 09 */
  

   /******************** 1 interface  ********************/
  0x09,   /* bLength: Interface Descriptor size */
  0x04,   /* bDescriptorType: */
  0x00,   /* bInterfaceNumber: Number of Interface */
  0x00,   /* bAlternateSetting: Alternate setting */
  0x02,   /* bNumEndpoints*/
  0xFF,   /* bInterfaceClass: Vendor  Class */
  0x00,   /* bInterfaceSubClass : none*/
  0x00,   /* nInterfaceProtocol */
  USBD_IDX_INTERFACE_STR,          /* iInterface: */
  /******************** Some interface over WinUSB Endpoints ********************/
  0x07,   /*Endpoint descriptor length = 7*/
  0x05,   /*Endpoint descriptor type */
  RAW_EPIN_ADDR,   /*Endpoint address (IN, address 1) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(RAW_EP_SIZE),
  HIBYTE(RAW_EP_SIZE),
  0x00,   /*Polling interval in milliseconds */
  
  0x07,   /*Endpoint descriptor length = 7 */
  0x05,   /*Endpoint descriptor type */
  RAW_EPOUT_ADDR,   /*Endpoint address (OUT, address 1) */
  0x02,   /*Bulk endpoint type */
  LOBYTE(RAW_EP_SIZE),
  HIBYTE(RAW_EP_SIZE),
  0x00,    /*Polling interval in milliseconds*/
 
};
  
#if defined ( __ICCARM__ ) /*!< IAR Compiler */
  #pragma data_alignment=4   
#endif
/* USB Standard Device Descriptor */
static uint8_t USBD_RAW_DeviceQualifierDesc[USB_LEN_DEV_QUALIFIER_DESC] =
{
  USB_LEN_DEV_QUALIFIER_DESC,
  USB_DESC_TYPE_DEVICE_QUALIFIER,
  0x00,
  0x02,
  0x00,
  0x00,
  0x00,
  0x40, //64bytes per packet
  0x01,
  0x00,
};

/**
  * @}
  */ 

/** @defgroup USBD_RAW_Private_Functions
  * @{
  */ 

/**
  * @brief  USBD_RAW_Init
  *         Initialize the TEMPLATE interface
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_RAW_Init (USBD_HandleTypeDef *pdev, 
                               uint8_t cfgidx)
{


    /* Open EP OUT */
    USBD_LL_OpenEP(pdev,
                   RAW_EPOUT_ADDR,
                   USBD_EP_TYPE_BULK,
                   RAW_EP_SIZE);

    /* Open EP IN */
    USBD_LL_OpenEP(pdev,
                   RAW_EPIN_ADDR,
                   USBD_EP_TYPE_BULK,
                   RAW_EP_SIZE);

    //Prepare to receive a packet
    USBD_LL_PrepareReceive(pdev,RAW_EPOUT_ADDR,rx_raw_packet,RAW_EP_SIZE);
  return USBD_OK;
}

/**
  * @brief  USBD_RAW_Init
  *         DeInitialize the TEMPLATE layer
  * @param  pdev: device instance
  * @param  cfgidx: Configuration index
  * @retval status
  */
static uint8_t  USBD_RAW_DeInit (USBD_HandleTypeDef *pdev, 
                                 uint8_t cfgidx)
{
	/* Close WinUSBComm EPs */
  USBD_LL_CloseEP(pdev, RAW_EPOUT_ADDR);
  USBD_LL_CloseEP(pdev, RAW_EPIN_ADDR);

  /* Free WinUSBComm Class Resources */
  pdev->pClassData  = NULL;

  return USBD_OK;

}
static uint8_t  USBD_RAW_Setup (USBD_HandleTypeDef *pdev,
                                USBD_SetupReqTypedef *req)
{

  switch (req->bmRequest & USB_REQ_TYPE_MASK)
  {
  case USB_REQ_TYPE_CLASS :
    switch (req->bRequest)
    {

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;
    }
    break;

  case USB_REQ_TYPE_STANDARD:
    switch (req->bRequest)
    {HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_10);

    default:
      USBD_CtlError (pdev, req);
      return USBD_FAIL;
    }
  }
  return USBD_OK;
}


/**
  * @brief  USBD_RAW_GetCfgDesc 
  *         return configuration descriptor
  * @param  length : pointer data length
  * @retval pointer to descriptor buffer
  */
static uint8_t  *USBD_RAW_GetCfgDesc (uint16_t *length)
{
  *length = sizeof (USBD_RAW_CfgDesc);
  return USBD_RAW_CfgDesc;
}

/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_RAW_DeviceQualifierDescriptor (uint16_t *length)
{
  *length = sizeof (USBD_RAW_DeviceQualifierDesc);
  return USBD_RAW_DeviceQualifierDesc;
}



/**
  * @brief  USBD_RAW_EP0_RxReady
  *         handle EP0 Rx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_RAW_EP0_RxReady (USBD_HandleTypeDef *pdev)
{

  return USBD_OK;
}
/**
  * @brief  USBD_RAW_EP0_TxReady
  *         handle EP0 TRx Ready event
  * @param  pdev: device instance
  * @retval status
  */
static uint8_t  USBD_RAW_EP0_TxReady (USBD_HandleTypeDef *pdev)
{

  return USBD_OK;
}

/**
  * @brief  USBD_RAW_IsoINIncomplete
  *         handle data ISO IN Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_RAW_IsoINIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}
/**
  * @brief  USBD_RAW_IsoOutIncomplete
  *         handle data ISO OUT Incomplete event
  * @param  pdev: device instance
  * @param  epnum: endpoint index
  * @retval status
  */
static uint8_t  USBD_RAW_IsoOutIncomplete (USBD_HandleTypeDef *pdev, uint8_t epnum)
{

  return USBD_OK;
}


/**
* @brief  DeviceQualifierDescriptor 
*         return Device Qualifier descriptor
* @param  length : pointer data length
* @retval pointer to descriptor buffer
*/
uint8_t  *USBD_RAW_GetDeviceQualifierDesc (uint16_t *length)
{
  *length = sizeof (USBD_RAW_DeviceQualifierDesc);
  return USBD_RAW_DeviceQualifierDesc;
}

/**
  * @}
  */ 


/**
  * @}
  */ 


/**
  * @}
  */ 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

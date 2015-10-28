/**
  ******************************************************************************
  * @file           : usbd_desc.h
  * @date           : 17/02/2015 16:42:58  
  * @version        : v1.0_Cube
  * @brief          : Header for usbd_desc file.
  ******************************************************************************
  * COPYRIGHT(c) 2015 STMicroelectronics
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  * 1. Redistributions of source code must retain the above copyright notice,
  * this list of conditions and the following disclaimer.
  * 2. Redistributions in binary form must reproduce the above copyright notice,
  * this list of conditions and the following disclaimer in the documentation
  * and/or other materials provided with the distribution.
  * 3. Neither the name of STMicroelectronics nor the names of its contributors
  * may be used to endorse or promote products derived from this software
  * without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __USBD_DESC__H__
#define __USBD_DESC__H__

/* Includes ------------------------------------------------------------------*/
#include "usbd_def.h"

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @{
  */
  
/** @defgroup USB_DESC
  * @brief general defines for the usb device library file
  * @{
  */ 

/** @defgroup USB_DESC_Exported_Defines
  * @{
  */

/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_TypesDefinitions
  * @{
  */
/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_Macros
  * @{
  */ 
/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_Variables
  * @{
  */

#define USBD_VID     1155
#define USBD_LANGID_STRING     1033
#define USBD_MANUFACTURER_STRING     "OPALE SECURITY"
#define USBD_PID_FS     0xFFFF
#define USBD_PRODUCT_STRING_FS     "HARDSPLOIT TOOL"
#define USBD_CONFIGURATION_STRING_FS     "HARDSPLOIT Config"
#define USBD_INTERFACE_STRING_FS     "HARDSPLOIT Interface"

#define USBD_SERIALNUMBER_STRING_FS     ""
#define USBD_SERIALNUMBER_STRING_FS_SIZE 17
extern USBD_DescriptorsTypeDef FS_Desc;
/**
  * @}
  */ 

/** @defgroup USBD_DESC_Exported_FunctionsPrototype
  * @{
  */ 

/**
  * @}
  */ 

#endif /* __USBD_DESC_H */

/**
  * @}
  */ 

/**
* @}
*/ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

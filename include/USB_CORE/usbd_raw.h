/**
  ******************************************************************************
  * @file    usbd_template_core.h
  * @author  MCD Application Team
  * @version V2.0.0
  * @date    18-February-2014
  * @brief   header file for the usbd_template_core.c file.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2014 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */ 

/* Includes ------------------------------------------------------------------*/

#ifndef __USB_RAW_CORE_H_
#define __USB_RAW_CORE_H_

#include  "usbd_ioreq.h"

extern USBD_ClassTypeDef  USBD_RAW_ClassDriver;
#define RAW_EP_SIZE                 64
#define RAW_EPIN_ADDR                 0x81
#define RAW_EPOUT_ADDR                0x02


#define USB_RAW_CONFIG_DESC_SIZ       (9 + 9+7+7)
#define  SIZE_FRAME 8192


void sendAndWaitIfNotReadyAndValidCommand(uint16_t size);
void endProcessCommandAllowReceiveAgain();
void sendAndWaitIfNotReady(uint16_t size);
void   processCommand();
#endif  // __USB_TEMPLATE_CORE_H_

/**
  * @}
  */ 

/**
  * @}
  */ 
  
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

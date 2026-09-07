/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32g4xx_ll_adc.h"
#include "stm32g4xx_ll_dac.h"
#include "stm32g4xx_ll_dma.h"
#include "stm32g4xx_ll_i2c.h"
#include "stm32g4xx_ll_rcc.h"
#include "stm32g4xx_ll_bus.h"
#include "stm32g4xx_ll_crs.h"
#include "stm32g4xx_ll_system.h"
#include "stm32g4xx_ll_exti.h"
#include "stm32g4xx_ll_cortex.h"
#include "stm32g4xx_ll_utils.h"
#include "stm32g4xx_ll_pwr.h"
#include "stm32g4xx_ll_tim.h"
#include "stm32g4xx_ll_usart.h"
#include "stm32g4xx_ll_gpio.h"

#if defined(USE_FULL_ASSERT)
#include "stm32_assert.h"
#endif /* USE_FULL_ASSERT */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW3_Pin LL_GPIO_PIN_2
#define SW3_GPIO_Port GPIOE
#define SW4_Pin LL_GPIO_PIN_3
#define SW4_GPIO_Port GPIOE
#define ENC2_Pin LL_GPIO_PIN_4
#define ENC2_GPIO_Port GPIOE
#define SW5_Pin LL_GPIO_PIN_13
#define SW5_GPIO_Port GPIOC
#define BC_Pin LL_GPIO_PIN_5
#define BC_GPIO_Port GPIOA
#define BL_Pin LL_GPIO_PIN_7
#define BL_GPIO_Port GPIOA
#define LED1_Pin LL_GPIO_PIN_0
#define LED1_GPIO_Port GPIOB
#define B0_Pin LL_GPIO_PIN_10
#define B0_GPIO_Port GPIOB
#define B1_Pin LL_GPIO_PIN_11
#define B1_GPIO_Port GPIOB
#define B2_Pin LL_GPIO_PIN_13
#define B2_GPIO_Port GPIOB
#define AC_Pin LL_GPIO_PIN_8
#define AC_GPIO_Port GPIOC
#define AL_Pin LL_GPIO_PIN_9
#define AL_GPIO_Port GPIOC
#define A0_Pin LL_GPIO_PIN_8
#define A0_GPIO_Port GPIOA
#define A1_Pin LL_GPIO_PIN_9
#define A1_GPIO_Port GPIOA
#define A2_Pin LL_GPIO_PIN_10
#define A2_GPIO_Port GPIOA
#define CHRG_Pin LL_GPIO_PIN_10
#define CHRG_GPIO_Port GPIOC
#define PEN_Pin LL_GPIO_PIN_11
#define PEN_GPIO_Port GPIOC
#define BLB6_Pin LL_GPIO_PIN_6
#define BLB6_GPIO_Port GPIOB
#define LED2_Pin LL_GPIO_PIN_7
#define LED2_GPIO_Port GPIOB
#define SW2_Pin LL_GPIO_PIN_0
#define SW2_GPIO_Port GPIOE
#define ENC1_Pin LL_GPIO_PIN_1
#define ENC1_GPIO_Port GPIOE
#ifndef NVIC_PRIORITYGROUP_0
#define NVIC_PRIORITYGROUP_0         ((uint32_t)0x00000007) /*!< 0 bit  for pre-emption priority,
                                                                 4 bits for subpriority */
#define NVIC_PRIORITYGROUP_1         ((uint32_t)0x00000006) /*!< 1 bit  for pre-emption priority,
                                                                 3 bits for subpriority */
#define NVIC_PRIORITYGROUP_2         ((uint32_t)0x00000005) /*!< 2 bits for pre-emption priority,
                                                                 2 bits for subpriority */
#define NVIC_PRIORITYGROUP_3         ((uint32_t)0x00000004) /*!< 3 bits for pre-emption priority,
                                                                 1 bit  for subpriority */
#define NVIC_PRIORITYGROUP_4         ((uint32_t)0x00000003) /*!< 4 bits for pre-emption priority,
                                                                 0 bit  for subpriority */
#endif

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

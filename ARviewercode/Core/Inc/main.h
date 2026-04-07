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

/*
 * File: main.h
 * Purpose: Central project header that exposes HAL handles, GPIO pin mappings,
 * and common firmware declarations used across the application.
 *
 * Functions in this file:
 * - HAL_TIM_MspPostInit: post-initializes timer GPIO after timer setup.
 * - Error_Handler: reports a fatal firmware error and halts execution.
 */
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

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
#define LSM6DR_INT1_Pin GPIO_PIN_2
#define LSM6DR_INT1_GPIO_Port GPIOE
#define LSM6DR_INT2_Pin GPIO_PIN_3
#define LSM6DR_INT2_GPIO_Port GPIOE
#define FMC_RST_Pin GPIO_PIN_3
#define FMC_RST_GPIO_Port GPIOD
#define LIS3MDLTR_INT_Pin GPIO_PIN_0
#define LIS3MDLTR_INT_GPIO_Port GPIOE
#define LIS3MDLTR_DRDY_Pin GPIO_PIN_1
#define LIS3MDLTR_DRDY_GPIO_Port GPIOE



/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "i2c.h"
#include "rtc.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_lcd.h"
#include "stm32f1xx_hal.h"
#include "rtc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
#define BCD2DEC(x) (((x) >> 4) * 10 + ((x) & 0x0F))

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
	char time[10];
	char date[10];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void set_timeFromCompile(void)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};

	int hour, minute, second, day, year;
	char month_str[4];
	 sscanf(__TIME__, "%d:%d:%d", &hour, &minute, &second);
	 sscanf(__DATE__, "%3s %d %d", month_str, &day, &year);

	 sTime.Hours = ((hour / 10) << 4) | (hour % 10);
	sTime.Minutes = ((minute / 10) << 4) | (minute % 10);
	sTime.Seconds = ((second / 10) << 4) | (second % 10);

	sDate.Date = ((day / 10) << 4) | (day % 10);
	sDate.Year = ((year - 2000) / 10 << 4) | ((year - 2000) % 10);

	if (!strcmp(month_str, "Jan"))
		sDate.Month = RTC_MONTH_JANUARY;
	else if (!strcmp(month_str, "Feb"))
		sDate.Month = RTC_MONTH_FEBRUARY;
	else if (!strcmp(month_str, "Mar"))
		sDate.Month = RTC_MONTH_MARCH;
	else if (!strcmp(month_str, "Apr"))
		sDate.Month = RTC_MONTH_APRIL;
	else if (!strcmp(month_str, "May"))
		sDate.Month = RTC_MONTH_MAY;
	else if (!strcmp(month_str, "Jun"))
		sDate.Month = RTC_MONTH_JUNE;
	else if (!strcmp(month_str, "Jul"))
		sDate.Month = RTC_MONTH_JULY;
	else if (!strcmp(month_str, "Aug"))
		sDate.Month = RTC_MONTH_AUGUST;
	else if (!strcmp(month_str, "Sep"))
		sDate.Month = RTC_MONTH_SEPTEMBER;
	else if (!strcmp(month_str, "Oct"))
		sDate.Month = RTC_MONTH_OCTOBER;
	else if (!strcmp(month_str, "Nov"))
		sDate.Month = RTC_MONTH_NOVEMBER;
	else if (!strcmp(month_str, "Dec"))
		sDate.Month = RTC_MONTH_DECEMBER;

	HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD);
	HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD);

	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
}
void set_time(void)
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	/*Initialize RTC and set time and date*/

	sTime.Hours = 0x12;
	sTime.Minutes = 0x45;
	sTime.Seconds = 0x00;

	if(HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
	sDate.WeekDay = RTC_WEEKDAY_WEDNESDAY;
	sDate.Month = RTC_MONTH_FEBRUARY;
	sDate.Date = 0x04;
	sDate.Year = 0x26;
	if(HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
	{
		Error_Handler();
	}
	HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
}

void set_alarm(void)
{
	RTC_AlarmTypeDef sAlarm = {0};
	  /** Enable the Alarm A
	  */
	  sAlarm.AlarmTime.Hours = 0x10;
	  sAlarm.AlarmTime.Minutes = 0x21;
	  sAlarm.AlarmTime.Seconds = 0x0;
	  sAlarm.Alarm = RTC_ALARM_A;
	  if (HAL_RTC_SetAlarm_IT(&hrtc, &sAlarm, RTC_FORMAT_BCD) != HAL_OK)
	  	  {
	  	    Error_Handler();
	  	  }
}

void get_Time()
{
	RTC_TimeTypeDef gTime;
	    RTC_DateTypeDef gDate;

	    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
	    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);

	    sprintf(time, "%02d:%02d:%02d",
	            BCD2DEC(gTime.Hours),
	            BCD2DEC(gTime.Minutes),
	            BCD2DEC(gTime.Seconds));

	    sprintf(date, "%02d-%02d-20%02d",
	            BCD2DEC(gDate.Date),
	            BCD2DEC(gDate.Month),
	            BCD2DEC(gDate.Year)
//				BCD2DEC(gDate.WeekDay)
				);
//	RTC_DateTypeDef gDate;
//	RTC_TimeTypeDef gTime;
//
//	HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
//	HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);
//
//	sprintf(time, "%02d:%02d:%02d", BCD2DEC(gTime.Hours),BCD2DEC(gTime.Minutes),BCD2DEC(gTime.Seconds));
//	//dd-mm-yy-wd
//	sprintf(date, "%02d-%02d-%02d-%02d", BCD2DEC(gDate.Date),BCD2DEC(gDate.Month),2026 + BCD2DEC(gDate.Year), BCD2DEC(gDate.WeekDay));
}

void display_time(void)
{
	lcd_send_cmd(0x80);
	lcd_send_string(time);
	lcd_send_cmd(0xc0);
	lcd_send_string(date);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  lcd_init();
  /* USER CODE END 2 */
   if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != 0x32F2) //
  {
//	  set_time();
	   set_timeFromCompile();
  }
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
//#define STEFANI
  while (1)
  {
    /* USER CODE END WHILE */
#ifdef STEFANI
	  lcd_put_cursor(0, 0);
	  lcd_send_string("Hello Stefani <3!");
	  lcd_put_cursor(1, 0);
	  lcd_send_string("My wife.");
	  HAL_Delay(50);
#endif
//	  lcd_put_cursor(0, 0);
//	  lcd_send_string("Hello Stefani <3!");
	   get_Time();
	   display_time();
	   HAL_Delay(500);
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE|RCC_OSCILLATORTYPE_LSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

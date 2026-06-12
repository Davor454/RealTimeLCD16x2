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
#include "usart.h"
#include "gpio.h"
#include "tim.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "i2c_lcd.h"
#include "stm32f1xx_hal.h"
#include "rtc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "stm32f1xx_hal_rtc.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */


/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STEFANI
#define BCD2DEC(x) (((x) >> 4) * 10 + ((x) & 0x0F))
#define DEC2BCD(x) ((((x) / 10) << 4) | ((x) % 10))
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
	extern UART_HandleTypeDef huart1;
	char uart_rx_char;
	char uart_buffer[40];
	uint8_t uart_index = 0;
/* USER CODE BEGIN PV */
	char time[16];
	char date[16];
	char temp_buffer[10];
	char hum_buffer[10]; // increased size to avoid overflow
	uint8_t Rh_byte1, Rh_byte2, Temp_byte1, Temp_byte2;
	uint16_t SUM;
	uint8_t Presence = 0;
	int Temperature = 0;
	int Humidity = 0;

RTC_TimeTypeDef prevTime = {0};
RTC_DateTypeDef prevDate = {0};
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/

/* USER CODE BEGIN PFP */
void SystemClock_Config(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void parse_and_set_time(char *str);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/

/* USER CODE BEGIN 0 */
/*Store date and weekday int BKP register*/
static void RTC_Store_DateWeekDayIntoBkpReg(RTC_DateTypeDef* dateRtc)
{
	uint32_t dateToStore;

	memcpy(&dateToStore, dateRtc, sizeof(dateToStore));

	HAL_RTCEx_BKUPWrite( &hrtc, RTC_BKP_DR2, (dateToStore >> 16)	);

	HAL_RTCEx_BKUPWrite( &hrtc, RTC_BKP_DR3, (dateToStore & 0xFFFF) );

}

static void RTC_LoadDateFromBkpReg(RTC_DateTypeDef* dateRtc)
{
	uint32_t dateToStore;
	if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1)!=0x32F2)
	{
		dateRtc->WeekDay = RTC_WEEKDAY_SATURDAY;
		dateRtc->Month   = RTC_MONTH_JANUARY;
		dateRtc->Date    = 1;
		dateRtc->Year    = 0;  // = 2000
		return;
	}
	dateToStore = HAL_RTCEx_BKUPRead( &hrtc, RTC_BKP_DR3 );

	dateToStore |= ( HAL_RTCEx_BKUPRead( &hrtc, RTC_BKP_DR2 ) << 16 );

	memcpy( dateRtc, &dateToStore, sizeof(dateToStore) );
}

/*CUSTOM CHARACTER CREATION FOR CELSIUS*/

void LCD_CreateCustomChar() {
	uint8_t degree_char[8] = {
	    0x00,
	    0x06,
	    0x09,
	    0x09,
	    0x06,
	    0x00,
	    0x00,
	    0x00
	};
    // Load custom char at location 0
    lcd_send_cmd(0x40); // Set CGRAM address to 0
    for (int i = 0; i < 8; i++) {
    	lcd_send_char(degree_char[i]);
    }
}

void parse_and_set_time(char *str)
{
    int y, m, d, hh, mm, ss;

    if (sscanf(str, "T:%d-%d-%d %d:%d:%d",
               &y, &m, &d, &hh, &mm, &ss) == 6)//&y, &m, &d, &weekday, &hh, &mm, &ss) == 7)
    {
        RTC_TimeTypeDef time = {0};
        RTC_DateTypeDef date = {0};

        time.Hours   = ((hh / 10) << 4) | (hh % 10);
		time.Minutes = ((mm / 10) << 4) | (mm % 10);
		time.Seconds = ((ss / 10) << 4) | (ss % 10);

		date.Date  = ((d / 10) << 4) | (d % 10);
		date.Month = ((m / 10) << 4) | (m % 10); //m;
		date.Year  = ((y - 2000) / 10 << 4) | ((y - 2000) % 10);
//        date.WeekDay = weekday;


        HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BCD);
        HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);

        RTC_Store_DateWeekDayIntoBkpReg(&date);
        RTC_SaveDayCounter();
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, 0x32F2);
    }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
if (huart->Instance == USART1)
    {
        if (uart_rx_char == '\n')
        {
            uart_buffer[uart_index] = '\0';
            parse_and_set_time(uart_buffer);
            uart_index = 0;
        }
        else
        {
            if (uart_index < sizeof(uart_buffer) - 1)
            {
                uart_buffer[uart_index++] = uart_rx_char;
            }
        }

        HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart_rx_char, 1);
    }
}
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

const char* WeekDayToStr(uint8_t weekday)
{
	switch(weekday)
	{
	case RTC_WEEKDAY_MONDAY: return "Mon";
	case RTC_WEEKDAY_TUESDAY: return "Tue";
	case RTC_WEEKDAY_WEDNESDAY: return "Wed";
	case RTC_WEEKDAY_THURSDAY: return "Thu";
	case RTC_WEEKDAY_FRIDAY: return "Fri";
	case RTC_WEEKDAY_SATURDAY: return "Sat";
	case RTC_WEEKDAY_SUNDAY: return "Sun";
	default:	return "???";
	}
}

void get_Time()
{
    RTC_TimeTypeDef gTime;
    RTC_DateTypeDef gDate;

    HAL_RTC_GetTime(&hrtc, &gTime, RTC_FORMAT_BCD);
    HAL_RTC_GetDate(&hrtc, &gDate, RTC_FORMAT_BCD);

    sprintf(time, "%02d:%02d:%02d---T:%02d", //-H:%02d%%
            BCD2DEC(gTime.Hours),
            BCD2DEC(gTime.Minutes),
            BCD2DEC(gTime.Seconds),
			Temperature);



    sprintf(date, "%02d-%02d-20%02d---%s",
            BCD2DEC(gDate.Date),
            BCD2DEC(gDate.Month),
            BCD2DEC(gDate.Year),
            WeekDayToStr(gDate.WeekDay));

    // Now display `display_buffer` and `date` separately on LCD
}

void display_time(void)
{
	lcd_send_cmd(0x80);
	lcd_send_string(time);
	lcd_send_char(0);
	lcd_send_char('C');
	lcd_send_cmd(0xc0);
	lcd_send_string(date);
}

void display_date(void)
{
	lcd_send_cmd(0xc0);
	lcd_send_string(date);
}

void display_humidity(void)
{
	lcd_send_cmd(0x88);
	uint8_t i;
	for(i = 8; i< 16; i++)
	{
		lcd_send_char(' ');
	}
	sprintf(hum_buffer, "-H:%02d%%", Humidity);
	lcd_send_cmd(0x88);
	lcd_send_string(hum_buffer);
}

void RTC_CheckDateIncrement(void)
{
	RTC_TimeTypeDef currentTime;
	RTC_DateTypeDef currentDate;

	 HAL_RTC_GetTime(&hrtc, &currentTime, RTC_FORMAT_BCD);

	 if(prevTime.Hours == 0x23 &&
	    prevTime.Minutes == 0x59 &&
		prevTime.Seconds == 0x59 &&
		currentTime.Hours == 0x0 &&
		currentTime.Minutes == 0x0)
	 {
		 HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BCD);

//		 HAL_RTC_SetDate(&hrtc, &currentDate, RTC_FORMAT_BCD);
		 RTC_Store_DateWeekDayIntoBkpReg(&currentDate);
		 HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR10, RTC_ReadTimeCounter(&hrtc)/86400);
	 }
	 prevTime = currentTime;
}

void RTC_CheckDateIncrement_Days(void)
{
    RTC_DateTypeDef currentDate;

    HAL_RTC_GetDate(&hrtc, &currentDate, RTC_FORMAT_BCD);

    if(memcmp(&currentDate, &prevDate, sizeof(RTC_DateTypeDef)) != 0)
    {
        RTC_Store_DateWeekDayIntoBkpReg(&currentDate);

        prevDate = currentDate;
    }
}

void delay_us (uint16_t us)
{
    __HAL_TIM_SET_COUNTER(&htim1, 0);  // Clear timer counter (Replace htim1 if using another timer)
    while (__HAL_TIM_GET_COUNTER(&htim1) < us); // Wait until counter reaches target microseconds
}

void Set_Pin_Output (GPIO_TypeDef *GPIOx, uint16_t Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // Push-pull output
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

void Set_Pin_Input (GPIO_TypeDef *GPIOx, uint16_t Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;     // Floating input
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

uint8_t DHT11_Start (void)
{
    uint8_t Response = 0;
    Set_Pin_Output(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin);   // Set pin as output
    HAL_GPIO_WritePin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin, GPIO_PIN_RESET); // Pull pin LOW
    HAL_Delay(18);                                        // Wait 18 milliseconds
    HAL_GPIO_WritePin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin, GPIO_PIN_SET);   // Pull pin HIGH
    delay_us(20);                                         // Wait 20 microseconds

    Set_Pin_Input(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin);    // Set pin as input
    delay_us(40);

    if (!(HAL_GPIO_ReadPin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin))) // Check if pin is LOW
    {
        delay_us(80);
        if ((HAL_GPIO_ReadPin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin))) Response = 1; // Check if pin goes HIGH
        else Response = -1;
    }

    // Wait for the pin to go back low before reading data bits
    uint16_t timeout = 0;
    while ((HAL_GPIO_ReadPin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin)) && timeout < 100) {
        delay_us(1);
        timeout++;
    }

    return Response;
}

uint8_t DHT11_Read (void)
{
    uint8_t i, j;
    for (j=0; j<8; j++)
    {
        while (!(HAL_GPIO_ReadPin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin))); // Wait for pin to go HIGH
        delay_us(40); // Wait for 40 microseconds

        if (!(HAL_GPIO_ReadPin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin))) // If pin is LOW, bit is 0
        {
            i &= ~(1<<(7-j));
        }
        else // If pin is still HIGH, bit is 1
        {
            i |= (1<<(7-j));
            while ((HAL_GPIO_ReadPin(DHT11_PIN_GPIO_Port, DHT11_PIN_Pin))); // Wait for pin to go LOW
        }
    }
    return i;
}


void RTC_RestoreDateAfterReset(void)
{
    RTC_DateTypeDef date = {0};

    RTC_LoadDateFromBkpReg(&date);

//    uint32_t current_days = RTC_ReadTimeCounter(&hrtc) / 86400;

    HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BCD);

    prevDate = date;
//    RTC_CheckDateIncrement_Days();
//
//    RTC_Store_DateWeekDayIntoBkpReg(&date);
//
//    HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR10,current_days);
}
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
#define RTC_MAGIC 0x32F2

int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE BEGIN SysInit */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */

  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_RTC_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  lcd_init();
  /* USER CODE END Init */

  /* USER CODE BEGIN 2 */

  static uint32_t last_clock_tick = 0;
  static uint32_t last_dht_tick = 0;//? maybe not needed

  LCD_CreateCustomChar();

  HAL_UART_Receive_IT(&huart1, (uint8_t *)&uart_rx_char, 1);

  if(HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) != RTC_MAGIC) //
  {
	  printf("First boot!\r\n");

	  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_MAGIC);
  }
  else
  {
	  printf("Backup register survived reset\r\n");
	  RTC_RestoreDateAfterReset();
	  HAL_RTC_GetDate(&hrtc, &prevDate, RTC_FORMAT_BCD);
  }

  //START TIMER TIM1

  HAL_TIM_Base_Start(&htim1);

  /* USER CODE END 2 */

  /* Infinite loop */

  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  /* USER CODE BEGIN 3 */
#ifndef STEFANI
	  lcd_put_cursor(0, 0);
	  lcd_send_string("Hello Stefani <3!");
	  lcd_put_cursor(1, 0);
	  lcd_send_string("My wife.");
	  HAL_Delay(50);
#endif
//	  lcd_put_cursor(0, 0);
//	  lcd_send_string("Hello Stefani <3!");

	  if(HAL_GetTick() - last_dht_tick >= 2000) // read every 2 seconds
	  {
		  last_dht_tick = HAL_GetTick();

		  Presence = DHT11_Start();

		  if(Presence == 1)
		  {
			  Rh_byte1 = DHT11_Read();
			  Rh_byte2 = DHT11_Read();
			  Temp_byte1 = DHT11_Read();
			  Temp_byte2 = DHT11_Read();
			  SUM = DHT11_Read();
			  if (SUM == (Rh_byte1 + Rh_byte2 + Temp_byte1 + Temp_byte2))
			  {
				  Humidity = Rh_byte1;
				  Temperature = Temp_byte1;
			  }
		  }
	  }

	  if (HAL_GetTick() - last_clock_tick >= 1000) {// it's updated every 1 second(1000ms)
			last_clock_tick = HAL_GetTick();

			RTC_CheckDateIncrement_Days();//condition is true every 1000 ms or 1 second
			get_Time();
			display_time();
//			update_display_non_blocking();
//			display_date();

			/*DONT USE HAL_DELAY functions it messes up the seconds clock while displaying
			* INSTEAD REPLACE display_time function with a STATE MACHINE*/
		}

	  /* USER CODE END 3 */
  }
  /* USER CODE END WHILE */
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

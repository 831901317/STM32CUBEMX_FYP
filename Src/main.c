/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "fatfs.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "SDdriver.h"




/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
int fputc(int ch, FILE *f)    
{
    HAL_UART_Transmit(&huart1, (unsigned char *)&ch, 1, 0xFFFF);   
    return ch;
}
uint16_t uart_value[3];
uint8_t aRxBuffer1;	//uart rx buff 





void WritetoSD(BYTE write_buff[],uint8_t bufSize);
char SD_FileName[] = "hello.txt";
uint8_t WriteBuffer[] = "01 write buff to sd \r\n";

//uint8_t test_sd =0;	//用于测试格式化
uint8_t write_cnt =0;	//写SD卡次数




void WritetoSD(BYTE write_buff[],uint8_t bufSize)
{
	FATFS fs;
	FIL file;
	uint8_t res=0;
	UINT Bw;	
	
	res = SD_init();		//SD卡初始化
	
	if(res == 1)
	{
		printf("SD卡初始化失败! \r\n");		
	}
	else
	{
		printf("SD卡初始化成功！ \r\n");		
	}
	
	res=f_mount(&fs,"0:",1);		//挂载
	
//	if(test_sd == 0)		//用于测试格式化
	if(res == FR_NO_FILESYSTEM)		//没有文件系统，格式化
	{
//		test_sd =1;				//用于测试格式化
		printf("没有文件系统! \r\n");		
		res = f_mkfs("", 0, 0);		//格式化sd卡
		if(res == FR_OK)
		{
			printf("格式化成功! \r\n");		
			res = f_mount(NULL,"0:",1); 		//格式化后先取消挂载
			res = f_mount(&fs,"0:",1);			//重新挂载	
			if(res == FR_OK)
			{
				printf("SD卡已经成功挂载，可以进进行文件写入测试!\r\n");
			}	
		}
		else
		{
			printf("格式化失败! \r\n");		
		}
	}
	else if(res == FR_OK)
	{
		printf("挂载成功! \r\n");		
	}
	else
	{
		printf("挂载失败! \r\n");
	}	
	
	res = f_open(&file,SD_FileName,FA_OPEN_ALWAYS |FA_WRITE);
	if((res & FR_DENIED) == FR_DENIED)
	{
		printf("卡存储已满，写入失败!\r\n");		
	}
	
	f_lseek(&file, f_size(&file));//确保写词写入不会覆盖之前的数据
	if(res == FR_OK)
	{
		printf("打开成功/创建文件成功！ \r\n");		
		res = f_write(&file,write_buff,bufSize,&Bw);		//写数据到SD卡
		if(res == FR_OK)
		{
			printf("文件写入成功！ \r\n");			
		}
		else
		{
			printf("文件写入失败！ \r\n");
		}		
	}
	else
	{
		printf("打开文件失败!\r\n");
	}	
	
	f_close(&file);						//关闭文件		
	f_mount(NULL,"0:",1);		 //取消挂载
	
}


void Get_SDCard_Capacity(void)
{
	FRESULT result;
	FATFS FS;
	FATFS *fs;
	DWORD fre_clust,AvailableSize,UsedSize;  
	uint16_t TotalSpace;
	uint8_t res;
	
	res = SD_init();		//SD卡初始化
	if(res == 1)
	{
		printf("SD卡初始化失败! \r\n");		
	}
	else
	{
		printf("SD卡初始化成功！ \r\n");		
	}
	
	/* 挂载 */
	res=f_mount(&FS,"0:",1);		//挂载
	if (res != FR_OK)
	{
		printf("FileSystem Mounted Failed (%d)\r\n", result);
	}

	res = f_getfree("0:", &fre_clust, &fs);  /* 根目录 */
	if ( res == FR_OK ) 
	{
		TotalSpace=(uint16_t)(((fs->n_fatent - 2) * fs->csize ) / 2 /1024);
		AvailableSize=(uint16_t)((fre_clust * fs->csize) / 2 /1024);
		UsedSize=TotalSpace-AvailableSize;              
		/* Print free space in unit of MB (assuming 512 bytes/sector) */
		printf("\r\n%d MB total drive space.\r\n""%d MB available.\r\n""%d MB  used.\r\n",TotalSpace, AvailableSize,UsedSize);
	}
	else 
	{
		printf("Get SDCard Capacity Failed (%d)\r\n", result);
	}		
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
  MX_SPI1_Init();
  MX_FATFS_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
	
	HAL_UART_Receive_IT(&huart1,&aRxBuffer1,1); 	//enable uart	

	printf(" mian \r\n");

	Get_SDCard_Capacity();	//得到使用内存并选择格式化



  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		
		
		
		WritetoSD(WriteBuffer,sizeof(WriteBuffer));		

		
		
		HAL_Delay(500);
		WriteBuffer[0] = WriteBuffer[0] +10;
		WriteBuffer[1] = WriteBuffer[1] +10;
		write_cnt ++;
		
		while(write_cnt > 10)
		{	
			printf(" while \r\n");
			HAL_Delay(500);
		}		
		
		
		
		
		
		
		
		
		
		
    /* USER CODE END WHILE */

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

  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(SD_CS_GPIO_Port, SD_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : SD_CS_Pin */
  GPIO_InitStruct.Pin = SD_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(SD_CS_GPIO_Port, &GPIO_InitStruct);

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

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

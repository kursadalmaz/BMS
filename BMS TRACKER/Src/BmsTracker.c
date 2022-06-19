/*-----------------------------------------------------------------------------------------------------------------------------------------*																																																																					
* KÜRÞAD ALMAZ ----------------------------------------------------------------------------------------------------------------------------	
* OSMANÝYE KORKUT ATA ÜNÝVERSÝTESÝ EFFICIENCY CHALLENGE BMS ÝZLEME-------------------------------------------------------------------------*
*/
/* Includes ------------------------------------------------------------------*/																																						
#include "BmsTracker.h"
/* Private variables ---------------------------------------------------------*/
UART_HandleTypeDef UART_InitStruct;
/*BMS ýn AKS ile iletiþimini saðlayacak datalar............................. .*/

uint8_t requestcCellVolt[13]= {0xA5, 0x40, 0x95, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x82};
uint8_t requestcSoC[13]			= {0xA5, 0x40, 0x90, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7D};
uint8_t requestcTemp[13]		= {0xA5, 0x40, 0x92, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7F};
uint8_t BmsTemp = 0;
uint8_t responseMsg[150];																																					
																															           
/**
  * @brief  UART2 Interrupt Service Routine
  */
void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UART_InitStruct);
}

/**
  * @brief  Buradaki callback belirtilen adette byte ulastiginda HAL_UART_IRQHandler tarafindan cagrilir!
	*					Daily BMS cihazindan gelen verileri kesme yöntemi ile alinir.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	
  if (huart->Instance == USART2)
  {
    /* 100ms timeout ile 1 byte yolla */
    //HAL_UART_Transmit(huart, &responseMsg[6], 1, 100);
		//HAL_UART_Transmit(huart, &responseMsg[7], 1, 100);
    /* Kesme modunda ulasan 1 byte alma */ 
    HAL_UART_Receive_IT(huart, responseMsg, 150);
		
  }
}

/**
	* @brief Bu fonksiyon Daily BMS cihazi ile haberlesme saglamasi için UART çevre birimi yapilandirir
	*/
void BmsTracker_init(void)
{
	/*
		PA2 > USART2_TX
		PA3 > USART2_RX
		Interrupt ile
	*/
	
	/* UART gerceklemesi icin init etmemiz (oncul tanimlamamiz) gereken birimler */
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitTypeDef TXENABLEPIN_InitStruct;
	
	/* Birimlerin CLK sinyallerini enable (aktif) etmemiz gerekiyor! */
	/* GPIO A portunun clock'unu aktiflestir */
	__HAL_RCC_USART2_CLK_ENABLE();
	/* USART 2 clock'unu aktiflestir */
	__HAL_RCC_GPIOA_CLK_ENABLE();
	
	TXENABLEPIN_InitStruct.Pin 		= GPIO_PIN_4;
  TXENABLEPIN_InitStruct.Mode 	= GPIO_MODE_OUTPUT_PP;
  TXENABLEPIN_InitStruct.Pull 	= GPIO_NOPULL;
  TXENABLEPIN_InitStruct.Speed 	= GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &TXENABLEPIN_InitStruct);
	
  GPIO_InitStruct.Pin 			= GPIO_PIN_2 | GPIO_PIN_3;
  GPIO_InitStruct.Mode 			= GPIO_MODE_AF_PP;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
  GPIO_InitStruct.Pull 			= GPIO_PULLUP;
  GPIO_InitStruct.Speed 		= GPIO_SPEED_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
  
  UART_InitStruct.Instance 						= USART2;
  UART_InitStruct.Init.BaudRate 			= 9600;
  UART_InitStruct.Init.WordLength 		= UART_WORDLENGTH_8B;
  UART_InitStruct.Init.StopBits 			= UART_STOPBITS_1;
  UART_InitStruct.Init.Parity 				= UART_PARITY_NONE;
  UART_InitStruct.Init.Mode 					= UART_MODE_TX_RX;
  UART_InitStruct.Init.HwFlowCtl 			= UART_HWCONTROL_NONE;
  UART_InitStruct.Init.OverSampling 	= UART_OVERSAMPLING_16;

	HAL_UART_Init(&UART_InitStruct);

  /* USART2 icin kesme biriminin oncul tanimlanmasi */
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
	HAL_UART_Receive_IT(&UART_InitStruct, responseMsg, 150);
}

/**
	* @brief Bu fonksiyon Daily BMS cihazindan Hücre voltajlarinin istenmesi için ilgili veri yapisini gönderir
	*/
void BmsRequestData(void)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
	HAL_UART_Transmit(&UART_InitStruct,requestcCellVolt,13,0xffff);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
}

void BmsRequestTemp(void)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
	HAL_UART_Transmit(&UART_InitStruct,requestcTemp,13,0xffff);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
}

void BmsRequestSoC(void)
{
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_SET);
	HAL_UART_Transmit(&UART_InitStruct,requestcSoC,13,0xffff);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_4,GPIO_PIN_RESET);
}
/**
	* @brief Kesme ile alinan voltaj mesajlari 2 byte uzunlugundadir ve byte'lar toplanarak hücre voltajlari mV cinsinden elde edilir.
	* @param Bu fonksiyon mV cinsinden elde edilen voltajlarin degerlerini döndürür. Voltaj dizisi pointer degisken olarak verilir. 
	*/
void get_Frame1Volts(uint16_t *cellFrame1)
{
	uint16_t a;
	if(responseMsg[2]==0x95)
	{
		for(a=0;a<150;a++)
		{
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x01)
			{
				cellFrame1[0] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[1] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[2] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x02)
			{
				cellFrame1[3] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[4] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[5] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x03)
			{
				cellFrame1[6] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[7] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[8] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x04)
			{
				cellFrame1[9] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[10] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[11] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x05)
			{
				cellFrame1[12] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[13] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[14] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x06)
			{
				cellFrame1[15] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[16] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[17] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x07)
			{
				cellFrame1[18] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[19] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[20] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x08)
			{
				cellFrame1[21] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[22] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[23] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x09)
			{
				cellFrame1[24] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[25] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[26] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x0A)
			{
				cellFrame1[27] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[28] = responseMsg[a+4]<<8	| responseMsg[a+5];
				cellFrame1[29] = responseMsg[a+6]<<8	| responseMsg[a+7];
			}
			if (responseMsg[a]==0x08&&responseMsg[a+1]==0x0B)
			{
				cellFrame1[30] = responseMsg[a+2]<<8 | responseMsg[a+3];
				cellFrame1[31] = responseMsg[a+4]<<8	| responseMsg[a+5];
			}
		}
		for(a=0;a<150;a++)
		{
			responseMsg[a]=0;
		}
	}

}

void get_SoC(uint16_t *Soc_totVal)
{
	uint16_t a;
	for(a=0;a<150;a++)
	{
		if(responseMsg[a]==0x90)
		{
			if (responseMsg[a]==0x90&&responseMsg[a+1]==0x08)
			{
				Soc_totVal[0] = responseMsg[a+8]<<8 | responseMsg[a+9];
				
			}
			if(responseMsg[a]==0x90&&responseMsg[a+1]==0x08)
			{
				Soc_totVal[1] = responseMsg[a+2]<<8 | responseMsg[a+3];
			}
			
				if(responseMsg[a]==0x90&&responseMsg[a+1]==0x08)
			{
				Soc_totVal[2] = (responseMsg[a+6]<<8 | responseMsg[a+7])-30000;
			}
			
			for(a=0;a<150;a++)
			{
				responseMsg[a]=0;
			}
		}
	}

}

void get_BmsTemp(uint8_t *temp)
{
	
	uint16_t a;
	for(a=0;a<150;a++)
	{
		if(responseMsg[a]==0x92)
		{
			if (responseMsg[a]==0x92 && responseMsg[a+1]==0x08)
			{
				*temp = (responseMsg[a+2])-40; //40offset
			}
			
			for(a=0;a<150;a++)
			{
				responseMsg[a]=0;
			}

		}
	}
	
	
}


/*																																						*/

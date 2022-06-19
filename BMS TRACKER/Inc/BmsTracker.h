/*-----------------------------------------------------------------------------------------------------------------------------------------*																																																																					
* KÜRSAD ALMAZ ----------------------------------------------------------------------------------------------------------------------------	
* OSMANIYE KORKUT ATA ÜNIVERSITESI EFFICIENCY CHALLENGE BMS IZLEME-------------------------------------------------------------------------*
*/
/* Define to prevent recursive inclusion -------------------------------------*/
/*																																						*/
#ifndef __BmsTracker_init
#define __BmsTracker_init
/*																																						*/

/* Includes ------------------------------------------------------------------*/
/*																																						*/
#include "main.h" 
/*																																						*/
/*																																						*/
/* Private includes ----------------------------------------------------------*/
/*																																						*/

/*																																						*/
/* Exported types ------------------------------------------------------------*/
/*																																						*/

/*																																						*/
/* Exported constants --------------------------------------------------------*/
/*																																						*/

/*																																						*/
/* Exported macro ------------------------------------------------------------*/
/*																																						*/

/*																																						*/
/* Exported functions prototypes ---------------------------------------------*/
/*																																						*/
void BmsTracker_init(void);
void BmsRequestData(void);
void BmsRequestSoC(void);
void BmsRequestTemp(void);
void get_Frame1Volts(uint16_t *cellFrame1);
void get_SoC(uint16_t *Soc_totVal);
void get_BmsTemp(uint8_t *temp);
/*																																						*/
/* Private defines -----------------------------------------------------------*/
/*																																						*/

/*																																						*/

#endif

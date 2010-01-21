/*******************************************************************************
* File Name          : SSD1303.c
* Author             : lxyppc
* Version            : V1.0
* Date               : 10-01-21
* Description        : SSD1303 operations
*                      the SSH1101A is compatible with SSD1303
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include "bsp.h"
#include "SSD1303.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define   SSD1303_PAGE_NUMBER           8
#define   SSD1303_COLUMN_NUMBER         128
#define   SSD1303_COLUMN_MARGIN_START   2
#define   SSD1303_COLUMN_MARGIN_END     2

/* Private macro -------------------------------------------------------------*/
#define   SSD1303_Buffer    (_SSD1303_Buffer + SSD1303_COLUMN_MARGIN_START)

/* Private variables ---------------------------------------------------------*/
static  u8  _SSD1303_Buffer[SSD1303_COLUMN_NUMBER*SSD1303_PAGE_NUMBER 
+ SSD1303_COLUMN_MARGIN_START + SSD1303_COLUMN_MARGIN_END];
static  u8  pageIndex = 0;

/* Private function prototypes -----------------------------------------------*/
void  WriteCommand(unsigned char command);
void  WriteData(unsigned char data);
void  OnPageTransferDone(void);

/*******************************************************************************
* Function Name  : WriteCommand
* Description    : Write command to the SSD1303
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WriteCommand(unsigned char command)
{
  SSD_A0_Low();
  SPI_SendByte(command);
  SPI_Wait();
}

/*******************************************************************************
* Function Name  : WriteData
* Description    : Write data to the SSD1303
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void WriteData(unsigned char data)
{
  SSD_A0_High();
  SPI_SendByte(data);
  SPI_Wait();
}

/*******************************************************************************
* Function Name  : SSD1303_Init
* Description    : Initialize the SSD1303
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SSD1303_Init(void)
{
  /* Generate a reset */
  SSD_Reset_Low();
  for(u32 i=5000;--i;);//��ʱ5uS����
  SSD_Reset_High();
  
  /*************************************************
  // SSD1303 Initialization Command
  *************************************************/
  // Lower Column Address
  WriteCommand(0x00); /* Set Lower Column Address */
  // High Column Address
  WriteCommand(0x10); /* Set Higher Column Address*/
  // Display Start Line
  WriteCommand(0x40); /* Set Display Start Line */
  // Contrast Control Register
  WriteCommand(0x81); /* Set Contrast Control */
  WriteCommand(0x1f); /* 0 ~ 255 0x1f*/
  // Re-map
  WriteCommand(0xA1); /* [A0]:column address 0 is map 
  to SEG0 , [A1]: columnaddress 131 is map to SEG0*/ 
  // Entire Display ON/OFF
  WriteCommand(0xA4); /* A4=ON */
  // Normal or Inverse Display
  WriteCommand(0XA6); /* Normal Display*/
  // Multiplex Ratio
  WriteCommand(0xA8); /* Set Multiplex Ratio */
  WriteCommand(0x3f); /* Set to 36 Mux*/
  // Set DC-DC
  WriteCommand(0xAD); /* Set DC-DC */
  WriteCommand(0x8B); /* 8B=ON, 8A=Off */
  // Display ON/OFF
  WriteCommand(0xAE); /* AF=ON , AE=OFF*/
  // Display Offset
  WriteCommand(0xD3); /* Set Display Offset */
  WriteCommand(0x00); /* No offset */
  // Display Clock Divide
  WriteCommand(0xD5); /* Set Clock Divide */
  WriteCommand(0x20); /* Set to 80Hz */
  // Area Color Mode
  WriteCommand(0xD8); /* Set Area Color On or Off*/
  WriteCommand(0x00); /* Mono Mode */
  // COM Pins Hardware Configuration
  WriteCommand(0xDA); /* Set Pins HardwareConfiguration */
  WriteCommand(0x12);
  // VCOMH
  WriteCommand(0xDB); /* Set VCOMH */
  WriteCommand(0x00);
  // VP
  WriteCommand(0xD9); /* Set VP */
  WriteCommand(0x22); /* P1=2 , P2=2 */
  WriteCommand(0xc0);/* Set COM scan direction */
  
  // Turn on the display
  WriteCommand(0xaf);
}

/*******************************************************************************
* Function Name  : OnPageTransferDone
* Description    : Called when each page transfer done
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  OnPageTransferDone(void)
{
  if(pageIndex == SSD1303_PAGE_NUMBER){
    pageIndex = 0;
    return;
  }
  WriteCommand(0xb0 + pageIndex);
  if(pageIndex == 0){
    WriteCommand(0x00);
    WriteCommand(0x10);
  }
  SSD_A0_High();
  DMA1_Channel5->CCR &= ((u32)0xFFFFFFFE);
  DMA1_Channel5->CNDTR = SSD1303_COLUMN_NUMBER+SSD1303_COLUMN_MARGIN_START + SSD1303_COLUMN_MARGIN_END;
  DMA1_Channel5->CMAR = (u32)(SSD1303_Buffer+SSD1303_COLUMN_NUMBER*pageIndex - SSD1303_COLUMN_MARGIN_START);
  //DMA_Cmd(DMA1_Channel5, ENABLE);
  DMA1_Channel5->CCR |= ((u32)0x00000001);
  pageIndex++;
}

/*******************************************************************************
* Function Name  : DMA1_Channel5_IRQHandler
* Description    : This function handles DMA1 Channel 5 interrupt request.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{
  if(DMA_GetITStatus(DMA1_IT_TC5)){
    DMA_ClearITPendingBit(DMA1_IT_GL5);
    OnPageTransferDone();
  }
}

/*******************************************************************************
* Function Name  : ShowData
* Description    : Show the data.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void  ShowData(u8 col, u8 set)
{
  unsigned char j;
  unsigned char dd = set ? 0xFF : 0x00;
  unsigned char* p = col*SSD1303_COLUMN_NUMBER + SSD1303_Buffer;
  for(j=0;j<SSD1303_COLUMN_NUMBER;j++){
    p[j] = dd;
  }
}

/*******************************************************************************
* Function Name  : SysTickHandler
* Description    : This function handles SysTick Handler.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void SysTickHandler(void)
{
  pageIndex = 0;
  OnPageTransferDone();
}
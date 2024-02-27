#include "main.h"

static void MPU_Config(void);
uint32_t sysclock = 0;

#if !(__ARMCC_VERSION >= 6010050)
uint8_t mpudata[128] __attribute__((at(0X20002000)));
#else
uint8_t mpudata[128] __attribute__((section(".bss.ARM.__at_0X20002000")));
#endif

int main(void)
{
  int i;
  uint8_t t = 0;
  /* Configure the MPU attributes */
  MPU_Config();
  sys_cache_enable();                  /* 打开L1-Cache */

  HAL_Init();

  sys_stm32_clock_init(240, 2, 2, 4);
  sysclock = HAL_RCC_GetSysClockFreq();
  LED_Config();
  usart_init(115200);
  Key_Init();

  while (1)
  {
    i = key_scan();
    if (i == WKUP_PRES)
    {
      mpu_set_protection(0X20002000, MPU_REGION_SIZE_128B,
        MPU_REGION_NUMBER0, MPU_INSTRUCTION_ACCESS_ENABLE,
        MPU_REGION_PRIV_RO_URO, MPU_ACCESS_NOT_SHAREABLE,
        MPU_ACCESS_NOT_CACHEABLE,
        MPU_ACCESS_BUFFERABLE); /* 只读,禁止共用,禁止 catch,允许缓冲 */
      printf("MPU open!\r\n"); /* 提示 MPU 打开 */
    }
    else if (i == KEY0_PRES)
    {
      printf("Start Writing data...\r\n");
      sprintf((char*)mpudata, "MPU test array %d", t);
      printf("Data Write finshed!\r\n");
    }
    else if (i == KEY1_PRES)
    {
      printf("Array data is:%s\r\n", mpudata);
    }
    else
    {
      delay_ms(10);
    }
    t++;
    if ((t % 50) == 0)
      HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_6);
  }
}



static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU as Strongly ordered for not defined regions */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}


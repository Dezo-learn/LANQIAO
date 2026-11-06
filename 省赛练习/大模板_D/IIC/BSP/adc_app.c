#include "adc_app.h"

float adc_read_os(ADC_HandleTypeDef * hadc)
{
  uint16_t adc_val;
  float adc_f;
  HAL_ADC_Start(hadc);
  adc_val = HAL_ADC_GetValue(hadc);
  adc_f = adc_val * 3.3f /65536.0f;
  return adc_f;
 
}

//float adc_read(ADC_HandleTypeDef * hadc)
//{
//  uint16_t adc_val;
//  float adc_f;
//  HAL_ADC_Start(hadc);
//  adc_val = HAL_ADC_GetValue(hadc);
//  adc_f = adc_val * 3.3f /4096.0f;
//  return adc_f;
// 
//}

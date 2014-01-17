/*****************************************************************************
 *   adctest.c:  main C entry file for NXP LPC11xx Family Microprocessors
 *
 *   Copyright(C) 2008, NXP Semiconductor
 *   All rights reserved.
 *
 *   History
 *   2009.12.07  ver 1.00    Preliminary version, first Release
 *
******************************************************************************/
#include "driver_config.h"
#include "target_config.h"
#include "timer32.h"

#include "adc.h"
#include "gpio.h"
#include "debug_printf.h"

#ifdef ADC_DEBUG
uint8_t ConvertDigital ( uint8_t digital )
{
  uint8_t ascii_char;

  if ( (digital >= 0) && (digital <= 9) )
  {
	ascii_char = digital + 0x30;	/* 0~9 */
  }
  else
  {
	ascii_char = digital - 0x0A;
	ascii_char += 0x41;				/* A~F */
  }
  return ( ascii_char );
}
#endif

#ifdef ADC_DEBUG
#define BAR_OUTPUT_LENGTH 60
#define ADC_COUNT_MAX 1023
#define CHANNEL_WIDTH 2
#define VOLT_WIDTH 4
#define MAX_BAR_LENGTH (BAR_OUTPUT_LENGTH-CHANNEL_WIDTH-1-VOLT_WIDTH-1-2)
#define SUPPLY_VOLTAGE 3.30
#define LED_PORT 0
#define LED_BIT 7

void FillString(char *s, char c, uint32_t len)
{
	if(!len)
		goto empty;
	while(--len)
		*s++ = c;
empty:
	*s = 0;
}
void ADCBar(int32_t Channel, uint32_t Counts)
{
	uint32_t ValueBarLen = ((Counts * (MAX_BAR_LENGTH+1)) - (ADC_COUNT_MAX/2 - 1)) / ADC_COUNT_MAX;
	char ValueBarBuf[BAR_OUTPUT_LENGTH+1];
	double ValueVolts = Counts*SUPPLY_VOLTAGE/ADC_COUNT_MAX;

	if(Channel >= 0)
	{
		if(ValueBarLen > MAX_BAR_LENGTH) ValueBarLen = MAX_BAR_LENGTH;
		FillString(ValueBarBuf, '#', ValueBarLen);
		debug_printf("%2d %d.%02dV |%s%*s|\n", (int)Channel,
				(int)ValueVolts, (int)((ValueVolts-(int)ValueVolts)*100),
				ValueBarBuf, (int)(MAX_BAR_LENGTH-ValueBarLen), "");
	}
	else
	{
		FillString(ValueBarBuf, '-', MAX_BAR_LENGTH+2);
		debug_printf("%-9s%s\n", "", ValueBarBuf);
	}
}
#endif

/******************************************************************************
**   Main Function  main()
******************************************************************************/
int main (void)
{
	 /* Basic chip initialization is taken care of in SystemInit() called
	   * from the startup code. SystemInit() and chip settings are defined
	   * in the CMSIS system_<part family>.c file.
	   */

  // variables
  uint32_t i = 0;
  double curr_volt;
  double freq;
  uint32_t counter_ticks = 0;
  double period;

  /* Initialize ADC  */
  ADCInit( ADC_CLK );

  /* LED Initialization code here */

  /* Initialize GPIO (sets up clock) */
  GPIOInit();
  /* Set LED port pin to output */
  GPIOSetDir( LED_PORT, LED_BIT, 1 );

  /* init and enable timer */
  init_timer32( 0, TIME_INTERVAL );
  enable_timer32( 0 );


  while(1)
  {
	/* Read one sample from the ADC port 'AD0' */
	ADCRead( 0 );
	while ( !ADCIntDone );
	ADCIntDone = 0;

	curr_volt = ADCValue[0]*(SUPPLY_VOLTAGE/((double)ADC_COUNT_MAX));
/*
#ifdef ADC_DEBUG
	// Print ADC Voltage as a real-time bar graph
	ADCBar(0, ADCValue[0]);
#endif
*/


	if( curr_volt >= 1.98 && curr_volt <= 2.02)
	{
		if(i == 0)
		{
			// indicate that we have started the timer
			//debug_printf("Started timer\n");
			i = 1;
			counter_ticks = 0;
			timer32_0_counter = 0;
		}
		// 200ms is the shortest period for a signal with freq up to 5Hz
		// This implies that 100ms is the time for half a period
		// Therefore, make sure at least 50ms have passed
		else if( timer32_0_counter > 50 )
		{
			i = 0;
			//debug_printf( "TIME_INTERVAL:  %d\n", TIME_INTERVAL);
			//debug_printf( "timer_counter_0:  %d\n", timer32_0_counter);
			counter_ticks = (timer32_0_counter) * 2;
			period = ((double)counter_ticks) / 1000;
			freq = 1/period;
			debug_printf( "Frequency (in Hz): %d\n", (int)(freq + 0.5) );
			timer32_0_counter = 0;
		}
	}

	/*if(curr_volt >= 1.98 && curr_volt <= 2.02)
	{
		if(i == 0)
		{
			// indicate that we have started the timer
			debug_printf("Started timer\n");
			i = 1;
			counter_ticks = 0;
			timer32_0_counter = 0;
		}
		else
		{
			i = 0;
			debug_printf( "TIME_INTERVAL:  %d\n", TIME_INTERVAL);
			debug_printf( "timer_counter_0:  %d\n", timer32_0_counter);
			counter_ticks = (timer32_0_counter) * 2;
			period = ((double)counter_ticks) / 1000;
			freq = 1/period;
			debug_printf( "Frequency (in Hz): %d\n", (int)freq );
			timer32_0_counter = 0;
		}
	}
	*/

	/* LED lighting and analog signal frequency calculation and printing code here */
	if((ADCValue[0]*(SUPPLY_VOLTAGE/((double)ADC_COUNT_MAX))) > (double)(SUPPLY_VOLTAGE)/2)
	{
		// turn on the LED
		GPIOSetValue( LED_PORT, LED_BIT, 1 );
	}
	else
	{
		// turn off the LED
		GPIOSetValue( LED_PORT, LED_BIT, 0 );
	}

  }
}

/******************************************************************************
**                            End Of File
******************************************************************************/

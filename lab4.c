#include "nios2_control.h"
#include "chario.h"
#include "timer.h"
#include "adc.h"
#include "leds.h"

/* place additional #define macros here */
#define TIMER1_STATUS   ((volatile unsigned int *) 0x10004020)

#define TIMER1_CONTROL  ((volatile unsigned int *) 0x10004024)

#define TIMER1_START_LO ((volatile unsigned int *) 0x10004028)

#define TIMER1_START_HI ((volatile unsigned int *) 0x1000402C)

#define TIMER1_SNAP_LO  ((volatile unsigned int *) 0x10004030)

#define TIMER1_SNAP_HI  ((volatile unsigned int *) 0x10004034)


#define TIMER3_STATUS   ((volatile unsigned int *) 0x10004060)

#define TIMER3_CONTROL  ((volatile unsigned int *) 0x10004064)

#define TIMER3_START_LO ((volatile unsigned int *) 0x10004068)

#define TIMER3_START_HI ((volatile unsigned int *) 0x1000406C)

#define TIMER3_SNAP_LO  ((volatile unsigned int *) 0x10004070)

#define TIMER3_SNAP_HI  ((volatile unsigned int *) 0x10004074)

#define TIMER_TO_BIT 0x1

#define LEDS      ((volatile unsigned int *) 0x10000010)

#define SWITCHES_DATA (volatile unsigned int *) 0x10000040

#define HEX_DISPLAYS (volatile unsigned int*) 0x10000020

/* define global program variables here */
#define TIMER1_INTERVAL 06250000 //0.125 second

#define TIMER3_INTERVAL 06250000 //0.25 seconds 12500000

unsigned int timer_count = 0;
volatile int timer_1_flag = 0;
volatile int timer_3_flag = 0;

unsigned int led_table[] =
{
0x202, 0x186, 0xCC, 0x78, 0x30,
};

/* place additional functions here */



/*-----------------------------------------------------------------*/

/* this routine is called from the_exception() in exception_handler.c */

void interrupt_handler(void)
{
      unsigned int ipending;

      /* read current value in ipending register */
      ipending = NIOS2_READ_IPENDING();

      /* do one or more checks for different sources using ipending value */
      if (ipending & (1<<14)){
     
      /* remember to clear interrupt sources */
      *TIMER1_STATUS = 0;
        
        *LEDS = led_table[timer_count%5];
        timer_count++;
        timer_1_flag = 1;
   }
   

   if (ipending & (1<<16)){
     
      *TIMER3_STATUS = 0;
        
        timer_3_flag = 1;
   }

      
      /* remember to clear interrupt sources */
      
}

/*-----------------------------------------------------------------*/

void Init (void)
{
      /* initialize software variables */
      

      /* set up each hardware interface */
      *TIMER1_START_LO = TIMER1_INTERVAL & 0xFFFF;
    *TIMER1_START_HI = (TIMER1_INTERVAL >> 16) & 0xFFFF;
   
    *TIMER3_START_LO = TIMER3_INTERVAL & 0xFFFF;
    *TIMER3_START_HI = (TIMER3_INTERVAL >> 16) & 0xFFFF;
   
      *TIMER1_CONTROL = 0x7; /* start timer, enable interrupts, continuous mode */
    *TIMER3_CONTROL = 0x7; /* start timer, enable interrupts, continuous mode */
      
      InitADC(2, 2);
      /* set up ienable */
      NIOS2_WRITE_IENABLE((1<<14) | (1<<16));

      /* enable global recognition of interrupts in procr. status reg. */
      NIOS2_WRITE_STATUS(1);
}

/*-----------------------------------------------------------------*/

int main (void)
{
      Init ();    /* perform software/hardware initialization */
      int show_dash = 0;
      
      unsigned int character = GetChar();
      if (character == '-'){
            show_dash = 1;
      }else{
            show_dash = 0;
      }
      
      PrintChar('\n');
      PrintString("ELEC 371 Lab 4 by Rachel, Julia\n");
      PrintString("\nHexadecimal result from A/D conversion: 0x??");
      
      while (1)
      {
            /* fill in body of infinite loop */
            if (timer_1_flag){
                  unsigned int switches = *SWITCHES_DATA & 0xF;
                  unsigned int hex_display = 0;
                  
                  for(int i = 0; i < 4; i++){
                        if(switches & (1 << i)){ //check if corresponding switch is on
                              if(show_dash){
                                    hex_display |= (0x40 << (i * 8)); //corresponds to - in hex display
                              } else {
                                    hex_display |= (0xBF << (i * 8)); //corrsponds to O in hex displays
                              }
                        }
                  }
                  
                  *HEX_DISPLAYS = hex_display;
                  timer_1_flag = 0;
            }
            
            if (timer_3_flag){
                  unsigned int adc_value;
                  adc_value = ADConvert();
                  
                  unsigned int sixteens = (adc_value >> 4) & 0xF;
                  unsigned int ones = adc_value & 0xF;
                  
                  PrintChar('\b');
                  PrintChar('\b');
                  
                  PrintHexDigit(sixteens);
                  PrintHexDigit(ones);
                  //PrintString("Hello");
                  timer_3_flag = 0;
            }
      }

      return 0;   /* never reached, but main() must return a value */
}

#include "hyperion_ui.h"
#include "hyperion_can.h"
#include "hyperion_io.h"

static THD_WORKING_AREA(waThread1, 512); // TODO
static THD_FUNCTION(Thread1, arg) {
  (void)arg;
  chRegSetThreadName("blinker");
  while (true) {
    palTogglePad(GPIOC, GPIOC_LED);       /* Orange.  */
    chThdSleepMilliseconds(500);
  }
}




/*
 * Application entry point.
 */
static ADCConfig adccfg = {};
#define ADC_BUF_DEPTH 2 // depth of buffer
#define ADC_CH_NUM 1    // number of used ADC channels
static volatile adcsample_t samples_buf[ADC_BUF_DEPTH * ADC_CH_NUM] = {5,}; // results array
static ADCConversionGroup adccg = {
   // this 3 fields are common for all MCUs
      // set to TRUE if need circular buffer, set FALSE otherwise
      TRUE,
      // number of channels
      (uint16_t)(ADC_CH_NUM),
      // callback function, set to NULL for begin
      NULL, NULL,
   // Resent fields are stm32 specific. They contain ADC control registers data.
   // Please, refer to ST manual RM0008.pdf to understand what we do.
      // CR1 register content, set to zero for begin
      0,
      // CR2 register content, set to zero for begin
	  ADC_CR2_SWSTART,
	    /* ADC SMPR1 */ 0,
	    /* ADC SMPR2 */ ADC_SMPR2_SMP_AN3(ADC_SAMPLE_480),
	    /* ADC SQR1 */ ADC_SQR1_NUM_CH(ADC_CH_NUM),
	    /* ADC SQR2 */ 0,
	    /* ADC SQR3 */ ADC_SQR3_SQ1_N(ADC_CHANNEL_IN3)
};

#define ADC_GRP2_NUM_CHANNELS   8
#define ADC_GRP2_BUF_DEPTH      16
adcsample_t adc_data[ADC_GRP2_NUM_CHANNELS * ADC_GRP2_BUF_DEPTH];


static const ADCConversionGroup adcgrpcfg2 = {
  TRUE,
  ADC_GRP2_NUM_CHANNELS,
  NULL, //adccallback,
  NULL, // adcerrorcallback,
  0,                        /* CR1 */
  ADC_CR2_SWSTART,          /* CR2 */
  0,                        /* SMPR2 */
  ADC_SMPR2_SMP_AN0(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN1(ADC_SAMPLE_56) |
  ADC_SMPR2_SMP_AN2(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN3(ADC_SAMPLE_56) |
  ADC_SMPR2_SMP_AN4(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN5(ADC_SAMPLE_56) |
  ADC_SMPR2_SMP_AN6(ADC_SAMPLE_56) | ADC_SMPR2_SMP_AN7(ADC_SAMPLE_56),
  ADC_SQR1_NUM_CH(ADC_GRP2_NUM_CHANNELS),
  ADC_SQR2_SQ8_N(ADC_CHANNEL_IN7) | ADC_SQR2_SQ7_N(ADC_CHANNEL_IN6),
  ADC_SQR3_SQ6_N(ADC_CHANNEL_IN5)   | ADC_SQR3_SQ5_N(ADC_CHANNEL_IN4) |
  ADC_SQR3_SQ4_N(ADC_CHANNEL_IN3)   | ADC_SQR3_SQ3_N(ADC_CHANNEL_IN2) |
  ADC_SQR3_SQ2_N(ADC_CHANNEL_IN1)   | ADC_SQR3_SQ1_N(ADC_CHANNEL_IN0)
};



int main(void)
{
    halInit();
    chSysInit();

    gfxInit();


    chThdCreateStatic(waThread1, sizeof(waThread1), NORMALPRIO, Thread1, NULL);
    hyperion_io_init();
    hyperion_ui_init();
    // hyperion_can_init();

    adcStart(&ADCD1, NULL);
    adcStartConversion(&ADCD1, &adcgrpcfg2, adc_data, ADC_GRP2_BUF_DEPTH);

 
  /*
   * Serves timer events.
   */

   int f = 0;

   uint32_t data = 0;

   // btn1 red = 1 << 9
   // btn1 green = 1 << 10
   // btn2 red = 1 << 1
   // btn2 green = 1 << 2
   // btn3 red = 1 << 11
   // btn3 green = 1 << 12
   // btn4 red = 1 << 3
   // btn4 green = 1 << 4
   // btn5 red = 1 << 13
   // btn5 green = 1 << 8
   // btn6 red = 1 << 5
   // btn6 green = 1 << 6
   // led0 = 1 << 14
   // led1 = 1 << 15
   // led2 = 1 << 16
   // led3 = 1 << 17
   // led4 = 1 << 18
   // led5 = 1 << 19
   // led6 = 1 << 20
   // led7 = 1 << 21
   // led8 = 1 << 22
   // led9 = 1 << 23
   uint32_t mm[] = {
     1 << 9,
     1 << 10,
     1 << 9 | 1 << 10,

     1 << 1,
     1 << 2,
     1 << 1 | 1 << 2,

     1 << 11,
     1 << 12,
     1 << 11 | 1 << 12,

     1 << 3,
     1 << 4,
     1 << 3 | 1 << 4,

     1 << 13,
     1 << 8,
     1 << 13 | 1 << 8,

     1 << 5,
     1 << 6,
     1 << 5 | 1 << 6,

     // bar
     1 << 14,
     1 << 15,
     1 << 16,
     1 << 17,
     1 << 18,
     1 << 19,
     1 << 20,
     1 << 21,
     1 << 22,
     1 << 23,

     // all,
     0xffffff,
     0,
     0xffffff,
     0,
     0xffffff,
     0,


   };



  while (true) {
//    shift_in_out(mm[f]);
//    ++f;
//    if (f == AB_ARRLEN(mm))
//    {
//        f = 0;
//    }
    chThdSleepMilliseconds(600);
  }



  return 0;
}

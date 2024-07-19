#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include "pico/float.h"

#define PWM1H_PIN 0
#define PWM1L_PIN 1
#define PWM2H_PIN 2
#define PWM2L_PIN 3

const int PWM_PERIOD = 400;
//const float PI = 3.14159265;
const int SINE_RESOLUTION = 250;
//const int DEADTIME = 0;
const int MIN_PULSE = 3;  //0 - 5

int pwm1_slice = 0;
int pwm2_slice = 0;

int i = -1;
int period = 0;
float scale = 1.0;  // 0.1 - 1.02

const int SINE_TABLE[SINE_RESOLUTION] = {
  0, 5, 10, 15, 20, 25, 30, 35, 39, 44,
  49, 54, 59, 64, 68, 73, 78, 82, 87, 91,
  96, 100, 105, 109, 113, 117, 121, 125, 129, 133,
  136, 140, 144, 147, 150, 154, 157, 160, 163, 166,
  168, 171, 174, 176, 178, 180, 183, 185, 186, 188,
  190, 191, 193, 194, 195, 196, 197, 198, 198, 199,
  199, 199, 200, 200, 199, 199, 199, 198, 198, 197,
  196, 195, 194, 193, 191, 190, 188, 186, 185, 183,
  180, 178, 176, 174, 171, 168, 166, 163, 160, 157,
  154, 150, 147, 144, 140, 136, 133, 129, 125, 121,
  117, 113, 109, 105, 100, 96, 91, 87, 82, 78,
  73, 68, 64, 59, 54, 49, 44, 39, 35, 30,
  25, 20, 15, 10, 5, 0, -5, -10, -15, -20,
  -25, -30, -35, -39, -44, -49, -54, -59, -64, -68,
  -73, -78, -82, -87, -91, -96, -100, -105, -109, -113,
  -117, -121, -125, -129, -133, -136, -140, -144, -147, -150,
  -154, -157, -160, -163, -166, -168, -171, -174, -176, -178,
  -180, -183, -185, -186, -188, -190, -191, -193, -194, -195,
  -196, -197, -198, -198, -199, -199, -199, -200, -200, -199,
  -199, -199, -198, -198, -197, -196, -195, -194, -193, -191,
  -190, -188, -186, -185, -183, -180, -178, -176, -174, -171,
  -168, -166, -163, -160, -157, -154, -150, -147, -144, -140,
  -136, -133, -129, -125, -121, -117, -113, -109, -105, -100,
  -96, -91, -87, -82, -78, -73, -68, -64, -59, -54,
  -49, -44, -39, -35, -30, -25, -20, -15, -10, -5
};

bool increment_pwm = false;
bool increment_scale = false;

struct repeating_timer timer;
unsigned long start = 0, stop = 0, count = 0, exectime = 0;


SerialUART debug(uart0, 16, 17);


void pwm_setup(void) {
  gpio_set_function(PWM1H_PIN, GPIO_FUNC_PWM);
  gpio_set_function(PWM1L_PIN, GPIO_FUNC_PWM);
  gpio_set_function(PWM2H_PIN, GPIO_FUNC_PWM);
  gpio_set_function(PWM2L_PIN, GPIO_FUNC_PWM);

  pwm1_slice = pwm_gpio_to_slice_num(PWM1H_PIN);
  pwm2_slice = pwm_gpio_to_slice_num(PWM2H_PIN);

  pwm_set_wrap(pwm1_slice, PWM_PERIOD - 1);
  pwm_set_wrap(pwm2_slice, PWM_PERIOD - 1);

  pwm_set_chan_level(pwm1_slice, PWM_CHAN_A, 0);
  pwm_set_chan_level(pwm1_slice, PWM_CHAN_B, 0);
  pwm_set_chan_level(pwm2_slice, PWM_CHAN_A, 0);
  pwm_set_chan_level(pwm2_slice, PWM_CHAN_B, 0);

  pwm_set_phase_correct(pwm1_slice, true);
  pwm_set_phase_correct(pwm2_slice, true);

  pwm_set_output_polarity(pwm1_slice, true, false);
  pwm_set_output_polarity(pwm2_slice, true, false);

  pwm_set_clkdiv_int_frac(pwm1_slice, 7, 10);
  pwm_set_clkdiv_int_frac(pwm2_slice, 7, 10);
}


void pwm_enable(void) {
  pwm_set_mask_enabled(0x0F);
}


void pwm_disable(void) {
  pwm_set_mask_enabled(0x00);
}


inline void pwm_period(int period) {
  pwm_set_chan_level(pwm1_slice, PWM_CHAN_A, period);
  pwm_set_chan_level(pwm1_slice, PWM_CHAN_B, period);
  pwm_set_chan_level(pwm2_slice, PWM_CHAN_A, PWM_PERIOD - period);
  pwm_set_chan_level(pwm2_slice, PWM_CHAN_B, PWM_PERIOD - period);
}


bool sine_timer(struct repeating_timer *t) {
  increment_pwm = true;
  increment_scale = true;
  return true;
}


void setup() {
  pwm_setup();
  pwm_enable();

  add_repeating_timer_us(-20000 / SINE_RESOLUTION, sine_timer, NULL, &timer);

  debug.begin(115200);
}


void loop() {
  if (increment_pwm) {
    //start = micros();
    increment_pwm = false;
    i++;
    period = PWM_PERIOD / 2 + scale * SINE_TABLE[i];

    if (period < MIN_PULSE) {
      period = 0;
    } else if (period > PWM_PERIOD - MIN_PULSE) {
      period = PWM_PERIOD;
    }

    pwm_period(period);
    //stop = micros();
    //exectime += stop - start;
    //count++;
    if (i == SINE_RESOLUTION - 1) {
      i = -1;
      //debug.println(exectime / count);
      //count = 0;
      //exectime = 0;
    }
  }
}
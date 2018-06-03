
#ifndef LM35DZ_H_
#define LM35DZ_H_

#define TEST_PATH "/home/Riemann/workspaces/ISD2/Task_2/test.txt"	//	path for test

//	Path for ADC channel:
#define ADC_PATH "/sys/bus/iio/devices/iio:device0/in_voltage0_raw"	//	path to read ADC value from channel 0

//	Path for GPIO pin
#define GPIO_PATH "/sys/class/gpio/gpio60/value"

//	Path for PWM pin to control:
#define PWM_DUTY "/sys/class/pwm/pwmchip0/pwm0/duty_cycle"
#define PWM_PERIOD "/sys/class/pwm/pwmchip0/pwm0/period"
#define PWM_ENABLE "/sys/class/pwm/pwmchip0/pwm0/enable"

//	Path for LED
#define LED "/sys/class/leds/beaglebone:green:usr3/brightness"

//	Prototypes:
double readTemp();
void gpioHeat(int input);
void onLED();
void offLED();
void pwmDuty(double tempInput);
void pwmPeriod();
void pwmEnable();

#endif

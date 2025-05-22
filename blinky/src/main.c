// Blinky sample code
//   Flash an LED at 50% duty cycle:
//   - 2Hz normally (on twice a second): 
//   - 1Hz if button is pressed (on once a second)
//
// Based off the Zephyr blinky sample application.
// - Designed to be compiled for BeagleY-AI's MCU R5
//   (because the custom hardware uses pins that are mapped to the MCU domain)
// 
// Hardware Interaction
// - Flash LED on GPIO7 (Zen Hat PYMNL header, pin 9)
// - Read button state on GPIO24 (Zen Hat rotary encoder push-button)
//
// Due to an issue with the device tree and the MUX / direction register, 
// user must currently execute the following two commands in Linux prior to loading:
//      gpioset gpiochip0 9=1
//      gpioget gpiochip0 10
//
// Known Issues:
//   1. Does not correctly configure pin-mux / direction register
//   2. Does not correctly setup clock-source, so cannot use the k_msleep() function (hangs)
//   3. Once running, the only way to reload a new version is to reboot the board: sudo reboot -f

#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

// 1,000,000 uSec = 1000 msec = 1 sec
#define DELAY_TIME_uS   (250 * 1000)

// Device tree nodes for pin aliases
#define LED0_NODE DT_ALIAS(led0)
#define BTN0_NODE DT_ALIAS(btn0)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec btn = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);

static void initialize_gpio(const struct gpio_dt_spec *pPin, int direction) 
{
	if (!gpio_is_ready_dt(pPin)) {
		printf("ERROR: GPIO pin not ready read; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}

	int ret = gpio_pin_configure_dt(pPin, direction);
	if (ret < 0) {
		printf("ERROR: GPIO Pin Configure issue; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}
}

int main(void)
{
	printf("Hello World! %s\n", CONFIG_BOARD_TARGET);

	initialize_gpio(&led, GPIO_OUTPUT_ACTIVE);
	initialize_gpio(&btn, GPIO_INPUT);

	bool led_state = true;
	while (true) {
		// Toggle LED
		printf("LED state: %s\n", led_state ? "OFF" : "ON");
		if (gpio_pin_toggle_dt(&led) < 0) {
			printf("ERROR: GPIO Pin Toggle DT\n");
			return 0;
		}
		led_state = !led_state;

		k_busy_wait(DELAY_TIME_uS);	

		// Read state: Flash slowly when button is pressed
		int state = gpio_pin_get_dt(&btn);
		printf("Button state: %s\n", state == 0 ? "Pressed" : "Not pressed");
		if (state < 0) {
			printf("ERROR reading button state: %d\n", state);
		}
		if (state == 0) {
			k_busy_wait(DELAY_TIME_uS);	
		}
	}
	return 0;
}

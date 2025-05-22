/*
 * R5 Sample Code for Shared Memory with Linux
 */
#include <stdio.h>
#include <stdlib.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <string.h>

#include "sharedDataLayout.h"

// Memory
// ----------------------------------------
#define SHARED_MEM_BTCM_START 0x00000000  // TRM p848
#define SHARED_MEM_ATCM_START 0x00041010  // TRM p849
#define BASE ((void*)(SHARED_MEM_BTCM_START))

// Access GPIO (for demonstration purposes)
// ----------------------------------------
// 1,000,000 uSec = 1000 msec = 1 sec
#define MICRO_SECONDS_PER_MILI_SECOND   (1000)
#define DEFAULT_LED_DELAY_MS            (100)

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

	printf("Contents of Shared Memory BTCM:\n");
	for (int i = MSG_OFFSET; i < END_MEMORY_OFFSET; i++) {
		uint8_t val = getSharedMem_uint8(BASE, i);
		printf("0x%08x = %2x (%c)\n", i, val, val);
	}

	// Setup defaults
	printf("Writing to BTCM...\n");
	char* msg = "Hello from R5 World (Take 7)!";
	for (int i = 0; i < strlen(msg); i++) {
		setSharedMem_uint8(BASE, MSG_OFFSET + i, msg[i]);
	}
	
	setSharedMem_uint32(BASE, LED_DELAY_MS_OFFSET, DEFAULT_LED_DELAY_MS);
	setSharedMem_uint32(BASE, IS_BUTTON_PRESSED_OFFSET, 0);
	setSharedMem_uint32(BASE, BTN_COUNT_OFFSET, 0);
	setSharedMem_uint32(BASE, LOOP_COUNT_OFFSET, 0);
	
	printf("Contents of Shared Memory BTCM After Write:\n");
	for (int i = MSG_OFFSET; i < END_MEMORY_OFFSET; i++) {
		uint8_t val = getSharedMem_uint8(BASE, i);
		printf("0x%08x = %2x (%c)\n", i, val, val);
	}


	bool led_state = true;
	uint32_t btnCount = 0;
	uint32_t loopCount = 0;
	while (true) {
		// Toggle LED
		printf("LED state: %s\n", led_state ? "OFF" : "ON");
		if (gpio_pin_toggle_dt(&led) < 0) {
			printf("ERROR: GPIO Pin Toggle DT\n");
			return 0;
		}
		led_state = !led_state;


		// Read GPIO state and share with Linux
		int state = gpio_pin_get_dt(&btn);
		bool isPressed = state == 0;
		// printf("Is button pressed? %d\n", isPressed);

		// Update shared count of butten pressed
		if (isPressed) {
			btnCount++;
		}
		loopCount++;

		// Update shared memory to Linux
		setSharedMem_uint32(BASE, IS_BUTTON_PRESSED_OFFSET, isPressed);
		setSharedMem_uint32(BASE, LOOP_COUNT_OFFSET, loopCount);
		setSharedMem_uint32(BASE, BTN_COUNT_OFFSET, btnCount);

		// Wait for delay (set by Linux app)
		uint32_t delay = getSharedMem_uint32(BASE, LED_DELAY_MS_OFFSET);
		k_busy_wait(delay * MICRO_SECONDS_PER_MILI_SECOND);	
	}
	return 0;
}

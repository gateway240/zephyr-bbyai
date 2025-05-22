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
#define SHARED_MEM_BTCM_START 0x00000000 // TRM p848
#define SHARED_MEM_ATCM_START 0x00041010 // TRM p849
#define BASE                  ((void *)(SHARED_MEM_BTCM_START))

// Access GPIO (for demonstration purposes)
// ----------------------------------------
// 1,000,000 uSec = 1000 msec = 1 sec
#define MICRO_SECONDS_PER_MILI_SECOND (1000)
#define DEFAULT_LED_DELAY_MS          (100)

// Device tree nodes for pin aliases
#define LED0_NODE DT_ALIAS(led0)
#define BTN0_NODE DT_ALIAS(btn0)
#define BTN1_NODE DT_ALIAS(btn1)

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);
static const struct gpio_dt_spec chrg = GPIO_DT_SPEC_GET(BTN0_NODE, gpios);
static const struct gpio_dt_spec sens = GPIO_DT_SPEC_GET(BTN1_NODE, gpios);

static void initialize_gpio(const struct gpio_dt_spec *pPin, int direction)
{
	if (!gpio_is_ready_dt(pPin)) {
		printk("ERROR: GPIO pin not ready read; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}

	int ret = gpio_pin_configure_dt(pPin, direction);
	if (ret < 0) {
		printk("ERROR: GPIO Pin Configure issue; direction %d\n", direction);
		exit(EXIT_FAILURE);
	}
}

// RC Timing: Discharge and read time to logic HIGH
static uint32_t read_charge_time_us()
{
	// Discharge the capacitor
	gpio_pin_set_dt(&chrg, 0);
	k_busy_wait(5000); // Waits 5 ms (5000 us)
	
	// Begin charging
	gpio_pin_set_dt(&chrg, 1);

	// Start timing
	uint32_t start = k_cycle_get_32();
	while (gpio_pin_get_dt(&sens) == 0) {
		if (k_cyc_to_us_floor32(k_cycle_get_32() - start) > 1000000) {
			break; // timeout after 1 second
		}
	}
	uint32_t elapsed_us = k_cyc_to_us_floor32(k_cycle_get_32() - start);
	return elapsed_us;
}

int main(void)
{
	printk("Hello World! %s\n", CONFIG_BOARD_TARGET);


	initialize_gpio(&led, GPIO_OUTPUT_ACTIVE);
	initialize_gpio(&sens, GPIO_INPUT);
	initialize_gpio(&chrg, GPIO_OUTPUT_INACTIVE);

	printk("Contents of Shared Memory BTCM:\n");
	for (int i = MSG_OFFSET; i < END_MEMORY_OFFSET; i++) {
		uint8_t val = getSharedMem_uint8(BASE, i);
		printk("0x%08x = %2x (%c)\n", i, val, val);
	}

	// Setup defaults
	printk("Writing to BTCM...\n");
	char *msg = "Hello from R5 World (Take 7)!";
	for (int i = 0; i < strlen(msg); i++) {
		setSharedMem_uint8(BASE, MSG_OFFSET + i, msg[i]);
	}

	setSharedMem_uint32(BASE, LED_DELAY_MS_OFFSET, DEFAULT_LED_DELAY_MS);
	setSharedMem_uint32(BASE, IS_BUTTON_PRESSED_OFFSET, 0);
	setSharedMem_uint32(BASE, BTN_COUNT_OFFSET, 0);
	setSharedMem_uint32(BASE, LOOP_COUNT_OFFSET, 0);

	printk("Contents of Shared Memory BTCM After Write:\n");
	for (int i = MSG_OFFSET; i < END_MEMORY_OFFSET; i++) {
		uint8_t val = getSharedMem_uint8(BASE, i);
		printk("0x%08x = %2x (%c)\n", i, val, val);
	}

	bool led_state = true;
	uint32_t btnCount = 0;
	uint32_t loopCount = 0;
	printk("RC timing (discharge-based analog read)\n");
	while (true) {
		// Toggle LED
		printk("LED state: %s\n", led_state ? "OFF" : "ON");
		if (gpio_pin_toggle_dt(&led) < 0) {
			printk("ERROR: GPIO Pin Toggle DT\n");
			return 0;
		}
		led_state = !led_state;

		// Read GPIO state and share with Linux
		int state = gpio_pin_get_dt(&sens);
		bool isPressed = state == 0;
		// printk("Is button pressed? %d\n", isPressed);

		// Update shared count of butten pressed
		if (isPressed) {
			btnCount++;
		}
		loopCount++;

		uint32_t charge_time_us = read_charge_time_us();
		printk("Charge time: %u us\n", charge_time_us);

		// Update shared memory to Linux
		setSharedMem_uint32(BASE, IS_BUTTON_PRESSED_OFFSET, isPressed);
		setSharedMem_uint32(BASE, LOOP_COUNT_OFFSET, loopCount);
		setSharedMem_uint32(BASE, BTN_COUNT_OFFSET, charge_time_us);

		// Wait for delay (set by Linux app)
		// uint32_t delay = getSharedMem_uint32(BASE, LED_DELAY_MS_OFFSET);
		// k_busy_wait(delay * MICRO_SECONDS_PER_MILI_SECOND);
		uint32_t delay = 500;
		k_busy_wait(delay * MICRO_SECONDS_PER_MILI_SECOND);
		// k_msleep(delay);  // Let other threads (like OpenAMP) run
	}
	return 0;
}

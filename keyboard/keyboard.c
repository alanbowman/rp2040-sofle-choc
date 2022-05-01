#include "keyboard.h"

#include "../neopixel/neopixel.h"
#include <pico/stdlib.h>
#include <pico/bootrom.h> // For reset_usb_boot

typedef uint32_t buttonsPressed;
buttonsPressed curr;
uint8_t keys[6];

#define NUM_BUTTONS 30

uint8_t* keyboard_get_keys_pressed() {
	return keys;
}

bool is_something_pressed() {
	bool somethingPressed = false;
	for (int i=0; i<6; i++) {
		if (keys[i] != 0) {
			somethingPressed = true;
			break;
		}
	}
	return somethingPressed;
}

void reset() {
	reset_usb_boot(0, 0);
}

void init_scan() {
	// Rows are outputs
	gpio_init(5);  gpio_set_dir(5,  true);
	gpio_init(6);  gpio_set_dir(6,  true);
	gpio_init(7);  gpio_set_dir(7,  true);
	gpio_init(8);  gpio_set_dir(8,  true);
	gpio_init(9);  gpio_set_dir(9,  true);
	// Cols are inputs with pull-ups
	gpio_init(21); gpio_set_dir(21, false); gpio_pull_up(21);
	gpio_init(23); gpio_set_dir(23, false); gpio_pull_up(23);
	gpio_init(20); gpio_set_dir(20, false); gpio_pull_up(20);
	gpio_init(22); gpio_set_dir(22, false); gpio_pull_up(22);
	gpio_init(26); gpio_set_dir(26, false); gpio_pull_up(26);
	gpio_init(27); gpio_set_dir(27, false); gpio_pull_up(27);
}

uint32_t scan() {
	uint32_t allVals;
	uint32_t retval = 0;

	gpio_put(5, false);
	gpio_put(6, true);
	gpio_put(7, true);
	gpio_put(8, true);
	gpio_put(9, true);
	sleep_ms(1);
	allVals = gpio_get_all();
	retval |= ((allVals & 1<<21) >> 16); // Pos 5
	retval |= ((allVals & 1<<23) >> 19); // Pos 4
	retval |= ((allVals & 1<<20) >> 17); // Pos 3
	retval |= ((allVals & 1<<22) >> 20); // Pos 2
	retval |= ((allVals & 1<<26) >> 25); // Pos 1
	retval |= ((allVals & 1<<27) >> 27); // Pos 0

	gpio_put(5, true);
	gpio_put(6, false);
	gpio_put(7, true);
	gpio_put(8, true);
	gpio_put(9, true);
	sleep_ms(1);
	allVals = gpio_get_all();
	retval |= ((allVals & 1<<21) >> 10); // Pos 5
	retval |= ((allVals & 1<<23) >> 13); // Pos 4
	retval |= ((allVals & 1<<20) >> 11); // Pos 3
	retval |= ((allVals & 1<<22) >> 14); // Pos 2
	retval |= ((allVals & 1<<26) >> 19); // Pos 1
	retval |= ((allVals & 1<<27) >> 21); // Pos 0

	gpio_put(5, true);
	gpio_put(6, true);
	gpio_put(7, false);
	gpio_put(8, true);
	gpio_put(9, true);
	sleep_ms(1);
	allVals = gpio_get_all();
	retval |= ((allVals & 1<<21) >> 4); // Pos 5
	retval |= ((allVals & 1<<23) >> 7); // Pos 4
	retval |= ((allVals & 1<<20) >> 5); // Pos 3
	retval |= ((allVals & 1<<22) >> 8); // Pos 2
	retval |= ((allVals & 1<<26) >> 13); // Pos 1
	retval |= ((allVals & 1<<27) >> 15); // Pos 0

	gpio_put(5, true);
	gpio_put(6, true);
	gpio_put(7, true);
	gpio_put(8, false);
	gpio_put(9, true);
	sleep_ms(1);
	allVals = gpio_get_all();
	retval |= ((allVals & 1<<21) << 2); // Pos 5
	retval |= ((allVals & 1<<23) >> 1); // Pos 4
	retval |= ((allVals & 1<<20) << 1); // Pos 3
	retval |= ((allVals & 1<<22) >> 2); // Pos 2
	retval |= ((allVals & 1<<26) >> 7); // Pos 1
	retval |= ((allVals & 1<<27) >> 9); // Pos 0

	gpio_put(5, true);
	gpio_put(6, true);
	gpio_put(7, true);
	gpio_put(8, true);
	gpio_put(9, false);
	sleep_ms(1);
	allVals = gpio_get_all();
	retval |= ((allVals & 1<<21) << 8); // Pos 5
	retval |= ((allVals & 1<<23) << 5); // Pos 4
	retval |= ((allVals & 1<<20) << 7); // Pos 3
	retval |= ((allVals & 1<<22) << 4); // Pos 2
	retval |= ((allVals & 1<<26) >> 1); // Pos 1
	retval |= ((allVals & 1<<27) >> 3); // Pos 0

	retval = ~(retval | 0xc0000000);

	//printf("%d - %u\n", allVals, retval);

	return retval;
}

void pressed(int btnNum) {

	setPixel(btnNum, 0x55000000);

	if (btnNum == 0) {
		reset();
	}

	// Find a free button slot
	/*
	for (int i=0; i<6; i++) {
		if (keys[i] == 0) {
			keys[i] = btnNum;
			break;
		}
	}
	*/
	keys[0] = btnNum;

	refreshPixels();
}

void unpressed(int btnNum) {
	setPixel(btnNum, 0x05050500);
	for (int i=0; i<6; i++) {
		if (keys[i] == btnNum) {
			keys[i] = 0;
		}
	}
	refreshPixels();
}

inline bool isPressed(buttonsPressed b, int btnNum) {
	if (b & 1<<btnNum) {
		return true;
	}
	return false;
}

void buttonsChanged(buttonsPressed curr, buttonsPressed prev) {
	for (int i=0; i<NUM_BUTTONS; i++) {
		if (isPressed(curr, i)) {
			if (!isPressed(prev, i)) {
				pressed(i);
			}
		} else {
			if (isPressed(prev, i)) {
				unpressed(i);
			}
		}
	}
}

void keyboard_init() {
	for (int i=0; i<6; i++) {
		keys[i] = 0;
	}
	init_scan();
}

void keyboard_task() {
	buttonsPressed curr;
	static buttonsPressed prev;

	const uint32_t interval_ms = 10;
	static uint64_t prev_time;
	uint64_t curr_time = get_absolute_time();
	if ( curr_time - prev_time < interval_ms) return;
	prev_time = curr_time;

	curr = scan();
	if (curr != prev) {
		buttonsChanged(curr, prev);
	}
	prev = curr;
}


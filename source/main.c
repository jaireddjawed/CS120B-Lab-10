/*	Author: lab
 *  Partner(s) Name: Jaired Jawed
 *	Lab Section:
 *	Assignment: Lab #  Exercise #
 *	Exercise Description: [optional - include for your own benefit]
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 */
#include <avr/io.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "bit.h"
#include "timer.h"
#include "scheduler.h"
#include "keypad.h"
#endif

enum States { WaitPress, OnPress } state;

unsigned char x = 0x00;
unsigned char output = 0x00;

int SM_Tick(int state) {
	x = GetKeypadKey();

	switch (state) {
		case WaitPress:
			switch (x) {
				case '\0':
					state = WaitPress;
					break;
				default:
					state = OnPress;
					break;
			}
			break;
		case OnPress:
		default:
			state = WaitPress;
			break;
	}

	switch (state) {
		case OnPress:
			output = 0x80;
			break;
		case WaitPress:
		default:
			output = 0x00;
			break;
	}

	PORTB = output;

	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xF0; PORTC = 0x0F;

	static task task1;
	task *tasks[] = { &task1 };
	unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = WaitPress;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &SM_Tick;

	TimerSet(50);
	TimerOn();

    /* Insert your solution below */
	unsigned short i;
    	while (1) {
		for (i = 0; i < numTasks; i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += 50;
		}
		while (!TimerFlag);
		TimerFlag = 0;
    	}
 
    return 1;
}

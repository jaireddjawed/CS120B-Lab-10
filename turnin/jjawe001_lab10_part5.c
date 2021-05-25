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

enum States { Start, PoundPress, OnePress, TwoPress, ThreePress, FourPress, Unlocked } sm1_state;
unsigned char output = 0x00;

void set_PWM(double frequency) {
	static double current_frequency;
	if (frequency != current_frequency) {
		if (!frequency) { TCCR3B &= 0x08; }
		else { TCCR3B |= 0x03; }

		if (frequency < 0.954) { OCR3A = 0xFFFF; }
		else if (frequency > 31250) { OCR3A = 0x0000; }
		else { OCR3A = (short) (8000000 / (128 * frequency)) - 1; }
		TCNT3 = 0;
		current_frequency = frequency;
	}
}

void PWM_on() {
	TCCR3A = (1 << COM3A0);
	TCCR3B = (1 << WGM32) | (1 << CS31) | (1 << CS30);
	set_PWM(0);
}

void PWM_off() {
	TCCR3A = 0x00;
	TCCR3B = 0x00;
}

char keys[5] = {'1', '2', '3', '4', '5'};
int changed = 0;

int SM_Tick(int state) {
	unsigned char x = GetKeypadKey();

	// # 1, 2, 3, 4, 5
	// B0 = 1
	//

	switch (sm1_state) {
		case Start:
			switch (x) {
				case '#':
					sm1_state = PoundPress;
					break;
				default:
					state = Start;
					break;
			}
			break;
		case PoundPress:
			if (x == keys[0]) {
				sm1_state = OnePress;
			}
			else {
				sm1_state = PoundPress;
			}
			break;
		case OnePress:
			if (x == keys[1]) {
				sm1_state = TwoPress;
			}
			else if (x == '#') {
				sm1_state = PoundPress;
			}
			else {
				sm1_state = OnePress;
			}
			break;
		case TwoPress:
			if (x == keys[2]) {
				sm1_state = ThreePress;
			}
			else if (x == '#') {
				sm1_state = PoundPress;
			}
			else {
				sm1_state = TwoPress;
			}
			break;
		case ThreePress:
			if (x == keys[3] && changed == 0) {
				sm1_state = FourPress;
			}
			else if (x == keys[3] && changed == 1) {
				sm1_state = Unlocked;
			}
			else if (x == '#') {
				sm1_state = PoundPress;
			}
			else { 
				sm1_state = ThreePress;
			}
			break;
		case FourPress:
			if (x == keys[4]) {
				sm1_state = Unlocked;
			}
			else if (x == '#') {
				sm1_state = PoundPress;
			}
			else {
				sm1_state = FourPress;
			}

			break;
		case Unlocked:
			sm1_state = Unlocked;
			break;
		default:
			sm1_state = Start;
			break;
	}
	
	switch (sm1_state) {
		case Unlocked:
			output = 0x01;
			break;
		default:
			output = 0x00;
	}

	return state;
}


enum SM2_States { SM2_Start, SM2_WaitPress, SM2_OnPress } sm2_state;

int SM2_Tick(int state) {
	switch (sm2_state) {
		case SM2_Start:
			sm2_state = SM2_WaitPress;
			break;
		case SM2_WaitPress:
			if ((~PINB & 0x80) == 0x80) {
				sm1_state = Start;
				sm2_state = SM2_OnPress;
			}
			break;
		case SM2_OnPress:
			sm2_state = SM2_WaitPress;
			break;
		default:
			sm2_state = SM2_Start;
			break;
	}
	switch (sm2_state) {
		case SM2_OnPress:
			output = 0x00;
			break;
		case SM2_WaitPress:
		default:
			break;
	}

	PORTB = output;
	return state;
}

// once the button on PORTA is pressed
// -> speaker goes through a 3 second melody
// the button can't do anything while the doorbell is ringing

enum SM3_State {SM3_Start, SM3_WaitPress, SM3_OnPress} sm3_state;
double notes[12] = {
	392.00,
	0.0,
	329.63,
	0.0,
	493.88,
	329.63,
	0.0,
	329.63,
	0.0,
	329.63,
	0.0,
	329.00
};

double times[12] = {
	9,
	11,
	18,
	20,
	22,
	24,
	26,
	28,
	35,
	37,
	40
};

int counter = 0;
int idx = 0;

int SM3_Tick(int state) {
	switch (state) {
		case SM3_Start:
			state = SM3_WaitPress;
			break;
		case SM3_WaitPress:
			if ((~PINA & 0x80) == 0x80) {
				state = SM3_OnPress;
			}
			else {
				state = SM3_WaitPress;
			}
			break;
		case SM3_OnPress:
			if (idx <= 11) {
				state = SM3_OnPress;
			}
			else {
				state = SM3_WaitPress;
			}
			break;
		default:
			state = SM3_Start;
			break;
	}


	switch (state) {
		case SM3_OnPress:
			if (idx <= 11) {
				set_PWM(notes[idx]);
				idx++;
			}
			break;
		case SM3_Start:
		case SM3_WaitPress:
		default:
			set_PWM(0);
			idx = 0;
			break;
	}

	return state;
}

enum SM4_States {SM4_Start, SM4_FirstCode, SM4_SecondCode, SM4_ThirdCode, SM4_FourthCode, SM4_Change} sm4_state;
char newCode[4] = {};

int SM4_Tick(int state) {
	unsigned char x = GetKeypadKey();
	switch (sm4_state) {
		case SM4_Start:
			if (x == '*' && (~PINB & 0x80) == 0x80) {
				sm4_state = SM4_FirstCode;
			}
			else {
				sm4_state = SM4_Start;
			}
			break;
		case SM4_FirstCode:
			if (x == '1') {
				newCode[0] = '1';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '2') {
				newCode[0] = '2';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '3') {
				newCode[0] = '3';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '4') {
				newCode[0] = '4';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '5') {
				newCode[0] = '5';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '6') {
				newCode[0] = '6';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '7') {
				newCode[0] = '7';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '8') {
				newCode[0] = '8';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '9') {
				newCode[0] = '9';
				sm4_state = SM4_SecondCode;
			}
			else if (x == '0') {
				newCode[0] = '0';
				sm4_state = SM4_SecondCode;
			}
			else if (x == 'A') {
				newCode[0] = 'A';
				sm4_state = SM4_SecondCode;
			}
			else if (x == 'B') {
				newCode[0] = 'B';
				sm4_state - SM4_SecondCode;
			}
			else if (x == 'C') {
				newCode[0] = 'C';
				sm4_state = SM4_SecondCode;
			}
			else if (x == 'D') {
				newCode[0] = 'D';
				sm4_state = SM4_SecondCode;
			}
			else {
				sm4_state = SM4_FirstCode;
			}
			break;
		case SM4_SecondCode:
			if (x == '1') {
				newCode[1] = '1';
				sm4_state = SM4_ThirdCode;
			}
			else if (x == '2') {
				newCode[1] = '2';	sm4_state = SM4_ThirdCode;

			}
			else if (x == '3') {
				newCode[1] = '3';
				sm4_state = SM4_ThirdCode;	
			}
			else if (x == '4') {
				newCode[1] = '4';	sm4_state = SM4_ThirdCode;

			}
			else if (x == '5') {
				newCode[1] = '5';	sm4_state = SM4_ThirdCode;

			}
			else if (x == '6') {
				newCode[1] = '6';	sm4_state = SM4_ThirdCode;

			}
			else if (x == '7') {
				newCode[1] = '7';	sm4_state = SM4_ThirdCode;

			}
			else if (x == '8') {
				newCode[1] = '8';
	sm4_state = SM4_ThirdCode;

			}
			else if (x == '9') {
				newCode[1] = '9';	sm4_state = SM4_ThirdCode;

			}
			else if (x == '0') {
				newCode[1] = '0';	sm4_state = SM4_ThirdCode;

			}
			else if (x == 'A') {
				newCode[1] = 'A';	sm4_state = SM4_ThirdCode;

			}
			else if (x == 'B') {
				newCode[1] = 'B';	sm4_state = SM4_ThirdCode;

			}
			else if (x == 'C') {
				newCode[1] = 'C';	sm4_state = SM4_ThirdCode;

			}
			else if (x == 'D') {
				newCode[1] = 'D';	sm4_state = SM4_ThirdCode;

			}
			else {
				sm4_state = SM4_SecondCode;
			}
			break;
		case SM4_ThirdCode:
			if (x == '1') {
				newCode[2] = '1';
				sm4_state = SM4_FourthCode;
			}
			else if (x == '2') {
				newCode[2] = '2';	sm4_state = SM4_FourthCode;

			}
			else if (x == '3') {
				newCode[2] = '3';		sm4_state = SM4_FourthCode;

			}
			else if (x == '4') {
				newCode[2] = '4';		sm4_state = SM4_FourthCode;

			}
			else if (x == '5') {
				newCode[2] = '5';		sm4_state = SM4_FourthCode;

			}
			else if (x == '6') {
				newCode[2] = '6';		sm4_state = SM4_FourthCode;

			}
			else if (x == '7') {
				newCode[2] = '7';		sm4_state = SM4_FourthCode;

			}
			else if (x == '8') {
				newCode[2] = '8';		sm4_state = SM4_FourthCode;

			}
			else if (x == '9') {
				newCode[2] = '9';		sm4_state = SM4_FourthCode;

			}
			else if (x == '0') {
				newCode[2] = '0';		sm4_state = SM4_FourthCode;

			}
			else if (x == 'A') {
				newCode[2] = 'A';		sm4_state = SM4_FourthCode;

			}
			else if (x == 'B') {
				newCode[2] = 'B';		sm4_state = SM4_FourthCode;

			}
			else if (x == 'C') {
				newCode[2] = 'C';		sm4_state = SM4_FourthCode;

			}
			else if (x == 'D') {
				newCode[2] = 'D';		sm4_state = SM4_FourthCode;

			}
			else {
				sm4_state = SM4_ThirdCode;
			}
			break;
		case SM4_FourthCode:
			if (x == '1') {
				newCode[3] = '1';
				sm4_state = SM4_Change;
			}
			else if (x == '2') {
				newCode[3] = '2';	sm4_state = SM4_Change;

			}
			else if (x == '3') {
				newCode[3] = '3';	sm4_state = SM4_Change;

			}
			else if (x == '4') {
				newCode[3] = '4';	sm4_state = SM4_Change;

			}
			else if (x == '5') {
				newCode[3] = '5';	sm4_state = SM4_Change;

			}
			else if (x == '6') {
				newCode[3] = '6';	sm4_state = SM4_Change;

			}
			else if (x == '7') {
				newCode[3] = '7';	sm4_state = SM4_Change;

			}
			else if (x == '8') {
				newCode[3] = '8';	sm4_state = SM4_Change;

			}
			else if (x == '9') {
				newCode[3] = '9';	sm4_state = SM4_Change;

			}
			else if (x == '0') {
				newCode[3] = '0';	sm4_state = SM4_Change;

			}
			else if (x == 'A') {
				newCode[3] = 'A';	sm4_state = SM4_Change;

			}
			else if (x == 'B') {
				newCode[3] = 'B';	sm4_state = SM4_Change;

			}
			else if (x == 'C') {
				newCode[3] = 'C';	sm4_state = SM4_Change;

			}
			else if (x == 'D') {
				newCode[3] = 'D';	sm4_state = SM4_Change;

			}
			else {
				sm4_state = SM4_FourthCode;
			}
			break;
		case SM4_Change:
			for (int i = 0; i < 4; i++) {
				keys[i] = newCode[i];
			}
			changed = 1;
			state = SM4_Start;
			break;
		default:
			state = SM4_Start;
			break;


	}

	return state;
}

int main(void) {
    /* Insert DDR and PORT initializations */
	DDRA = 0x00; PORTA = 0xFF;
	DDRB = 0x7F; PORTB = 0x80;
	DDRC = 0xF0; PORTC = 0x0F;

	static task task1, task2, task3, task4;
	task *tasks[] = { &task1, &task2, &task3, &task4 };
	unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	task1.state = Start;
	task1.period = 50;
	task1.elapsedTime = task1.period;
	task1.TickFct = &SM_Tick;

	task2.state = SM2_Start;
	task2.period = 50;
	task2.elapsedTime = task2.period;
	task2.TickFct = &SM2_Tick;

	task3.state = SM3_Start;
	task3.period = 200;
	task3.elapsedTime = task3.period;
	task3.TickFct = &SM3_Tick;

	task4.state = SM4_Start;
	task4.period = 50;
	task4.elapsedTime = task4.period;
	task4.TickFct = &SM4_Tick;

	TimerSet(50);
	TimerOn();
	PWM_on();

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

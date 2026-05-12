#ifndef COMD_H
#define COMD_H

#include <stdint.h>

typedef enum
{
	check_CPD,
	page,
	addr,
	check_eq,
	value1,
	value2,
	check_enter
} state;

typedef void (*task_ptr)(void);

typedef struct
{
	uint8_t key;
	state next_state;
	task_ptr task;
	uint8_t val;
} fsm_node;

state p_state;

volatile int state_i;
volatile char rx_char;

uint8_t set_toggle_LCD;
uint8_t set_config_sensor;
uint8_t set_page_num;
uint8_t set_reg_value;
uint8_t set_reg_addr;

void init_avr_USART3(void);
void USART3_Comd_parse_fsm(state, uint8_t);

#endif
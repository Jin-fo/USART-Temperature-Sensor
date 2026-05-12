/********************************************************************
 * File:        comd.c
 * Author:      Jin Yuan Chen
 * Created:     4/24/2026
 *
 * @file comd.c
 * @brief Command-driven finite state machine (FSM) for USART3 parsing.
 *
 * @details
 * This module implements a UART-based FSM parser that interprets
 * serial commands to control:
 * - Register address (reg_addr)
 * - Register value (reg_value)
 * - Page selection (page_num)
 * - Execution trigger (config flag)
 *
 * Input is processed in real-time via USART3 RX interrupt.
 ********************************************************************/

#include "global.h"
#include "comd.h"

//=================================================================//
//                     FSM GLOBAL CONTROL FLAGS                    //
//=================================================================//

/**
 * @brief RX ready flag
 */
volatile uint8_t rx_ready = 1;

//=================================================================//
//                        FORWARD DECLARATIONS                     //
//=================================================================//

void nop(void);
void set_addr(void);
void set_val1(void);
void set_val2(void);
void set_page(void);
void tog_LCD(void);
void set_update(void);


//=================================================================//
//                         FSM STATE TABLES                        //
//=================================================================//

/**
 * @brief FSM: Check command prefix ('C' or 'P')
 *
 * Pseudo-logic:
 * @code
 * if input == 'C' -> go to addr state
 * if input == 'P' -> go to page state
 * else -> stay in check state
 * @endcode
 */
const fsm_node is_check_CPD [] =
{
	//valid key
    {'C', addr,        nop, 0},
    {'P', page,        nop, 0},
	{'D', check_enter, tog_LCD,  0},
	//invalid key
    {',', check_CPD,   nop,      0}
};

/**
 * @brief FSM: Parse register address
 */
const fsm_node is_addr [] =
{
	//valid key
    {'0', check_eq, set_addr, 0},
    {'1', check_eq, set_addr, 1},
    {'2', check_eq, set_addr, 2},
    {'3', check_eq, set_addr, 3},
    {'4', check_eq, set_addr, 4},
    {'5', check_eq, set_addr, 5},
    {'6', check_eq, set_addr, 6},
    {'7', check_eq, set_addr, 7},
    {'8', check_eq, set_addr, 8},
    {'9', check_eq, set_addr, 9},
	//invalid key
    {',', check_CPD, nop,     0}
};

/**
 * @brief FSM: Parse page selection
 */
const fsm_node is_page [] =
{
	//valid key
    {'0', check_enter,	set_page, 0},
    {'1', check_enter,	set_page, 1},
	{'2', check_enter,	set_page, 2},
	//invalid key
    {',', check_CPD,	nop,      0}
};

/**
 * @brief FSM: Check '=' separator
 */
const fsm_node is_check_eq [] =
{
	//valid key
    {'=', value1,    nop,  0},
	//invalid key
    {',', check_CPD, nop,     0}
};

/**
 * @brief FSM: First digit of value
 */
const fsm_node is_value1 [] =
{
	//valid key
    {'0', value2,		set_val1, 0},
    {'1', value2,		set_val1, 1},
    {'2', value2,		set_val1, 2},
    {'3', value2,		set_val1, 3},
    {'4', value2,		set_val1, 4},
    {'5', value2,		set_val1, 5},
    {'6', value2,		set_val1, 6},
    {'7', value2,		set_val1, 7},
    {'8', value2,		set_val1, 8},
    {'9', value2,		set_val1, 9},
	{'A', value2,		set_val1, 0x0A},
	{'B', value2,		set_val1, 0x0B},
	{'C', value2,		set_val1, 0x0C},
	{'D', value2,		set_val1, 0x0D},
	{'E', value2,		set_val1, 0x0E},
	{'F', value2,		set_val1, 0x0F},
	//invalid key
    {',', check_CPD,	nop, 0}
};

/**
 * @brief FSM: Second digit of value
 */
const fsm_node is_value2 [] =
{
	//valid key
    {'0', check_enter,	set_val2, 0x00},
    {'1', check_enter,	set_val2, 0x01},
    {'2', check_enter,	set_val2, 0x02},
    {'3', check_enter,	set_val2, 0x03},
    {'4', check_enter,	set_val2, 0x04},
    {'5', check_enter,	set_val2, 0x05},
    {'6', check_enter,	set_val2, 0x06},
    {'7', check_enter,	set_val2, 0x07},
    {'8', check_enter,	set_val2, 0x08},
    {'9', check_enter,	set_val2, 0x09},
	{'A', check_enter,	set_val2, 0x0A},
	{'B', check_enter,	set_val2, 0x0B},
	{'C', check_enter,	set_val2, 0x0C},
	{'D', check_enter,	set_val2, 0x0D},
	{'E', check_enter,	set_val2, 0x0E},
	{'F', check_enter,	set_val2, 0x0F},
		
	{'\r', check_CPD,	set_update, 0},
	//invalid key
    {',', check_CPD,	nop, 0}
};

const fsm_node is_check_enter [] =
{	
	//valid key
	{'\r', check_CPD, set_update, 0},
	//invalid key
	{',', check_CPD, nop, 0}
	
};

/**
 * @brief FSM state lookup table
 */
const fsm_node *state_table[7] =
{
    is_check_CPD,
    is_page,
    is_addr,
    is_check_eq,
    is_value1,
    is_value2,
	is_check_enter,
};

//=================================================================//
//                        FSM ACTION FUNCTIONS                     //
//=================================================================//

void nop() {
}

/**
 * @brief Set register address
 */
void set_addr(void) {
    set_reg_addr = state_table[p_state][state_i].val;
}
/**
 * @brief Set first digit of value
 */
void set_val1(void) {
    set_reg_value = state_table[p_state][state_i].val;
	set_config_sensor = 1;
}

/**
 * @brief Set second digit and finalize value
 */
void set_val2(void) {
    set_reg_value = (set_reg_value << 4) | state_table[p_state][state_i].val;
	set_config_sensor = 1;
}

/**
 * @brief Set page number
 */
void set_page(void) { 
	set_page_num = state_table[p_state][state_i].val;
}

void tog_LCD(void) { 
	set_toggle_LCD = toggle_LCD ^ 1; 
}

/**
 * @brief Set update flag
 */
void set_update(void) {
	reg_addr = set_reg_addr;
	reg_value = set_reg_value;
	config_sensor = set_config_sensor;
	toggle_LCD = set_toggle_LCD;
	page_num = set_page_num;
}

//=================================================================//
//                     FSM PARSER CORE FUNCTION                    //
//=================================================================//

/**
 * @brief Process incoming UART character through FSM
 *
 * @param ps   Current FSM state
 * @param key  Input character
 *
 * @details
 * Pseudo-logic:
 * @code
 * 1. Reset index
 * 2. Search matching FSM entry
 * 3. Execute associated task
 * 4. Transition to next state
 * @endcode
 */
void USART3_Comd_parse_fsm(state ps, uint8_t key)
{
    state_i = 0;

    while ((state_table[ps][state_i].key != key) &&
           (state_table[ps][state_i].key != ','))
    {
        state_i++;
    }

    state_table[ps][state_i].task();
    p_state = state_table[ps][state_i].next_state;
}

//=================================================================//
//                    USART3 INTERRUPT HANDLER                    //
//=================================================================//

/**
 * @brief USART3 RX complete interrupt handler
 *
 * @details
 * Reads incoming byte and feeds it into FSM parser.
 */
ISR(USART3_RXC_vect)
{
    rx_char = USART3.RXDATAL;
    USART3_Comd_parse_fsm(p_state, rx_char);
}

//=================================================================//
//                    USART3 INITIALIZATION                        //
//=================================================================//

/**
 * @brief Initialize USART3 for FSM command input
 *
 * @details
 * Configures baud rate, frame format, RX interrupt,
 * and input pin direction.
 */
void init_avr_USART3()
{
    USART3.BAUD  = 139;
    USART3.CTRLB = 0x80;
    USART3.CTRLC = 0x03;

    USART3.CTRLA |= USART_RXCIE_bm;

    PORTB.DIR &= ~PIN1_bm;
}
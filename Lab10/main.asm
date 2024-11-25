.nolist
.include "avr128db48def.inc"
.list

;***************************************************************************
;*
;* Title: ADC_MCP9700A_2.0
;* Author: Jin Y Chen
;* Version: 2.0
;* Last updated: 11/13/2024
;* Target: avr128db48
;*
;* DESCRIPTION
;*
;*
;*
;*
;* VERSION HISTORY
;* 1.0 Original version
;***************************************************************************
.cseg
initialize:
	;Enable Reference Voltage
	ldi r16, 0x03
	sts VREF_ADC0REF, r16

	;Enable Analog input
	ldi r16, 0x04
	sts PORTE_PIN3CTRL, r16

	;ADC Presec to 64
	ldi r16, 0x0A
	sts ADC0_CTRLC, r16

	;Positive ADC input
	ldi r16, 0x0B
	sts ADC0_MUXPOS, r16

	;ADC mode
	ldi r16, 0x01
	sts ADC0_CTRLA, r16

	//---SER_LED display with USART---//
	;Barud Rate
	ldi r16, LOW(1667)
	sts USART3_BAUDL, r16
	ldi r16, HIGH(1667)
	sts USART3_BAUDH, r16

	;Formate
	ldi r16, 0x03
	sts USART3_CTRLC, r16

	;transmitor
	ldi r16, 0x40
	sts USART3_CTRLB, r16

	;IO
	sbi VPORTB_DIR, 0 ;transmitor output
	cbi VPORTE_DIR, 3 ;reciever input

main:
;setup clear_screen
	rcall clear_screen

	;display_post_home
	rcall display_post_home
	rcall delay_1s

;re-display clear_screen
	get_data:
		rcall clear_screen
;check_start_convert
	start_convert:
		ldi r16, 0x01
		sts ADC0_COMMAND, r16

	check_convert:
		lds r16, ADC0_INTFLAGS
		sbrs r16, 0
		rjmp check_convert

		lds r16, ADC0_RESL
		lds r17, ADC0_RESH

;Task 3 and Task 4
	rcall to_voltage_2500
	//rcall to_voltage_5	;~work

	rcall push_data			;push data[r16L:r17H]

	rcall pull_data
	rcall display_bin
	rcall next_line

	rcall pull_data
	rcall display_hex
	rcall next_line

	rcall pull_data
	rcall display_dec
	rcall next_line

;Task 4
	rcall pull_data
	rcall display_tem

	rcall delay_1s
	rjmp get_data

;***************************************************************************
;* 
;* "display_bin" - display the binary voltage 
;*
;* Description:
;* 
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r16, r17
;*
;* Returns: r20
;*
;* Notes: 
;*	
;***************************************************************************
display_bin:
	//data formate
	ldi r20, 'B'
	rcall USART_TX
	ldi r20, 'I'
	rcall USART_TX
	ldi r20, ':'
	rcall USART_TX

	//write binary
	ldi r18, 4 ;loop control
write_bit_H:
	sbrs r17, 7
	ldi r20, '0'
	sbrc r17, 7
	ldi r20, '1'

	rcall USART_TX
	lsl r17
	dec r18
	brne write_bit_H

	ldi r18, 8
write_bit_L:
	sbrs r16, 7
	ldi r20, '0'
	sbrc r16, 7
	ldi r20, '1'

	rcall USART_TX
	lsl r16
	dec r18
	brne write_bit_L
	ret

;***************************************************************************
;* 
;* "display_hex" - display the hex-decimal voltage 
;*
;* Description:
;* 
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r16, r17
;*
;* Returns: r20
;*
;* Notes: 
;*	
;***************************************************************************
display_hex:
	//data formate
	ldi r20, 'H'
	rcall USART_TX
	ldi r20, 'x'
	rcall USART_TX
	ldi r20, ':'
	rcall USART_TX

	//write hex
	mov r19, r17				;r19(msig-HIGH-bit)
	mov r18, r17				;r18(lsig-HIGH-bit)

	rcall write_most_bit
	rcall write_least_bit

	mov r19, r16
	mov r18, r16

	rcall write_most_bit
	rcall write_least_bit
	ret

;***************************************************************************
;* 
;* "display_dec" - display the decimal voltage 
;*
;* Description:
;* 
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r16, r17
;*
;* Returns: r20
;*
;* Notes: 
;*	
;***************************************************************************
display_dec:
	//data format
	ldi r20, 'V'
	rcall USART_TX
	ldi r20, 'o'
	rcall USART_TX
	ldi r20, ':'
	rcall USART_TX

	rcall bin16_to_BCD
	mov r18, r24				;digit 5th
	rcall write_least_bit		
	sts dDec5, r20

	mov r19, r23				;digit 4th
	mov r18, r23				;digit 3th
	rcall write_most_bit
	sts dDec4, r20
	rcall write_least_bit
	sts dDec3, r20

	mov r19, r22				;digit 2th
	mov r18, r22				;digit 1th
	rcall write_most_bit
	sts dDec2, r20
	rcall write_least_bit
	sts dDec1, r20

	//units
	ldi r20, 'm'
	rcall USART_TX
	ldi r20, 'V'
	rcall USART_TX
	ret

;***************************************************************************
;* 
;* "display_tem" - display the temperature with off-set
;*
;* Description:
;* 
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r16, r17
;*
;* Returns: r20
;*
;* Notes: 
;*	10 mV = 1 Celius with -50C offset
;***************************************************************************
display_tem:
	ldi r20, 'T'
	rcall USART_TX
	ldi r20, 'm'
	rcall USART_TX
	ldi r20, ':'
	rcall USART_TX
//voltage offset in binary
	ldi r18, LOW(500)
	ldi r19, HIGH(500)

	sbc r16, r18
	sbc r17, r19
	
	brcs overflow
	rjmp write_decimal

	//check negative
overflow:
	ldi r20, 0x2D
	rcall USART_TX
	com r16
	ldi r18, 1
	adc r16, r18
	com r17

write_decimal:
	rcall bin16_to_BCD
	mov r18, r24				;digit 5th
	rcall write_least_bit		

	mov r19, r23				;digit 4th
	mov r18, r23				;digit 3th
	rcall write_most_bit
	rcall write_least_bit

	mov r19, r22				;digit 2th
	mov r18, r22				;digit 1th
	rcall write_most_bit

	//decimal point
	ldi r20, 0x2E
	rcall USART_TX

	rcall write_least_bit
	//units
	ldi r20, 0x43
	rcall USART_TX
	ret

.include "subroutines.inc"

;***************************************************************************
;*
;* Title: Recieve BUffer to LCD at High speed with interupt 
;* Author:				Jin Yuan Chen
;* Version:				1.0
;* Last updated:		
;* Target:				AVR128DB48 @ 4.0MHz
;*
;* DESCRIPTION
;* Uses a positive edge triggered pin change interrupt to transmite signal recieved to LCD
;*
;* VERSION HISTORY
;* 1.0 Original version
;***************************************************************************
.dseg
message: .byte 64
flag: .byte 1

.cseg					;start of code segment
reset:
 	jmp start			;reset vector executed a power ON

.org USART1_RXC_vect
	rjmp USART1_RX

start:
	;USART: Buad rate typical
	ldi r16, low(1667)
	sts USART3_BAUDL, r16
	ldi r16, high(1667)
	sts USART3_BAUDH, r16
	;USART: Baud Rate highspeed
	ldi r16, low(139)
	sts USART1_BAUDL, r16
	ldi r16, high(139)
	sts USART1_BAUDH, r16        
	;USART: Formate
	ldi r16, 0x03
	sts USART3_CTRLC, r16
	sts USART1_CTRLC, r16
	ldi r16, 0x40			;USART: TX
	sts USART3_CTRLB, r16
	ldi r16, 0x80			;USART: RX
	sts USART1_CTRLB, r16

	sbi VPORTB_DIR, 0		;output
	cbi VPORTC_DIR, 1		;input

	;Configure interrupt
	ldi r16, 0x80		
	sts USART1_CTRLA, r16

	ldi YL, low(message)
	ldi YH, high(message)

	sei						;enable global interrupts

main:

	ldi r16, '|'
	sts USART3_TXDATAL, r16
	ldi r16, '-'
	sts USART3_TXDATAL, r16

	ldi YL, low(message)
	ldi YH, high(message)

	ldi r16, 0x00
	sts flag, r16

	RX_complete:
		lds r16, flag
		cpi r16, 0x01
		breq rest
		rjmp RX_complete

;rest pointer
	rest:
	ldi YL, low(message)
	ldi YH, high(message)

;start transmite
	TX:
	TX_clear:
		lds r17, USART3_STATUS
		sbrs r17, 5
		rjmp TX_clear

	ld r16, Y+
	sts USART3_TXDATAL, r16

	TX_complete:
		cpi r16, 0x0A
		breq main
		rjmp TX

;Interrupt service routine for any PORTE pin change IRQ
USART1_RX:
	cli				;clear global interrupt enable, I = 0
	push r16		;save r16 then SREG, note I = 0
	in r16, CPU_SREG
	push r16

	lds r16, USART1_RXDATAL
	st Y+, r16

	cpi r16, 0x0A
	breq set_one
	rjmp set_zero

	set_one:
	ldi r16, 0x01
	sts flag, r16
	rjmp exit

	set_zero:
	ldi r16, 0x00
	sts flag, r16
	
	exit:
	pop r16			;restore SREG then r16
	out CPU_SREG, r16	;note I in SREG now = 0
	pop r16
	sei				;SREG I = 1
	reti			;return from PORTE pin change ISR
;Note: reti does not set I on an AVR128DB48

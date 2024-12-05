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

.cseg					;start of code segment
reset:
 	jmp start			;reset vector executed a power ON

.org PORTC_PORT_vect	;delay? let 1 complete character to recive before intrupte  
	jmp portc_isr		;vector for all PORTE pin change IRQs


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
	lds r16, PORTC_PIN1CTRL	;set ISC for PC0 to pos. edge
	ori r16, 0x02			
	sts PORTC_PIN1CTRL, r16

	sei						;enable global interrupts
main:
	ldi YL, low(message)
	ldi YH, high(message)
	
	USART_RX:
	lds r16, USART1_RXDATAL
	st Y+, r16

	RX_complete:
		cpi r16, 0x0A
		breq rest
		rjmp USART_RX

	rest:
	ldi YL, low(message)
	ldi YH, high(message)

	USART_TX:
	ld r16, Y+
	sts USART3_TXDATAL, r16
	
	TX_complete:
		cpi r16, 0x0A
		breq main
		rjmp USART_TX

	rjmp main


;Interrupt service routine for any PORTE pin change IRQ
portc_ISR:
	cli				;clear global interrupt enable, I = 0
	push r16		;save r16 then SREG, note I = 0
	in r16, CPU_SREG
	push r16

	;Determine which pins of PORTE have IRQs
	lds r16, PORTC_INTFLAGS	;check for PC0 IRQ flag set
	sbrc r16, 1
	rcall USART_TX			;execute subroutine for PC0

	pop r16			;restore SREG then r16
	out CPU_SREG, r16	;note I in SREG now = 0
	pop r16
	sei				;SREG I = 1
	reti			;return from PORTE pin change ISR
;Note: reti does not set I on an AVR128DB48

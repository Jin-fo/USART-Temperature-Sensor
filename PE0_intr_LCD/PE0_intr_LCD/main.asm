;***************************************************************************
;*
;* Title: Interrupt Driven Counting of Pushbutton Presses
;* Author:				Jin Yuan Chen
;* Version:				2.0
;* Last updated:		
;* Target:				AVR128DB48 @ 4.0MHz
;*
;* DESCRIPTION
;* Uses a positive edge triggered pin change interrupt to count the number
;* of times a pushbutton is pressed and display it on the LCD. The pushbutton is connected
;* to PE0. The counts are stored in a byte memory variable. The counts is display through
;* PB1 to SERLCD.
;*
;* VERSION HISTORY
;* 1.0 Original version
;***************************************************************************

.dseg
PB_count: .byte 1		;pushbutton presses count memory variable.

.cseg					;start of code segment
reset:
 	jmp start			;reset vector executed a power ON

.org PORTE_PORT_vect
	jmp porte_isr		;vector for all PORTE pin change IRQs


start:
	;USART: baud rate
	ldi r16, LOW(1667)
	sts USART3_BAUDL, r16
	ldi r16, HIGH(1667)
	sts USART3_BAUDH, r16

	;USART: format
	ldi r16, 0x03
	sts USART3_CTRLC, r16
	
	;USART: TX
	ldi r16, 0x40
	sts USART3_CTRLB, r16

    ; Configure I/O port
	sbi VPORTB_DIR, 0	;PB0 USART output- send signal to LCD
	cbi VPORTE_DIR, 0	;PE0 input- gets output from pushbutton debouce ckt.

	ldi r16, 0x00		;make initial count 0
	sts PB_count, r16

	;Configure interrupt
	lds r16, PORTE_PIN0CTRL	;set ISC for PE0 to pos. edge
	ori r16, 0x02
	sts PORTE_PIN0CTRL, r16

	sei					;enable global interrupts
    
main_loop:				;main program loop
	//clear
	ldi r19, '|'
	rcall USART_TX
	ldi r19, '-'
	rcall USART_TX

	//conversion
	lds r17, PB_count
	rcall to_ASCII_hex

	//transmite
	mov r19, r18
	rcall USART_TX
	mov r19, r17
	rcall USART_TX

	rjmp main_loop

;Interrupt service routine for any PORTE pin change IRQ
porte_ISR:
	cli				;clear global interrupt enable, I = 0
	push r16		;save r16 then SREG, note I = 0
	in r16, CPU_SREG
	push r16

	;Determine which pins of PORTE have IRQs
	lds r16, PORTE_INTFLAGS	;check for PE0 IRQ flag set
	sbrc r16, 0
	rcall PB_sub			;execute subroutine for PE0

	pop r16			;restore SREG then r16
	out CPU_SREG, r16	;note I in SREG now = 0
	pop r16
	sei				;SREG I = 1
	reti			;return from PORTE pin change ISR
;Note: reti does not set I on an AVR128DB48

;Subroutine called by porte_ISR
PB_sub:				;PE0's task to be done
	lds r16, PB_count		;get current count for PB
	inc r16					;increment count
	sts PB_count, r16		;store new count
	ldi r16, PORT_INT0_bm	;clear IRQ flag for PE0, change to 0x01 for avr
	sts PORTE_INTFLAGS, r16
	ret

to_ASCII_hex:
	mov r18, r17
	//masking ms 4-bit to ascii
	lsr r18
	lsr r18
	lsr r18
	lsr r18
	andi r18, 0x0f
	ori r18, 0x30
	//masking ls 4-bit to ascii
	andi r17, 0x0f
	ori r17, 0x30
	ret 

USART_TX:
	TX_clear:
		lds r20, USART3_STATUS
		sbrs r20, 5
		rjmp TX_clear

	sts USART3_TXDATAL, r19
	ret 



	
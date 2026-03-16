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
	lds r16, PB_count
	ldi r17, 0x00
	rcall bin16_to_BCD

	rcall to_ASCII_hex

	//transmite
	mov r19, r23
	rcall USART_TX
	mov r19, r22
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
	ldi r16, 0x01	;clear IRQ flag for PE0, change to 0x01 for avr
	sts PORTE_INTFLAGS, r16
	ret

to_ASCII_hex:
	//masking ms 4-bit to ascii
	mov r23, r22
	lsr r23
	lsr r23
	lsr r23
	lsr r23
	andi r23, 0x0f
	ori r23, 0x30
	//masking ls 4-bit to ascii
	andi r22, 0x0f
	ori r22, 0x30
	ret 

USART_TX:
	TX_clear:
		lds r20, USART3_STATUS
		sbrs r20, 5
		rjmp TX_clear

	sts USART3_TXDATAL, r19
	ret 


bin16_to_BCD:
	ldi r19, 0 ;high byte of divisor for div16u
	ldi r18, 10 ;low byte of the divisor for div16u

	rcall div16u ;divide original binary number by 10
	mov r22, r14 ;result is BCD digit 0 (least significant digit)
	rcall div16u ;divide result from first division by 10, gives digit 1
	swap r14 ;swap digit 1 for packing
	or r22, r14 ;pack

	rcall div16u ;divide result from second division by 10, gives digit 2
	mov r23, r14 ;place in r23
	rcall div16u ;divide result from third division by 10, gives digit 3
	swap r14 ;swap digit 3 for packing
	or r23, r14 ;pack

	rcall div16u ;divide result from fourth division by 10, gives digit 4
	mov r24, r14 ;place in r24

ret
;Subroutine div16u is from Atmel application note AVR200
;***************************************************************************
;*
;* "div16u" - 16/16 Bit Unsigned Division
;*
;* This subroutine divides the two 16-bit numbers
;*# "dd16uH:dd16uL" (dividend) and "dv16uH:dv16uL" (divisor).
;* The result is placed in "dres16uH:dres16uL" and the remainder in
;* "drem16uH:drem16uL".
;*  
;* Number of words :19
;* Number of cycles :235/251 (Min/Max)
;* Low registers used :2 (drem16uL,drem16uH)
;* High registers used  :5 (dres16uL/dd16uL,dres16uH/dd16uH,dv16uL,dv16uH,
;*    dcnt16u)
;*
;***************************************************************************
;***** Subroutine Register Variables
.def drem16uL=r14
.def drem16uH=r15
.def dres16uL=r16
.def dres16uH=r17
.def dd16uL =r16
.def dd16uH =r17
.def dv16uL =r18
.def dv16uH =r19
.def dcnt16u =r20
;***** Code
div16u:
	clr drem16uL			;clear remainder Low byte
	sub drem16uH,drem16uH	;clear remainder High byte and carry
	ldi dcnt16u,17			;init loop counter
d16u_1:
	rol dd16uL				;shift left dividend
	rol dd16uH
	dec dcnt16u				;decrement counter
	brne d16u_2				;if done
	ret						;return
d16u_2:
	rol drem16uL			;shift dividend into remainder
	rol drem16uH
	sub drem16uL,dv16uL		;remainder = remainder - divisor
	sbc drem16uH,dv16uH		;
	brcc d16u_3				;if result negative
	add drem16uL,dv16uL		;restore remainder
	adc drem16uH,dv16uH
	clc						;clear carry to be shifted into result
	rjmp d16u_1				;else
d16u_3: sec					;set carry to be shifted into result
	rjmp d16u_1
	

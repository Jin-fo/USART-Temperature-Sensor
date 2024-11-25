.dseg
post_home: .db "ESE280 Fall 2024", 0x0D, 0x0A, "Jin", 0x0D, 0x0A, "Laboratory 10", 0x0D, 0x0A, "Temperature Sensor", 0x00//next line

dLOW:  .byte 1
dHIGH: .byte 1

dDec1: .byte 1
dDec2: .byte 1
dDec3: .byte 1
dDec4: .byte 1
dDec5: .byte 1

.cseg
//---Subtrotine---//

;***************************************************************************
;* 
;* "clear_screen" - Clear the LCD
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
;* Parameters: 
;*
;* Returns: 
;*
;* Notes: 
;*
;***************************************************************************
clear_screen:
	ldi r20, '|'
	rcall USART_TX
	ldi r20, '-'
	rcall USART_TX
	ret

;***************************************************************************
;* 
;* "display_post_home" - show the home screen message
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
;* Parameters: 
;*
;* Returns: 
;*
;* Notes: 
;*
;***************************************************************************
display_post_home:
	//insert code here
	clear:
		ldi ZH, high(post_home)
		ldi ZL, LOW(post_home)

	start_write:
		lpm r16, Z+
		tst r16
		breq end_write
		mov r20, r16
		rcall USART_TX
		rjmp start_write

	end_write:
		ret

;***************************************************************************
;* 
;* "to_voltage_2500" - convert to voltage
;*
;* Description:
;*	volatage is in mV 
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r16, r17,
;*
;* Returns: r16, r17
;*
;* Notes: 
;*
;***************************************************************************
to_voltage_2500: 
	ldi r18, LOW(2500)
	ldi r19, HIGH(2500)
	rcall mpy16u			//use 24-bits or 3-bytes i.e r21 is empty
		
	//12 right shift by ignoring r18 + 4 right shift
	clc
	ldi r18, 4
	loop:
	lsr r20
	ror r19
	dec r18
	brne loop

	mov r16, r19
	mov r17, r20

	ret

to_voltage_5: //in V
	ldi r18, LOW(5)
	ldi r19, HIGH(5)
	rcall mpy16u //

	clc
	//not sure why shift 3 times, got by testing
	ldi r20, 3
	loop1:
	lsr r19
	ror r18
	dec r20
	brne loop1

	//result voltage stored
	mov r16, r18
	mov r17, r19
	ret

;***************************************************************************
;* 
;* "push_data" - push data memory
;*
;* Description:
;*	store that into 2 register of the data memory named dLOW:dHIGH
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
;* Returns: 
;*
;* Notes: 
;*
;***************************************************************************
push_data:
	sts dLOW,  r16
	sts dHIGH, r17
	ret

;***************************************************************************
;* 
;* "pull_data" - pull from data memory
;*
;* Description:
;*	read the 2 register of the data memory named dLOW:dHIGH
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: 
;*
;* Returns: r16, r17
;*
;* Notes: 
;*
;***************************************************************************
pull_data:
	lds r16,  dLOW
	lds r17, dHIGH
	ret

;***************************************************************************
;* 
;* "write_most_bit" - Get 4-Most bit
;*
;* Description:
;*	masking most signifcant bit(r19)
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r19
;*
;* Returns:
;*
;* Notes: 
;*
;***************************************************************************
write_most_bit:
	lsr r19
	lsr r19
	lsr r19
	lsr r19
	mov r20, r19
	rcall to_ASCII
	rcall USART_TX
	ret

;***************************************************************************
;* 
;* "write_least_bit" - Get 4-least bit
;*
;* Description:
;*	masking least signifcant bit(r18)
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r18
;*
;* Returns: 
;*
;* Notes: 
;*
;***************************************************************************
write_least_bit:
	andi r18, 0x0f
	mov r20, r18
	rcall to_ASCII
	rcall USART_TX
	ret

;***************************************************************************
;* 
;* "to_ASCII" - convert to ASCII character
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
;* Parameters: write_least_bit, write_most_bit,
;*
;* Returns:
;*
;* Notes: 
;*
;***************************************************************************
to_ASCII:
	cpi r20, 10
	brsh concat_x4
	concat_x3:
	ori r20, 0x30
	ret

	concat_x4:
	ori r20, 0x40
	subi r20, 9
	ret

;***************************************************************************
;* 
;* "next_line" - go to next line
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
;* Parameters:
;*
;* Returns: r20
;*
;* Notes: 
;*
;***************************************************************************
next_line:
	ldi r20, 0x0D //new_line
	rcall USART_TX
	ldi r20, 0x0A
	rcall USART_TX

;***************************************************************************
;* 
;* "USART_TX" - Send with USART transmit
;*
;* Description:
;* Send ASCII character in r20 for USART to transmit
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r20
;*
;* Returns: 
;*
;* Notes: 
;*
;***************************************************************************
USART_TX:
	lds r21, USART3_STATUS
	sbrs r21, 5
	rjmp USART_TX

	sts USART3_TXDATAL, r20
	ret

;***************************************************************************
;*
;* "mpy16u" - 16x16 Bit Unsigned Multiplication
;*
;* This subroutine multiplies the two 16-bit register variables
;* mp16uH:mp16uL and mc16uH:mc16uL.
;* The result is placed in m16u3:m16u2:m16u1:m16u0.
;*  
;* Number of words :14 + return
;* Number of cycles :153 + return
;* Low registers used :None
;* High registers used  :7 (mp16uL,mp16uH,mc16uL/m16u0,mc16uH/m16u1,m16u2,
;*                          m16u3,mcnt16u)
;*
;***************************************************************************
;***** Subroutine Register Variables
.def mc16uL =r16 ;multiplicand low byte
.def mc16uH =r17 ;multiplicand high byte
.def mp16uL =r18 ;multiplier low byte
.def mp16uH =r19 ;multiplier high byte

.def m16u0 =r18 ;result byte 0 (LSB)
.def m16u1 =r19 ;result byte 1
.def m16u2 =r20 ;result byte 2
.def m16u3 =r21 ;result byte 3 (MSB)
.def mcnt16u =r22 ;loop counter
;***** Code
mpy16u:
	clr m16u3 ;clear 2 highest bytes of result
	clr m16u2
	ldi mcnt16u,16 ;init loop counter
	lsr mp16uH
	ror mp16uL

m16u_1:
	brcc noad8 ;if bit 0 of multiplier set
	add m16u2,mc16uL ;add multiplicand Low to byte 2 of res
	adc m16u3,mc16uH ;add multiplicand high to byte 3 of res
noad8:
	ror m16u3 ;shift right result byte 3
	ror m16u2 ;rotate right result byte 2
	ror m16u1 ;rotate result byte 1 and multiplier High
	ror m16u0 ;rotate result byte 0 and multiplier Low
	dec mcnt16u ;decrement loop counter
	brne m16u_1 ;if not done, loop more
	ret

;***************************************************************************
;*
;* "bin16_to_BCD" - 16-bit Binary to BCD Conversion
;*
;* Description: Converts a 16-bit unsigned binary number to a five digit
;* packed BCD number. Uses subroutine div16u from Atmel application note AVR200
;*
;* Author: Ken Short
;* Version: 0.0
;* Last updated: 111320
;* Target: ATmega4809 or AVR128DB48
;* Number of words:
;* Number of cycles:
;* Low registers modified: r14, r15
;* High registers modified: r16, r17, r18, r19, r20, r22, r23, r24
;*
;* Parameters: r17:r16 16-bit unsigned right justified number to be converted.
;* Returns: r24:r23:r22 five digit packed BCD result.
;*
;* Notes:
;* Subroutine uses repeated division by 10 to perform conversion.
;* Calls div16u from application note AVR200
;***************************************************************************
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

;***************************************************************************
;* 
;* "delay_1s" - Response delay seconds
;*
;* Description:
;* Send ASCII character in r20 for USART to transmit
;* Author:
;* Version:
;* Last updated:
;* Target:
;* Number of words:
;* Number of cycles:
;* Low registers modified:
;* High registers modified:
;*
;* Parameters: r20
;*
;* Returns: 
;*
;* Notes: 
;*
;***************************************************************************
delay_1s:
	ldi r30, low(5234) ;input update delay restriction
	ldi r31, high(5234)
	outer_loop:
	ldi r29, $ff
	inner_loop:
	dec r29
	brne inner_loop
	sbiw r31:r30, 1
	brne outer_loop
	ret


/*ESE280 Fall 2024
<student’s name>
Laboratory 09
<shortened lab title>*/
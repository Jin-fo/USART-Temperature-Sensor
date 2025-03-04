.nolist
.include "avr128db48def.inc"
.list

.dseg
message: .byte 64
.cseg

start:
	ldi r16, low(1667)
	sts USART3_BAUDL, r16
	ldi r16, high(1667)
	sts USART3_BAUDH, r16

	ldi r16, low(139)
	sts USART1_BAUDL, r16
	ldi r16, high(139)
	sts USART1_BAUDH, r16        

	ldi r16, 0x03
	sts USART3_CTRLC, r16
	sts USART1_CTRLC, r16

	ldi r16, 0x40
	sts USART3_CTRLB, r16
	ldi r16, 0x80
	sts USART1_CTRLB, r16

	sbi VPORTB_DIR, 0
	cbi VPORTC_DIR, 1

main:
	ldi YL, low(message)
	ldi YH, high(message)
	
	RX_clear:
		lds r17, USART1_RXDATAH;
		sbrs r17, 7
		rjmp RX_clear

	lds r16, USART1_RXDATAL
	st Y+, r16

	RX_complete:
		cpi r16, 0x0A
		breq TX
		rjmp RX_clear

	TX: 
	ldi YL, low(message)
	ldi YH, high(message)

	TX_clear:
		lds r17, USART3_STATUS
		sbrs r17, 5
		rjmp TX_clear

	ld r16, Y+
	sts USART3_TXDATAL, r16
	
	TX_complete:
		cpi r16, 0x0A
		breq main
		rjmp TX_clear

	rjmp main

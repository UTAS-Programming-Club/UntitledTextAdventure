	org $BFF0 ; start at header?

	; header info
	db "NES",$1a     ; NES header magic
	db $1            ; PRG-ROM pages
	db $1            ; CHR-ROM pages
	db %00000000     ; mapper info
	db %00000000     ; ...
	db 0             ; ram pages
	db 0,0,0,0,0,0,0 ; unused

; GLOBAL VARIABLES
CURRENT_OPTION equ $0304
MAX_OPTION     equ $0301
FRESH		   equ $0303
TILEMAP        equ $0700 ; this should fit perfectly fine

TEXT_PTR   equ $0010 ;  $10 should be fine too? zero page
SCREEN_PTR equ $0012 ; two after should be safe
OPTION_PTR equ $0014
PTR        equ $0016

BUTTONS    equ $0018 ; main buttons query
A          equ $0019
UP         equ $001A
DOWN       equ $001B

TEXT       equ $001C ; holds two bytes as a pointer so it can be looped through
SCREEN_POS equ $001E ; required to cache last point
OPT_OFFSET equ $0020 ; offset into opt table
Y_PTR	   equ $0022 ; instead of PTR
TEXT_Y     equ $0024 ; we have the space to store this information
TEMP_POS   equ $0025 ; required for options offsetting

WATCH1 equ $0305

inc_scpos:
	clc
	lda SCREEN_POS
	adc #1
	sta SCREEN_POS
	lda SCREEN_POS+1
	adc #0
	sta SCREEN_POS+1
	rti

; every time the screen refreshes
nmihandler:
	; if fresh? continue clearing text
	lda FRESH
	cmp #1
	bne _nmi_draw_text_if

	; is fresh at the end point? $23C0
	lda SCREEN_POS
	cmp #$C0
	bne _nmi_continue_clear
	lda SCREEN_POS+1
	cmp #$23
	beq _nmi_clear_over
_nmi_continue_clear:

	; clear screen
	lda SCREEN_POS+1
	sta $2006
	lda SCREEN_POS
	sta $2006
	
	ldx #0
_nmi_loop:
	lda #$0
	sta $2007

	clc
	lda SCREEN_POS
	adc #1
	sta SCREEN_POS
	lda SCREEN_POS+1
	adc #0
	sta SCREEN_POS+1

	inx
	cpx #64
	bne _nmi_loop

	jmp _nmi_else ; no point trying to draw text
_nmi_clear_over:
	lda #0
	sta FRESH
	sta SCREEN_POS
	lda #$20
	sta SCREEN_POS+1
	
	jmp _nmi_else
_nmi_draw_text_if:
	; is text pointing at zero?
	ldy #0
	lda (TEXT), y
	cmp #0
	bne _nmi_draw_text_ignore ; end now

	jmp _nmi_draw_options
_nmi_draw_text_ignore:

	; load initial screen_pos
	lda SCREEN_POS+1
	sta $2006
	lda SCREEN_POS
	sta $2006
	
	lda (TEXT),y
	; newline? very scary!
	cmp #10
	beq _nl_routine

	clc
	sbc #"."

	; render
	sta $2007 ; written, and done with

	; now fix stuff for next loop
	; load next screen pos
	clc
	lda SCREEN_POS
	adc #1
	sta SCREEN_POS
	lda SCREEN_POS+1
	adc #0
	sta SCREEN_POS+1

	clc
	lda TEXT
	adc #1
	sta TEXT
	lda TEXT+1
	adc #0
	sta TEXT+1

	jmp _nmi_draw_options 
_nl_routine:
	; increment y
	ldy TEXT_Y
	iny
	sty TEXT_Y

	; set screen to next position as per y
	lda #<y_offsets_low
	sta Y_PTR
	lda #>y_offsets_low
	sta Y_PTR+1

	; think we can do this?
	lda (Y_PTR), y
	sta SCREEN_POS

	lda #<y_offsets_high
	sta Y_PTR
	lda #>y_offsets_high
	sta Y_PTR+1

	lda (Y_PTR), y
	sta SCREEN_POS+1

	; progress text
	clc
	lda TEXT
	adc #1
	sta TEXT
	lda TEXT+1
	adc #0
	sta TEXT+1

_nmi_draw_options:
	ldx #0
_nmi_opt_loop:
	; draw at location
	; loads Y position
	txa ; move x into a
	clc
	; add opt_offset to a
	adc OPT_OFFSET
	tay ; transfer to Y as offset

	; copied from earlier
	lda #<y_offsets_low
	sta Y_PTR
	lda #>y_offsets_low
	sta Y_PTR+1

	lda (Y_PTR), y
	sta TEMP_POS 

	lda #<y_offsets_high
	sta Y_PTR
	lda #>y_offsets_high
	sta Y_PTR+1

	lda (Y_PTR), y
	sta TEMP_POS+1

	lda TEMP_POS+1
	sta $2006
	lda TEMP_POS
	sta $2006

	; draw nothing? or something
	cpx CURRENT_OPTION
	beq _nmi_opt_selected

	lda #0
	sta $2007

	jmp _nmi_opt_else
_nmi_opt_selected:
	; clear otherwise
	lda #15
	sta $2007

_nmi_opt_else:
	inx
	cpx MAX_OPTION
	bne _nmi_opt_loop ; not at max options? try again

_nmi_else:
	; set scroll every frame bc I am lazy
	lda #$00
	sta $2005
	sta $2005
	
	lda #$02  ; transfer from $02
	sta $4014 ; $0200 -> DMA

	rti       ; return

; irq handler
irqhandler:
	rti

startgame:
	sei ; disable interrupts
	cld ; clear decimal

	ldx #$ff
	txs ; set up stack
	inx ; x := 0
	stx $2000 ; disable graphics options
	stx $2001 ; screen of
	stx $4015 ; disable sound
	stx $4010 ; disable DMC
	lda #$40
	sta $4017 ; disable sound IRQ
	lda #0
waitvblank:
	bit $2002 ; check PPU status
	bpl waitvblank
	lda #0
clearmemory: ; clear from $0000-$07FF
	sta $0000,x
	sta $0100,x
	sta $0300,x
	sta $0400,x
	sta $0500,x
	sta $0600,x
	sta $0700,x
	lda #$FF
	sta $0200,x
	inx ; x :{ 1 - 255
	cpx #$00 ; cmp to 0
	bne clearmemory
init:
	lda #$0
	sta CURRENT_OPTION
	sta MAX_OPTION
	sta FRESH ; don't need to clear 
	sta A
	sta UP
	sta DOWN
	sta SCREEN_POS
	sta TEXT_Y

	lda #1
	sta FRESH ; testing clearing behaviour

	lda #$20
	sta SCREEN_POS+1
waitvblank2:
	bit $2002
	bpl waitvblank2 ; PPU check

	lda $2002
	lda #$3F
	sta $2006
	lda #$00
	sta $2006
	ldx #$00
copypalloop:
	lda initial_palette,x
	sta $2007
	inx
	cpx #4
	bne copypalloop

	lda #$02
	sta $4014

	; somehow forgot this?
	lda #<title_setup
	sta SCREEN_PTR
	lda #>title_setup
	sta SCREEN_PTR+1

	jsr screen_toptr
	jsr arbitrary_subroutine

	lda #%10000000 ; NMI on vblank
	sta $2000

	lda #%00011110 ; sprites, background
	sta $2001


; loop {}
forever:

loop_waitvblank:
	bit $2002 ; check PPU status
	bpl loop_waitvblank

	; reading joypad 1
readjoy:
	lda #$01
	sta $4016
	sta BUTTONS
	lsr
	sta $4016 ; register is now filled
_readjoy_loop:
	lda $4016
	lsr
	rol BUTTONS
	bcc _readjoy_loop

;;  A
	lda BUTTONS
	and #%10000000
	cmp #0
	beq _readjoy_nota
	lda #1
	sta A
	jmp _readjoy_else1
_readjoy_nota:
	lda A
	cmp #1
	bne _readjoy_full_release
	lda #2
	sta A
	jmp _readjoy_else1
_readjoy_full_release:
	lda #0
	sta A
_readjoy_else1:

;;  UP
	lda BUTTONS
	and #%00001000
	cmp #0
	beq _readjoy_notup
	lda #1
	sta UP
	jmp _readjoy_else2
_readjoy_notup:
	lda UP
	cmp #1
	bne _readjoy_full_uprelease
	lda #2
	sta UP
	jmp _readjoy_else2
_readjoy_full_uprelease:
	lda #0
	sta UP
_readjoy_else2:

;;  DOWN
	lda BUTTONS
	and #%00000100
	cmp #0
	beq _readjoy_notdown
	lda #1
	sta DOWN
	jmp _readjoy_else3
_readjoy_notdown:
	lda DOWN
	cmp #1
	bne _readjoy_full_downrelease
	lda #2
	sta DOWN
	jmp _readjoy_else3
_readjoy_full_downrelease:
	lda #0
	sta DOWN
_readjoy_else3:

;;  ACTION
	lda A
	cmp #2
	bne _forever_noa
	jsr select_option
_forever_noa:
	lda UP
	cmp #2
	bne _forever_noup
	lda #-1
	jsr move_cursor
_forever_noup:
	lda DOWN
	cmp #2
	bne _forever_nodown
	lda #1
	jsr move_cursor
_forever_nodown:

	jmp forever

select_option:
	jsr option_toptr
	jsr arbitrary_subroutine
	jsr screen_toptr ; now reset
	jsr arbitrary_subroutine

	lda #1
	sta FRESH
	lda #0
	sta TEXT_Y
	sta SCREEN_POS
	lda #$20
	sta SCREEN_POS+1

	rts

move_cursor:
	; requires lda to be set appropriately
	cmp #-1
	beq _move_cursor_up
_move_cursor_down:
	ldx CURRENT_OPTION
	inx
	cpx MAX_OPTION
	beq _cursor_correct
	stx CURRENT_OPTION
	rts ; otherwise it should be fine :3
_cursor_correct:
	lda #0
	sta CURRENT_OPTION
	rts
_move_cursor_up:
	ldx CURRENT_OPTION
	dex
	cpx #-1
	beq _cursorup_correct
	stx CURRENT_OPTION
	rts
_cursorup_correct:
	ldx MAX_OPTION
	dex
	stx CURRENT_OPTION
	rts

arbitrary_subroutine:
	jmp (PTR)
	rts

screen_toptr:
	lda SCREEN_PTR
	sta PTR
	lda SCREEN_PTR+1
	sta PTR+1
	rts

option_toptr:
	lda OPTION_PTR
	sta PTR
	lda OPTION_PTR+1
	sta PTR+1
	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

title_setup:
	lda #<title
	sta TEXT
	lda #>title
	sta TEXT+1

	lda #<title_options
	sta OPTION_PTR
	lda #>title_options
	sta OPTION_PTR+1

	lda #2
	sta MAX_OPTION

	lda #6 ; guessing
	sta OPT_OFFSET 

	rts
title_options:	
	lda CURRENT_OPTION
	cmp #0 ; can skip this eventually
	bne _title_opt_1

_title_opt_0:
	lda #<room_intro
	sta SCREEN_PTR
	lda #>room_intro
	sta SCREEN_PTR+1
	
	rts
_title_opt_1:
	; just loop back to now
	lda #<title_setup
	sta SCREEN_PTR
	lda #>title_setup
	sta SCREEN_PTR+1
	
	rts

room_intro:
	lda #<firstroom
	sta TEXT
	lda #>firstroom
	sta TEXT+1
	
	lda #<room_intro_options
	sta OPTION_PTR
	lda #>room_intro_options
	sta OPTION_PTR+1

	lda #2
	sta MAX_OPTION
	
	lda #3 ; guessing again
	sta OPT_OFFSET

	rts
room_intro_options:
	lda CURRENT_OPTION
	cmp #0 ; can skip this eventually
	bne _intro_opt_1

_intro_opt_0:
	lda #<room_ext
	sta SCREEN_PTR
	lda #>room_ext
	sta SCREEN_PTR+1
	
	rts
_intro_opt_1:
	lda #<title_setup
	sta SCREEN_PTR
	lda #>title_setup
	sta SCREEN_PTR+1
	
	rts

room_ext:
	lda #<extroom
	sta TEXT
	lda #>extroom
	sta TEXT+1
	
	lda #<room_ext_options
	sta OPTION_PTR
	lda #>room_ext_options
	sta OPTION_PTR+1

	lda #2
	sta MAX_OPTION
	
	lda #3
	sta OPT_OFFSET

	rts
room_ext_options:
	lda CURRENT_OPTION
	cmp #0 ; can skip this eventually
	bne _ext_opt_1

_ext_opt_0:
	lda #<room_intro
	sta SCREEN_PTR
	lda #>room_intro
	sta SCREEN_PTR+1
	
	rts
_ext_opt_1:
	lda #<title_setup
	sta SCREEN_PTR
	lda #>title_setup
	sta SCREEN_PTR+1
	
	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

clear_nametable:
	; starts at $2000
	lda #$20
	sta $2006
	lda #$00
	sta $2006

	; write 960 bytes
	; 30 x 32
	ldx #$0
	ldy #$0

	lda #0 ; first tile should be okay?
_clear_nametable_loop:
	sta $2007
	inx

	cpx #30
	bne _clear_nametable_dinc_x
	
	iny
	ldx #$0

_clear_nametable_dinc_x:
	cpy #32
	bne _clear_nametable_loop

	; reset dma position
	lda #$20
	sta $2006
	lda #$00
	sta $2006

	rts

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

initial_palette:
	db $1F,$21,$30,$30
y_offsets_high:
	db $20, $20, $20, $20, $20, $20, $20, $20, $21, $21, $21, $21, $21, $21, $21, $21, $22, $22, $22, $22, $22, $22, $22, $22, $23, $23, $23, $23, $23, $23, $23, $23
y_offsets_low:
	db $00, $20, $40, $60, $80, $A0, $C0, $E0, $00, $20, $40, $60, $80, $A0, $C0, $E0, $00, $20, $40, $60, $80, $A0, $C0, $E0, $00, $20, $40, $60, $80, $A0, $C0, $E0

title:
	db 10,"UNTITLED:TEXT:ADVENTURE:GAME",10,10,"BY:THE:UTAS:PROGRAMMING:CLUB",10,"CURRENTLY:UNIMPLEMENTED:",10,10,"::START:GAME",10,"::QUIT",0
firstroom:
	db 10,"FIRST:ROOM!",10,10,"::GO:EAST",10,"::QUIT",0
extroom:
	db 10,"EXTENSION:ROOM!",10,10,"::GO:WEST",10,"::QUIT",0
option:
	db "> ",0

	org $FFFA
	dw nmihandler
	dw startgame
	dw irqhandler

chr_rom_start:
	include "font.asm"

; safety padding :3c
chr_rom_end:
	ds 8192-(chr_rom_end-chr_rom_start)

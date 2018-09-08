;redcode-94
;name Test
;author Aritz Erkiaga
;assert 1

	for	0
	This is a validation warrior f*r hMARS.
	Its goal is to lose or tie (win) depending
	on the loaded code and its behavior, so
	that the slightest difference may become
	apparent without having to compare core
	contents.
	It uses a custom pseudo-instruction, 'XXX'
	that behaves like a normal instruction
	except from the fact that it's loaded as
	a random instruction, with both fields set
	to random values between the A and B
	fields, inclusive. This means that it will
	probably not run on other simulators.
	rof

SIZE	equ	16
DIST	equ	8
TIME	equ	100

point	spl	area
count   djn.b	0,	#TIME
loop	add.f	}point,	count
	djn.b	loop,	#SIZE
	add.ab	count,	count
	mod.b	count,	#2
	jmz.b	clear,	count
	jmp	#0	;self-tie
bomb
	for	DIST
	dat	0,	0
	rof
area
	for	SIZE
	xxx	-DIST,	DIST
	rof

	for	DIST
	dat	0,	0
	rof

clear	mov.i	bomb,	area
	jmp	clear,	>clear

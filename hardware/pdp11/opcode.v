//
// Single-operand instructions.
//
`define	CLR		10'o0050	// d = 0
`define	CLRB		10'o1050
`define	COM		10'o0051	// d = ~b
`define	COMB		10'o1051
`define	INC		10'o0052	// d = b + 1
`define	INCB		10'o1052
`define	DEC		10'o0053	// d = b - 1
`define	DECB		10'o1053
`define	NEG		10'o0054	// d = 0 - b
`define	NEGB		10'o1054
`define	TST		10'o0057	// d = b
`define	TSTB		10'o1057
`define	ASR		10'o0062	// d|c = b >> 1
`define	ASRB		10'o1062
`define	ASL		10'o0063	// c|d = b << 1
`define	ASLB		10'o1063
`define	ROR		10'o0060	// d|c = c|b
`define	RORB		10'o1060
`define	ROL		10'o0061	// c|d = b|c
`define	ROLB		10'o1061
`define	SWAB		10'o0003	// d = swab (b)
`define	ADC		10'o0055	// d = b + c
`define	ADCB		10'o1055
`define	SBC		10'o0056	// d = b - c
`define	SBCB		10'o1056
`define	SXT		10'o0067	// d = n ? -1 : 0
`define	MFPS		10'o1067	// d = ps
`define	MTPS		10'o1064	// ps = b

// Nonstandard commands.
`define	INC2		10'o0072	// d = b + 2
`define	DEC2		10'o0073	// d = b - 2

//
// Double-operand instructions.
//
`define	MOV		10'o01??	// d = a
`define	MOVB		10'o11??
`define	CMP		10'o02??	// d = a - b (no register store)
`define	CMPB		10'o12??
`define	ADD		10'o06??	// d = a + b
`define	SUB		10'o16??	// d = b - a
`define	ASH		10'o072?	// d = a>0 ? b<<a | b>>(-a)
`define	ASHC		10'o073?	// TODO
`define	MUL		10'o070?	// TODO
`define	DIV		10'o071?	// TODO
`define	BIT		10'o03??	// d = a & b (no register store)
`define	BITB		10'o13??
`define	BIC		10'o04??	// d = ~a & b
`define	BICB		10'o14??
`define	BIS		10'o05??	// d = a | b
`define	BISB		10'o15??
`define	XOR		10'o074?	// d = a ^ b

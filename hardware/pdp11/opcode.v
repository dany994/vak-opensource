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
`define	MOV		10'o01xx	// d = a
`define	MOVB		10'o11xx
`define	CMP		10'o02xx	// d = a - b (no register store)
`define	CMPB		10'o12xx
`define	ADD		10'o06xx	// d = a + b
`define	SUB		10'o16xx	// d = b - a
`define	ASH		10'o072x	// d = a>0 ? b<<a | b>>(-a)
`define	ASHC		10'o073x	// TODO
`define	MUL		10'o070x	// TODO
`define	DIV		10'o071x	// TODO
`define	BIT		10'o03xx	// d = a & b (no register store)
`define	BITB		10'o13xx
`define	BIC		10'o04xx	// d = ~a & b
`define	BICB		10'o14xx
`define	BIS		10'o05xx	// d = a | b
`define	BISB		10'o15xx
`define	XOR		10'o074x	// d = a ^ b

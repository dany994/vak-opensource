`timescale 1ns / 1ps
//
// ALU.
//
`include "opcode.v"

module alu (
	input wire [9:0] op,	// op code
	input wire [15:0] a,		// `source' register
	input wire [15:0] b,		// `destination' register
	input wire [7:0] ps,		// processor state register
	output reg [15:0] d,		// result
	output reg [7:0] psr		// processor state result
);

`define n	psr[3]			// negative result
`define z	psr[2]			// zero result
`define v	psr[1]			// overflow result
`define c	psr[0]			// carry result

	always @(op or a or b or ps or d) begin
		casez (op)
		default: begin
			d = b;
			psr = ps;
		end

		//
		// Single-operand instructions.
		//
		`CLR: begin		// d = 0
			d = 0;
			psr = ps;
			`c = 0;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`CLRB: begin
			d = { b[15:8], 8'd0 };
			psr = ps;
			`c = 0;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`COM: begin		// d = ~b
			d = ~b;
			psr = ps;
			`c = 1;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`COMB: begin
			d = { b[15:8], ~b[7:0] };
			psr = ps;
			`c = 1;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`INC: begin		// d = b + 1
			d = b + 1;
			psr = ps;
			`v = (b == 16'h7fff);
			`n = d[15];
			`z = (d == 0);
		end
		`INCB: begin
			d = { b[15:8], (b[7:0] + 1'd1) };
			psr = ps;
			`v = (b[7:0] == 8'h7f);
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`INC2: begin		// d = b + 2 (nonstandard)
			d = b + 2;
			psr = ps;
		end
		`DEC: begin		// d = b - 1
			d = b - 1;
			psr = ps;
			`v = (b == 16'h8000);
			`n = d[15];
			`z = (d == 0);
		end
		`DECB: begin
			d = { b[15:8], (b[7:0] - 1'd1) };
			psr = ps;
			`v = (b[7:0] == 8'h80);
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`DEC2: begin		// d = b - 2 (nonstandard)
			d = b - 2;
			psr = ps;
		end
		`NEG: begin		// d = 0 - b
			d = - b;
			psr = ps;
			`c = (d != 0);
			`v = (d == 16'h8000);
			`n = d[15];
			`z = (d == 0);
		end
		`NEGB: begin
			d = { b[15:8], -b[7:0] };
			psr = ps;
			`c = (d != 0);
			`v = (d[7:0] == 8'h80);
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`TST: begin		// d = b
			d = b;
			psr = ps;
			`c = 0;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`TSTB: begin
			d = b;
			psr = ps;
			`c = 0;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`ASR: begin		// d|c = b >> 1
			psr = ps;
			{ d, `c } = { b[15], b };
			`v = d[15] ^ `c;
			`n = d[15];
			`z = (d == 0);
		end
		`ASRB: begin
			psr = ps;
			{ d, `c } = { b[15:8], b[7], b[7:0] };
			`v = d[7] ^ `c;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`ASL: begin		// c|d = b << 1
			psr = ps;
			{ `c, d } = { b, 1'd0 };
			`v = d[15] ^ `c;
			`n = d[15];
			`z = (d == 0);
		end
		`ASLB: begin
			psr = ps;
			{ d[15:8], `c, d[7:0] } = { b, 1'd0 };
			`v = d[7] ^ `c;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`ROR: begin		// d|c = c|b
			psr = ps;
			{ d, `c } = { `c, b };
			`v = d[15] ^ `c;
			`n = d[15];
			`z = (d == 0);
		end
		`RORB: begin
			psr = ps;
			{ d, `c } = { b[15:8], `c, b[7:0] };
			`v = d[7] ^ `c;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`ROL: begin		// c|d = b|c
			psr = ps;
			{ `c, d } = { b, `c };
			`v = d[15] ^ `c;
			`n = d[15];
			`z = (d == 0);
		end
		`ROLB: begin
			psr = ps;
			{ d[15:8], `c, d[7:0] } = { b, `c };
			`v = d[7] ^ `c;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`SWAB: begin		// d = swab (b)
			d = { b[7:0], b[15:8] };
			psr = ps;
			`c = 0;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`ADC: begin		// d = b + c
			psr = ps;
			d = b + {15'h0, `c};
			`c = (b == 16'hffff && ps[0] == 1);
			`v = (b == 16'h7fff && ps[0] == 1);
			`n = d[15];
			`z = (d == 0);
		end
		`ADCB: begin
			psr = ps;
			d = { b[15:8], (b[7:0] + {7'h0, `c}) };
			`c = (b[7:0] == 8'hff && ps[0] == 1);
			`v = (b[7:0] == 8'h7f && ps[0] == 1);
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`SBC: begin		// d = b - c
			psr = ps;
			d = b - {15'h0, `c};
			`c = (b == 0) && (ps[0] != 0);
			`v = (b == 16'h8000);
			`n = d[15];
			`z = (d == 0);
		end
		`SBCB: begin
			psr = ps;
			d = { b[15:8], (b[7:0] - {7'h0, `c}) };
			`c = (b[7:0] == 0) && (ps[0] != 0);
			`v = (b[7:0] == 8'h80);
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`SXT: begin		// d = n ? -1 : 0
			d = ps[3] ? 16'hffff : 0;
			psr = ps;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`MFPS: begin		// d = ps
			d = { ps[7], ps[7], ps[7], ps[7],
				ps[7], ps[7], ps[7], ps[7], ps };
			psr = ps;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`MTPS: begin		// ps = b
			d = b;
			psr = { b[7:5], ps[4], b[3:0] };
		end

		//
		// Double-operand instructions.
		//
		`MOV: begin		// d = a
			d = a;
			psr = ps;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`MOVB: begin
			d = { b[15:8], a[7:0] };
			psr = ps;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`CMP: begin		// d = a - b (no register store)
			psr = ps;
			{ `c, d } = { 1'd1, a } - b;
			`v = (a[15] != b[15]) && (d[15] == b[15]);
			`n = d[15];
			`z = (d == 0);
		end
		`CMPB: begin
			psr = ps;
			{ d[15:8], `c, d[7:0] } = { b[15:8],
				({ 1'd1, a[7:0] } - b[7:0]) };
			`v = (a[7] != b[7]) && (d[7] == b[7]);
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`ADD: begin		// d = a + b
			psr = ps;
			{ `c, d } = a + b;
			`v = (a[15] == b[15]) && (d[15] != a[15]);
			`n = d[15];
			`z = (d == 0);
		end
		`SUB: begin		// d = b - a
			psr = ps;
			{ `c, d } = {1'd1, b} - a;
			`v = (a[15] != b[15]) && (d[15] == a[15]);
			`n = d[15];
			`z = (d == 0);
		end
		`ASH: begin		// d = a>0 ? b<<a | b>>(-a)
			psr = ps;
			if (a[5])
				{ `c, d } = { 1'd0, b } << (5'd1 + ~a[4:0]);
			else
				{ d, `c } = { b, 1'd0 } >> a[4:0];
			`v = b[15] ^ d[15];
			`n = d[15];
			`z = (d == 0);
		end
		`BIT: begin		// d = a & b (no register store)
			d = a & b;
			psr = ps;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`BITB: begin
			d = { b[15:8], (a[7:0] & b[7:0]) };
			psr = ps;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`BIC: begin		// d = ~a & b
			d = ~a & b;
			psr = ps;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`BICB: begin
			d = { b[15:8], (~a[7:0] & b[7:0]) };
			psr = ps;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`BIS: begin		// d = a | b
			d = a | b;
			psr = ps;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		`BISB: begin
			d = { b[15:8], (a[7:0] | b[7:0]) };
			psr = ps;
			`v = 0;
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`XOR: begin		// d = a ^ b
			d = a ^ b;
			psr = ps;
			`v = 0;
			`n = d[15];
			`z = (d == 0);
		end
		endcase
	end

endmodule

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
		casex (op)
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
			`v = (b == 8'h80);
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
			`v = (d == 8'h80);
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
			d = b + `c;
			`c = (b == 16'hffff && ps[0] == 1);
			`v = (b == 16'h7fff && ps[0] == 1);
			`n = d[15];
			`z = (d == 0);
		end
		`ADCB: begin
			psr = ps;
			d = { b[15:8], (b[7:0] + `c) };
			`c = (b[7:0] == 8'hff && ps[0] == 1);
			`v = (b[7:0] == 8'h7f && ps[0] == 1);
			`n = d[7];
			`z = (d[7:0] == 0);
		end
		`SBC: begin		// d = b - c
			psr = ps;
			d = b - `c;
			`c = (b == 0) && (ps[0] != 0);
			`v = (b == 16'h8000);
			`n = d[15];
			`z = (d == 0);
		end
		`SBCB: begin
			psr = ps;
			d = { b[15:8], (b[7:0] - `c) };
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

`ifdef TEST_ALU
//
// ALU test bench.
//
module alu_test;

	// Inputs
	reg [9:0] op;		// op code
	reg [15:0] a;			// `source' register
	reg [15:0] b;			// `destination' register
	reg [7:0] ps;			// processor state register

	// Outputs
	wire [15:0] d;			// result
	wire [7:0] psr;			// processor state result

task display_op;
	input [9:0] op;
begin
	casex (op)
	`CLR:  $write ("CLR");
	`CLRB: $write ("CLRB");
	`COM:  $write ("COM");
	`COMB: $write ("COMB");
	`INC:  $write ("INC");
	`INCB: $write ("INCB");
	`INC2: $write ("INC2");
	`DEC:  $write ("DEC");
	`DECB: $write ("DECB");
	`DEC2: $write ("DEC2");
	`NEG:  $write ("NEG");
	`NEGB: $write ("NEGB");
	`TST:  $write ("TST");
	`TSTB: $write ("TSTB");
	`ASR:  $write ("ASR");
	`ASRB: $write ("ASRB");
	`ASL:  $write ("ASL");
	`ASLB: $write ("ASLB");
	`ROR:  $write ("ROR");
	`RORB: $write ("RORB");
	`ROL:  $write ("ROL");
	`ROLB: $write ("ROLB");
	`SWAB: $write ("SWAB");
	`ADC:  $write ("ADC");
	`ADCB: $write ("ADCB");
	`SBC:  $write ("SBC");
	`SBCB: $write ("SBCB");
	`SXT:  $write ("SXT");
	`MFPS: $write ("MFPS");
	`MTPS: $write ("MTPS");
	`MOV:  $write ("MOV");
	`MOVB: $write ("MOVB");
	`CMP:  $write ("CMP");
	`CMPB: $write ("CMPB");
	`ADD:  $write ("ADD");
	`SUB:  $write ("SUB");
	`ASH:  $write ("ASH");
	`BIT:  $write ("BIT");
	`BITB: $write ("BITB");
	`BIC:  $write ("BIC");
	`BICB: $write ("BICB");
	`BIS:  $write ("BIS");
	`BISB: $write ("BISB");
	`XOR:  $write ("XOR");
	default: $write ("?OP?");
	endcase
end
endtask

task show;
	input [9:0] op;		// op code
	input [15:0] a;			// `source' register
	input [15:0] b;			// `destination' register
	input [7:0] ps;			// processor state register
	input [15:0] d;			// result
	input [7:0] psr;		// processor state result
	input ok;
begin
	$write ("(%0d) ", $time);
	display_op (op);
	$write (" %h, %h [%b] -> %h [%b]", a, b, ps, d, psr);
	if (psr[3])
		$write (" N");
	if (psr[2])
		$write (" Z");
	if (psr[1])
		$write (" V");
	if (psr[0])
		$write (" C");
	if (ok)
		$display (" - Ok");
	else
		$display (" - ERROR");
end
endtask

	// Instantiate the Unit Under Test (UUT)
	alu uut (
		.op (op),
		.a (a),
		.b (b),
		.ps (ps),
		.d (d),
		.psr (psr)
	);

	initial begin
		// Initialize Inputs
		op = 0;
		a = 0;
		b = 0;
		ps = 0;

		// Wait 100 ns for global reset to finish
		#100;

		// Add stimulus here
		$display ("(%0d) ", $time, "started ALU testing");

		op = `CLR; a = 'h5555; b = 'h3333; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 0 && psr == 'b0100);

		op = `CLRB; a = 'h5555; b = 'h3333; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h3300 && psr == 'b0100);

		op = `COM; a = 'h5555; b = 'h3333; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'hcccc && psr == 'b1001);

		op = `COMB; a = 'h5555; b = 'h3333; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h33cc && psr == 'b1001);

		op = `INC; a = 'h5555; b = 'h7fff; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h8000 && psr == 'b1010);

		op = `INC; a = 'h5555; b = 'hffff; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 0 && psr == 'b0100);

		op = `INCB; a = 'h5555; b = 'h33ff; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h3300 && psr == 'b0100);

		op = `INC2; a = 'h5555; b = 'h3fff; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h4001 && psr == 'b1111);

		op = `DEC; a = 'h5555; b = 'h8000; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h7fff && psr == 'b0010);

		op = `DECB; a = 'h5555; b = 'h3300; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h33ff && psr == 'b1000);

		op = `DEC2; a = 'h5555; b = 'h4001; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h3fff && psr == 'b1111);

		op = `NEG; a = 'h5555; b = 'h7fff; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h8001 && psr == 'b1001);

		op = `NEGB; a = 'h5555; b = 'h7fff; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h7f01 && psr == 'b0001);

		op = `TST; a = 'h5555; b = 'haaaa; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'haaaa && psr == 'b1000);

		op = `TSTB; a = 'h5555; b = 'haa00; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'haa00 && psr == 'b0100);

		op = `ASR; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'hd52a && psr == 'b1001);

		op = `ASRB; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'haa2a && psr == 'b0011);

		op = `ASL; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h54aa && psr == 'b0011);

		op = `ASLB; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'haaaa && psr == 'b1010);

		op = `ROR; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h552a && psr == 'b0011);

		op = `RORB; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'haa2a && psr == 'b0011);

		op = `ROL; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h54aa && psr == 'b0011);

		op = `ROLB; a = 'h3333; b = 'haa55; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'haaaa && psr == 'b1010);

		op = `SWAB; a = 'h3333; b = 'haa55; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h55aa && psr == 'b0000);

		op = `ADC; a = 'h3333; b = 'h7fff; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h8000 && psr == 'b1010);

		op = `ADCB; a = 'h3333; b = 'h7fff; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h7f00 && psr == 'b0101);

		op = `SBC; a = 'h3333; b = 'h8000; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h7fff && psr == 'b0010);

		op = `SBCB; a = 'h3333; b = 'h8000; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h80ff && psr == 'b1001);

		op = `SXT; a = 'h3333; b = 'h8000; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'hffff && psr == 'b1001);

		op = `SXT; a = 'h3333; b = 'h8000; ps = 'b0000;
		#50 show (op, a, b, ps, d, psr,
			d == 'h0000 && psr == 'b0100);

		op = `MFPS; a = 'h3333; b = 'h8000; ps = 'b1010;
		#50 show (op, a, b, ps, d, psr,
			d == 'b1010 && psr == 'b0000);

		op = `MTPS; a = 'h3333; b = 'h00ff; ps = 'b1010;
		#50 show (op, a, b, ps, d, psr,
			d == 'h00ff && psr == 'b11101111);

		//
		// Double-operand instructions.
		//
		op = `MOV; a = 'h3333; b = 'h7fff; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h3333 && psr == 'b0001);

		op = `MOVB; a = 'h3333; b = 'haaaa; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'haa33 && psr == 'b0001);

		op = `CMP; a = 'h3333; b = 'h4444; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'heeef && psr == 'b1000);

		op = `CMPB; a = 'h3333; b = 'h4444; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h44ef && psr == 'b1000);

		op = `ADD; a = 'h9999; b = 'h8888; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h2221 && psr == 'b0011);

		op = `SUB; a = 'h9999; b = 'h8888; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'heeef && psr == 'b1000);

		op = `ASH; a = 'h0007; b = 'h9999; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h0133 && psr == 'b0010);

		op = `ASH; a = 'h0039; b = 'h9999; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'hcc80 && psr == 'b1000);

		op = `BIT; a = 'h1234; b = 'h5678; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h1230 && psr == 'b0001);

		op = `BITB; a = 'h1234; b = 'h5678; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h5630 && psr == 'b0001);

		op = `BIC; a = 'h1234; b = 'h5678; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h4448 && psr == 'b0001);

		op = `BICB; a = 'h1234; b = 'h5678; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h5648 && psr == 'b0001);

		op = `BIS; a = 'h2134; b = 'h5678; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h777c && psr == 'b0001);

		op = `BISB; a = 'h2134; b = 'h5678; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h567c && psr == 'b0001);

		op = `XOR; a = 'h1234; b = 'h5678; ps = 'b1111;
		#50 show (op, a, b, ps, d, psr,
			d == 'h444c && psr == 'b0001);

		#50 $display ("(%0d) ", $time, "finished ALU testing");
		$finish;
	end

endmodule
`endif

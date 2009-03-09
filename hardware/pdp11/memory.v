`timescale 1ns / 1ps
//
// Behavioral RAM.
//
module memory (
	input wire [15:0] addr,			// address bus
	input wire we,				// write enable
	input wire clk,				// write clock
	input wire byte,			// byte mode
	input wire [15:0] w,			// write data bus
	output wire [0:15] d			// read data bus
);
	// 2 kbytes of memory
	reg [0:15] memory [1023:0];

	// Addressed word
	wire [15:0] word = memory [addr[10:1]];

	// Read memory
	assign d = addr[0] ?
		(word >> 8) :		// byte read
		word;			// word-aligned read

	// Write memory on positive clk, when we is set.
	always @(posedge clk) begin
		if (we == 1) begin
			if (byte) begin
				// byte write
				if (addr[1])	// high byte
					memory [addr[10:1]] = { w[7:0], word[7:0] };
				else		// low byte
					memory [addr[10:1]] = { word[15:8], w[7:0] };
			end else begin
				// word write
				memory [addr[10:1]] = w;
			end
		end
	end

	initial begin
		// Initialize RAM contents on start.
		`include "memory-initial.v"
	end
endmodule

`ifdef TEST_MEMORY
//
// RAM file test bench.
//
module memory_test;

	// Inputs
	reg [15:0] addr;
	reg we;
	reg clk;
	reg [15:0] w;

	// Outputs
	wire [15:0] d;

	// Instantiate the Unit Under Test (UUT)
	memory uut (
		.addr(addr),
		.we(we),
		.clk(clk),
		.w(w),
		.d(d)
	);

	initial begin
		// Initialize Inputs
		addr = 0;
		we = 0;
		clk = 0;
		w = 0;

		// Wait 100 ns for global reset to finish
		#100;

		// Add stimulus here
		we = 1;
		#50 addr =   0; w = 16'h0FF0; #50 clk = 1; #50 clk = 0;
		#50 addr =   2; w = 16'h2002; #50 clk = 1; #50 clk = 0;
		#50 addr =   4; w = 16'h4004; #50 clk = 1; #50 clk = 0;
		#50 addr =   6; w = 16'h6006; #50 clk = 1; #50 clk = 0;
		#50 addr =   8; w = 16'h8008; #50 clk = 1; #50 clk = 0;
		#50 addr = 'ha; w = 16'ha00a; #50 clk = 1; #50 clk = 0;
		#50 addr = 'hc; w = 16'hc00c; #50 clk = 1; #50 clk = 0;
		#50 addr = 'he; w = 16'he00e; #50 clk = 1; #50 clk = 0;
		we = 0;

		$monitor ($time, ": registers written, start reading");
		#50 addr =   6; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   7; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   8; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   9; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr = 'ha; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr = 'hb; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr = 'hc; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr = 'hd; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr = 'he; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr = 'hf; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   0; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   1; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   2; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   3; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   4; #50 $display ($time, ": %h - %h", addr, d);
		#50 addr =   5; #50 $display ($time, ": %h - %h", addr, d);

		$finish;
	end

endmodule
`endif

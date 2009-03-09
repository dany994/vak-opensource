`timescale 1ns / 1ps
//
// Behavioral ROM.
//
module rom (
	input wire [8:0] addr,
	output reg [15:0] data
);
	always @(addr) begin
		case (addr)
		9'h000: data = 16'h0000;
		9'h001: data = 16'h0001;
		9'h002: data = 16'h0010;
		9'h003: data = 16'h0100;
		9'h004: data = 16'h1000;
		9'h005: data = 16'h1000;
		9'h006: data = 16'h1100;
		9'h007: data = 16'h1010;
		9'h008: data = 16'h1001;
		9'h009: data = 16'h1001;
		9'h00a: data = 16'h1010;
		9'h00b: data = 16'h1100;
		9'h00c: data = 16'h1001;
		9'h00d: data = 16'h1001;
		9'h00e: data = 16'h1101;
		9'h00f: data = 16'h1111;
		endcase
	end
endmodule

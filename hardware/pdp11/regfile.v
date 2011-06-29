`timescale 1ns / 1ps
//
// Register file.
//
module regfile (
    input wire [2:0] sela,
    input wire [2:0] selb,
    input wire we,
    input wire clk,
    input wire reset,
    input wire [15:0] w,
    output wire [15:0] a,
    output wire [15:0] b
);
    reg [15:0] r [7:0];

    // Output R[a] on bus A
    assign a = r [sela];

    // Output R[b] on bus B
    assign b = r [selb];

    // Write value from bus W to R[b].
    // On positive clk, when we is set.
    always @(posedge clk) begin
        if (we == 1) begin
            r [selb] = w;
            if (selb == 7)
                $display ("        set PC := %h", w);
            else if (selb == 6)
                $display ("        set SP := %h", w);
            else
                $display ("        set R%d := %h", selb, w);
        end
    end

    // On reset, PC is cleared.
    always @(negedge clk or posedge reset) begin
        if (reset) begin
            r [7] = 0;
            $display ("        reset PC");
        end
    end

endmodule

`ifdef TEST_REGFILE
//
// Register file test bench.
//
module regfile_test;

    // Inputs
    reg [0:2] sela;
    reg [0:2] selb;
    reg we;
    reg clk;
    reg [0:15] w;

    // Outputs
    wire [0:15] a;
    wire [0:15] b;

    // Instantiate the Unit Under Test (UUT)
    regfile uut (
        .sela(sela),
        .selb(selb),
        .we(we),
        .clk(clk),
        .w(w),
        .a(a),
        .b(b)
    );

    initial begin
        // Initialize Inputs
        sela = 0;
        selb = 0;
        we = 0;
        clk = 0;
        w = 0;

        // Wait for global reset to finish
        #50;

        // Add stimulus here
        we = 1;
        #50 selb = 0; w = 16'hAAAA; #50 clk = 1; #50 clk = 0;
        #50 selb = 1; w = 16'h1111; #50 clk = 1; #50 clk = 0;
        #50 selb = 2; w = 16'h2222; #50 clk = 1; #50 clk = 0;
        #50 selb = 3; w = 16'h3333; #50 clk = 1; #50 clk = 0;
        #50 selb = 4; w = 16'h4444; #50 clk = 1; #50 clk = 0;
        #50 selb = 5; w = 16'h5555; #50 clk = 1; #50 clk = 0;
        #50 selb = 6; w = 16'h6666; #50 clk = 1; #50 clk = 0;
        #50 selb = 7; w = 16'h7777; #50 clk = 1; #50 clk = 0;
        we = 0;

        $monitor ($time, ": registers written, start reading");
        #50 sela = 1; selb = 3; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);
        #50 sela = 2; selb = 4; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);
        #50 sela = 3; selb = 5; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);
        #50 sela = 4; selb = 6; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);
        #50 sela = 5; selb = 7; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);
        #50 sela = 6; selb = 0; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);
        #50 sela = 7; selb = 1; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);
        #50 sela = 0; selb = 2; #50 $display ($time, ": %h - %h // %h - %h", sela, a, selb, b);

        $finish;
    end

endmodule
`endif

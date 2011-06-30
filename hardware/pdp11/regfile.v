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
        if (reset) begin
            // On reset, PC is cleared.
            r [7] = 0;
            $display ("(%0d) reset PC", $time);
        end else if (we == 1) begin
            r [selb] = w;
            if (selb == 7)
                $display ("(%0d) set PC := %o", $time, w);
            else if (selb == 6)
                $display ("(%0d) set SP := %o", $time, w);
            else
                $display ("(%0d) set R%d := %o", $time, selb, w);
        end
    end

endmodule

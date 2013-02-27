#!/usr/bin/env perl

$gates = 64;
$steps = 2000;
$loops = 400;
srand (123);

print qq/module top();
    reg [$gates-1:0] a;
    reg [$gates-1:0] b;
    reg [$gates-1:0] c;
/;

@binop  = ('%s & %s', '~(%s & %s)', '(%s | %s)', '~(%s | %s)', '%s ^ %s', '~(%s ^ %s)');

for ($i = 0; $i < $gates; ++$i) {
    $x = $i + 1 + int(rand() * ($gates - 1));
    if ($x >= $gates) {
        $x = $x - $gates;
    }

    $y = $i + 1 + int(rand() * ($gates - 1));
    if ($y >= $gates) {
        $y = $y - $gates;
    }

    $op = $binop[int(rand() * 6)];

    printf "    always @(*) b[$i] <= $op;\n", "a[$x]", "a[$y]";
}

for ($i = 0; $i < $gates; ++$i) {
    $x = $i + 1 + int(rand() * ($gates - 1));
    if ($x >= $gates) {
        $x = $x - $gates;
    }

    $y = $i + 1 + int(rand() * ($gates - 1));
    if ($y >= $gates) {
        $y = $y - $gates;
    }

    $op = $binop[int(rand() * 6)];

    printf "    always @(*) c[$i] <= $op;\n", "b[$x]", "b[$y]";
}

print qq/
    integer i;
    initial begin
        a = 0;
        b = 0;
        c = 0;
        for (i=0; i<$loops; i=i+1) begin
/;

for ($i = 0; $i < $steps; ++$i) {
    if (rand() < 0.2) {
        printf "            #1;\n";
    }
    $x = int(rand() * $gates);
    printf "            a[$x] <= ~a[$x];\n";
    #printf "            \$display (\"%%x %%x %%x\", a, b, c);\n";
}

#            \$display ("%x %x %x", a, b, c);
print qq/
        end
        \$display ("%x %x %x", a, b, c);
        \$finish;
    end
endmodule
/;

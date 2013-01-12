// Copyright (c) 2004 Robert Nordier.  All rights reserved.

GET "LIBHDR"

MANIFEST $( GSZ=500; LSZ=500 $)
MANIFEST $( M.N=0; M.L=1; M.P=2; M.G=3 $)

GLOBAL $( SYSPRINT:150; G:151; L:152; LN:153; SECT:154; LOFF:155;
OCODE:156
$)

STATIC $( LINE=0; COL=0; CH='*N' $)

LET START() = VALOF
$(
    LET GVEC = VEC GSZ
    LET LVEC = VEC LSZ
    SYSPRINT := OUTPUT()
    OCODE := FINDOUTPUT("ASM")
    SELECTOUTPUT(OCODE)
    G := GVEC
    L := LVEC
    FOR I = 0 TO GSZ DO G!I := 0
    LOFF := 2000
    SECT := 0
    LN := 0
    ASSEM()
    EPILOG()
    ENDWRITE()
    RESULTIS 0
$)

AND ASSEM() BE
$(
    LET F,A,I,K = 0,0,0,0
    RCH()
    F := CH
    SWITCHON F INTO $(
    DEFAULT:
        ERROR(6)
    CASE ENDSTREAMCH:
        GENER(0, 0, 0, 0)
        RETURN
    CASE '*S':CASE '*N':
        LOOP
    CASE 'G':
        RCH()
        A := RDN()
        UNLESS A<GSZ ERROR(5)
        UNLESS CH='L' ERROR(4)
        RCH()
        G!A := LOFF + RDN()
        LOOP
    CASE 'Z':
        LOFF := LOFF + 500
        LOOP
    CASE '$':
        LOOP
    CASE '0':CASE '1':CASE '2': CASE '3': CASE '4':
    CASE '5':CASE '6':CASE '7': CASE '8': CASE '9':
        F := 'B'
        A := LOFF + RDN()
        ENDCASE
    CASE 'L':CASE 'S':CASE 'A':CASE 'J':
    CASE 'T':CASE 'F':CASE 'K':CASE 'X':
        RCH()
        IF CH='I' THEN $( I := TRUE; RCH() $)
        K := CH='G' -> M.G, CH='P' -> M.P, CH='L' -> M.L, M.N
        UNLESS K=0 THEN RCH()
        A := RDN()
        IF K=M.L THEN
            A := A + LOFF
        ENDCASE
    CASE 'D':
        RCH()
        IF CH='L' THEN $( K := K + 1; RCH() $)
        A := RDN()
        IF K=M.L THEN
            A := A + LOFF
        ENDCASE
    CASE 'C':
        RCH()
        A := RDN()
        ENDCASE
    $)
    GENER(F, A, I, K)
$) REPEAT

AND EPILOG() BE
$(
    SECT := 1
    EMIT(".globl G")
    EMIT(".align 4")
    EMIT("G:")
    FOR I=0 TO GSZ - 1
        EMIT(".long %S%N // %N", G!I=0 -> "", "L", G!I, I)
$)

AND GENER(F1, A1, I1, K1) BE
$(
    STATIC $( XL=0;
        F=0; A=0; I=0; K=0
        F0=0; A0=0; I0=0; K0=0
    $)
    SWITCHON F INTO $(
    DEFAULT:
        ERROR(7)
    CASE 0:
        ENDCASE
    CASE 'B':
        UNLESS LN<=LSZ THEN ERROR(1)
        L!LN := A
        LN := LN + 1
        ENDCASE
    CASE 'L':
        IF F1='X' DO
            TEST (5 <= A1 <= 21)
                ENDCASE
            OR IF A1=32 | (35 <= A1 <= 37)
                EMIT("movl eax,ebx")
    CASE 'A':
        TEST NOT I $(
            EMIT("%S %S,e%Sx",
                 (F='L' & (K=M.P | K=M.G) -> "lea",
                 (F='A' & NOT I & K=M.N -> "addl", "movl")),
                 ADDR(A, I, K),
                 (F='A' & K NE M.N -> "b", "a"))
            UNLESS K=M.N THEN $(
                EMIT("shrl $2,e%Sx", (F='A' -> "b", "a"))
                IF F='A' THEN
                    EMIT("addl ebx,eax")
            $)
        $) OR
               EMIT("%S %S,eax", (F='A' -> "addl", "movl"),
                     ADDR(A, TRUE, K))
        ENDCASE
    CASE 'S':
        TEST NOT I THEN
            EMIT("movl eax,%S", ADDR(A, TRUE, K))
        OR $(
            EMIT("movl %S,ebx", ADDR(A, I, K))
            EMIT("movl eax,(,ebx,4)")
        $)
        ENDCASE
    CASE 'J':CASE 'T':CASE 'F':
        IF F NE 'J' & I=TRUE | K NE M.L THEN
            ERROR(8)
        UNLESS F='J' DO $(
            IF (F0='X' & (10 <= A0 <= 15)) $(
                TEST F='T' DO
                    EMIT("j%S %SL%N",
                         (A0=10 -> "e",
                          A0=11 -> "ne",
                          A0=12 -> "l",
                          A0=13 -> "ge",
                          A0=14 -> "g", "le"),
                          (I -> "**", ""), A)
                OR
                    EMIT("j%S %SL%N",
                         (A0=10 -> "ne",
                          A0=11 -> "e",
                          A0=12 -> "ge",
                          A0=13 -> "l",
                          A0=14 -> "le", "g"),
                          (I -> "**", ""), A)
                ENDCASE
            $)
            EMIT("testl eax,eax")
        $)
        EMIT("j%S %SL%N", (F='T' -> "nz" , F='F' -> "z" , "mp"),
             (I -> "**" , ""), A)
        ENDCASE
    CASE 'K':
        IF I | K NE M.N THEN
            ERROR(8)
        EMIT("movl ebp,ebx")
        EMIT("addl $%N,ebp", A << 2)
        EMIT("movl ebx,(ebp)")
        EMIT("movl $1f,4(ebp)")
        EMIT("jmp **eax")
        EMIT("1:")
        ENDCASE
    CASE 'X':
        SWITCHON A INTO $(
        DEFAULT:
            ERROR(9)
        CASE 1:
            EMIT("movl (,eax,4),eax")
            ENDCASE
        CASE 2:
            EMIT("negl eax")
            ENDCASE
        CASE 3:
            EMIT("xorl $-1,eax")
            ENDCASE
        CASE 4:
            EMIT("movl 4(ebp),ebx")
            EMIT("movl (ebp),ebp")
            EMIT("jmp **ebx")
            ENDCASE
        CASE 5:CASE 6:CASE 7:
            UNLESS A=5 DO EMIT("cltd")
            TEST NOT I0 & K0=M.N DO $(
                EMIT("movl %S,ecx", ADDR(A0, I0, K0))
                EMIT("i%Sl ecx", (A=5 -> "mul", "div"))
            $) OR
                EMIT("i%Sl %S", (A=5 -> "mul", "div"),
                     ADDR(A0, I0, K0))
            IF A=7 DO EMIT("movl edx,eax")
            ENDCASE
        CASE 8:
            EMIT("addl %S,eax", ADDR(A0, I0, K0))
            ENDCASE
        CASE 9:
            EMIT("subl %S,eax", ADDR(A0, I0, K0))
            ENDCASE
        CASE 10:CASE 11:CASE 12:CASE 13:CASE 14:CASE 15:
            EMIT("cmpl %S,eax", ADDR(A0, I0, K0))
            IF F1='F' | F1='T' ENDCASE
            EMIT("set%S al",
                 A=10 -> "ne",
                 A=11 -> "e",
                 A=12 -> "ge",
                 A=13 -> "l",
                 A=14 -> "le", "g")
            EMIT("movzbl al,eax")
            EMIT("decl eax")
            ENDCASE
        CASE 16:CASE 17:
            TEST NOT I0 & K0=M.N & A0<32 DO
                EMIT("sh%Cl $%N,eax", A=16 -> 'l', 'r', A0)
            ELSE $(
                EMIT("movl %S,ecx", ADDR(A0, I0, K0))
                EMIT("jecxz 1f")
                EMIT("decl ecx")
                EMIT("sh%Cl $1,eax", A=16 -> 'l', 'r')
                EMIT("sh%Cl cl,eax", A=16 -> 'l', 'r')
                EMIT("1:")
            $)
            ENDCASE
        CASE 18:CASE 19:CASE 20:
            EMIT("%Sl %S,eax",
                 (A=18 -> "and", A=19 -> "or", "xor"),
                 ADDR(A0, I0, K0))
            ENDCASE
        CASE 21:
            EMIT("xorl $-1,eax")
            EMIT("xorl %S,eax", ADDR(A0, I0, K0))
            ENDCASE
        CASE 22:
            EMIT("jmp finish")
            ENDCASE
        CASE 23:
            EMIT("movl $L%N,esi", XL)
            EMIT("movl (esi),ecx")
            EMIT("movl 4(esi),edx")
            EMIT("jecxz 2f")
            EMIT("1:")
            EMIT("addl $8,esi")
            EMIT("cmpl (esi),eax")
            EMIT("je 3f")
            EMIT("loop 1b")
            EMIT("2:")
            EMIT("jmp **edx")
            EMIT("3:")
            EMIT("jmp **4(esi)")
            L!LN := XL
            LN := LN + 1
            XL := XL + 1
            ENDCASE
        CASE 24:
            EMIT("call _selectinput")
            ENDCASE
        CASE 25:
            EMIT("call _selectoutput")
            ENDCASE
        CASE 26:
            EMIT("call _rdch")
            ENDCASE
        CASE 27:
            EMIT("call _wrch")
            ENDCASE
        CASE 28:
            EMIT("call _findinput")
            ENDCASE
        CASE 29:
            EMIT("call _findoutput")
            ENDCASE
        CASE 30:
            EMIT("jmp stop")
            ENDCASE
        CASE 31:
            EMIT("movl (ebp),eax")
            ENDCASE
        CASE 32:
            EMIT("movl eax,ebp")
            EMIT("jmp **ebx")
            ENDCASE
        CASE 33:
            EMIT("call _endread")
            ENDCASE
        CASE 34:
            EMIT("call _endwrite")
            ENDCASE
        CASE 35:
            EMIT("movl ebp,esi")
            EMIT("movl ebx,ecx")
            EMIT("incl ecx")
            EMIT("shll $2,ecx")
            EMIT("addl ecx,esi")
            EMIT("movl (ebp),ecx")
            EMIT("movl ecx,(esi)")
            EMIT("movl 4(ebp),ecx")
            EMIT("movl ecx,4(esi)")
            EMIT("movl ebp,ecx")
            EMIT("shrl $2,ecx")
            EMIT("movl ecx,8(esi)")
            EMIT("movl ebx,12(esi)")
            EMIT("movl esi,ebp")
            EMIT("jmp **eax")
            ENDCASE
        CASE 36:
            EMIT("shll $2,eax")
            EMIT("addl ebx,eax")
            EMIT("movzbl (eax),eax")
            ENDCASE
        CASE 37:
            EMIT("shll $2,eax")
            EMIT("addl ebx,eax")
            EMIT("movl 16(ebp),ebx")
            EMIT("movb bl,(eax)")
            ENDCASE
        CASE 38:
            EMIT("call _input")
            ENDCASE
        CASE 39:
            EMIT("call _output")
            ENDCASE
        CASE 40:
            EMIT("call _unrdch")
            ENDCASE
        CASE 41:
            EMIT("call _rewind")
        $)
        ENDCASE
    CASE 'D':
        SECT := 1
        EMIT(".long %S%N", (K=M.L -> "L", ""), A)
        SECT := 0
        ENDCASE
    CASE 'C':
        SECT := 1
        EMIT(".byte %N", A)
        IF F1 NE 'C' THEN
            EMIT(".align 4,0")
        SECT := 0
    $)
    F0 := F; A0 := A; I0 := I; K0 := K
    F := F1; A := A1; I := I1; K := K1
$)

AND ADDR(A, I, K) = VALOF
$(
    LET T = TABLE 0,0,0,0,0,0,0,0,0,0,0,0,0,0
    AND ADDBYTE(S, C) BE
    $(
        LET N = GETBYTE(S, 0)
        N := N + 1
        PUTBYTE(S, N, C)
        PUTBYTE(S, 0, N)
    $)
    AND APPEND(D, S) BE
    $(
        LET N = GETBYTE(S, 0)
        FOR J = 1 TO N DO
            ADDBYTE(D, GETBYTE(S, J))
    $)
    AND WRN(S, N) BE
    $(
        IF N<0 DO $( ADDBYTE(S, '-'); N := -N $)
        IF N>=10 DO WRN(S, N / 10)
        ADDBYTE(S, '0' + N REM 10)
    $)
    PUTBYTE(T, 0, 0)
    IF NOT I & (K=M.N | K=M.L) THEN
        ADDBYTE(T, '$')
    IF K=M.L THEN
        ADDBYTE(T, 'L')
    WRN(T, K=M.L -> A, NOT I & K=M.N -> A, A << 2)
    IF K=M.P | K=M.G THEN
        APPEND(T, K=M.P -> "(ebp)", "(edi)")
    RESULTIS T
$)

AND EMIT(FMT, A, B, C, D, E) BE
$(
    STATIC $( PSECT=0 $)
    UNLESS SECT=PSECT DO $(
        EMIT1(SECT=0 -> ".text", ".data")
        PSECT := SECT
    $)
    UNLESS LN=0 $(
        EMIT1("//.align 4")
        FOR I = 0 TO LN - 1 DO
            WRITEF("L%N:*N", L!I)
        LN := 0
    $)
    EMIT1(FMT, A, B, C, D, E)
$)

AND EMIT1(FMT, A, B, C, D, E) BE
$(
    WRITEF(FMT, A, B, C, D, E)
    WRCH('*N')
$)

AND RDN() = VALOF
$(
    LET X, N, I = 0, FALSE, 0
    IF CH='-' THEN $(
        N := TRUE
        RCH()
    $)
    WHILE '0' <= CH <= '9' $(
        X := X * 10 + (CH - '0')
        I := I + 1
        RCH()
    $)
    IF I=0 THEN
        ERROR(3)
    RESULTIS N -> -X, X
$)

AND RCH() BE
$(
    RCH1()
    UNLESS CH='/' RETURN
    $( RCH1(); IF CH=ENDSTREAMCH DO ERROR(2) $) REPEATUNTIL CH='*N'
$) REPEAT

AND RCH1() BE
$(
    TEST CH='*N' THEN $(
        LINE := LINE + 1
        COL := 1
    $) OR
        COL := COL + 1
    CH := RDCH()
$)

AND ERROR(N) BE
$(
    SELECTOUTPUT(SYSPRINT)
    UNLESS LINE=0
        WRITEF("SYSIN(%N,%N): ", LINE, COL)
    WRITEF("ERROR %N*N", N)
    STOP(1)
$)

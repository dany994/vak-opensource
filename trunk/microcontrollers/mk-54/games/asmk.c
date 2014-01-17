/*
 * Parser for MK-61 source files.
 *
 * Copyright (C) 2014 Serge Vakulenko, <serge@vak.ru>
 *
 * Permission to use, copy, modify, and distribute this software
 * and its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that the copyright notice and this
 * permission notice and warranty disclaimer appear in supporting
 * documentation, and that the name of the author not be used in
 * advertising or publicity pertaining to distribution of the
 * software without specific, written prior permission.
 *
 * The author disclaim all warranties with regard to this
 * software, including all implied warranties of merchantability
 * and fitness.  In no event shall the author be liable for any
 * special, indirect or consequential damages or any damages
 * whatsoever resulting from loss of use, data or profits, whether
 * in an action of contract, negligence or other tortious action,
 * arising out of or in connection with the use or performance of
 * this software.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

/*
 * Types of lexemes.
 */
enum {
    LEOF = 1,           /* end of file */
    LEOL,               /* end of line */
    LNAME,              /* identifier */
    LKEY,               /* machine register */
    LNUM,               /* integer number */
};

/*
 * Sizes of tables.
 * Hash sizes should be powers of 2!
 */
#define HASHSZ  256             /* symbol name hash table size */
#define HCMDSZ  256             /* instruction hash table size */
#define STSIZE  (HASHSZ*9/10)   /* symbol name table size */

/*
 * Main opcode table.
 */
struct optable {
    unsigned opcode;                    /* instruction code */
    const char *name;                   /* instruction name */
    unsigned type;                      /* flags */
};

#define FPREF   0x0001                  /* Need F prefix */
#define FPREK   0x0002                  /* Need K prefix */
#define FREG    0x0004                  /* Register number follows */
#define FADDR   0x0008                  /* Jump address follows */

const struct optable optable [] = {
    { 0x0a, ","         },
    { 0x0b, "/-/"       },
    { 0x0c, "ВП"        },
    { 0x0d, "Сx"        },
    { 0x0e, "В^"        },
    { 0x0f, "Вx",       FPREF },
    { 0x10, "+"         },
    { 0x11, "-"         },
    { 0x12, "x"         },
    { 0x13, "/"         },
    { 0x14, "<->"       },
    { 0x15, "10^x",     FPREF },
    { 0x16, "e^x",      FPREF },
    { 0x17, "lg",       FPREF },
    { 0x18, "ln",       FPREF },
    { 0x19, "arcsin",   FPREF },
    { 0x1a, "arccos",   FPREF },
    { 0x1b, "arctg",    FPREF },
    { 0x1c, "sin",      FPREF },
    { 0x1d, "cos",      FPREF },
    { 0x1e, "tg",       FPREF },
    { 0x20, "пи",       FPREF },
    { 0x21, "корень",   FPREF },
    { 0x22, "x^2",      FPREF },
    { 0x23, "1/x",      FPREF },
    { 0x24, "x^y",      FPREF },
    { 0x25, "⟳",        FPREF },
    { 0x26, "МГ",       FPREK },
    { 0x27, "-",        FPREK },
    { 0x28, "x",        FPREK },
    { 0x29, "/",        FPREK },
    { 0x2a, "МЧ",       FPREK },
    { 0x30, "ЧМ",       FPREK },
    { 0x31, "|x|",      FPREK },
    { 0x32, "ЗН",       FPREK },
    { 0x33, "ГМ",       FPREK },
    { 0x34, "[x]",      FPREK },
    { 0x35, "{x}",      FPREK },
    { 0x36, "max",      FPREK },
    { 0x37, "/\\",      FPREK },
    { 0x38, "\\/",      FPREK },
    { 0x39, "(+)",      FPREK },
    { 0x3a, "ИНВ",      FPREK },
    { 0x3b, "СЧ",       FPREK },
    { 0x40, "xП",       FREG },
    { 0x50, "С/П",      },
    { 0x51, "БП",       FADDR },
    { 0x52, "В/О",      },
    { 0x53, "ПП",       FADDR },
    { 0x54, "НОП",      FPREK },
    { 0x55, "1",        FPREK },
    { 0x56, "2",        FPREK },
    { 0x57, "x#0",      FPREF | FADDR },
    { 0x58, "L2",       FPREF | FADDR },
    { 0x59, "x≥0",      FPREF | FADDR },
    { 0x5a, "L3",       FPREF | FADDR },
    { 0x5b, "L1",       FPREF | FADDR },
    { 0x5c, "x<0",      FPREF | FADDR },
    { 0x5d, "L0",       FPREF | FADDR },
    { 0x5e, "x=0",      FPREF | FADDR },
    { 0x60, "Пx",       FREG },
    { 0x70, "x#0",      FPREK | FREG },
    { 0x80, "БП",       FPREK | FREG },
    { 0x90, "x>=0",     FPREK | FREG },
    { 0xa0, "ПП",       FPREK | FREG },
    { 0xb0, "xП",       FPREK | FREG },
    { 0xc0, "x<0",      FPREK | FREG },
    { 0xd0, "Пx",       FPREK | FREG },
    { 0xe0, "x=0",      FPREK | FREG },
    { 0 },
};

/*
 * Character classes.
 */
#define ISHEX(c)        (ctype[(c)&0377] & 1)
#define ISOCTAL(c)      (ctype[(c)&0377] & 2)
#define ISDIGIT(c)      (ctype[(c)&0377] & 4)
#define ISLETTER(c)     (ctype[(c)&0377] & 8)

const char ctype [256] = {
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,8,0,0,0,0,0,0,0,0,0,8,0,7,7,7,7,7,7,7,7,5,5,0,0,0,0,0,0,
    8,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,8,
    0,9,9,9,9,9,9,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
    8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,0,
};

struct {
    unsigned value;
    unsigned undef;
} stab [STSIZE];

unsigned count;
unsigned char code[105];
unsigned char relinfo[105];
char *infile;
int line;                               /* Source line number */
int stabfree;
char space [STSIZE*8];                  /* Area for symbol names */
int lastfree;                           /* Free space offset */
char name [256];
unsigned intval;
int extref;
int blexflag, backlex, blextype;
short hashtab [HASHSZ], hashctab [HCMDSZ];

/* Forward declarations. */
unsigned getaddr (void);

/*
 * Fatal error message.
 */
void uerror (char *fmt, ...)
{
    va_list ap;

    va_start (ap, fmt);
    fprintf (stderr, "as: ");
    if (infile)
        fprintf (stderr, "%s, ", infile);
    if (line)
        fprintf (stderr, "%d: ", line);
    vfprintf (stderr, fmt, ap);
    va_end (ap);
    fprintf (stderr, "\n");
    exit (1);
}

/*
 * Suboptimal 32-bit hash function.
 * Copyright (C) 2006 Serge Vakulenko.
 */
unsigned hash_rot13 (s)
    const char *s;
{
    unsigned hash, c;

    hash = 0;
    while ((c = (unsigned char) *s++) != 0) {
        hash += c;
        hash -= (hash << 13) | (hash >> 19);
    }
    return hash;
}

void hashinit ()
{
    int i, h;
    const struct optable *p;

    for (i=0; i<HCMDSZ; i++)
        hashctab[i] = -1;
    for (p=optable; p->name; p++) {
        h = hash_rot13 (p->name) & (HCMDSZ-1);
        while (hashctab[h] != -1)
            if (--h < 0)
                h += HCMDSZ;
        hashctab[h] = p - optable;
    }
    for (i=0; i<HASHSZ; i++)
        hashtab[i] = -1;
}

/*
 * Get a number.
 * 1234 - decimal
 * 01234 - octal
 */
void getnum (c)
    int c;
{
    char *cp;
    int leadingzero;

    for (cp=name; ISDIGIT(c); c=getchar())
        *cp++ = c - '0';
    ungetc (c, stdin);
    intval = 0;
    /* Decimal. */
    for (c=1; ; c*=10) {
        if (--cp < name)
            return;
        intval += *cp * c;
    }
}

void getname (c)
    int c;
{
    char *cp;

    for (cp=name; ISLETTER (c) || ISDIGIT (c); c=getchar())
        *cp++ = c;
    *cp = 0;
    ungetc (c, stdin);
}

int lookcmd ()
{
    int i, h;

    h = hash_rot13 (name) & (HCMDSZ-1);
    while ((i = hashctab[h]) != -1) {
        if (! strcmp (optable[i].name, name))
            return (i);
        if (--h < 0)
            h += HCMDSZ;
    }
    return (-1);
}

char *alloc (len)
{
    int r;

    r = lastfree;
    lastfree += len;
    if (lastfree > sizeof(space))
        uerror ("out of memory");
    return (space + r);
}

int lookname ()
{
    int i, h;

    /* Search for symbol name. */
    h = hash_rot13 (name) & (HASHSZ-1);
    while ((i = hashtab[h]) != -1) {
        if (! strcmp (stab[i].n_name, name))
            return (i);
        if (--h < 0)
            h += HASHSZ;
    }

    /* Add a new symbol to table. */
    i = stabfree++;
    if (i >= STSIZE)
        uerror ("symbol table overflow");
    stab[i].n_len = strlen (name);
    stab[i].n_name = alloc (1 + stab[i].n_len);
    strcpy (stab[i].n_name, name);
    stab[i].value = 0;
    stab[i].undef = 1;
    hashtab[h] = i;
    return (i);
}

/*
 * Read a lexical element, return it's type and store a value into *val.
 * Return the type code.
 */
int getlex (pval)
    int *pval;
{
    int c;

    if (blexflag) {
        blexflag = 0;
        *pval = blextype;
        return (backlex);
    }
    for (;;) {
        switch (c = getchar()) {
        case '#':
skiptoeol:  while ((c = getchar()) != '\n')
                if (c == EOF)
                    return (LEOF);
        case '\n':
            ++line;
            c = getchar ();
            if (c == '#')
                goto skiptoeol;
            ungetc (c, stdin);
        case ';':
            *pval = line;
            return (LEOL);
        case ' ':
        case '\t':
            continue;
        case EOF:
            return (LEOF);
        case '\\':
            c = getchar ();
            if (c=='<')
                return (LLSHIFT);
            if (c=='>')
                return (LRSHIFT);
            ungetc (c, stdin);
            return ('\\');
        case '\'':      case '%':
        case '^':       case '&':       case '|':       case '~':
        case '+':       case '-':       case '*':       case '/':
        case '"':       case ',':       case '[':       case ']':
        case '(':       case ')':       case '{':       case '}':
        case '<':       case '>':       case '=':       case ':':
            return (c);
        case '0':
            ungetc (c, stdin);
            c = '0';
        case '1':       case '2':       case '3':
        case '4':       case '5':       case '6':       case '7':
        case '8':       case '9':
            getnum (c);
            return (LNUM);
        default:
            if (! ISLETTER (c))
                uerror ("bad character: \\%o", c & 0377);
            getname (c);
            if (name[0] == '.') {
                if (name[1] == 0)
                    return ('.');
            }
            return (LNAME);
        }
    }
}

void ungetlex (val, type)
{
    blexflag = 1;
    backlex = val;
    blextype = type;
}

/*
 * Get a jump address.
 * Return the value.
 * A copy of value is saved in intval.
 */
unsigned getaddr()
{
    int clex;
    int cval, s2;

    /* look a first lexeme */
    clex = getlex (&cval);
    switch (clex) {
    default:
        uerror ("operand missing");
    case LNUM:
        return intval;
    case LNAME:
        cval = lookname();
        if (stab[cval].undef) {
            extref = cval;
            intval = 0;
        } else
            intval = stab[cval].value;
        return intval;
    }
}

/*
 * Build and emit a machine instruction code.
 */
void makecmd (opcode, type)
    unsigned opcode;
{
    int clex;
    unsigned offset;
    int cval, clobber_reg, negate_literal;

    offset = 0;
    negate_literal = 0;

    /*
     * GCC can generate "j" instead of "jr".
     * Need to detect it early.
     */
    if (type == (FAOFF28 | FDSLOT)) {
        clex = getlex (&cval);
        ungetlex (clex, cval);
        if (clex == LREG) {
            if (opcode == 0x08000000) { /* j - replace by jr */
                opcode = 0x00000008;
                type = FRS1 | FDSLOT;
            }
            if (opcode == 0x0c000000) { /* jal - replace by jalr */
                opcode = 0x00000009;
                type = FRD1 | FRS2 | FDSLOT;
            }
        }
    }

    /*
     * First register.
     */
    cval = 0;
    clobber_reg = 0;
    if (type & FRD1) {
        clex = getlex (&cval);
        if (clex != LREG)
            uerror ("bad rd register");
        opcode |= cval << 11;           /* rd, ... */
    }
    if (type & FRT1) {
        clex = getlex (&cval);
        if (clex != LREG)
            uerror ("bad rt register");
        opcode |= cval << 16;           /* rt, ... */
    }
    if (type & FRS1) {
frs1:   clex = getlex (&cval);
        if (clex != LREG)
            uerror ("bad rs register");
        if (cval == 0 && (opcode == 0x0000001a ||   /* div */
                          opcode == 0x0000001b)) {  /* divu */
            /* Div instruction with three args.
             * Treat it as a 2-arg variant. */
            if (getlex (&cval) != ',')
                uerror ("comma expected");
            goto frs1;
        }
        opcode |= cval << 21;           /* rs, ... */
    }
    if (type & FRTD) {
        opcode |= cval << 16;           /* rt equals rd */
    }
    if ((type & FMOD) && (type & (FRD1 | FRT1 | FRS1)))
        clobber_reg = cval;

    /*
     * Second register.
     */
    if (type & FRD2) {
        if (getlex (&cval) != ',')
            uerror ("comma expected");
        clex = getlex (&cval);
        if (clex != LREG)
            uerror ("bad rd register");
        opcode |= cval << 11;           /* .., rd, ... */
    }
    if (type & FRT2) {
        if (getlex (&cval) != ',')
            uerror ("comma expected");
        clex = getlex (&cval);
        if (clex != LREG) {
            if ((type & FRD1) && (type & FSA)) {
                /* Second register operand omitted.
                 * Need to restore the missing operand. */
                ungetlex (clex, cval);
                cval = (opcode >> 11) & 31; /* get 1-st register */
                opcode |= cval << 16;       /* use 1-st reg as 2-nd */
                goto fsa;
            }
            uerror ("bad rt register");
        }
        opcode |= cval << 16;           /* .., rt, ... */
    }
    if (type & FRS2) {
        clex = getlex (&cval);
        if (clex != ',') {
            if ((opcode & 0xfc00003f) != 0x00000009)
                uerror ("comma expected");
            /* Jalr with one argument.
             * Treat as if the first argument is $31. */
            ungetlex (clex, cval);
            cval = (opcode >> 11) & 31; /* get 1-st register */
            opcode |= cval << 21;       /* use 1-st reg as 2-nd */
            opcode |= 31 << 11;         /* set 1-st reg to 31 */
            clobber_reg = 31;
            goto done3;
        }
        clex = getlex (&cval);
        if (clex != LREG) {
            if ((type & FRT1) && (type & FOFF16)) {
                /* Second register operand omitted.
                 * Need to restore the missing operand. */
                ungetlex (clex, cval);
                cval = (opcode >> 16) & 31; /* get 1-st register */
                opcode |= cval << 21;       /* use 1-st reg as 2-nd */
                goto foff16;
            }
            uerror ("bad rs register");
        }
        opcode |= cval << 21;           /* .., rs, ... */
    }

    /*
     * Third register.
     */
    if (type & FRT3) {
        clex = getlex (&cval);
        if (clex != ',') {
            /* Three-operand instruction used with two operands.
             * Need to restore the missing operand. */
            ungetlex (clex, cval);
            cval = (opcode >> 21) & 31;
            opcode &= ~(31 << 21);                  /* clear 2-nd register */
            opcode |= ((opcode >> 11) & 31) << 21;  /* use 1-st reg as 2-nd */
            opcode |= cval << 16;                   /* add 3-rd register */
            goto done3;
        }
        clex = getlex (&cval);
        if (clex != LREG) {
            if ((type & FRD1) && (type & FRS2)) {
                /* Three-operand instruction used with literal operand.
                 * Convert it to immediate type. */
                unsigned newop;
                switch (opcode & 0xfc0007ff) {
                case 0x00000020: newop = 0x20000000; break; // add -> addi
                case 0x00000021: newop = 0x24000000; break; // addu -> addiu
                case 0x00000024: newop = 0x30000000; break; // and -> andi
                case 0x00000025: newop = 0x34000000; break; // or -> ori
                case 0x0000002a: newop = 0x28000000; break; // slt -> slti
                case 0x0000002b: newop = 0x2c000000; break; // sltu -> sltiu
                case 0x00000022: newop = 0x20000000;        // sub -> addi, negate
                                 negate_literal = 1; break;
                case 0x00000023: newop = 0x24000000;        // subu -> addiu, negate
                                 negate_literal = 1; break;
                case 0x00000026: newop = 0x38000000; break; // xor -> xori
                default:
                    uerror ("bad rt register");
                    return;
                }
                ungetlex (clex, cval);
                cval = (opcode >> 11) & 31;         /* get 1-st register */
                newop |= cval << 16;                /* set 1-st register */
                newop |= opcode & (31 << 21);       /* set 2-nd register */
                opcode = newop;
                type = FRT1 | FRS2 | FOFF16 | FMOD;
                goto foff16;
            }
            uerror ("bad rt register");
        }
        opcode |= cval << 16;           /* .., .., rt */
    }
    if (type & FRS3) {
        clex = getlex (&cval);
        if (clex != ',') {
            /* Three-operand instruction used with two operands.
             * Need to restore the missing operand. */
            ungetlex (clex, cval);
            cval = (opcode >> 16) & 31;
            opcode &= ~(31 << 16);                  /* clear 2-nd register */
            opcode |= ((opcode >> 11) & 31) << 16;  /* use 1-st reg as 2-nd */
            opcode |= cval << 21;                   /* add 3-rd register */
            goto done3;
        }
        clex = getlex (&cval);
        if (clex != LREG)
            uerror ("bad rs register");
        opcode |= cval << 21;           /* .., .., rs */
    }
done3:

    /*
     * Immediate argument.
     */
    if (type & FSEL) {
        /* optional COP0 register select */
        clex = getlex (&cval);

    } else if (type & (FCODE | FCODE16 | FSA)) {
        /* Non-relocatable offset */
        if (type & FSA) {
            if (getlex (&cval) != ',')
                uerror ("comma expected");
            clex = getlex (&cval);
            if (clex == LREG && type == (FRD1 | FRT2 | FSA | FMOD)) {
                /* Literal-operand shift instruction used with register operand.
                 * Convert it to 3-register type. */
                unsigned newop;
                switch (opcode & 0xffe0003f) {
                case 0x00200002: newop = 0x00000046; break; // ror -> rorv
                case 0x00000000: newop = 0x00000004; break; // sll -> sllv
                case 0x00000003: newop = 0x00000007; break; // sra -> srav
                case 0x00000002: newop = 0x00000006; break; // srl -> srlv
                default:
                    uerror ("bad shift amount");
                    return;
                }
                newop |= opcode & (0x3ff << 11);    /* set 1-st and 2-nd regs */
                newop |= cval << 21;                /* set 3-rd register */
                opcode = newop;
                type = FRD1 | FRT2 | FRS3 | FMOD;
                goto done3;
            }
            ungetlex (clex, cval);
        }
fsa:    offset = getaddr();
        switch (type & (FCODE | FCODE16 | FSA)) {
        case FCODE:                     /* immediate shifted <<6 */
            opcode |= offset << 6;
            break;
        case FCODE16:                   /* immediate shifted <<16 */
            opcode |= offset << 16;
            break;
        case FSA:                       /* shift amount */
            opcode |= (offset & 0x1f) << 6;
            break;
        }
    }

    /*
     * Last argument.
     */
    if (type & FRSB) {
        if (getlex (&cval) != '(')
            uerror ("left par expected");
        clex = getlex (&cval);
        if (clex != LREG)
            uerror ("bad rs register");
        if (getlex (&cval) != ')')
            uerror ("right par expected");
        opcode |= cval << 21;           /* ... (rs) */
    }
    if (type & FSIZE) {
        if (getlex (&cval) != ',')
            uerror ("comma expected");
        offset = getaddr();
        opcode |= ((offset - 1) & 0x1f) << 11; /* bit field size */
    }
    if (type & FMSB) {
        if (getlex (&cval) != ',')
            uerror ("comma expected");
        offset += getaddr();
        if (offset > 32)
            offset = 32;
        opcode |= ((offset - 1) & 0x1f) << 11; /* msb */
    }
done:

    /* Output resulting values. */
    //TODO
    count++;
}

void pass1 ()
{
    int clex;
    int cval, tval, nbytes;
    unsigned addr;

    for (;;) {
        clex = getlex (&cval);
        switch (clex) {
        case LEOF:
            return;
        case LEOL:
            continue;
        case LNAME:
            cval = lookcmd();
            clex = getlex (&tval);
            if (clex == ':') {
                /* Label. */
                cval = lookname();
                stab[cval].value = count;
                stab[cval].undef = 0;
                continue;
            }
            /* Machine instruction. */
            if (cval < 0)
                uerror ("bad instruction");
            ungetlex (clex, tval);
            makecmd (optable[cval].opcode, optable[cval].type);
            break;
        case LNUM:
            /* Local label. */
            //TODO
            clex = getlex (&tval);
            if (clex != ':')
                uerror ("bad digital label");
            continue;
        default:
            uerror ("bad syntax");
        }
        clex = getlex (&cval);
        if (clex != LEOL) {
            if (clex == LEOF)
                return;
            uerror ("bad instruction arguments");
        }
    }
}

void pass2 ()
{
    int i;

    for (i=0; i<stabfree; i++) {
        /* Undefined label is fatal. */
        if (stab[i].undef)
            uerror ("name undefined", stab[i].n_name);
    }
    for (i=0; i<count; i++) {
        if (relinfo[i]) {
            /* Use value of the label. */
            code[i] = stab[code[i]].value;
        }
    }
}

void usage ()
{
    fprintf (stderr, "Usage:\n");
    fprintf (stderr, "  asmk [infile]\n");
    exit (1);
}

int main (argc, argv)
    char *argv[];
{
    int i;
    char *cp;

    /*
     * Parse options.
     */
    for (i=1; i<argc; i++) {
        switch (argv[i][0]) {
        case '-':
            for (cp=argv[i]+1; *cp; cp++) {
                switch (*cp) {
                default:
                    fprintf (stderr, "Unknown option: %s\n", cp);
                    usage();
                }
            }
            break;
        default:
            if (infile)
                uerror ("too many input files");
            infile = argv[i];
            break;
        }
    }
    if (! infile && isatty(0))
        usage();

    /*
     * Setup input-output.
     */
    if (infile && ! freopen (infile, "r", stdin))
        uerror ("cannot open %s", infile);

    line = 1;
    hashinit ();                        /* Initialize hash tables */
    pass1 ();                           /* First pass */
    pass2 ();                           /* Second pass */

    //TODO: print result
    return 0;
}

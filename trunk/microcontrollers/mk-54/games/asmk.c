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
#include <ctype.h>

/*
 * Types of lexemes.
 */
enum {
    LEOF = 1,           /* end of file */
    LEOL,               /* end of line */
    LNUM,               /* integer number */
    LNAME,              /* label identifier */
    LFUNC,              /* F key */
    LKKEY,              /* K key */
    LCMD,               /* instruction */
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
    { 0x0a, ",",        },
    { 0x0b, "/-/",      },
    { 0x0c, "ВП",       },
    { 0x0d, "Сx",       },
    { 0x0e, "В^",       },
    { 0x0f, "Вx",       FPREF },
    { 0x10, "+",        },
    { 0x11, "-",        },
    { 0x12, "x",        },
    { 0x13, "/",        },
    { 0x14, "<->",      },
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

struct {
    char *name;
    unsigned len;
    unsigned value;
    unsigned undef;
} label [STSIZE];

unsigned count;
unsigned char code[105];
unsigned char relinfo[105];
char *infile;
int line;                               /* Source line number */
int labelfree;
char space [STSIZE*8];                  /* Area for symbol names */
int lastfree;                           /* Free space offset */
char name [256];
int intval;
int extref;
int blexflag, backlex;
short hashtab [HASHSZ], hashctab [HCMDSZ];

/* Forward declarations. */
unsigned getreg (void);

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

    for (cp=name; c>='0' && c<='9'; c=getchar())
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

    for (cp=name; !isspace(c); c=getchar())
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

int looklabel()
{
    int i, h;

    /* Search for symbol name. */
    h = hash_rot13 (name) & (HASHSZ-1);
    while ((i = hashtab[h]) != -1) {
        if (! strcmp (label[i].name, name))
            return (i);
        if (--h < 0)
            h += HASHSZ;
    }

    /* Add a new symbol to table. */
    i = labelfree++;
    if (i >= STSIZE)
        uerror ("symbol table overflow");
    label[i].len = strlen (name);
    label[i].name = alloc (1 + label[i].len);
    strcpy (label[i].name, name);
    label[i].value = 0;
    label[i].undef = 1;
    hashtab[h] = i;
    return (i);
}

/*
 * Read a lexical element.
 * Return the type code.
 */
int getlex()
{
    int c;

    if (blexflag) {
        blexflag = 0;
        return (backlex);
    }
    for (;;) {
        switch (c = getchar()) {
        case ';':
            /* Comment to end of line. */
skiptoeol:  while ((c = getchar()) != '\n')
                if (c == EOF)
                    return (LEOF);
        case '\n':
            /* New line. */
            ++line;
            c = getchar ();
            if (c == ';')
                goto skiptoeol;
            ungetc (c, stdin);
            return (LEOL);
        case ' ':
        case '\t':
            /* Spaces ignored. */
            continue;
        case EOF:
            /* End of file. */
            return (LEOF);
        case ':':
            /* Syntax deimiters. */
            return (c);
        case '0':       case '1':       case '2':       case '3':
        case '4':       case '5':       case '6':       case '7':
        case '8':       case '9':
            /* Decimal constant. */
            getnum (c);
            return (LNUM);
        default:
            if (isspace (c))
                continue;
            getname (c);
            intval = lookcmd();
            if (intval >= 0)
                return (LCMD);
            return (LNAME);
        }
    }
}

void ungetlex (val)
{
    blexflag = 1;
    backlex = val;
}

/*
 * Get register number 0..9, a..d.
 * Return the value.
 */
unsigned getreg()
{
    int clex, i;
    static const struct {
        char *name;
        int value;
    } tab[] = {
        { "a", 10 }, { "A", 10 }, { "а", 10 }, { "А", 10 },
        { "b", 11 }, { "B", 11 }, { "б", 11 }, { "Б", 11 },
        { "c", 12 }, { "C", 12 }, { "с", 12 }, { "С", 12 },
        { "d", 13 }, { "D", 13 }, { "д", 13 }, { "Д", 13 },
        { "e", 14 }, { "E", 14 }, { "е", 14 }, { "Е", 14 },
        { 0 },
    };

    /* look a first lexeme */
    clex = getlex();
    switch (clex) {
    default:
        uerror ("operand missing");
    case LNUM:
        if (intval > 9)
            uerror ("bad register number '%u'", intval);
        return intval;
    case LNAME:
        for (i=0; tab[i].name; i++)
            if (strcmp (name, tab[i].name) == 0) {
                intval = tab[i].value;
                return intval;
            }
        uerror ("bad register number '%s'", name);
        return 0;
    }
}

/*
 * Build and emit a machine instruction code.
 */
int makecmd (opcode, type, f_flag, k_flag)
    unsigned opcode;
{
    /* Verify F and K prefixes. */
    if ((type & FPREF) && ! f_flag)
        uerror ("F prefix missing");
    if (! (type & FPREF) && f_flag)
        uerror ("excessive F prefix");
    if ((type & FPREK) && ! f_flag)
        uerror ("K prefix missing");
    if (! (type & FPREK) && f_flag)
        uerror ("excessive K prefix");

    /* Register number follows */
    if (type & FREG) {
        int reg = getreg();
        opcode |= reg;
    }

    /* Output resulting values. */
    code[count++] = opcode;

    /* Whether jump address follows. */
    return (type & FADDR) != 0;
}

void pass1 ()
{
    int clex, i;
    int f_seen = 0, k_seen = 0, need_address = 0;

    for (;;) {
        clex = getlex();
        switch (clex) {
        case LEOF:
            if (need_address)
                uerror ("jump address required");
            return;
        case LEOL:
            continue;
        case LFUNC:
            if (need_address)
                uerror ("jump address required");
            if (f_seen)
                uerror ("duplicate F key");
            f_seen = 1;
            k_seen = 0;
            continue;
        case LKKEY:
            if (need_address)
                uerror ("jump address required");
            if (k_seen)
                uerror ("duplicate K key");
            k_seen = 1;
            f_seen = 0;
            continue;
        case LNAME:
            /* Named label. */
            if (f_seen)
                uerror ("unexpected F key before label");
            if (k_seen)
                uerror ("unexpected K key before label");
            i = looklabel();
            clex = getlex();
            if (clex == ':') {
                /* Label defined. */
                i = looklabel();
                label[i].value = count;
                label[i].undef = 0;
                continue;
            }
            /* Label referenced. */
            ungetlex (clex);
            if (! need_address)
                uerror ("unused jump address");
            relinfo[count] = 1;
            makecmd (i, 0, 0, 0);
            break;
        case LNUM:
            /* Numeric label or address. */
            if (f_seen)
                uerror ("unexpected F key before label");
            if (k_seen)
                uerror ("unexpected K key before label");
            clex = getlex();
            if (clex == ':') {
                /* Digital label. */
                if (intval != count)
                    uerror ("incorrect label value %d, expected %d", intval, count);
            } else {
                ungetlex (clex);
                if (need_address) {
                    /* Jump address. */
                    i = (intval / 10) << 4 | (intval % 10);
                    makecmd (i, 0, 0, 0);
                } else {
                    /* Enter decimal digit. */
                    if (intval > 9)
                        uerror ("unknown opcode '%d'", intval);
                    makecmd (intval, 0, 0, 0);
                }
            }
            continue;
        case LCMD:
            /* Machine instruction. */
            if (need_address)
                uerror ("jump address required");
            i = intval;
            need_address = makecmd (optable[i].opcode,
                optable[i].type, f_seen, k_seen);
            break;
        default:
            uerror ("bad syntax");
        }
    }
}

void pass2 ()
{
    int i;

    for (i=0; i<labelfree; i++) {
        /* Undefined label is fatal. */
        if (label[i].undef)
            uerror ("label '%s' undefined", label[i].name);
    }
    for (i=0; i<count; i++) {
        if (relinfo[i]) {
            /* Use value of the label. */
            code[i] = label[code[i]].value;
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

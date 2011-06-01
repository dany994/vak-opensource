/*
 * Интерфейс через JTAG к процессору Элвис Мультикор.
 * Автор: С.Вакуленко.
 *
 * Этот файл распространяется в надежде, что он окажется полезным, но
 * БЕЗ КАКИХ БЫ ТО НИ БЫЛО ГАРАНТИЙНЫХ ОБЯЗАТЕЛЬСТВ; в том числе без косвенных
 * гарантийных обязательств, связанных с ПОТРЕБИТЕЛЬСКИМИ СВОЙСТВАМИ и
 * ПРИГОДНОСТЬЮ ДЛЯ ОПРЕДЕЛЕННЫХ ЦЕЛЕЙ.
 *
 * Вы вправе распространять и/или изменять этот файл в соответствии
 * с условиями Генеральной Общественной Лицензии GNU (GPL) в том виде,
 * как она была опубликована Фондом Свободного ПО; либо версии 2 Лицензии
 * либо (по вашему желанию) любой более поздней версии. Подробности
 * смотрите в прилагаемом файле 'COPYING.txt'.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>

#include "target.h"
#include "adapter.h"
//#include "arm-jtag.h"
#include "localize.h"

struct _target_t {
    adapter_t   *adapter;
    const char  *cpu_name;
    unsigned    cpuid;
    unsigned    flash_addr;
    unsigned    flash_bytes;
};

#if defined (__CYGWIN32__) || defined (MINGW32)
/*
 * Задержка в миллисекундах: Windows.
 */
#include <windows.h>

void mdelay (unsigned msec)
{
    Sleep (msec);
}
#else
/*
 * Задержка в миллисекундах: Unix.
 */
void mdelay (unsigned msec)
{
    usleep (msec * 1000);
}
#endif

unsigned target_read_word (target_t *t, unsigned address)
{
    unsigned value = 0;

//    t->adapter->mem_ap_write (t->adapter, MEM_AP_TAR, address);
//    value = t->adapter->mem_ap_read (t->adapter, MEM_AP_DRW);
    if (debug_level) {
        fprintf (stderr, "word read %08x from %08x\n",
            value, address);
    }
    return value;
}

/*
 * Запись слова в память.
 */
void target_write_word (target_t *t, unsigned address, unsigned data)
{
    if (debug_level) {
        fprintf (stderr, _("word write %08x to %08x\n"), data, address);
    }
//    t->adapter->mem_ap_write (t->adapter, MEM_AP_TAR, address);
//    t->adapter->mem_ap_write (t->adapter, MEM_AP_DRW, data);
}

/*
 * Устанавливаем соединение с адаптером JTAG.
 */
target_t *target_open (int need_reset)
{
    target_t *t;

    t = calloc (1, sizeof (target_t));
    if (! t) {
        fprintf (stderr, _("Out of memory\n"));
        exit (-1);
    }
    t->cpu_name = "Unknown";

    /* Ищем адаптер. */
    t->adapter = adapter_open_pickit2 ();
    if (! t->adapter) {
        fprintf (stderr, _("No JTAG adapter found.\n"));
        exit (-1);
    }
#if 0
    /* Проверяем идентификатор процессора. */
    unsigned idcode = t->adapter->get_idcode (t->adapter);
    if (debug_level)
        fprintf (stderr, "idcode %08X\n", idcode);

    /* Проверяем идентификатор ARM Debug Interface v5. */
    if (idcode != 0x4ba00477) {
        /* Device not detected. */
        if (idcode == 0xffffffff || idcode == 0)
            fprintf (stderr, _("No response from device -- check power is on!\n"));
        else
            fprintf (stderr, _("No response from device -- unknown idcode 0x%08X!\n"),
                idcode);
        t->adapter->close (t->adapter);
        exit (1);
    }
    t->adapter->reset_cpu (t->adapter);

    /* Проверяем идентификатор процессора. */
    t->cpuid = target_read_word (t, CPUID);
    switch (t->cpuid) {
    case 0x412fc230:    /* Миландр 1986ВМ91Т */
        t->cpu_name = _("Microchip PIC32");
        t->flash_addr = 0x08000000;
        t->flash_bytes = 128*1024;
        break;
    default:
        /* Device not detected. */
        fprintf (stderr, _("Unknown CPUID=%08x.\n"), t->cpuid);
        t->adapter->close (t->adapter);
        exit (1);
    }

    /* Подача тактовой частоты на периферийные блоки. */
    target_write_word (t, PER_CLOCK, 0xFFFFFFFF);

    /* Запрет прерываний. */
    target_write_word (t, ICER0, 0xFFFFFFFF);
#endif
    return t;
}

/*
 * Close the device.
 */
void target_close (target_t *t)
{
#if 0
    t->adapter->reset_cpu (t->adapter);

    /* Пускаем процессор. */
    target_write_word (t, DCB_DHCSR, DBGKEY);
    t->adapter->dp_read (t->adapter, DP_CTRL_STAT);
    t->adapter->mem_ap_write (t->adapter, MEM_AP_CSW, 0);
    t->adapter->dp_read (t->adapter, DP_CTRL_STAT);

    t->adapter->reset_cpu (t->adapter);
#endif
    t->adapter->close (t->adapter);
}

const char *target_cpu_name (target_t *t)
{
    return t->cpu_name;
}

unsigned target_idcode (target_t *t)
{
    return t->cpuid;
}

unsigned target_flash_bytes (target_t *t)
{
    return t->flash_bytes;
}

/*
 * Стирание всей flash-памяти.
 */
int target_erase (target_t *t, unsigned addr)
{
    printf (_("Erase: %08X..."), t->flash_addr);
    fflush (stdout);

    // TODO
    printf (_(" done\n"));
    return 1;
}

/*
 * Стирание одного блока памяти
 */
int target_erase_block (target_t *t, unsigned addr)
{
    printf (_("Erase block: %08X..."), addr);
    fflush (stdout);

    // TODO
    printf (_(" done\n"));
    return 1;
}

/*
 * Чтение данных из памяти.
 */
void target_read_block (target_t *t, unsigned addr,
    unsigned nwords, unsigned *data)
{
//fprintf (stderr, "target_read_block (addr = %x, nwords = %d)\n", addr, nwords);
    while (nwords > 0) {
        unsigned n = 10;
        if (n > nwords)
            n = nwords;
        t->adapter->read_data (t->adapter, addr, n, data);
        if (t->adapter->stalled) {
            if (debug_level > 1)
                fprintf (stderr, "MEM-AP read data <<<WAIT>>>\n");
            continue;
        }
        addr += n<<2;
        data += n;
        nwords -= n;
    }
//fprintf (stderr, "    done (addr = %x)\n", addr);
}

void target_write_block (target_t *t, unsigned addr,
    unsigned nwords, unsigned *data)
{
#if 0
    unsigned i;

    t->adapter->mem_ap_write (t->adapter, MEM_AP_TAR, addr);
    for (i=0; i<nwords; i++, addr+=4, data++) {
        if (debug_level) {
            fprintf (stderr, _("block write %08x to %08x\n"), *data, addr);
        }
        t->adapter->mem_ap_write (t->adapter, MEM_AP_DRW, *data);
    }
#endif
}

/*
 * Программирование одной страницы памяти (до 256 слов).
 * Страница должна быть предварительно стёрта.
 */
void target_program_block (target_t *t, unsigned pageaddr,
    unsigned nwords, unsigned *data)
{
    // TODO
}

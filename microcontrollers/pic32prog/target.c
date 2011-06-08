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
#include "localize.h"

struct _target_t {
    adapter_t   *adapter;
    const char  *cpu_name;
    unsigned    cpuid;
    unsigned    flash_addr;
    unsigned    flash_bytes;
};

static const struct {
    unsigned devid;
    const char *name;
    unsigned bytes;
} pic32mx_dev[] = {
    {0x04A07053, "110F016B",  16*1024},
    {0x04A09053, "110F016C",  16*1024},
    {0x04A0B053, "110F016D",  16*1024},
    {0x04A06053, "120F032B",  32*1024},
    {0x04A08053, "120F032C",  32*1024},
    {0x04A0A053, "120F032D",  32*1024},
    {0x04A01053, "210F016B",  16*1024},
    {0x04A03053, "210F016C",  16*1024},
    {0x04A05053, "210F016D",  16*1024},
    {0x04A00053, "220F032B",  32*1024},
    {0x04A02053, "220F032C",  32*1024},
    {0x04A04053, "220F032D",  32*1024},
    {0x00938053, "360F512L", 512*1024},
    {0x00934053, "360F256L", 256*1024},
    {0x0092D053, "340F128L", 128*1024},
    {0x0092A053, "320F128L", 128*1024},
    {0x00916053, "340F512H", 512*1024},
    {0x00912053, "340F256H", 256*1024},
    {0x0090D053, "340F128H", 128*1024},
    {0x0090A053, "320F128H", 128*1024},
    {0x00906053, "320F064H",  64*1024},
    {0x00902053, "320F032H",  32*1024},
    {0x00978053, "460F512L", 512*1024},
    {0x00974053, "460F256L", 256*1024},
    {0x0096D053, "440F128L", 128*1024},
    {0x00952053, "440F256H", 256*1024},
    {0x00956053, "440F512H", 512*1024},
    {0x0094D053, "440F128H", 128*1024},
    {0x00942053, "420F032H",  32*1024},
    {0x04307053, "795F512L", 512*1024},
    {0x0430E053, "795F512H", 512*1024},
    {0x04306053, "775F512L", 512*1024},
    {0x0430D053, "775F512H", 512*1024},
    {0x04312053, "775F256L", 256*1024},
    {0x04303053, "775F256H", 256*1024},
    {0x04417053, "764F128L", 128*1024},
    {0x0440B053, "764F128H", 128*1024},
    {0x04341053, "695F512L", 512*1024},
    {0x04325053, "695F512H", 512*1024},
    {0x04311053, "675F512L", 512*1024},
    {0x0430C053, "675F512H", 512*1024},
    {0x04305053, "675F256L", 256*1024},
    {0x0430B053, "675F256H", 256*1024},
    {0x04413053, "664F128L", 128*1024},
    {0x04407053, "664F128H", 128*1024},
    {0x04411053, "664F064L",  64*1024},
    {0x04405053, "664F064H",  64*1024},
    {0x0430F053, "575F512L", 512*1024},
    {0x04309053, "575F512H", 512*1024},
    {0x04333053, "575F256L", 256*1024},
    {0x04317053, "575F256H", 256*1024},
    {0x0440F053, "564F128L", 128*1024},
    {0x04403053, "564F128H", 128*1024},
    {0x0440D053, "564F064L",  64*1024},
    {0x04401053, "564F064H",  64*1024},
    {0x04400053, "534F064H",  64*1024},
    {0x0440C053, "534F064L",  64*1024},
    {0}
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
target_t *target_open ()
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
    if (! t->adapter)
        t->adapter = adapter_open_mpsse ();
    if (! t->adapter) {
        fprintf (stderr, _("No JTAG adapter found.\n"));
        exit (-1);
    }

    /* Проверяем идентификатор процессора. */
    t->cpuid = t->adapter->get_idcode (t->adapter);
    unsigned i;
    for (i=0; t->cpuid != pic32mx_dev[i].devid; i++) {
        if (pic32mx_dev[i].devid == 0) {
            /* Device not detected. */
            fprintf (stderr, _("Unknown CPUID=%08x.\n"), t->cpuid);
            t->adapter->close (t->adapter);
            exit (1);
        }
    }
    t->cpu_name = pic32mx_dev[i].name;
    t->flash_addr = 0x1d000000;
    t->flash_bytes = pic32mx_dev[i].bytes;
    return t;
}

/*
 * Close the device.
 */
void target_close (target_t *t)
{
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
fprintf (stderr, "target_read_block (addr = %x, nwords = %d)\n", addr, nwords);
    while (nwords >= 256) {
        unsigned n = 256;
        t->adapter->read_data (t->adapter, addr, n, data);
        addr += n<<2;
        data += n;
        nwords -= n;
    }
fprintf (stderr, "    done (addr = %x)\n", addr);
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

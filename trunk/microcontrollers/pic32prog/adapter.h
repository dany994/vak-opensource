/*
 * Обобщённый JTAG-адаптер. Программный интерфейс нижнего уровня.
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
#include <stdarg.h>

typedef struct _adapter_t adapter_t;

struct _adapter_t {
    /*
     * Флаг, указывающий, что предыдущая транзакция AP read/write
     * не завершилась и требует повтора. Устанавливается и сбрасывается
     * функциями dp_read() и mem_ap_read().
     */
    int stalled;

    /*
     * Обязательные функции.
     */
    void (*close) (adapter_t *a);
    unsigned (*get_idcode) (adapter_t *a);
    void (*load_executable) (adapter_t *a);
    void (*read_data) (adapter_t *a, unsigned addr, unsigned nwords, unsigned *data);
    void (*program_data) (adapter_t *a, unsigned addr, unsigned nwords, unsigned *data);
    unsigned (*read_word) (adapter_t *a, unsigned addr);
    void (*erase_chip) (adapter_t *a);
};

adapter_t *adapter_open_pickit2 (void);
adapter_t *adapter_open_mpsse (void);

void mdelay (unsigned msec);
extern int debug_level;

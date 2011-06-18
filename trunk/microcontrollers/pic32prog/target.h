/*
 * Программный интерфейс управления целевым процессором
 * через адаптер JTAG.
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

typedef struct _target_t target_t;

target_t *target_open (void);
void target_close (target_t *t);
void target_use_executable (target_t *t);

unsigned target_idcode (target_t *t);
const char *target_cpu_name (target_t *t);
unsigned target_flash_width (target_t *t);
unsigned target_flash_bytes (target_t *t);
void target_print_devcfg (target_t *t);

void target_read_block (target_t *t, unsigned addr,
	unsigned nwords, unsigned *data);

int target_erase (target_t *t);
void target_program_block (target_t *t, unsigned addr,
	unsigned nwords, unsigned *data);

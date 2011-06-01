/*
 * Интерфейс через адаптер FT2232 к процессору Элвис Мультикор.
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
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <usb.h>

#include "adapter.h"
#include "pickit2.h"

typedef struct {
    /* Общая часть */
    adapter_t adapter;

    /* Доступ к устройству через libusb. */
    usb_dev_handle *usbdev;

} pickit2_adapter_t;

/*
 * Identifiers of USB adapter.
 */
#define MICROCHIP_VID           0x04d8
#define PICKIT2_PID             0x0033  /* Microchip PICkit 2 */

/*
 * USB endpoints.
 */
#define OUT_EP                  0x01
#define IN_EP                   0x81

#define TIMO_MSEC               1500

static void pickit2_send (pickit2_adapter_t *a, unsigned argc, ...)
{
    va_list ap;
    unsigned i;
    char buf [64];

    memset (buf, CMD_END_OF_BUFFER, 64);
    va_start (ap, argc);
    for (i=0; i<argc; ++i)
        buf[i] = va_arg (ap, int);
    va_end (ap);
    if (usb_interrupt_write (a->usbdev, OUT_EP, buf, 64, TIMO_MSEC) < 0) {
        fprintf (stderr, "PICkit2: error sending packet\n");
        usb_release_interface (a->usbdev, 0);
        usb_close (a->usbdev);
        exit (-1);
    }
}

static void pickit2_close (adapter_t *adapter)
{
    pickit2_adapter_t *a = (pickit2_adapter_t*) adapter;

    usb_release_interface (a->usbdev, 0);
    usb_close (a->usbdev);
    free (a);
}

/*
 * Read the Device Identification code
 */
static unsigned pickit2_get_idcode (adapter_t *adapter)
{
    //pickit2_adapter_t *a = (pickit2_adapter_t*) adapter;
    unsigned idcode = 0;

    //TODO
    //pickit2_send (a, 6, 31, 32, 0, 1);
    //idcode = pickit2_recv (a);
    return idcode;
}

/*
 * Чтение блока памяти.
 * Предварительно в регистр DP_SELECT должен быть занесён 0.
 * Количество слов не больше 10, иначе переполняется буфер USB.
 */
static void pickit2_read_data (adapter_t *adapter,
    unsigned addr, unsigned nwords, unsigned *data)
{
    //pickit2_adapter_t *a = (pickit2_adapter_t*) adapter;
#if 0
    /* Пишем адрес в регистр TAR. */
    pickit2_mem_ap_write (adapter, MEM_AP_TAR, addr);

    /* Запрашиваем данные через регистр DRW.
     * Первое чтение не выдаёт значения. */
    pickit2_send (a, 1, 1, 4, JTAG_IR_APACC, 0);
    pickit2_send (a, 0, 0, 32 + 3, (MEM_AP_DRW >> 1 & 6) | 1, 0);
    unsigned i;
    for (i=0; i<nwords; i++) {
        pickit2_send (a, 1, 1, 4, JTAG_IR_APACC, 0);
        pickit2_send (a, 0, 0, 32 + 3, (MEM_AP_DRW >> 1 & 6) | 1, 1);
    }
    /* Шлём пакет. */
    pickit2_flush_output (a);

    /* Извлекаем и обрабатываем данные. */
    for (i=0; i<nwords; i++) {
        unsigned long long reply;
        memcpy (&reply, a->input + i*a->bytes_per_word, sizeof (reply));
        reply = pickit2_fix_data (a, reply);
        adapter->stalled = ((unsigned) reply & 7) != 2;
        data[i] = reply >> 3;
    }
#endif
}

/*
 * Инициализация адаптера F2232.
 * Возвращаем указатель на структуру данных, выделяемую динамически.
 * Если адаптер не обнаружен, возвращаем 0.
 */
adapter_t *adapter_open_pickit2 (void)
{
    pickit2_adapter_t *a;
    struct usb_bus *bus;
    struct usb_device *dev;

    usb_init();
    usb_find_busses();
    usb_find_devices();
    for (bus = usb_get_busses(); bus; bus = bus->next) {
        for (dev = bus->devices; dev; dev = dev->next) {
            if (dev->descriptor.idVendor == MICROCHIP_VID &&
                (dev->descriptor.idProduct == PICKIT2_PID))
                goto found;
        }
    }
    /*fprintf (stderr, "USB adapter not found: vid=%04x, pid=%04x\n",
        MICROCHIP_VID, PICKIT2_PID);*/
    return 0;
found:
    /*fprintf (stderr, "found USB adapter: vid %04x, pid %04x, type %03x\n",
        dev->descriptor.idVendor, dev->descriptor.idProduct,
        dev->descriptor.bcdDevice);*/
    a = calloc (1, sizeof (*a));
    if (! a) {
        fprintf (stderr, "Out of memory\n");
        return 0;
    }
    a->usbdev = usb_open (dev);
    if (! a->usbdev) {
        fprintf (stderr, "PICkit2: usb_open() failed\n");
        free (a);
        return 0;
    }
    if (usb_set_configuration (a->usbdev, 2) < 0) {
        fprintf (stderr, "PICkit2: cannot set USB configuration\n");
failed: usb_release_interface (a->usbdev, 0);
        usb_close (a->usbdev);
        free (a);
        return 0;
    }
    if (usb_claim_interface (a->usbdev, 0) < 0) {
        fprintf (stderr, "PICkit2: cannot claim USB interface\n");
        goto failed;
    }

    pickit2_send (a, 1, CMD_GET_VERSION);

    char reply [64];
    if (usb_interrupt_read (a->usbdev, IN_EP, reply, 64, TIMO_MSEC) != 64) {
        fprintf (stderr, "PICkit2: cannot get adapter version\n");
        goto failed;
    }
    fprintf (stderr, "Found PICkit 2 adapter: Version %d.%d.%d\n",
        (unsigned char) reply[0], (unsigned char) reply[1],
        (unsigned char) reply[2]);

    /* Обязательные функции. */
    a->adapter.close = pickit2_close;
    a->adapter.get_idcode = pickit2_get_idcode;
    a->adapter.read_data = pickit2_read_data;
    return &a->adapter;
}

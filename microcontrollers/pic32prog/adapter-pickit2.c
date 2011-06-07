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

    unsigned char reply [64];

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

#define IFACE                   0
#define TIMO_MSEC               1500

static void pickit2_send (pickit2_adapter_t *a, unsigned argc, ...)
{
    va_list ap;
    unsigned i;
    unsigned char buf [64];

    memset (buf, CMD_END_OF_BUFFER, 64);
    va_start (ap, argc);
    for (i=0; i<argc; ++i)
        buf[i] = va_arg (ap, int);
    va_end (ap);
    if (debug_level > 0) {
        int k;
        fprintf (stderr, "---Send");
        for (k=0; k<i; ++k) {
            if (k != 0 && (k & 15) == 0)
                fprintf (stderr, "\n       ");
            fprintf (stderr, " %02x", buf[k]);
        }
        fprintf (stderr, "\n");
    }
    if (usb_interrupt_write (a->usbdev, OUT_EP, (char*) buf, 64, TIMO_MSEC) < 0) {
        fprintf (stderr, "PICkit2: error sending packet\n");
        usb_release_interface (a->usbdev, IFACE);
        usb_close (a->usbdev);
        exit (-1);
    }
}

static void pickit2_recv (pickit2_adapter_t *a)
{
    if (usb_interrupt_read (a->usbdev, IN_EP, (char*) a->reply,
            64, TIMO_MSEC) != 64) {
        fprintf (stderr, "PICkit2: error receiving packet\n");
        usb_release_interface (a->usbdev, IFACE);
        usb_close (a->usbdev);
        exit (-1);
    }
    if (debug_level > 0) {
        int k;
        fprintf (stderr, "--->>>>");
        for (k=0; k<64; ++k) {
            if (k != 0 && (k & 15) == 0)
                fprintf (stderr, "\n       ");
            fprintf (stderr, " %02x", a->reply[k]);
        }
        fprintf (stderr, "\n");
    }
}

static void pickit2_close (adapter_t *adapter)
{
    pickit2_adapter_t *a = (pickit2_adapter_t*) adapter;

    /* Exit programming mode. */
    pickit2_send (a, 18, CMD_CLEAR_UPLOAD_BUFFER, CMD_EXECUTE_SCRIPT, 15,
        SCRIPT_JT2_SETMODE, 5, 0x1f,
        SCRIPT_VPP_OFF,
        SCRIPT_MCLR_GND_ON,
        SCRIPT_VPP_PWM_OFF,
        SCRIPT_SET_ICSP_PINS, 6,
        SCRIPT_SET_ICSP_PINS, 2,
        SCRIPT_SET_ICSP_PINS, 3,
        SCRIPT_DELAY_LONG, 10,
        SCRIPT_BUSY_LED_OFF);

    /* Detach power from the board. */
    pickit2_send (a, 4, CMD_EXECUTE_SCRIPT, 2,
        SCRIPT_VDD_OFF,
        SCRIPT_VDD_GND_ON);

    /* Disable reset. */
    pickit2_send (a, 3, CMD_EXECUTE_SCRIPT, 1,
        SCRIPT_MCLR_GND_OFF);

    /* Read board status. */
    pickit2_send (a, 2, CMD_CLEAR_UPLOAD_BUFFER, CMD_READ_STATUS);
    pickit2_recv (a);
//fprintf (stderr, "PICkit2: status %02x%02x\n", a->reply[1], a->reply[0]);

//fprintf (stderr, "PICkit2: close\n");
    usb_release_interface (a->usbdev, IFACE);
    usb_close (a->usbdev);
    free (a);
}

/*
 * Read the Device Identification code
 */
static unsigned pickit2_get_idcode (adapter_t *adapter)
{
    pickit2_adapter_t *a = (pickit2_adapter_t*) adapter;
    unsigned idcode;

    /* Read device id. */
    pickit2_send (a, 12, CMD_CLEAR_UPLOAD_BUFFER, CMD_EXECUTE_SCRIPT, 9,
        SCRIPT_JT2_SENDCMD, 4,
        SCRIPT_JT2_SENDCMD, 1,
        SCRIPT_JT2_XFERDATA32_LIT, 0, 0, 0, 0);
    pickit2_send (a, 1, CMD_UPLOAD_DATA);
    pickit2_recv (a);
//fprintf (stderr, "PICkit2: read id, %d bytes: %02x %02x %02x %02x\n", a->reply[0], a->reply[1], a->reply[2], a->reply[3], a->reply[4]);
    if (a->reply[0] != 4)
        return 0;
    idcode = a->reply[1] | a->reply[2] << 8 | a->reply[3] << 16 | a->reply[4] << 24;
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
failed: usb_release_interface (a->usbdev, IFACE);
        usb_close (a->usbdev);
        free (a);
        return 0;
    }
    if (usb_claim_interface (a->usbdev, IFACE) < 0) {
        fprintf (stderr, "PICkit2: cannot claim USB interface\n");
        goto failed;
    }

    /* Read version of adapter. */
    pickit2_send (a, 2, CMD_CLEAR_UPLOAD_BUFFER, CMD_GET_VERSION);
    pickit2_recv (a);
    fprintf (stderr, "PICkit2: Version %d.%d.%d\n",
        a->reply[0], a->reply[1], a->reply[2]);

    /* Detach power from the board. */
    pickit2_send (a, 4, CMD_EXECUTE_SCRIPT, 2,
        SCRIPT_VDD_OFF,
        SCRIPT_VDD_GND_ON);

    /* Setup power voltage 3.3V, fault limit 2.81V. */
    unsigned vdd = (unsigned) (3.3 * 32 + 10.5) << 6;
    unsigned vdd_limit = (unsigned) ((2.81 / 5) * 255);
    pickit2_send (a, 4, CMD_SET_VDD, vdd, vdd >> 8, vdd_limit);

    /* Setup reset voltage 3.28V, fault limit 2.26V. */
    unsigned vpp = (unsigned) (3.28 * 18.61);
    unsigned vpp_limit = (unsigned) (2.26 * 18.61);
    pickit2_send (a, 4, CMD_SET_VPP, 0x40, vpp, vpp_limit);

    /* Setup serial speed. */
    unsigned divisor = 10;
    pickit2_send (a, 4, CMD_EXECUTE_SCRIPT, 2,
        SCRIPT_SET_ICSP_SPEED, divisor);

    /* Reset active low. */
    pickit2_send (a, 3, CMD_EXECUTE_SCRIPT, 1,
        SCRIPT_MCLR_GND_ON);

    /* Read board status. */
    pickit2_send (a, 2, CMD_CLEAR_UPLOAD_BUFFER, CMD_READ_STATUS);
    pickit2_recv (a);
    unsigned status = a->reply[0] | a->reply[1] << 8;
    if (debug_level > 0)
        fprintf (stderr, "PICkit2: status %04x\n", status);
    if ((status & ~STATUS_RESET) != (STATUS_VDD_GND_ON | STATUS_VPP_GND_ON)) {
        fprintf (stderr, "PICkit2: invalid status = %04x\n", status);
        goto failed;
    }

    /* Enable power to the board. */
    pickit2_send (a, 4, CMD_EXECUTE_SCRIPT, 2,
        SCRIPT_VDD_GND_OFF,
        SCRIPT_VDD_ON);

    /* Read board status. */
    pickit2_send (a, 2, CMD_CLEAR_UPLOAD_BUFFER, CMD_READ_STATUS);
    pickit2_recv (a);
    status = a->reply[0] | a->reply[1] << 8;
    if (debug_level > 0)
        fprintf (stderr, "PICkit2: status %04x\n", status);
    if (status != (STATUS_VDD_ON | STATUS_VPP_GND_ON)) {
        fprintf (stderr, "PICkit2: invalid status = %04x.\n", status);
        goto failed;
    }

    /* Enter programming mode. */
    pickit2_send (a, 42, CMD_CLEAR_UPLOAD_BUFFER, CMD_EXECUTE_SCRIPT, 39,
        SCRIPT_VPP_OFF,
        SCRIPT_MCLR_GND_ON,
        SCRIPT_VPP_PWM_ON,
        SCRIPT_BUSY_LED_ON,
        SCRIPT_SET_ICSP_PINS, 0,
        SCRIPT_DELAY_LONG, 20,
        SCRIPT_MCLR_GND_OFF,
        SCRIPT_VPP_ON,
        SCRIPT_DELAY_SHORT, 23,
        SCRIPT_VPP_OFF,
        SCRIPT_MCLR_GND_ON,
        SCRIPT_DELAY_SHORT, 47,
        SCRIPT_WRITE_BYTE_LITERAL, 0xb2,
        SCRIPT_WRITE_BYTE_LITERAL, 0xc2,
        SCRIPT_WRITE_BYTE_LITERAL, 0x12,
        SCRIPT_WRITE_BYTE_LITERAL, 0x0a,
        SCRIPT_MCLR_GND_OFF,
        SCRIPT_VPP_ON,
//        SCRIPT_DELAY_SHORT, 235,
        SCRIPT_DELAY_LONG, 1,
        SCRIPT_SET_ICSP_PINS, 2,
        SCRIPT_JT2_SETMODE, 6, 0x1f,
        SCRIPT_JT2_SENDCMD, 4,
        SCRIPT_JT2_SENDCMD, 7,
        SCRIPT_JT2_XFERDATA8_LIT, 0);

    /* User functions. */
    a->adapter.close = pickit2_close;
    a->adapter.get_idcode = pickit2_get_idcode;
    a->adapter.read_data = pickit2_read_data;
    return &a->adapter;
}

typedef struct _prog_t prog_t;

prog_t *prog_open (char *devname);
void prog_close (prog_t *c);
void prog_set_rst (prog_t *c, int on);
void prog_set_sck (prog_t *c, int on);
void prog_set_data (prog_t *c, int on);
unsigned char prog_get_data (prog_t *c);

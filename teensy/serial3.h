void serial3_begin(uint32_t divisor);
void serial3_format(uint32_t format);
void serial3_end(void);
void serial3_set_transmit_pin(uint8_t pin);
void serial3_putchar(uint32_t c);
void serial3_write(const void *buf, unsigned int count);
void serial3_flush(void);
int serial3_write_buffer_free(void);
int serial3_available(void);
int serial3_getchar(void);
int serial3_peek(void);
void serial3_clear(void);
void uart2_status_isr(void);

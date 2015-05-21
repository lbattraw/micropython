#ifndef _INCLUDED_UART_H_
#define _INCLUDED_UART_H_

// uncomment to enable 9 bit formats
//#define SERIAL_9BIT_SUPPORT

#define SERIAL_7E1 0x02
#define SERIAL_7O1 0x03
#define SERIAL_8N1 0x00
#define SERIAL_8N2 0x04
#define SERIAL_8E1 0x06
#define SERIAL_8O1 0x07
#define SERIAL_7E1_RXINV 0x12
#define SERIAL_7O1_RXINV 0x13
#define SERIAL_8N1_RXINV 0x10
#define SERIAL_8N2_RXINV 0x14
#define SERIAL_8E1_RXINV 0x16
#define SERIAL_8O1_RXINV 0x17
#define SERIAL_7E1_TXINV 0x22
#define SERIAL_7O1_TXINV 0x23
#define SERIAL_8N1_TXINV 0x20
#define SERIAL_8N2_TXINV 0x24
#define SERIAL_8E1_TXINV 0x26
#define SERIAL_8O1_TXINV 0x27
#define SERIAL_7E1_RXINV_TXINV 0x32
#define SERIAL_7O1_RXINV_TXINV 0x33
#define SERIAL_8N1_RXINV_TXINV 0x30
#define SERIAL_8N2_RXINV_TXINV 0x34
#define SERIAL_8E1_RXINV_TXINV 0x36
#define SERIAL_8O1_RXINV_TXINV 0x37
#ifdef SERIAL_9BIT_SUPPORT
#define SERIAL_9N1 0x84
#define SERIAL_9E1 0x8E
#define SERIAL_9O1 0x8F
#define SERIAL_9N1_RXINV 0x94
#define SERIAL_9E1_RXINV 0x9E
#define SERIAL_9O1_RXINV 0x9F
#define SERIAL_9N1_TXINV 0xA4
#define SERIAL_9E1_TXINV 0xAE
#define SERIAL_9O1_TXINV 0xAF
#define SERIAL_9N1_RXINV_TXINV 0xB4
#define SERIAL_9E1_RXINV_TXINV 0xBE
#define SERIAL_9O1_RXINV_TXINV 0xBF
#endif
// bit0: parity, 0=even, 1=odd
// bit1: parity, 0=disable, 1=enable
// bit2: mode, 1=9bit, 0=8bit
// bit3: mode10: 1=10bit, 0=8bit
// bit4: rxinv, 0=normal, 1=inverted
// bit5: txinv, 0=normal, 1=inverted
// bit6: unused
// bit7: actual data goes into 9th bit


#define BAUD2DIV(baud)  (((F_CPU * 2) + ((baud) >> 1)) / (baud))
#define BAUD2DIV3(baud) (((F_BUS * 2) + ((baud) >> 1)) / (baud))

// C language implementation
//
#ifdef __cplusplus
extern "C" {
#endif
void serial_begin(uint32_t divisor);
void serial_format(uint32_t format);
void serial_end(void);
void serial_set_transmit_pin(uint8_t pin);
void serial_putchar(uint32_t c);
void serial_write(const void *buf, unsigned int count);
void serial_flush(void);
int serial_write_buffer_free(void);
int serial_available(void);
int serial_getchar(void);
int serial_peek(void);
void serial_clear(void);
void serial_print(const char *p);
void serial_phex(uint32_t n);
void serial_phex16(uint32_t n);
void serial_phex32(uint32_t n);

void serial2_begin(uint32_t divisor);
void serial2_format(uint32_t format);
void serial2_end(void);
void serial2_set_transmit_pin(uint8_t pin);
void serial2_putchar(uint32_t c);
void serial2_write(const void *buf, unsigned int count);
void serial2_flush(void);
int serial2_write_buffer_free(void);
int serial2_available(void);
int serial2_getchar(void);
int serial2_peek(void);
void serial2_clear(void);

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

typedef enum {
    UART_BIT_RATE_300     = 300,
    UART_BIT_RATE_600     = 600,
    UART_BIT_RATE_1200     = 1200,	
    UART_BIT_RATE_2400     = 2400,
    UART_BIT_RATE_4800     = 4800,
    UART_BIT_RATE_9600     = 9600,
    UART_BIT_RATE_19200   = 19200,
    UART_BIT_RATE_38400   = 38400,
    UART_BIT_RATE_57600   = 57600,
    UART_BIT_RATE_74880   = 74880,
    UART_BIT_RATE_115200 = 115200,
    UART_BIT_RATE_230400 = 230400,
    UART_BIT_RATE_256000 = 256000,
    UART_BIT_RATE_460800 = 460800,
    UART_BIT_RATE_921600 = 921600
} UartBaudRate;

// THIS IS ALL WRONG, not based on real Teensy values
typedef enum {
    UART_FIVE_BITS = 0x0,
    UART_SIX_BITS = 0x1,
    UART_SEVEN_BITS = 0x2,
    UART_EIGHT_BITS = 0x3
} UartBitsNum4Char;


typedef enum {
    UART_ONE_STOP_BIT             = 0,
    UART_ONE_HALF_STOP_BIT        = 1,
    UART_TWO_STOP_BIT             = 2
} UartStopBitsNum;

typedef enum {
    UART_NONE_BITS = 0,
    UART_ODD_BITS   = 1,
    UART_EVEN_BITS = 2
} UartParityMode;

typedef enum {
    UART_STICK_PARITY_DIS   = 0,
    UART_STICK_PARITY_EN    = 1|2
} UartExistParity;
// ---------------------------------------------------------- END OF WRONG SECTION


typedef enum {
    UART_NONE_CTRL,
    UART_HARDWARE_CTRL,
    UART_XON_XOFF_CTRL
} UartFlowCtrl;

typedef enum {
    UART_EMPTY,
    UART_UNDER_WRITE,
    UART_WRITE_OVER
} RcvMsgBuffState;

typedef struct {
    uint32_t     RcvBuffSize;
    uint8_t     *pRcvMsgBuff;
    uint8_t     *pWritePos;
    uint8_t     *pReadPos;
    uint8_t      TrigLvl;
    RcvMsgBuffState  BuffState;
} RcvMsgBuff;

typedef struct {
    uint32_t   TrxBuffSize;
    uint8_t   *pTrxBuff;
} TrxMsgBuff;

typedef enum {
    UART_BAUD_RATE_DET,
    UART_WAIT_SYNC_FRM,
    UART_SRCH_MSG_HEAD,
    UART_RCV_MSG_BODY,
    UART_RCV_ESC_CHAR,
} RcvMsgState;

typedef struct {
    UartBaudRate      baud_rate;
    UartBitsNum4Char  data_bits;
    UartExistParity   exist_parity;
    UartParityMode    parity;
    UartStopBitsNum   stop_bits;
    UartFlowCtrl      flow_ctrl;
    RcvMsgBuff        rcv_buff;
    TrxMsgBuff        trx_buff;
    RcvMsgState       rcv_state;
    int               received;
    int               buff_uart_no;  //indicate which uart use tx/rx buffer
    UART_TypeDef	   *Instance;
} UartDevice;


//void uart_init(UartBaudRate uart1_br, UartBaudRate uart2_br, UartBaudRate uart3_br);
int uart1_rx(void);
int uart2_rx(void);
int uart3_rx(void);
void uart_tx_one_char(uint8_t uart, uint8_t TxChar);


// +++++Defines from serial1/2/3.c
////////////////////////////////////////////////////////////////
// Tunable parameters (relatively safe to edit these numbers)
////////////////////////////////////////////////////////////////

#define TX_BUFFER_SIZE 64 // number of outgoing bytes to buffer
#define RX_BUFFER_SIZE 64 // number of incoming bytes to buffer
#define IRQ_PRIORITY  64  // 0 = highest priority, 255 = lowest


////////////////////////////////////////////////////////////////
// changes not recommended below this point....
////////////////////////////////////////////////////////////////

#ifdef SERIAL_9BIT_SUPPORT
static uint8_t use9Bits = 0;
#define BUFTYPE uint16_t
#else
#define BUFTYPE uint8_t
#define use9Bits 0
#endif

static volatile BUFTYPE tx_buffer[TX_BUFFER_SIZE];
static volatile BUFTYPE rx_buffer[RX_BUFFER_SIZE];
static volatile uint8_t transmitting = 0;
//static volatile uint8_t *transmit_pin=NULL;
#if TX_BUFFER_SIZE > 255
static volatile uint16_t tx_buffer_head = 0;
static volatile uint16_t tx_buffer_tail = 0;
#else
static volatile uint8_t tx_buffer_head = 0;
static volatile uint8_t tx_buffer_tail = 0;
#endif
#if RX_BUFFER_SIZE > 255
static volatile uint16_t rx_buffer_head = 0;
static volatile uint16_t rx_buffer_tail = 0;
#else
static volatile uint8_t rx_buffer_head = 0;
static volatile uint8_t rx_buffer_tail = 0;
#endif
// +++++End of defines from serial1/2/3.c

#endif // _INCLUDED_UART_H_

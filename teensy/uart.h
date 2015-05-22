/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2013, 2014 Damien P. George
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
typedef enum {
    PYB_UART_0 = 0,
    PYB_UART_1 = 1,
    PYB_UART_2 = 2,
    PYB_UART_NONE = -1,
} pyb_uart_t;
typedef struct
{
  uint32_t BaudRate;                  /*!< This member configures the UART communication baud rate. */
  uint32_t WordLength;                /*!< Specifies the number of data bits transmitted or received in a frame.
                                           This parameter can be a value of @ref UART_Word_Length */
  uint32_t StopBits;                  /*!< Specifies the number of stop bits transmitted.
                                           This parameter can be a value of @ref UART_Stop_Bits */
  uint32_t Parity;                    /*!< Specifies the parity mode.
                                           This parameter can be a value of @ref UART_Parity
                                           @note When parity is enabled, the computed parity is inserted
                                                 at the MSB position of the transmitted data (9th bit when
                                                 the word length is set to 9 data bits; 8th bit when the
                                                 word length is set to 8 data bits). */
  uint32_t Uart_id;					/*!< Specifies the port number (0-2) */
}UART_InitTypeDef;
// #define USART6              ((USART_TypeDef *) USART6_BASE)
// #define USART6_BASE           (APB2PERIPH_BASE + 0x1400)
// UARTx = USART6;
// uart_obj->uart.Instance = UARTx;
// HAL_UART_Init(&uart_obj->uart);
typedef struct _pyb_uart_obj_t pyb_uart_obj_t;
extern const mp_obj_type_t pyb_uart_type;
void uart_init0(void);
void uart_deinit(void);
void uart_irq_handler(mp_uint_t uart_id);
bool uart_rx_any(pyb_uart_obj_t *uart_obj);
int uart_rx_char(pyb_uart_obj_t *uart_obj);
void uart_tx_strn(pyb_uart_obj_t *uart_obj, const char *str, uint len);
void uart_tx_strn_cooked(pyb_uart_obj_t *uart_obj, const char *str, uint len);

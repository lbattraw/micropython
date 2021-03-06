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

#include <stdio.h>
#include <string.h>

#include "py/nlr.h"
#include "py/runtime.h"
#include MICROPY_HAL_H
#include "bufhelper.h"
#include "uart.h"
#include "teensy_uart.h"

/// \moduleref pyb
/// \class UART - duplex serial communication bus
///
/// UART implements the standard UART/USART duplex serial communications protocol.  At
/// the physical level it consists of 2 lines: RX and TX.
///
/// See usage model of I2C.  UART is very similar.  Main difference is
/// parameters to init the UART bus:
///
///     from pyb import UART
///
///     uart = UART(1, 9600)                         # init with given baudrate
///     uart.init(9600, bits=8, stop=1, parity=None) # init with given parameters
///
/// Bits can be 8 or 9, stop can be 1 or 2, parity can be None, 0 (even), 1 (odd).
///
/// Extra method:
///
///     uart.any()               # returns True if any characters waiting

struct _pyb_uart_obj_t {
    mp_obj_base_t base;
    //pyb_uart_t uart_id : 8;
    bool is_enabled : 1;
    UartDevice *uart;
    //UART_TypeDef Instance;
    //UART_HandleTypeDef uart;
};

UartDevice uart0, uart1, uart2;

/* STM version:

struct _pyb_uart_obj_t {
    mp_obj_base_t base;
    UART_HandleTypeDef uart;            // this is 17 words big
    IRQn_Type irqn;
    pyb_uart_t uart_id : 8;
    bool is_enabled : 1;
    byte char_width;                    // 0 for 7,8 bit chars, 1 for 9 bit chars
    uint16_t char_mask;                 // 0x7f for 7 bit, 0xff for 8 bit, 0x1ff for 9 bit
    uint16_t timeout;                   // timeout waiting for first char
    uint16_t timeout_char;              // timeout waiting between chars
    uint16_t read_buf_len;              // len in chars; buf can hold len-1 chars
    volatile uint16_t read_buf_head;    // indexes first empty slot
    uint16_t read_buf_tail;             // indexes first full slot (not full if equals head)
    byte *read_buf;                     // byte or uint16_t, depending on char size
};

*/
// #define UART0                   (*(KINETISK_UART_t *)0x4006A000)


//TODO add support for USB VCP to all functions as UART_id 3 (Or maybe as 0 with that as default?)
//What do Freescale docs call UARTs: 0 - 2 or 1 - 3?
//


bool uart_init(pyb_uart_obj_t *uart_obj, uint32_t baudrate) {
	UartDevice *uh = (UartDevice *)&uart_obj->uart;
	//printf("uart_init, uart_obj: %x  Handle: %x  UartDevice: %x\n", &uart_obj, uh, uart_obj->uart);
	printf("uart_init calling for port ID %d at %d baud\n",uh->uart_id, uh->baud_rate);
	
	switch (uh->uart_id) {
	case PYB_UART_NONE:
		puts("uart_init called for UART_NONE");
		break;
	case PYB_UART_0:
		printf("Calling serial0_begin for port %d at %d baud\n", (int)uh->uart_id, (int)baudrate);
		serial0_begin(BAUD2DIV(baudrate));
		break;
	case PYB_UART_1:
		printf("Calling serial1_begin for port %d at %d baud\n", (int)uh->uart_id, (int)baudrate);
		serial1_begin(BAUD2DIV(baudrate));
		break;
	case PYB_UART_2:
		printf("Calling serial2_begin for port %d at %d baud\n", (int)uh->uart_id, (int)baudrate);
		serial2_begin(BAUD2DIV3(baudrate));
		break;
	default:
		puts("uart_init called for undefined UART ID, returning false");
		return(false);
	}
   
   /*
   TODO this section should be changing the settings in hardware to reflect *uh:
   
   uh->baud_rate = baudrate;
   uh->data_bits = 8;
   uh->stop_bits = 1;
   uh->parity = 0;  // 0: None   1: ODD   2: EVEN
   uh->flow_ctrl = 0;  // 0 - none 1 - full
   */
   printf("UART(%d) initialized at baud rate %d\n", (int)uh->uart_id, (int)uh->baud_rate);
   return true;
}

bool uart_rx_any(pyb_uart_obj_t *uart_obj) {
	switch (uart_obj->uart->uart_id) {
	case PYB_UART_NONE:
		puts("uart_rx_any called for UART_NONE");
		break;
        case PYB_UART_0:
        	return(serial0_available());
        	break;
        case PYB_UART_1:
        	return(serial1_available());
        	break;
        case PYB_UART_2:
        	return(serial2_available());
        	break;
        }
        return false;
}

int uart_rx_char(pyb_uart_obj_t *uart_obj) {
	switch (uart_obj->uart->uart_id) {
	case PYB_UART_NONE:
		puts("uart_rx_char called for UART_NONE");
		break;
        case PYB_UART_0:
        	return(serial0_getchar());
        	break;
        case PYB_UART_1:
        	return(serial1_getchar());
        	break;
        case PYB_UART_2:
        	return(serial2_getchar());
        	break;
        }
        return(-1);
}

void uart_tx_char(pyb_uart_obj_t *uart_obj, int c) {
        switch (uart_obj->uart->uart_id) {
        case PYB_UART_NONE:
        	puts("uart_tx_char called for UART_NONE");
        	break;
        case PYB_UART_0:
        	serial0_putchar(c);
        	break;
        case PYB_UART_1:
        	serial1_putchar(c);
        	break;
        case PYB_UART_2:
        	serial2_putchar(c);
        	break;
        }
}

void uart_tx_str(pyb_uart_obj_t *uart_obj, const char *str) {
	switch (uart_obj->uart->uart_id) {
	case PYB_UART_NONE:
		puts("uart_tx_str called for UART_NONE");
		break;
        case PYB_UART_0:
        	serial0_write(str, strlen(str));
        	break;
        case PYB_UART_1:
        	serial1_write(str, strlen(str));
        	break;
        case PYB_UART_2:
        	serial2_write(str, strlen(str));
        	break;
        }
}

void uart_tx_strn(pyb_uart_obj_t *uart_obj, const char *str, uint len) {
	switch (uart_obj->uart->uart_id) {
	case PYB_UART_NONE:
		puts("uart_tx_strn called for UART_NONE");
		break;
        case PYB_UART_0:
        	serial0_write(str, len);
        	break;
        case PYB_UART_1:
        	serial1_write(str,len);
        	break;
        case PYB_UART_2:
        	serial2_write(str, len);
        	break;
        }
}

void uart_tx_strn_cooked(pyb_uart_obj_t *uart_obj, const char *str, uint len) {
    for (const char *top = str + len; str < top; str++) {
        if (*str == '\n') {
            uart_tx_char(uart_obj, '\r');
        }
        uart_tx_char(uart_obj, *str);
    }
}

/******************************************************************************/
/* Micro Python bindings                                                      */

STATIC void pyb_uart_print(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind) {
    pyb_uart_obj_t *self = self_in;
    if (!self->is_enabled) {
        mp_printf(print, "UART(%lu)", self->uart->uart_id);
    } else {
#if 0
        mp_printf(print, "UART(%lu, baudrate=%u, bits=%u, stop=%u",
            self->uart_id, self->uart->Init.BaudRate,
            self->uart.Init.WordLength == UART_WORDLENGTH_8B ? 8 : 9,
            self->uart.Init.StopBits == UART_STOPBITS_1 ? 1 : 2);
        if (self->uart.Init.Parity == UART_PARITY_NONE) {
            mp_print_str(print, ", parity=None)");
        } else {
            mp_printf(print, ", parity=%u)", self->uart.Init.Parity == UART_PARITY_EVEN ? 0 : 1);
        }
#endif
    }
}

/// \method init(uart_id, baudrate, *, bits=8, stop=1, parity=None)
///
/// Initialise the UART with the given parameters:
///
///   - `baudrate` is the clock rate.
///   - `bits` is the number of bits per byte, 8 or 9.
///   - `stop` is the number of stop bits, 1 or 2.
///   - `parity` is the parity, `None`, 0 (even) or 1 (odd).
STATIC const mp_arg_t pyb_uart_init_args[] = {
    { MP_QSTR_baudrate, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 115200} },
    { MP_QSTR_uart_id,   MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 0} },
    { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 8} },
    { MP_QSTR_stop,     MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 1} },
    { MP_QSTR_parity,   MP_ARG_KW_ONLY | MP_ARG_OBJ,  {.u_obj = mp_const_none} },
};
#define PYB_UART_INIT_NUM_ARGS MP_ARRAY_SIZE(pyb_uart_init_args)

/*typedef struct {
    UartBaudRate      baud_rate;
    int	   			   uart_id;
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
} UartDevice; */

STATIC mp_obj_t pyb_uart_init_helper(pyb_uart_obj_t *self, uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
	UartDevice *uart = self->uart;
	
    // parse args 
    //printf("In pyb_uart_init_helper before mp_arg_parse_all self: %x  self->uart: %x  passed %d args, expecting up to %d args for initialization (baud, bits per byte, stop bits, parity)\n", self, &self->uart, n_args, PYB_UART_INIT_NUM_ARGS);
    mp_arg_val_t vals[PYB_UART_INIT_NUM_ARGS];
    mp_arg_parse_all(n_args, args, kw_args, PYB_UART_INIT_NUM_ARGS, pyb_uart_init_args, vals);
    //printf("In pyb_uart_init_helper pt2, port ID: %d, baud rate: %d\n", (int)vals[1].u_int, (int)vals[0].u_int);
    
    switch(vals[1].u_int)
    {
    	case 0:
    		uart = &uart0;
    		break;
    	case 1:
    		uart = &uart1;
    		break;
    	case 2:
    		uart = &uart2;
    		break;
    	default:
    		return(NULL);
    }
    uart->baud_rate = vals[0].u_int;
    uart->uart_id = vals[1].u_int;
    uart->data_bits = vals[2].u_int;
    switch (vals[3].u_int) {
        case 2: uart->stop_bits = 2; break;
        default: uart->stop_bits = 1; break;
    }
    if (vals[4].u_obj == mp_const_none) {
        uart->parity = 0;
    } else {
        mp_int_t parity = mp_obj_get_int(vals[3].u_obj);
        uart->parity = parity;
    }
    uart_init(self, vals[0].u_int);
    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
///
/// Construct a UART object on the given bus; `bus` can be 1-3
/// With no additional parameters, the UART object is created but not
/// initialised (it has the settings from the last initialisation of
/// the bus, if any).  If extra arguments are given, the bus is initialised.
/// See `init` for parameters of initialisation.
///
/// The physical pins of the UART busses are:
///
///  TBD
///
STATIC mp_obj_t pyb_uart_make_new(mp_obj_t type_in, uint n_args, uint n_kw, const mp_obj_t *args) {
    // check arguments
    puts("Start: pyb_uart_make_new");
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // create object
    pyb_uart_obj_t *o = m_new_obj(pyb_uart_obj_t);
    o->base.type = &pyb_uart_type;

    // work out port
    o->uart->uart_id = 0;

    if (MP_OBJ_IS_STR(args[0])) {
        o->uart->uart_id = mp_obj_get_int(args[0]);
    }

    if (n_args > 1 || n_kw > 0) {
        // start the peripheral
        mp_map_t kw_args;
        mp_map_init_fixed_table(&kw_args, n_kw, args + n_args);
        puts("In pyb_uart_make_new, calling pyb_uart_init_helper");
        pyb_uart_init_helper(o, n_args - 1, args + 1, &kw_args);
    }

    return o;
}

// Entry point from external function call
STATIC mp_obj_t pyb_uart_init(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
	//printf("In pyb_uart_init, calling pyb_uart_init_helper, arg count: %d  arg 0: %d  arg 1: %d  args: %x  mp_map: %x\n",n_args, (int)mp_obj_get_int(args[0]), (int)mp_obj_get_int(args[1]), args, kw_args);
	return pyb_uart_init_helper(args[0], n_args, args, kw_args);
    //return pyb_uart_init_helper(args[0], n_args - 1, args + 1, kw_args);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_uart_init_obj, 1, pyb_uart_init);

/// \method deinit()
/// Turn off the UART bus.
STATIC mp_obj_t pyb_uart_deinit(mp_obj_t self_in) {
    //pyb_uart_obj_t *self = self_in;
    // TODO
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_deinit_obj, pyb_uart_deinit);

/// \method any()
/// Return `True` if any characters waiting, else `False`.
STATIC mp_obj_t pyb_uart_any(mp_obj_t self_in) {
    pyb_uart_obj_t *self = self_in;
    if (uart_rx_any(self)) {
        return mp_const_true;
    } else {
        return mp_const_false;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(pyb_uart_any_obj, pyb_uart_any);

/// \method send(send, *, uart_id=0, timeout=5000)
/// Send data on the bus:
///
///   - `send` is the data to send (an integer to send, or a buffer object).
///   - `timeout` is the timeout in milliseconds to wait for the send.
///	  - `uart_id` is the UART number to use when sending the data
/// Return value: `None`.

STATIC const mp_arg_t pyb_uart_write_args[] = {
    { MP_QSTR_write, MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_uart_id, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 0} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
};
#define PYB_UART_WRITE_NUM_ARGS MP_ARRAY_SIZE(pyb_uart_write_args)

STATIC mp_obj_t pyb_uart_write(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // TODO assumes transmission size is 8-bits 
    mp_uint_t wstr_len;
    // parse args
    mp_arg_val_t vals[PYB_UART_WRITE_NUM_ARGS];
    mp_arg_parse_all(n_args , args , kw_args, PYB_UART_WRITE_NUM_ARGS, pyb_uart_write_args, vals);

    const char *wstr = mp_obj_str_get_data(vals[0].u_obj, &wstr_len);

    printf("pyb_uart_write UART ID: %d  data: %s\n",(int)vals[1].u_int, wstr);
    switch((int)vals[1].u_int)
    {
    	case 0:
    		serial0_write(wstr, wstr_len);
    		break;
    	case 1:
    		serial1_write(wstr, wstr_len);
    		break;
    	case 2:
    		serial2_write(wstr, wstr_len);
    		break;
    	default:
    		break;
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_uart_write_obj, 1, pyb_uart_write);


// Helper function for reading UART strings
// Pass UART object to indicate which UART to use, mp_obj_t for string buffer to write to||length to receive
// Stops on a CR (Omitting the CR and any LF characters)
// Returns number of characters received
int serial_read_string(pyb_uart_obj_t *uart, mp_obj_t *rstr)
{
	GET_STR_DATA_LEN(rstr, str_data, str_len);
	int len = 0, ch = -1;
	
	do
	{
		ch = -1;
		while(ch == -1) // getchar returns -1 if no char is available
		{
			switch(uart.uart_id)
			{
				case 0:
					ch = serial0_getchar();
					break;
				case 1:
					ch = serial1_getchar();
					break;
				case 2:
					ch = serial2_getchar();
					break;
				default:
					return(0); // If an illegal UART is specified, return 0 characters
					break;
			}
		}
		if(ch != 10) // LF
		{
			if(ch != 13)
				*bufptr++ = ch;
			else
				break;
		}
		//Ignore any LF characters;
		//Don't store CRs and stop when one received
	}
	while((bufptr - buf) - buflen > 0);
	return(bufptr - buf);
}


/// \method read(read, *, timeout=5000)
///
/// Receive data on the bus:
///
///   - `recv` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: if `recv` is an integer then a new buffer of the bytes received,
/// otherwise the same buffer that was passed in to `recv`.
STATIC const mp_arg_t pyb_uart_read_args[] = {
    { MP_QSTR_read,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
};
#define PYB_UART_READ_NUM_ARGS MP_ARRAY_SIZE(pyb_uart_read_args)

/*
STATIC mp_obj_t pyb_uart_read(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // TODO assumes transmission size is 8-bits wide
    int retlen = -1;
    pyb_uart_obj_t *self = args[0];
    mp_obj_t rb_obj;

    // parse args
    mp_arg_val_t vals[PYB_UART_READ_NUM_ARGS];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, PYB_UART_READ_NUM_ARGS, pyb_uart_read_args, vals);
    printf("pyb_uart_read arg 0: %d\n", vals[0].u_int);
    
    //Use the given buffer if first arg is not int
    if(MP_OBJ_IS_INT(vals[0]))
    {
    	vstr *r_vstr;
    	// allocate a new bytearray of given length
    	vstr_init(&r_vstr, vals[0].u_int);
    	
    	//mp_buffer_info_t bufinfo;
		//pyb_buf_get_for_recv(vals[0].u_obj, &bufinfo);
		//Need to add timeout code (vals[1])
		retlen = serial_read_string(&self->uart, &o_ret);
    
    (void)self;
    if(retlen < 0)
    	return mp_const_none;
    else
    	return o_ret;
}*/


// From stmhal
STATIC mp_uint_t pyb_uart_read(mp_obj_t self_in, void *buf_in, mp_uint_t size, int *errcode) {
    pyb_uart_obj_t *self = self_in;
    byte *buf = buf_in;

    // check that size is a multiple of character width
    if (size & self->char_width) {
        *errcode = EIO;
        return MP_STREAM_ERROR;
    }

    // convert byte size to char size
    size >>= self->char_width;

    // make sure we want at least 1 char
    if (size == 0) {
        return 0;
    }

    // wait for first char to become available
    if (!uart_rx_wait(self, self->timeout)) {
        // we can either return 0 to indicate EOF (then read() method returns b'')
        // or return EAGAIN error to indicate non-blocking (then read() method returns None)
        return 0;
    }

    // read the data
    byte *orig_buf = buf;
    for (;;) {
        int data = uart_rx_char(self);
        if (self->char_width == CHAR_WIDTH_9BIT) {
            *(uint16_t*)buf = data;
            buf += 2;
        } else {
            *buf++ = data;
        }
        if (--size == 0 || !uart_rx_wait(self, self->timeout_char)) {
            // return number of bytes read
            return buf - orig_buf;
        }
    }
}


STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_uart_read_obj, 1, pyb_uart_read);

STATIC const mp_map_elem_t pyb_uart_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&pyb_uart_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit), (mp_obj_t)&pyb_uart_deinit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_any), (mp_obj_t)&pyb_uart_any_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&pyb_uart_write_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&pyb_uart_read_obj },
};

/* STMHAL Methods to Implement
STATIC const mp_map_elem_t pyb_uart_locals_dict_table[] = {
    // instance methods

    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&pyb_uart_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit), (mp_obj_t)&pyb_uart_deinit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_any), (mp_obj_t)&pyb_uart_any_obj },

    /// \method read([nbytes])
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&mp_stream_read_obj },
    /// \method readall()
    { MP_OBJ_NEW_QSTR(MP_QSTR_readall), (mp_obj_t)&mp_stream_readall_obj },
    /// \method readline()
    { MP_OBJ_NEW_QSTR(MP_QSTR_readline), (mp_obj_t)&mp_stream_unbuffered_readline_obj},
    /// \method readinto(buf[, nbytes])
    { MP_OBJ_NEW_QSTR(MP_QSTR_readinto), (mp_obj_t)&mp_stream_readinto_obj },
    /// \method write(buf)
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&mp_stream_write_obj },

    { MP_OBJ_NEW_QSTR(MP_QSTR_writechar), (mp_obj_t)&pyb_uart_writechar_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readchar), (mp_obj_t)&pyb_uart_readchar_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_sendbreak), (mp_obj_t)&pyb_uart_sendbreak_obj },

    // class constants
    { MP_OBJ_NEW_QSTR(MP_QSTR_RTS), MP_OBJ_NEW_SMALL_INT(UART_HWCONTROL_RTS) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_CTS), MP_OBJ_NEW_SMALL_INT(UART_HWCONTROL_CTS) },
}; */


STATIC MP_DEFINE_CONST_DICT(pyb_uart_locals_dict, pyb_uart_locals_dict_table);

const mp_obj_type_t pyb_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .print = pyb_uart_print,
    .make_new = pyb_uart_make_new,
    .locals_dict = (mp_obj_t)&pyb_uart_locals_dict,
};

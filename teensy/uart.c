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
    pyb_uart_t uart_id;
    bool is_enabled;
    UartDevice uart;
    //UART_HandleTypeDef uart;
};


bool uart_init(pyb_uart_obj_t *uart_obj, uint32_t baudrate) {
   UartDevice *uh = &uart_obj->uart;
   /*
    PRIOR Working Code
    // init UARTx
    uart_obj->uart.Instance = UARTx;
    HAL_UART_Init(&uart_obj->uart);

    uart_obj->is_enabled = true; */
   
   printf("Memset of size %d \n",sizeof(*uh));
    memset(uh, 0, sizeof(*uh));
    uh->baud_rate = baudrate;
    uh->data_bits = 8;
    uh->stop_bits = 1;
    uh->parity = 0;  // 0: None   1: ODD   2: EVEN
    uh->flow_ctrl = 0;  // 0 - none 1 - full
    puts("serial.begin calling: ");
        switch (uart_obj->uart_id) {
        case PYB_UART_1:
        	serial_begin(BAUD2DIV(baudrate));
        	break;
        case PYB_UART_2:
        	serial2_begin(BAUD2DIV(baudrate));
        	break;
        case PYB_UART_3:
        	serial3_begin(BAUD2DIV3(baudrate));
        	break;
        }
    printf("UART(%d) initialized at baud rate %d\n", uart_obj->uart_id, baudrate);
    //serial_write(str, strlen(str));
    return true; //uart_init2(uart_obj);
}

bool uart_rx_any(pyb_uart_obj_t *uart_obj) {
	switch (uart_obj->uart_id) {
        case PYB_UART_1:
        	return(serial_available());
        	break;
        case PYB_UART_2:
        	return(serial2_available());
        	break;
        case PYB_UART_3:
        	return(serial3_available());
        	break;
        }
        return false;
}

int uart_rx_char(pyb_uart_obj_t *uart_obj) {
	switch (uart_obj->uart_id) {
        case PYB_UART_1:
        	return(serial_getchar());
        	break;
        case PYB_UART_2:
        	return(serial2_getchar());
        	break;
        case PYB_UART_3:
        	return(serial3_getchar());
        	break;
        }
        return(-1);
}



void uart_tx_char(pyb_uart_obj_t *uart_obj, int c) {
        switch (uart_obj->uart_id) {
        case PYB_UART_1:
        	serial_putchar(c);
        	break;
        case PYB_UART_2:
        	serial2_putchar(c);
        	break;
        case PYB_UART_3:
        	serial3_putchar(c);
        	break;
        }
}

void uart_tx_str(pyb_uart_obj_t *uart_obj, const char *str) {
	switch (uart_obj->uart_id) {
        case PYB_UART_1:
        	serial_write(str, strlen(str));
        	break;
        case PYB_UART_2:
        	serial2_write(str, strlen(str));
        	break;
        case PYB_UART_3:
        	serial3_write(str, strlen(str));
        	break;
        }
}

void uart_tx_strn(pyb_uart_obj_t *uart_obj, const char *str, uint len) {
	switch (uart_obj->uart_id) {
        case PYB_UART_1:
        	serial_write(str, len);
        	break;
        case PYB_UART_2:
        	serial2_write(str,len);
        	break;
        case PYB_UART_3:
        	serial3_write(str, len);
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
        mp_printf(print, "UART(%lu)", self->uart_id);
    } else {
#if 0
        mp_printf(print, "UART(%lu, baudrate=%u, bits=%u, stop=%u",
            self->uart_id, self->uart.Init.BaudRate,
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

/// \method init(baudrate, *, bits=8, stop=1, parity=None)
///
/// Initialise the UART with the given parameters:
///
///   - `baudrate` is the clock rate.
///   - `bits` is the number of bits per byte, 8 or 9.
///   - `stop` is the number of stop bits, 1 or 2.
///   - `parity` is the parity, `None`, 0 (even) or 1 (odd).
STATIC const mp_arg_t pyb_uart_init_args[] = {
    { MP_QSTR_baudrate, MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = 115200} },
    { MP_QSTR_bits,     MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 8} },
    { MP_QSTR_stop,     MP_ARG_KW_ONLY | MP_ARG_INT,  {.u_int = 1} },
    { MP_QSTR_parity,   MP_ARG_KW_ONLY | MP_ARG_OBJ,  {.u_obj = mp_const_none} },
};
#define PYB_UART_INIT_NUM_ARGS MP_ARRAY_SIZE(pyb_uart_init_args)

STATIC mp_obj_t pyb_uart_init_helper(pyb_uart_obj_t *self, uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // parse args 
    mp_arg_val_t vals[PYB_UART_INIT_NUM_ARGS];
    mp_arg_parse_all(n_args, args, kw_args, PYB_UART_INIT_NUM_ARGS, pyb_uart_init_args, vals);
    
    printf("In pyb_uart_init_helper, size: %d  \n", &self->uart, sizeof(self->uart));
    puts("Performing memset on &self->uart");
    memset(&self->uart, 0, sizeof(self->uart));
    puts("Memset complete");

    /*
    // set the UART configuration values
    UART_InitTypeDef *init = &self->uart.Init;
    init->BaudRate = vals[0].u_int;
    init->WordLength = vals[1].u_int == 8 ? UART_WORDLENGTH_8B : UART_WORDLENGTH_9B;
    switch (vals[2].u_int) {
        case 1: init->StopBits = UART_STOPBITS_1; break;
        default: init->StopBits = UART_STOPBITS_2; break;
    }
    if (vals[3].u_obj == mp_const_none) {
        init->Parity = UART_PARITY_NONE;
    } else {
        mp_int_t parity = mp_obj_get_int(vals[3].u_obj);
        init->Parity = (parity & 1) ? UART_PARITY_ODD : UART_PARITY_EVEN;
    }
    init->Mode = UART_MODE_TX_RX;
    init->HwFlowCtl = UART_HWCONTROL_NONE;
    init->OverSampling = UART_OVERSAMPLING_16;*/

    // init UART (if it fails, it's because the port doesn't exist)
    printf("Initializing UART number %d at %d baud ...\n", self->uart_id, vals[0].u_int);
    uart_init(self, vals[0].u_int);
    puts("Init complete!");
    return mp_const_none;
}

/// \classmethod \constructor(bus, ...)
///
/// Construct a UART object on the given bus.  `bus` can be 1-6, or 'XA', 'XB', 'YA', or 'YB'.
/// With no additional parameters, the UART object is created but not
/// initialised (it has the settings from the last initialisation of
/// the bus, if any).  If extra arguments are given, the bus is initialised.
/// See `init` for parameters of initialisation.
///
/// The physical pins of the UART busses are:
///
///   - `UART(4)` is on `XA`: `(TX, RX) = (X1, X2) = (PA0, PA1)`
///   - `UART(1)` is on `XB`: `(TX, RX) = (X9, X10) = (PB6, PB7)`
///   - `UART(6)` is on `YA`: `(TX, RX) = (Y1, Y2) = (PC6, PC7)`
///   - `UART(3)` is on `YB`: `(TX, RX) = (Y9, Y10) = (PB10, PB11)`
///   - `UART(2)` is on: `(TX, RX) = (X3, X4) = (PA2, PA3)`
STATIC mp_obj_t pyb_uart_make_new(mp_obj_t type_in, uint n_args, uint n_kw, const mp_obj_t *args) {
    // check arguments
    mp_arg_check_num(n_args, n_kw, 1, MP_OBJ_FUN_ARGS_MAX, true);

    // create object
    pyb_uart_obj_t *o = m_new_obj(pyb_uart_obj_t);
    o->base.type = &pyb_uart_type;

    // work out port
    o->uart_id = 0;

    if (MP_OBJ_IS_STR(args[0])) {
        const char *port = mp_obj_str_get_str(args[0]);
        o->uart_id = mp_obj_get_int(args[0]);
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
	printf("In pyb_uart_init, calling pyb_uart_init_helper, arg count: %d  arg 0: %x  args: %x  mp_map: %x\n",n_args, args[0], args, kw_args);
    return pyb_uart_init_helper(args[0], n_args - 1, args + 1, kw_args);
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

/// \method send(send, *, timeout=5000)
/// Send data on the bus:
///
///   - `send` is the data to send (an integer to send, or a buffer object).
///   - `timeout` is the timeout in milliseconds to wait for the send.
///
/// Return value: `None`.
STATIC const mp_arg_t pyb_uart_send_args[] = {
    { MP_QSTR_send,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
};
#define PYB_UART_SEND_NUM_ARGS MP_ARRAY_SIZE(pyb_uart_send_args)

STATIC mp_obj_t pyb_uart_send(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // TODO assumes transmission size is 8-bits wide

    pyb_uart_obj_t *self = args[0];

    // parse args
    mp_arg_val_t vals[PYB_UART_SEND_NUM_ARGS];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, PYB_UART_SEND_NUM_ARGS, pyb_uart_send_args, vals);

#if 0
    // get the buffer to send from
    mp_buffer_info_t bufinfo;
    uint8_t data[1];
    pyb_buf_get_for_send(vals[0].u_obj, &bufinfo, data);

    // send the data
    HAL_StatusTypeDef status = HAL_UART_Transmit(&self->uart, bufinfo.buf, bufinfo.len, vals[1].u_int);

    if (status != HAL_OK) {
        // TODO really need a HardwareError object, or something
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception, "HAL_UART_Transmit failed with code %d", status));
    }
#else
    (void)self;
#endif

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_uart_send_obj, 1, pyb_uart_send);

/// \method recv(recv, *, timeout=5000)
///
/// Receive data on the bus:
///
///   - `recv` can be an integer, which is the number of bytes to receive,
///     or a mutable buffer, which will be filled with received bytes.
///   - `timeout` is the timeout in milliseconds to wait for the receive.
///
/// Return value: if `recv` is an integer then a new buffer of the bytes received,
/// otherwise the same buffer that was passed in to `recv`.
STATIC const mp_arg_t pyb_uart_recv_args[] = {
    { MP_QSTR_recv,    MP_ARG_REQUIRED | MP_ARG_OBJ, {.u_obj = MP_OBJ_NULL} },
    { MP_QSTR_timeout, MP_ARG_KW_ONLY | MP_ARG_INT, {.u_int = 5000} },
};
#define PYB_UART_RECV_NUM_ARGS MP_ARRAY_SIZE(pyb_uart_recv_args)

STATIC mp_obj_t pyb_uart_recv(uint n_args, const mp_obj_t *args, mp_map_t *kw_args) {
    // TODO assumes transmission size is 8-bits wide

    pyb_uart_obj_t *self = args[0];

#if 0
    // parse args
    mp_arg_val_t vals[PYB_UART_RECV_NUM_ARGS];
    mp_arg_parse_all(n_args - 1, args + 1, kw_args, PYB_UART_RECV_NUM_ARGS, pyb_uart_recv_args, vals);

    // get the buffer to receive into
    mp_buffer_info_t bufinfo;
    mp_obj_t o_ret = pyb_buf_get_for_recv(vals[0].u_obj, &bufinfo);

    // receive the data
    HAL_StatusTypeDef status = HAL_UART_Receive(&self->uart, bufinfo.buf, bufinfo.len, vals[1].u_int);

    if (status != HAL_OK) {
        // TODO really need a HardwareError object, or something
        nlr_raise(mp_obj_new_exception_msg_varg(&mp_type_Exception, "HAL_UART_Receive failed with code %d", status));
    }

    // return the received data
    if (o_ret == MP_OBJ_NULL) {
        return vals[0].u_obj;
    } else {
        return mp_obj_str_builder_end(o_ret);
    }
#else
    (void)self;
    return mp_const_none;
#endif
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(pyb_uart_recv_obj, 1, pyb_uart_recv);

STATIC const mp_map_elem_t pyb_uart_locals_dict_table[] = {
    // instance methods
    { MP_OBJ_NEW_QSTR(MP_QSTR_init), (mp_obj_t)&pyb_uart_init_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_deinit), (mp_obj_t)&pyb_uart_deinit_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_any), (mp_obj_t)&pyb_uart_any_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_send), (mp_obj_t)&pyb_uart_send_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_recv), (mp_obj_t)&pyb_uart_recv_obj },
};

STATIC MP_DEFINE_CONST_DICT(pyb_uart_locals_dict, pyb_uart_locals_dict_table);

const mp_obj_type_t pyb_uart_type = {
    { &mp_type_type },
    .name = MP_QSTR_UART,
    .print = pyb_uart_print,
    .make_new = pyb_uart_make_new,
    .locals_dict = (mp_obj_t)&pyb_uart_locals_dict,
};

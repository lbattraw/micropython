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

#include <stdlib.h>

#include "py/objtype.h"
#include "py/runtime.h"

typedef struct _mp_obj_object_t {
    mp_obj_base_t base;
} mp_obj_object_t;

STATIC mp_obj_t object_make_new(mp_obj_t type_in, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    (void)args;
    mp_arg_check_num(n_args, n_kw, 0, 0, false);
    mp_obj_object_t *o = m_new_obj(mp_obj_object_t);
    o->base.type = type_in;
    return o;
}

#if MICROPY_CPYTHON_COMPAT
STATIC mp_obj_t object___init__(mp_obj_t self) {
    (void)self;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(object___init___obj, object___init__);

STATIC mp_obj_t object___new__(mp_obj_t cls) {
    if (!MP_OBJ_IS_TYPE(cls, &mp_type_type) || !mp_obj_is_instance_type((mp_obj_type_t*)cls)) {
        nlr_raise(mp_obj_new_exception_msg(&mp_type_TypeError,
                    "__new__ arg must be a user-type"));
    }
    mp_obj_t o = MP_OBJ_SENTINEL;
    mp_obj_t res = mp_obj_instance_make_new(cls, 1, 0, &o);
    return res;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(object___new___fun_obj, object___new__);
STATIC MP_DEFINE_CONST_STATICMETHOD_OBJ(object___new___obj, (const mp_obj_t)&object___new___fun_obj);

STATIC const mp_map_elem_t object_locals_dict_table[] = {
    #if MICROPY_CPYTHON_COMPAT
    { MP_OBJ_NEW_QSTR(MP_QSTR___init__), (mp_obj_t)&object___init___obj },
    #endif
    #if MICROPY_CPYTHON_COMPAT
    { MP_OBJ_NEW_QSTR(MP_QSTR___new__), (mp_obj_t)&object___new___obj },
    #endif
};

STATIC MP_DEFINE_CONST_DICT(object_locals_dict, object_locals_dict_table);
#endif

const mp_obj_type_t mp_type_object = {
    { &mp_type_type },
    .name = MP_QSTR_object,
    .make_new = object_make_new,
    #if MICROPY_CPYTHON_COMPAT
    .locals_dict = (mp_obj_t)&object_locals_dict,
    #endif
};

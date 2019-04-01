import sys
from ctypes import *

libln = None
IS_PYTHON3 = sys.version_info > (3,)

def is_py3():
    return sys.version_info > (3,)

def init():
    global libln
    if not libln is None:
        return
    libln = cdll.LoadLibrary("liblightnet.so")
    # print("initialize liblightnet.so")

def cleanup():
    global libln
    libln = None

def str2bytes(s):
    if IS_PYTHON3:
        return s.encode('utf8')
    else:
        return s

def str_array(str_list):
    array = (c_char_p * len(str_list))();
    for i in range(len(str_list)):
        array[i] = str2bytes(str_list[i])
    return array

def alloc(init_or_size, size=None):
    return create_string_buffer(init_or_size, size)

def cast_buf(buf, ctype):
    return cast(buf, POINTER(ctype))

def version():
    buf = alloc(20)
    libln.ln_sprint_version(buf)
    if IS_PYTHON3:
        return str(buf.value, encoding = "utf8")
    else:
        return str(buf.value)

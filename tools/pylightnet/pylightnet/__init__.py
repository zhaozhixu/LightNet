import sys
import os

sys.path.append(os.path.split(os.path.realpath(__file__))[0])

import lib
import arch
import context
import msg
import name
import option
import util
import infer

lib.init()

__version__ = lib.version()

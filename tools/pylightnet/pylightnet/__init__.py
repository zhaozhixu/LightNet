import sys
import os

sys.path.append(os.path.split(os.path.realpath(__file__))[0])

import lib
import arch
import context
import msg
import option
import util
from handler import handler

lib.init()
arch.init()

__version__ = option.version()

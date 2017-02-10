#!/usr/bin/env python
"""Basic python Example
The example will make the GPS Module ON.
"""

import os
import sys

if not os.getegid() == 0:
    sys.exit('Script must be run as root')


from time import sleep
from pyA20.gpio import gpio
from pyA20.gpio import port

__author__ = "Sanjeev"
__copyright__ = "Copyright 2017"
__credits__ = ["Sanjeev"]
__license__ = "GPL"
__version__ = "1.0"
__maintainer__ = __author__
__email__ = "sanjeev-dlh.kumar@st.com"


led = port.PA7

gpio.init()
gpio.setcfg(led, gpio.OUTPUT)

gpio.output(led, 1)
sleep(4)
gpio.output(led, 0)
sleep(10)

print ("GPS ON")


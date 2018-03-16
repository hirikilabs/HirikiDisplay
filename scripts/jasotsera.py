#!/usr/bin/env python

import requests
import time
import sys

if len(sys.argv) > 1:
    text = sys.argv[1]
    r = requests.get('http://172.16.30.164/alarm?text=' + str(text) + '&p=20&t=10')

    print(r)


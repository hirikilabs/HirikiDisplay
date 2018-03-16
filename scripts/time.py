#!/usr/bin/env python

import datetime
import requests
import sys

if len(sys.argv) < 2:
    print("IP?")
    exit(1)

date = datetime.datetime.now()

printdate = date.strftime("%H : %M : %S     %d - %m - %Y")

r = requests.get('http://' + sys.argv[1] + '/scroll?text='+printdate+"&speed=20")

print(r)


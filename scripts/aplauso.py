#!/usr/bin/env python

import requests
import sys
import time

if len(sys.argv) < 2:
    print("IP?")
    exit(1)

musica = [50, 261, -100, 261, 261, 293, -100, 261, -300, 329, -100, 349]
largo = len(musica)

request = "http://" + sys.argv[1] + "/center?p=APLAUSO"
r=requests.get(request)
time.sleep(0.5)

request = "http://" + sys.argv[1] + "/music?p=" + str(musica[0])

for i in range(1, largo):
    if musica[i] < 0:
        request = request + "&p=" + str(abs(musica[i]))
    else:
        request = request + "&n" + str(i) + "=" + str(musica[i])

print(request)

r = requests.get(request)

request = "http://" + sys.argv[1] + "/blink"
time.sleep(2)
r = requests.get(request)

print(r)


#!/usr/bin/env python

import requests
import json
import sys
from urllib.parse import quote

if len(sys.argv) < 2:
    print("IP?")
    exit(1)


# get coindesk json
r = requests.get('https://api.coindesk.com/v1/bpi/currentprice.json')
data = json.loads(r.text)


text = quote("BTC - EUR: " + data["bpi"]["EUR"]["rate"])
r = requests.get('http://' + sys.argv[1] + '/scroll?text='+text+"&speed=20")

print(r)


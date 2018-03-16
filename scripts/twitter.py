#!/usr/bin/env python

import requests
import tweepy
from urllib.parse import quote

changes = {
        'á':'a',
        'é':'e',
        'í':'i',
        'ó':'o',
        'ú':'u',
        'ñ':'n',
        'Ñ':'N',
        '"':'\'',
        '…':'...',
        '@':'',
        }

def multipleReplace(text, wordDict):
    for key in wordDict:
        text = text.replace(key, wordDict[key])
    return text

# get hiriki last tweet
auth = tweepy.OAuthHandler("GzSOu9c1JRywGKFBJZ1BwBZVa", 
        "pvWvuHEghyybj5ynLdigNuDV9OC2FfTR7HvKkKupM6xNsNHlKx")
auth.set_access_token("15015560-xkGafnUUctLV2ZZDBCWcb4SwopppCLso92pbdoXBx"
        , "rEqaA12o9DRwGfNSq8jYo2nooWjqFDnfGuG61thniUvtK")

api = tweepy.API(auth)
tweets = api.user_timeline("hirikilabs")

text = quote(multipleReplace(tweets[0].text, changes))
print(text)
r = requests.get('http://172.16.30.164/scroll?text='+text+"&speed=20")

print(r.text)


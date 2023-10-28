# -*- coding: utf-8 -*-

import urllib.parse
import urllib.request
import time

###############################

CoinThingIP = "192.168.178.81"
ColorSet = 1

###############################

print()
print()
if ColorSet != 0:
    print("Set color set to: {0}".format(ColorSet))
    url = 'http://{0}/action/set?colorset={1}'.format(CoinThingIP, ColorSet)
    urllib.request.urlopen(url)
    print("\n>>  Wait for restart, blue screen/open settings screen...")
    print(">>    Enter to proceed...")
    input()

print("Settings test...")
print("  - Get settings")
url = 'http://{0}'.format(CoinThingIP)
urllib.request.urlopen(url)
print("  - Set settings")
url = 'http://{0}/action/set?json='.format(CoinThingIP)
url += urllib.parse.quote('''{"mode":3,"coins":[{"id":"bitcoin","symbol":"BTC","name":"Bitcoin"},{"id":"ethereum","symbol":"ETH","name":"Ethereum"},{"id":"binancecoin","symbol":"BNB","name":"Binance Coin"},{"id":"tether","symbol":"USDT","name":"Tether"},{"id":"cardano","symbol":"ADA","name":"Cardano"},{"id":"ripple","symbol":"XRP","name":"XRP"},{"id":"dogecoin","symbol":"DOGE","name":"Dogecoin"},{"id":"polkadot","symbol":"DOT","name":"Polkadot"},{"id":"usd-coin","symbol":"USDC","name":"USD Coin"},{"id":"bitcoin-cash","symbol":"BCH","name":"Bitcoin Cash"}],"currencies":[{"currency":"EUR","symbol":"€"},{"currency":"USD","symbol":"$"}],"swap_interval":0,"chart_period":4,"chart_style":0,"number_format":1,"heartbeat":true}''')
urllib.request.urlopen(url)
print("\n>>  Let CoinThing run a bit with settings...")
print(">>    Enter to proceed...")
input()

print("Reset for delivery...")
url = "http://{0}/action/reset/all".format(CoinThingIP)
urllib.request.urlopen(url)

print("Yep...")
print()
print()

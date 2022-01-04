# -*- coding: utf-8 -*-

import urllib.parse
import urllib.request


###############################

CoinThingIP = "192.168.178.72"

###############################

print("Start selftest...")
url = "http://{0}/action/selftest".format(CoinThingIP)
urllib.request.urlopen(url)

print("\n>>  Wait for selftest end, blue screen/open settings screen...")
print(">>    Enter to proceed...")
input()

print("Settings test...")
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

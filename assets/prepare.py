import os
import requests
import subprocess
import time
import urllib.parse
import urllib.request


###############################

CoinThingId = 8903790

###############################


profile = """<?xml version="1.0"?>
<WLANProfile xmlns="http://www.microsoft.com/networking/WLAN/profile/v1">
    <name>{0}</name>
    <SSIDConfig>
        <SSID>
            <name>{0}</name>
        </SSID>
    </SSIDConfig>
    <connectionType>ESS</connectionType>
    <connectionMode>auto</connectionMode>
    <MSM>
        <security>
            <authEncryption>
                <authentication>WPA2PSK</authentication>
                <encryption>AES</encryption>
                <useOneX>false</useOneX>
            </authEncryption>
            <sharedKey>
                <keyType>passPhrase</keyType>
                <protected>false</protected>
                <keyMaterial>{1}</keyMaterial>
            </sharedKey>
        </security>
    </MSM>
    <MacRandomization xmlns="http://www.microsoft.com/networking/WLAN/profile/v3">
        <enableRandomization>false</enableRandomization>
    </MacRandomization>
</WLANProfile>
"""

APpassword = "pnvxvyvi"  # version dependent CoinThing AP password
CoinThing = "CoinThing-" + str(CoinThingId)
filename = "cointhing.xml"

f = open(filename, "w")
f.write(profile.format(CoinThing, APpassword))
f.close()

if 1 == 1:
    print("Add WLAN profile: " + CoinThing)

    subprocess.run('netsh wlan add profile filename="' + filename + '"')

    print("Enter to proceed...")
    input()

    print("Connect PC to CoinThing AP...")
    subprocess.run('netsh wlan connect name="' + CoinThing + '"')

    print("Enter to proceed...")
    input()

    os.remove(filename)

    try:
        url = "http://192.168.4.1/wifisave"
        payload = {"s": os.environ['FRITZ_SSID'],
                   "p": os.environ['FRITZ_PWD']}
        header = {"Content-type": "application/x-www-form-urlencoded"}

        print("Connect CoinThing to " + os.environ['FRITZ_SSID'] + "...")
        requests.post(url, data=payload, headers=header)

        print("Enter to proceed...")
        input()

    except:
        pass


print("Connect PC to " + os.environ['FRITZ_SSID'] + "...")
subprocess.run('netsh wlan connect name="' + os.environ['FRITZ_SSID'] + '"')
time.sleep(5)

print("Start selftest...")
url = "http://{0}/action/selftest".format(CoinThing)
urllib.request.urlopen(url)

print("Enter to proceed...")
input()

print("Settings test...")
url = 'http://{0}/action/set?json='.format(CoinThing)
url += urllib.parse.quote('''{"mode":3,"coins":[{"id":"bitcoin","symbol":"BTC","name":"Bitcoin"},{"id":"ethereum","symbol":"ETH","name":"Ethereum"},{"id":"binancecoin","symbol":"BNB","name":"Binance Coin"},{"id":"tether","symbol":"USDT","name":"Tether"},{"id":"cardano","symbol":"ADA","name":"Cardano"},{"id":"ripple","symbol":"XRP","name":"XRP"},{"id":"dogecoin","symbol":"DOGE","name":"Dogecoin"},{"id":"polkadot","symbol":"DOT","name":"Polkadot"},{"id":"usd-coin","symbol":"USDC","name":"USD Coin"},{"id":"bitcoin-cash","symbol":"BCH","name":"Bitcoin Cash"}],"currencies":[{"currency":"EUR","symbol":"€"},{"currency":"USD","symbol":"$"}],"swap_interval":0,"chart_period":4,"chart_style":0,"number_format":1,"heartbeat":true}''')

urllib.request.urlopen(url)

print("Enter to proceed...")
input()

print("Reset for delivery...")
url = "http://{0}/action/reset/all".format(CoinThing)
urllib.request.urlopen(url)

print("Yep...")
print()
print()

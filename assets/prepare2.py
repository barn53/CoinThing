import urllib.parse
import urllib.request
import json
import argparse
import time

settings = {
    "mode":3,
    "coins":[
        {"id":"bitcoin","symbol":"BTC","name":"Bitcoin"},
        {"id":"ethereum","symbol":"ETH","name":"Ethereum"},
        {"id":"binancecoin","symbol":"BNB","name":"Binance Coin"},
        {"id":"tether","symbol":"USDT","name":"Tether"},
        {"id":"cardano","symbol":"ADA","name":"Cardano"},
        {"id":"ripple","symbol":"XRP","name":"XRP"},
        {"id":"dogecoin","symbol":"DOGE","name":"Dogecoin"},
        {"id":"polkadot","symbol":"DOT","name":"Polkadot"},
        {"id":"usd-coin","symbol":"USDC","name":"USD Coin"},
        {"id":"bitcoin-cash","symbol":"BCH","name":"Bitcoin Cash"}],
    "currencies":[
        {"currency":"EUR","symbol":"€"},
        {"currency":"USD","symbol":"$"}],
    "swap_interval":0,
    "chart_period":4,
    "chart_style":0,
    "number_format":1,
    "heartbeat":True}

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--ip", help="The IP address of the CoinThing (e.g. 192.168.178.38)", required=True)
    args = parser.parse_args()
    ip_address = args.ip.replace("/", "")
    ip_address = args.ip.replace(":", "")
    ip_address = args.ip.replace("https", "")
    ip_address = args.ip.replace("http", "")

    try:

        print("\nSettings test...")
        print("  - Get settings HTML")
        url = 'http://{0}'.format(ip_address)
        with urllib.request.urlopen(url) as response:
            pass

        print("  - Set settings")
        url = 'http://{0}/action/set?json='.format(ip_address)
        url += urllib.parse.quote(json.dumps(settings))
        with urllib.request.urlopen(url) as response:
            pass

        print("  - brightness 80")
        url = 'http://{0}/action/set?brightness=80'.format(ip_address)
        with urllib.request.urlopen(url) as response:
            pass

        url = 'http://{0}/settings.json'.format(ip_address)
        with urllib.request.urlopen(url) as response:
            pass

        print("  - brightness 255")
        url = 'http://{0}/action/set?brightness=255'.format(ip_address)
        with urllib.request.urlopen(url) as response:
            pass

        url = 'http://{0}/colorset'.format(ip_address)
        with urllib.request.urlopen(url) as response:
            content = response.read()
            print("\nColorset: " + str(content))

        print("\nLet CoinThing run a bit with settings")
        for _ in range(30):
            time.sleep(1)
            print(".", end='', flush=True)

        print("\n\nReset for delivery...")
        url = "http://{0}/action/reset/all".format(ip_address)
        print (url)
        with urllib.request.urlopen(url) as response:
            pass

        print("\n\n----------------")
        print("--- Success! ---")
        print("----------------")

    except Exception as e:
        print(f"\nException: {str(e)}")

    print()
    print()

if __name__ == "__main__":
    main()

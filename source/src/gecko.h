#pragma once
#include <Arduino.h>

class HttpJson;

struct Coin {
    const char* id;
    const char* symbol;
    const char* name;
};
static const Coin coins[] = {
    { "bitcoin", "BTC", "Bitcoin" },
    { "ethereum", "ETH", "Ethereum" },
    { "tether", "USDT", "Tether" },
    { "ripple", "XRP", "XRP" },
    { "litecoin", "LTC", "Litecoin" },
    { "cardano", "ADA", "Cardano" },
    { "polkadot", "DOT", "Polkadot" },
    { "bitcoin-cash", "BCH", "Bitcoin Cash" },
    { "stellar", "XLM", "Stellar" },
    { "chainlink", "LINK", "Chainlink" },
    { "binancecoin", "BNB", "Binance Coin" },
    { "usd-coin", "USDC", "USD Coin" },
    { "wrapped-bitcoin", "WBTC", "Wrapped Bitcoin" },
    { "bitcoin-cash-sv", "BSV", "Bitcoin SV" },
    { "eos", "EOS", "EOS" },
    { "monero", "XMR", "Monero" },
    { "tron", "TRX", "TRON" },
    { "nem", "XEM", "NEM" },
    { "celsius-degree-token", "CEL", "Celsius Network" },
    { "okb", "OKB", "OKB" },
    { "theta-token", "THETA", "Theta Network" },
    { "tezos", "XTZ", "Tezos" },
    { "vechain", "VET", "VeChain" },
    { "crypto-com-chain", "CRO", "Crypto.com Coin" },
    { "havven", "SNX", "Synthetix Network Token" },
    { "neo", "NEO", "NEO" },
    { "cosmos", "ATOM", "Cosmos" },
    { "aave", "AAVE", "Aave" },
    { "dai", "DAI", "Dai" },
    { "uniswap", "UNI", "Uniswap" },
    { "leo-token", "LEO", "LEO Token" },
    { "dogecoin", "DOGE", "Dogecoin" },
    { "compound-ether", "CETH", "cETH" },
    { "huobi-token", "HT", "Huobi Token" },
    { "binance-usd", "BUSD", "Binance USD" },
    { "iota", "MIOTA", "IOTA" },
    { "cdai", "CDAI", "cDAI" },
    { "yearn-finance", "YFI", "yearn.finance" },
    { "dash", "DASH", "Dash" },
    { "filecoin", "FIL", "Filecoin" }
};

bool isCoin(const char* coin);

class Gecko {

public:
    Gecko(HttpJson& http);

    bool ping() const;
    bool coinPriceChange(const char* coin, const char* currency, double& price, double& price_usd, double& change) const;
    bool coinChart(const char* coin, const char* currency, std::vector<double>& prices, double& max, double& min) const;

    bool coinDetails(const String& coinOrSymbol, String& coin, String& symbol, String& name) const;
    bool isValidCoin(const char* coinOrSymbol) const;
    bool isValidCurrency(const char* currency) const;

private:
    // fallbacks use (slow) API calls
    bool coinDetailsAPI(const char* coin, String& symbol, String& name) const;
    bool isValidCoinAPI(const char* coin) const;

    HttpJson& m_http;
};

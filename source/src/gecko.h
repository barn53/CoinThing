#pragma once
#include "settings.h"
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
    { "polkadot", "DOT", "Polkadot" },
    { "ripple", "XRP", "XRP" },
    { "cardano", "ADA", "Cardano" },
    { "litecoin", "LTC", "Litecoin" },
    { "bitcoin-cash", "BCH", "Bitcoin Cash" },
    { "chainlink", "LINK", "Chainlink" },
    { "stellar", "XLM", "Stellar" },
    { "binancecoin", "BNB", "Binance Coin" },
    { "usd-coin", "USDC", "USD Coin" },
    { "wrapped-bitcoin", "WBTC", "Wrapped Bitcoin" },
    { "bitcoin-cash-sv", "BSV", "Bitcoin SV" },
    { "monero", "XMR", "Monero" },
    { "eos", "EOS", "EOS" },
    { "aave", "AAVE", "Aave" },
    { "tezos", "XTZ", "Tezos" },
    { "havven", "SNX", "Synthetix Network Token" },
    { "nem", "XEM", "NEM" },
    { "tron", "TRX", "TRON" },
    { "cosmos", "ATOM", "Cosmos" },
    { "theta-token", "THETA", "Theta Network" },
    { "uniswap", "UNI", "Uniswap" },
    { "neo", "NEO", "NEO" },
    { "vechain", "VET", "VeChain" },
    { "crypto-com-chain", "CRO", "Crypto.com Coin" },
    { "celsius-degree-token", "CEL", "Celsius Network" },
    { "okb", "OKB", "OKB" },
    { "compound-ether", "CETH", "cETH" },
    { "dai", "DAI", "Dai" },
    { "iota", "MIOTA", "IOTA" },
    { "compound-usd-coin", "CUSDC", "cUSDC" },
    { "leo-token", "LEO", "LEO Token" },
    { "maker", "MKR", "Maker" },
    { "dash", "DASH", "Dash" },
    { "dogecoin", "DOGE", "Dogecoin" },
    { "zcash", "ZEC", "Zcash" },
    { "binance-usd", "BUSD", "Binance USD" },
    { "huobi-token", "HT", "Huobi Token" },
    { "yearn-finance", "YFI", "yearn.finance" },
    { "filecoin", "FIL", "Filecoin" },
    { "solana", "SOL", "Solana" },
    { "ftx-token", "FTT", "FTX Token" },
    { "avalanche-2", "AVAX", "Avalanche" },
    { "ethereum-classic", "ETC", "Ethereum Classic" },
    { "kusama", "KSM", "Kusama" },
    { "compound-governance-token", "COMP", "Compound" },
    { "sushi", "SUSHI", "Sushi" },
    { "zilliqa", "ZIL", "Zilliqa" },
    { "cdai", "CDAI", "cDAI" },
    { "waves", "WAVES", "Waves" },
    { "decred", "DCR", "Decred" },
    { "the-graph", "GRT", "The Graph" },
    { "uma", "UMA", "UMA" },
    { "huobi-btc", "HBTC", "Huobi BTC" },
    { "elrond-erd-2", "EGLD", "Elrond" },
    { "loopring", "LRC", "Loopring" },
    { "near", "NEAR", "Near" },
    { "republic-protocol", "REN", "REN" },
    { "omisego", "OMG", "OMG Network" },
    { "ontology", "ONT", "Ontology" },
    { "paxos-standard", "PAX", "Paxos Standard" },
    { "renbtc", "RENBTC", "renBTC" },
    { "nano", "NANO", "Nano" },
    { "blockstack", "STX", "Blockstack" },
    { "thorchain", "RUNE", "THORChain" },
    { "hedera-hashgraph", "HBAR", "Hedera Hashgraph" },
    { "terra-luna", "LUNA", "Terra" },
    { "algorand", "ALGO", "Algorand" },
    { "0x", "ZRX", "0x" },
    { "icon", "ICX", "ICON" },
    { "reserve-rights-token", "RSR", "Reserve Rights Token" },
    { "basic-attention-token", "BAT", "Basic Attention Token" },
    { "true-usd", "TUSD", "TrueUSD" },
    { "nexo", "NEXO", "NEXO" },
    { "digibyte", "DGB", "DigiByte" },
    { "nxm", "NXM", "Nexus Mutual" },
    { "qtum", "QTUM", "Qtum" },
    { "iostoken", "IOST", "IOST" },
    { "husd", "HUSD", "HUSD" },
    { "aave-link", "ALINK", "Aave LINK" },
    { "enjincoin", "ENJ", "Enjin Coin" },
    { "energy-web-token", "EWT", "Energy Web Token" },
    { "ampleforth", "AMPL", "Ampleforth" },
    { "zencash", "ZEN", "Horizen" },
    { "bitcoin-cash-abc-2", "BCHA", "Bitcoin Cash ABC" },
    { "swissborg", "CHSB", "SwissBorg" },
    { "curve-dao-token", "CRV", "Curve DAO Token" },
    { "kyber-network", "KNC", "Kyber Network" },
    { "balancer", "BAL", "Balancer" },
    { "ocean-protocol", "OCEAN", "Ocean Protocol" },
    { "empty-set-dollar", "ESD", "Empty Set Dollar" },
    { "ethos", "VGX", "Voyager Token" },
    { "bittorrent-2", "BTT", "BitTorrent" },
    { "quant-network", "QNT", "Quant" },
    { "siacoin", "SC", "Siacoin" },
    { "band-protocol", "BAND", "Band Protocol" },
    { "arweave", "AR", "Arweave" },
    { "bitcoin-gold", "BTG", "Bitcoin Gold" }
};

bool isCoin(const char* coin);

class Gecko {

public:
    Gecko(HttpJson& http);

    bool ping() const;
    bool coinPriceChange(const char* coin, const char* currency, double& price, double& price_usd, double& change) const;
    bool coinChart(const char* coin, const char* currency, std::vector<double>& prices, double& max, double& min, Settings::Chart chart) const;

    bool coinDetails(const String& coinOrSymbol, String& coin, String& symbol, String& name) const;
    bool isValidCoin(const char* coinOrSymbol) const;
    bool isValidCurrency(const char* currency) const;

private:
    // fallbacks use (slow) API calls
    bool coinDetailsAPI(const char* coin, String& symbol, String& name) const;
    bool isValidCoinAPI(const char* coin) const;

    HttpJson& m_http;
};

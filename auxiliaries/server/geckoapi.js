var express = require('express')
const https = require('https')
const fs = require('fs')
var cors = require('cors')
const { nextTick } = require('process')
var ip = require("ip");
var app = express()
app.use(cors())

var key = fs.readFileSync(__dirname + '/certificate/selfsigned.key')
var cert = fs.readFileSync(__dirname + '/certificate/selfsigned.crt')
var options = {
    key: key,
    cert: cert
}

function fileToJSON(filename) {
    return JSON.parse(fs.readFileSync(__dirname + filename))
}

var server = https.createServer(options, app)
var port = 3443

var requestCounter = 0
var proCounter = 0
var priceCounter = 0
var chartCounter = 0
var startDate = new Date()
var lastDate = new Date()
var thisDate = new Date()

server.listen(port, function () {
    console.log('Gecko API server address: https://%s:%d', ip.address(), port);
    console.log('Set Gecko Fake API on CoinThing: http://192.168.178.0/action/set?fakegeckoserver=https://%s:%d', ip.address(), port);
})

app.get('/*', function (req, res, next) {

    priceCall = false
    chartCall = false
    proAPI = false

    ++requestCounter
    if (req.path.match(/simple\/price/)) {
        ++priceCounter
        priceCall = true
    } else if (req.path.match(/\/market_chart/)) {
        ++chartCounter
        chartCall = true
    }
    if (req.query['x_cg_pro_api_key']) {
        ++proCounter
        proAPI = true
    }

    lastDate = thisDate
    thisDate = new Date()

    console.log('')
    console.log('')
    console.log('---------------------------------------------------')
    console.log('request: ' + requestCounter + ', price: ' + priceCounter + ', chart: ' + chartCounter)
    seconds = ((thisDate - lastDate) / 1000)
    minutes = ((thisDate - startDate) / 1000 / 60)
    console.log(thisDate.toLocaleTimeString() + ", delta: " + seconds + "s, reqs/min: " + requestCounter / minutes)
    console.log(req.path)
    console.log(JSON.stringify(req.query))
    if (proAPI) {
        console.log('Pro API request key: ' + req.query['x_cg_pro_api_key'])
    }
    // console.log('user-agent: ' + req.get('user-agent'))

    if (false) {
        res.status(404).send('Sorry, cant find that');
        return
    }

    if (proAPI) {
        //if (false) {
        res.status(401)
        console.log('Simulate unauthorized')
        res.send('Unauthorized');
        return
    }

    //if (!proAPI && requestCounter % 10 == 0) {
    //if (requestCounter % 10 == 0) {
    if (!proAPI && priceCall && priceCounter % 5 == 0) {
        //if (!proAPI && chartCall && chartCounter % 5 == 0) {
        //if (false) {
        res.status(429)
        res.header({
            'Retry-After': 120
        })
        console.log('Simulate rate limit exceeded')
        res.send('Rate Limit Exceeded!');
        return
    }

    next()
})


app.get('/api/v3/simple/price', function (req, res) {
    file = '/geckofakedata/price_coin.json'
    if (priceCounter % 2 == 0) {
        file = '/geckofakedata/price_coin2.json'
    }

    priceCoin = fileToJSON(file)
    price = { [req.query.ids]: priceCoin }  // [req.query.ids] notation puts the variable contents as key name
    // console.log('result: ' + JSON.stringify(price))
    res.send(JSON.stringify(price))
})


app.get('/api/v3/coins/(*)/market_chart', function (req, res) {
    file = '/geckofakedata/market_chart.json'
    if (chartCounter % 2 == 0) {
        file = '/geckofakedata/market_chart2.json'
    }

    marketChart = fileToJSON(file)
    // console.log('result: ' + JSON.stringify(marketChart))
    res.send(JSON.stringify(marketChart))
})


app.get('/api/v3/ping', function (req, res) {
    console.log('API ping')
    res.send(JSON.stringify({ "gecko_says": "(V3) To the Moon!" }))
})

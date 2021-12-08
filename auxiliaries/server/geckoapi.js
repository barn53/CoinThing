var express = require('express')
const https = require('https')
const fs = require('fs')
var cors = require('cors')
const { nextTick } = require('process')
var app = express()
app.use(cors())

var key = fs.readFileSync(__dirname + '/certificate/selfsigned.key')
var cert = fs.readFileSync(__dirname + '/certificate/selfsigned.crt')
var options = {
    key: key,
    cert: cert
}

function fileToJSON() {
    return JSON.parse(fs.readFileSync(__dirname + filename))
}


var server = https.createServer(options, app)
var port = 3443

server.listen(port, function () {
    console.log('Gecko API server port: %d', port)
})

app.get('/*', function (req, res, next) {
    console.log('user-agent: ' + req.get('user-agent'))
    next()
})


app.get('/api/v3/simple/price', function (req, res) {
    console.log(req.path)
    // console.log(req.query)
    console.log('coin ids: ' + req.query.ids)

    priceCoin = fileToJSON('/geckofakedata/price_coin.json')
    price = { [req.query.ids]: priceCoin }  // [req.query.ids] notation puts the variable contents as key name
    console.log('result: ' + JSON.stringify(price))
    res.send(JSON.stringify(price))
})


app.get('/api/v3/coins/(*)/market_chart', function (req, res) {
    console.log(req.path)
    console.log(req.query)
    console.log('coin id: ' + req.params[0])

    marketChart = fileToJSON('/geckofakedata/market_chart.json')
    console.log('result: ' + JSON.stringify(marketChart))
    res.send(JSON.stringify(marketChart))
})

app.get('/api/v3/ping', function (req, res) {
    console.log('API ping')
    res.send(JSON.stringify({ "gecko_says": "(V3) To the Moon!" }))
})

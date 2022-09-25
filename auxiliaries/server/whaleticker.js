var express = require('express')
var cors = require('cors')
var app = express()
app.use(cors())

var server = require('http').createServer(app)
var port = 3000

server.listen(port, function () {
    console.log('WhaleTicker server port: %d', port)
    console.log('Serve path: ' + __dirname + '/../../source/data')
})

app.use(express.static(__dirname + '/'))
app.use(express.static(__dirname + '/../../source/data'))
app.use(express.static(__dirname + '/../../source/html'))

app.get('/action/set', function (req, res) {

    for (const [key, value] of Object.entries(req.query)) {
        console.log(`${key}: ${value}`);
    }

    if (req.query.json) {
        res.type("application/json")
        res.send(req.query.json)
    }
    else {
        res.send('1')
    }
})

app.get('/action/get/name', function (req, res) {
    res.send('WhaleTicker Server')
})

app.get('/action/get/version', function (req, res) {
    res.send('v0.0.0')
})

app.get('/action/reset/all', function (req, res) {
    res.send('1')
})

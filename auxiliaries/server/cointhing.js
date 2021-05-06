var express = require('express')
var app = express()
var server = require('http').createServer(app)
var port = 3000

server.listen(port, function () {
    console.log('CoinThing server port: %d', port)
    console.log('Serve path: ' + __dirname + '/../../source/data')
})

app.use(express.static(__dirname + '/../../source/data'))
app.get('/action/set', function (req, res) {

    for (const [key, value] of Object.entries(req.query)) {
        console.log(`${key}: ${value}`);
    }

    res.send('1')
})

var express = require('express')
var cors = require('cors')
var app = express()
app.use(cors())

var server = require('http').createServer(app)
var port = 3333

server.listen(port, function () {
    console.log('Gecko API server port: %d', port)
})

app.get('/', function(req, res) {
    res.send('1')
    console.log(req.get('user-agent'))
})

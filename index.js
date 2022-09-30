var express = require('express')
var app = express()
var server = require("http").Server(app);
var io = require('socket.io')(server);
var mysql = require('mysql2')
var mqtt = require('mqtt')
var exportCharts = require('./public/export.js')


app.use(express.static("public"))
app.set("view engine", "ejs")
app.set("views", "./views")

app.get('/', function (requie, response) {
    response.render("home.ejs");
})
app.get('/none', function (requie, response) {
    response.render("none.ejs");
})
app.get('/devices', function (requie, response) {
    response.render("devices.ejs");
})
app.get('/history', function (requie, response) {
    response.render("history.ejs");
})


server.listen(3000, function () {
    console.log("My project listening on port 3000")
})

var client = mqtt.connect('mqtt://broker.hivemq.com:1883', { clientId: 'clientId-mackcestno1' });
var topic1 = "light";
var topic2 = "pump";
var topic3 = "fan";
var topic4 = "heater";

var topic_main = ["mackcest/data/sensor"];

console.log("connected flag  " + client.connected);
client.on("connect", function () {
    console.log("connected mqtt " + client.connected);
});

client.on("error", function (error) {
    console.log("Can't connect" + error);
    process.exit(1)
});

var con = mysql.createConnection({
    host: 'localhost',
    port: 3306,
    user: 'root',
    password: '12112000',
    database: 'mackcest_database'
});

con.connect(function (err) {
    if (err) throw err;
    console.log("mysql connected");
    var sql = "CREATE TABLE IF NOT EXISTS myhome(ID int(10) not null primary key auto_increment, Time datetime not null, Temperature int(3) not null, Humidity int(3) not null, Light int(5) not null, SoilMisture int(3) not null)"
    con.query(sql, function (err) {
        if (err)
            throw err;
        console.log("Table created");
    });
})
client.subscribe(topic_main);


var humi_graph = [];
var temp_graph = [];
var date_graph = [];
var soil_graph = [];
var m_time
var newTemp
var newHumi
var newLight
var newSoilMisture
// DateTime
var cnt_check = 0;
client.on('message', function (topic, message, packet) {
    const objData = JSON.parse(message)
    if (topic == topic_main) {
        cnt_check = cnt_check + 1
        newTemp = objData.Temperature;
        newHumi = objData.Humidity;
        newLight = objData.Light;
        newSoilMisture = objData.SoilMisture;
    }
    if (cnt_check == 1) {
        cnt_check = 0
        console.log("ready to save")
        var n = new Date()
        var month = n.getMonth() + 1
        var Date_and_Time = n.getFullYear() + "-" + month + "-" + n.getDate() + " " + n.getHours() + ":" + n.getMinutes() + ":" + n.getSeconds();
        var sql = "INSERT INTO myhome (Time, Temperature, Humidity, Light, SoilMisture) VALUES ('" + Date_and_Time.toString() + "', '" + newTemp + "', '" + newHumi + "', '" + newLight + "', '" + newSoilMisture + "')"
        con.query(sql, function (err, result) {
            if (err) throw err;
            console.log("Table inserted");
            // console.log(Date_and_Time + " " + newTemp + " " + newHumi + " " + newLight + " " + newSoilMisture)
        });
        var sql = "DELETE FROM myhome where Temperature > 200 or Humidity > 200; "
        con.query(sql, function (err, result) {
            if (err) throw err;
            console.log("Table fillter");
            // console.log(Date_and_Time + " " + newTemp + " " + newHumi + " " + newLight + " " + newSoilMisture)
        });
        exportCharts(con, io)
    }
})

// Socket Control Devices
io.on('connection', function (socket) {
    console.log("day la socket")
    console.log(socket.id + " connected")
    socket.on('disconnect', function () {
        console.log(socket.id + " disconnected")
    })

    socket.on("server-send-soil_graph", (data) =>{
        console.log(data)
    }
    )

    socket.on("lightChange", function (data) {
        console.log(data)
        if (data == "on") {
            console.log('light ON')
            client.publish(topic1, 'lightOn');
        }
        else {
            console.log('light OFF')
            client.publish(topic1, 'lightOff');
        }
    })

    socket.on("pumpChange", function (data) {
        if (data == "on") {
            console.log('pump ON')
            client.publish(topic2, 'pumpOn');
        }
        else {
            console.log(' pump OFF')
            client.publish(topic2, 'pumpOff');
        }
    })

    socket.on("fanChange", function (data) {
        if (data == "on") {
            console.log('fan ON')
            client.publish(topic3, 'fanOn');
        }
        else {
            console.log('fan OFF')
            client.publish(topic3, 'fanOff');
        }
    })

    socket.on("heaterChange", function (data) {
        if (data == "on") {
            console.log('heater ON')
            client.publish(topic4, 'heaterOn');
        }
        else {
            console.log('heater OFF')
            client.publish(topic4, 'heaterOff');
        }
    })

    // Send data to History page
    var sql1 = "SELECT * FROM myhome ORDER BY ID"
    con.query(sql1, function (err, result, fields) {
        if (err) throw err;
        console.log("Full Data selected");
        var fullData = []
        result.forEach(function (value) {
            var m_time = value.Time.toString().slice(4, 24);
            fullData.push({ id: value.ID, time: m_time, temp: value.Temperature, humi: value.Humidity, light: value.Light, soil: value.SoilMisture })
        })
        io.sockets.emit('send-full', fullData)
    })
})

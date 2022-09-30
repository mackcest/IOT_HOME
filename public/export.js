var humi_graph = [];
var temp_graph = [];
var date_graph = [];
var soil_graph = [];
function exportChart(con, io) {
    var m_time
    var newTemp
    var newHumi
    var newLight
    var newSoil
    var sql1 = "SELECT * FROM myhome ORDER BY ID DESC limit 1"
    con.query(sql1, function (err, result, fields) {
        if (err) throw err;
        // console.log("Data selected");
        result.forEach(function (value) {
            m_time = value.Time.toString().slice(16, 24);
            newTemp = value.Temperature
            newHumi = value.Humidity
            newLight = value.Light
            newSoil = value.SoilMisture
            // console.log(m_time + " " + value.Temperature + " " + value.Humidity + " " + value.Light + " "+ value.SoilMisture);
            io.sockets.emit('server-update-data', { id: value.ID, time: m_time, temp: value.Temperature, humi: value.Humidity, light: value.Light, soil :value.SoilMisture })
        })
        if (soil_graph.length < 10) {
            soil_graph.push(newSoil);
        }
        else {
            for (s = 0; s < 9; s++) {
                soil_graph[s] = soil_graph[s + 1];
            }
            soil_graph[9] = newSoil;
        }

        if (humi_graph.length < 10) {
            humi_graph.push(newHumi);
        }
        else {
            for (i = 0; i < 9; i++) {
                humi_graph[i] = humi_graph[i + 1];
            }
            humi_graph[9] = newHumi;
        }

        if (temp_graph.length < 10) {
            temp_graph.push(newTemp);
        }
        else {
            for (u = 0; u < 9; u++) {
                temp_graph[u] = temp_graph[u + 1];
            }
            temp_graph[9] = newTemp;
        }

        if (date_graph.length < 10) {
            date_graph.push(m_time);
        }
        else {
            for (x = 0; x < 9; x++) {
                date_graph[x] = date_graph[x + 1];
            }
            date_graph[9] = m_time;
        }
        io.sockets.emit("server-send-soil_graph", soil_graph);
        io.sockets.emit("server-send-humi_graph", humi_graph);
        io.sockets.emit("server-send-temp_graph", temp_graph);
        io.sockets.emit("server-send-date_graph", date_graph);
    });
}
module.exports = exportChart
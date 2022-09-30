var socket = io("http://localhost:3000")

socket.on("server-update-data", function (data) {
    // Home page
    $("#currentTemp").html(data.temp)
    $("#currentHumi").html(data.humi)
    $("#currentLight").html(data.light)
    $("#currentSoilMisture").html(data.soil)
    // Warning mode
    var warningSection = document.getElementById("warningSection")
    if (data.temp > 26) {
        warningSection.classList.add("warning-mode-on")
    } else {
        warningSection.classList.remove("warning-mode-on")
    }
    //History page
    $("#id-content").append("<div class='h-para'>" + data.id + "</div>")
    $("#time-content").append("<div class='h-para'>" + data.time + "</div>")
    $("#temp-content").append("<div class='h-para'>" + data.temp + "</div>")
    $("#humi-content").append("<div class='h-para'>" + data.humi + "</div>")
    $("#light-content").append("<div class='h-para'>" + data.light + "</div>")
    $("#soil-content").append("<div class='h-para'>" + data.soil + "</div>")
})

socket.on("send-full", function (data) {
    // History page
    $("#soil-content").html("")
    $("#light-content").html("")
    $("#time-content").html("")
    $("#temp-content").html("")
    $("#humi-content").html("")
    $("#id-content").html("")
    data.forEach(function (item) {
        $("#soil-content").append("<div class='h-para'>" + item.soil + "</div>")
        $("#light-content").append("<div class='h-para'>" + item.light + "</div>")
        $("#time-content").append("<div class='h-para'>" + item.time + "</div>")
        $("#temp-content").append("<div class='h-para'>" + item.temp + "</div>")
        $("#humi-content").append("<div class='h-para'>" + item.humi + "</div>")
        $("#id-content").append("<div class='h-para'>" + item.id + "</div>")
    })
})

// ---- Control devices ----
function light() {
    var checkBox = document.getElementById("light");
    var state11 = document.getElementById("state-btn11")
    if (checkBox.checked == true) {
        socket.emit("lightChange", "on")
        state11.innerHTML = "On";
    } else {
        socket.emit("lightChange", "off")
        state11.innerHTML = "Off";
    }
}


function pump() {
    var checkBox = document.getElementById("pump");
    var state2 = document.getElementById("state-btn2")
    if (checkBox.checked == true) {
        socket.emit("pumpChange", "on")
        state2.innerHTML = "On";
        //tempAlert('LED OFF',1000);
    } else {
        socket.emit("pumpChange", "off")
        state2.innerHTML = "Off";
        //tempAlert('LED OFF',1000);
    }
}

function fan() {
    var checkBox = document.getElementById("fan");
    var state3 = document.getElementById("state-btn3")
    if (checkBox.checked == true) {
        socket.emit("fanChange", "on")
        state3.innerHTML = "On";
    } else {
        socket.emit("fanChange", "off")
        state3.innerHTML = "Off";
    }
}

function heater() {
    var checkBox = document.getElementById("heater");
    var state4 = document.getElementById("state-btn4")
    if (checkBox.checked == true) {
        socket.emit("heaterChange", "on")
        state4.innerHTML = "On";
    } else {
        socket.emit("heaterChange", "off")
        state4.innerHTML = "Off";
    }
}


// ---- RTC ----
function tempAlert(msg, duration) {
    var el = document.createElement("div");
    el.setAttribute("style", "position:absolute ;top:8px; right:16px; background-color: #E4E8F9; color: #707070 ; font-size:24px; padding: 10px;border-radius: 5px; width: 15%; text-align: center;   box-shadow: 0 0rem 2rem #4b565f3f;");
    el.innerHTML = msg;
    setTimeout(function () {
        el.parentNode.removeChild(el);
    }, duration);
    document.body.appendChild(el);
}
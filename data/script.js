// ===== HTML ELEMENTS =====

const connectionStatus = document.getElementById("connectionStatus");
const robotStatus = document.getElementById("robotStatus");
const statusDot = document.querySelector(".status-dot");

const startBtn = document.getElementById("startBtn");
const stopBtn = document.getElementById("stopBtn");
const calibrateBtn = document.getElementById("calibrateBtn");

const applyBtn = document.getElementById("applyBtn");
const applyMessage = document.getElementById("applyMessage");

let calibrationTimer = null;

// ===== CHECK ESP32 CONNECTION =====

async function checkConnection() {
    console.log("Checking ESP32 connection...");

    try {
        const response = await fetch("/status");

        if (!response.ok) {
            throw new Error("ESP32 not responding");
        }

        const status = await response.text();

        if (status.trim() === "connected") {
            connectionStatus.textContent = "ESP32 Connected";

            if (statusDot) {
                statusDot.style.backgroundColor = "#0ca404";
            }
        } else {
            throw new Error("Invalid ESP32 response");
        }

    } catch (error) {
        connectionStatus.textContent = "ESP32 not connected";

        if (statusDot) {
            statusDot.style.backgroundColor = "#ff3333";
        }

        console.error("Connection error:", error);
    }
}

checkConnection();
setInterval(checkConnection, 3000);


// ===== SEND COMMANDS TO ESP32 =====

async function sendCommand(action) {

    try {
        const response = await fetch("/command", {
            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify({
                action: action
            })
        });

        if (!response.ok) {
            throw new Error("Command failed");
        }

        const result = await response.text();

        console.log("ESP32 response:", result);

        // Cancel any previous calibration timer
clearTimeout(calibrationTimer);

if (action === "start") {
    robotStatus.textContent = "RUNNING";
}

else if (action === "stop") {
    robotStatus.textContent = "STOPPED";
}

else if (action === "calibrate") {

    robotStatus.textContent = "CALIBRATING";

    calibrationTimer = setTimeout(function () {
        robotStatus.textContent = "STANDBY";
    }, 10000);
}

    } catch (error) {
        console.error("Command error:", error);
        robotStatus.textContent = "COMMAND FAILED";
    }
}


// ===== BUTTON EVENTS =====

startBtn.addEventListener("click", function () {
    console.log("START CLICKED!");
    robotStatus.textContent = "SENDING START...";
    sendCommand("start");
});
stopBtn.addEventListener("click", function () {
    console.log("STOP CLICKED!");
    sendCommand("stop");
});

calibrateBtn.addEventListener("click", function () {
    console.log("CALIBRATE CLICKED!");
    sendCommand("calibrate");
});


// ===== SPEED SLIDERS =====

const sliders = [
    ["baseSpeed", "baseSpeedValue"],
    ["maxSpeed", "maxSpeedValue"],
    ["turnSpeed", "turnSpeedValue"]
];

sliders.forEach(function (item) {

    const slider = document.getElementById(item[0]);
    const value = document.getElementById(item[1]);

    slider.addEventListener("input", function () {
        value.textContent = slider.value;
    });
});


// ===== APPLY SETTINGS =====

applyBtn.addEventListener("click", async function () {

    // PID values
    const kp = Number(document.getElementById("kp").value);
    const ki = Number(document.getElementById("ki").value);
    const kd = Number(document.getElementById("kd").value);

    // Speed values
    const baseSpeed = Number(document.getElementById("baseSpeed").value);
    const maxSpeed = Number(document.getElementById("maxSpeed").value);
    const turnSpeed = Number(document.getElementById("turnSpeed").value);

    // Validate PID inputs
    const pidInputs = ["kp", "ki", "kd"];

    for (let id of pidInputs) {

        const input = document.getElementById(id);

        if (
            input.value.trim() === "" ||
            isNaN(input.value) ||
            Number(input.value) < 0
        ) {
            applyMessage.textContent = "Invalid PID values!";
            return;
        }
    }

    // Validate speed
    if (baseSpeed > maxSpeed) {
        applyMessage.textContent = "Invalid base speed!";
        return;
    }

    // Create configuration object
    const robotConfig = {
        pid: {
            kp: kp,
            ki: ki,
            kd: kd
        },

        speed: {
            base: baseSpeed,
            max: maxSpeed,
            turn: turnSpeed
        }
    };

    console.log("Robot Configuration:");
    console.log(robotConfig);

    // Send configuration to ESP32
    try {

        const response = await fetch("/settings", {
            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify(robotConfig)
        });

        if (!response.ok) {
            throw new Error("ESP32 rejected settings");
        }

        const result = await response.text();

        // Save configuration locally after ESP32 accepts it
        localStorage.setItem(
            "ja3RobotConfig",
            JSON.stringify(robotConfig)
        );

        applyMessage.textContent = result;

        console.log("Settings sent successfully!");

    } catch (error) {

        console.error("Sending settings failed:", error);

        applyMessage.textContent = "Failed to send settings!";
    }
});


// ===== LOAD SAVED SETTINGS =====

function loadSettings() {

    const savedData = localStorage.getItem("ja3RobotConfig");

    if (!savedData) {
        return;
    }

    const config = JSON.parse(savedData);

    // Restore PID
    document.getElementById("kp").value = config.pid.kp;
    document.getElementById("ki").value = config.pid.ki;
    document.getElementById("kd").value = config.pid.kd;

    // Restore speed
    document.getElementById("baseSpeed").value = config.speed.base;
    document.getElementById("maxSpeed").value = config.speed.max;
    document.getElementById("turnSpeed").value = config.speed.turn;

    // Update slider labels
    document.getElementById("baseSpeedValue").textContent = config.speed.base;
    document.getElementById("maxSpeedValue").textContent = config.speed.max;
    document.getElementById("turnSpeedValue").textContent = config.speed.turn;

    console.log("Saved settings loaded!");
}


// ===== INITIALIZE =====

loadSettings();
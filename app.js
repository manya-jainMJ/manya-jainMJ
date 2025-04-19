// MQTT Configuration
const mqttConfig = {
    host: 'test.mosquitto.org',
    port: 9001,
    protocol: 'ws',
    path: '',
    clientId: 'webClient-' + Math.random().toString(16).substr(2, 8),
    clean: true,
    connectTimeout: 4000,
    reconnectPeriod: 1000,
    username: '',
    password: ''
};

// Connect to test.mosquitto.org MQTT WebSocket
const mqttClient = mqtt.connect(mqttConfig); 

mqttClient.on("connect", () => {
    console.log("✅ Connected to MQTT!");
    mqttClient.subscribe("home/elderly/emergency", (err) => {
        if (!err) {
            console.log("📡 Subscribed to Emergency Topic!");
        }
    });
});

// When an MQTT message is received
mqttClient.on("message", (topic, message) => {
    console.log(`📩 Message Received: ${message.toString()}`);

    if (topic === "home/elderly/emergency") {
        const sosElement = document.getElementById("sos");
        
        if (message.toString() === "SOS" || message.toString() === "Emergency Alert: Help Needed!") {
            sosElement.innerText = "🚨 Emergency! SOS Activated!";
            sosElement.style.color = "red";
            sosElement.style.fontWeight = "bold";
        } else {
            sosElement.innerText = message.toString();
            sosElement.style.color = "";
            sosElement.style.fontWeight = "";
        }
    }
});

// Debugging Errors
mqttClient.on("error", (err) => {
    console.error("❌ MQTT Error: ", err);
});
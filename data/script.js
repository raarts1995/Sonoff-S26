function pageLoad() {
	getMaxInputFields();
	getAllSettings();
	getRelayState();
	getSWVer();
	getSSIDList();
}

//get a list of available SSIDs
function getSSIDList() {
	openNotification("Getting network names...", 0);
	sendRequest("getSSIDList", getSSIDListHandler);
}
function getSSIDListHandler(data) {
	if (data == null) {
		openNotification("No networks found");
		return;
	}
	var ssids = data.split(",");
	openNotification("found " + ssids.length.toString() + " networks");
	var list = document.getElementById("ssid");
	list.innerHTML = "";
	if (ssids.length == 0) {
		list.innerHTML = "<option value='0'>-</option>";
		return;
	}
	for (var i = 0; i < ssids.length; i++) {
		list.innerHTML += "<option value='" + ssids[i] + "'>" + ssids[i] + "</option>";
	}
}

//get and set wifi settings
function setWiFiSettings() {
	var ssid = document.getElementById("ssid").value;
	var pass = document.getElementById("pass").value;
	console.log("ssid: " + ssid + ", pass: " + pass);
	var request = "setWiFiSettings?ssid=" + ssid + "&pass=" + pass;
	sendRequest(request, setWiFiSettingsHandler);
}
function setWiFiSettingsHandler(data) {
	if (data == "ok") {
		openNotification("WiFi settings saved. Restart for the changes to have effect.");
	}
}
function getWiFiSettings() {
	sendRequest("getWiFiSettings", getWiFiSettingsHandler);
}
function getWiFiSettingsHandler(data) {
	document.getElementById("currentSSID").innerHTML = ((data != "") ? data : "No network configured");
}

function getMDNSSettings() {
	sendRequest("getMQTTSettings", getMQTTSettingsHandler);
}
function getMDNSSettingsHandler(data) {
	console.log(data);
	jsonData = JSON.parse(data);
	
	document.getElementById("mdnsEnabled").value = jsonData.mdnsEnabled;
	document.getElementById("mdnsName").value = jsonData.mdnsName;
}
function setMDNSSettings() {
	var mdnsEnabled = document.getElementById("mdnsEnabled").checked;
	var mdnsName = document.getElementById("mdnsName").value;
	console.log("MDNS name: " + mdnsName);
	var request = "setMDNSSettings?enabled=" + (mdnsEnabled ? "1" : "0") + "&name=" + mdnsName;
	sendRequest(request, setMDNSSettingsHandler);
}
function setMDNSSettingsHandler(data) {
	if (data == "ok") {
		openNotification("MDNS settings saved. Restart for the changes to have effect.");
	}
}

function getMQTTSettings() {
	sendRequest("getMQTTSettings", getMQTTSettingsHandler);
}
function getMQTTSettingsHandler(data) {
	console.log(data);
	jsonData = JSON.parse(data);
	console.log(jsonData);
	
	document.getElementById("mqttEnabled").checked = jsonData.mqttEnabled;
	document.getElementById("mqttName").value = jsonData.mqttName;
	document.getElementById("mqttHost").value = jsonData.mqttHost;
	document.getElementById("mqttPort").value = jsonData.mqttPort;
	document.getElementById("mqttTopic").value = jsonData.mqttTopic;
	document.getElementById("mqttReportEnabled").checked = jsonData.mqttReportEnabled;
	document.getElementById("mqttReportTopic").value = jsonData.mqttReportTopic;
}
function setMQTTSettings() {
	var mqttEnabled = document.getElementById("mqttEnabled").checked;
	var mqttName = document.getElementById("mqttName").value;
	var mqttHost = document.getElementById("mqttHost").value;
	var mqttPort = document.getElementById("mqttPort").value;
	var mqttTopic = document.getElementById("mqttTopic").value;
	var mqttReportEnabled = document.getElementById("mqttReportEnabled").checked;
	var mqttReportTopic = document.getElementById("mqttReportTopic").value;
	
	console.log("MQTT host: " + mqttHost + ", port: " + mqttPort);
	console.log("MQTT topic: " + mqttTopic);
	var request = "setMQTTSettings?enabled=" + (mqttEnabled ? "1" : "0") + 
		"&name=" + mqttName + 
		"&host=" + mqttHost + 
		"&port=" + mqttPort + 
		"&topic=" + mqttTopic +
		"&report=" + (mqttReportEnabled ? "1" : "0") + 
		"&reportTopic=" + mqttReportTopic;
	sendRequest(request, setMQTTSettingsHandler);
}
function setMQTTSettingsHandler(data) {
	if (data == "ok") {
		openNotification("MQTT settings saved. Restart for the changes to have effect.");
	}
}

var currentRelayState = false;

function toggleOnOff() {
	sendRequest("toggleRelayState", getRelayStateHandler);
}

function getRelayState() {
	sendRequest("getRelayState", getRelayStateHandler);
}

function getRelayStateHandler(data) {
	currentRelayState = (data == "on");
	updateRelayButton();
}

function setRelayState(state) {
	sendRequest("setRelayState?state=" + (state ? "on" : "off"), setRelayStateHandler);
}

function setRelayStateHandler(data) {
	currentRelayState = (data == "on");
	openNotification("Relay turned " + (currentRelayState ? "on" : "off"));
	updateRelayButton();
}

function updateRelayButton() {
	if (currentRelayState)
		document.getElementById("btnOnOff").classList.add("on");
	else
		document.getElementById("btnOnOff").classList.remove("on");
}

function getAllSettings() {
	sendRequest("getAllSettings", getAllSettingsHandler);
}

function getAllSettingsHandler(data) {
	console.log(data);
	jsonData = JSON.parse(data);
	console.log(jsonData);
	
	document.getElementById("currentSSID").innerHTML = jsonData.ssid;
	
	document.getElementById("mdnsEnabled").checked = jsonData.mdnsEnabled;
	document.getElementById("mdnsName").value = jsonData.mdnsName;
	
	document.getElementById("mqttEnabled").checked = jsonData.mqttEnabled;
	document.getElementById("mqttName").value = jsonData.mqttName;
	document.getElementById("mqttHost").value = jsonData.mqttHost;
	document.getElementById("mqttPort").value = jsonData.mqttPort;
	document.getElementById("mqttTopic").value = jsonData.mqttTopic;
	document.getElementById("mqttReportEnabled").checked = jsonData.mqttReportEnabled;
	document.getElementById("mqttReportTopic").value = jsonData.mqttReportTopic;
	
	enableMDNS();
	enableMQTT();
	mdnsNameChanged();
}

function getMaxInputFields() {
	sendRequest("getMaxInputFields", getMaxInputFieldsHandler);
}

function getMaxInputFieldsHandler(data) {
	console.log(data);
	jsonData = JSON.parse(data);
	console.log(jsonData);
	document.getElementById("pass").maxLength = jsonData.maxPassLength - 1;
	document.getElementById("mdnsName").maxLength = jsonData.maxMdnsLength - 1;
	document.getElementById("mqttName").maxLength = jsonData.maxMqttNameLength - 1;
	document.getElementById("mqttHost").maxLength = jsonData.maxMqttHostLength - 1;
	document.getElementById("mqttTopic").maxLength = jsonData.maxMqttTopicLength - 1;
	document.getElementById("mqttReportTopic").maxLength = jsonData.maxMqttTopicLength - 1;
}

function getSWVer() {
	sendRequest("getSWVer", getSWVerHandler);
}

function getSWVerHandler(data) {
	console.log(data);
	jsonData = JSON.parse(data);
	console.log(jsonData);
	document.getElementById("swVer").innerHTML = jsonData.swVer;
	document.getElementById("compileDate").innerHTML = jsonData.compileDate;
}

function restart() {
	console.log("Restart clicked");
	sendRequest("restart");
}
function restartHandler(data) {
	if (data == "ok")
		openNotification("The device will restart. You will most likely loose connection.");
}

var requestPending = false;

function sendRequest(addr, func = null) {
	/*if (requestPending)
		console.log("Waiting for previous request to complete");
	while (requestPending)
		;*/
	
	console.log("requesting: " + addr);
	requestPending = true;
	var xhr = new XMLHttpRequest();
	xhr.timeout = 5000; //ms
	xhr.open("GET", addr, true);
	xhr.onload = function() {
		console.log("Request finished");
		requestPending = false;
		if (func != null)
			func(this.responseText);
	}
	xhr.onerror = function() {
		console.log("Request failed");
		openNotification("Request failed")
		requestPending = false;
		console.log("error");
	}
	xhr.ontimeout = function() {
		console.log("Request timed out");
		openNotification("Request timed out")
		requestPending = false;
		console.log("timeout");
	}
	xhr.send();
}

//open and close the main menu
function toggleMenu() {
	if (!document.getElementById("content").classList.contains("open"))
		openMenu();
	else
		closeMenu();
}

function openMenu() {
	document.getElementById("content").classList.add("open");
}
function closeMenu() {
	document.getElementById("content").classList.remove("open");
}

//show and hide the notification bar
function openNotification(msg, timeout=2500) {
	document.getElementById("notification").innerHTML = msg;
	document.getElementById("notification").classList.add("open");
	if (timeout != 0)
		setTimeout(closeNotification, timeout);
}
function closeNotification() {
	document.getElementById("notification").classList.remove("open");
}

//show the wifi settings or the weather data
/*function showWiFiSettings() {
	showView("WiFiSettings");
	getWiFiSettings();
	getSSIDList();
}*/
function showView(view) {
	Array.from(document.getElementsByClassName("views")).forEach(function(v) {v.style.display = "none";});
	document.getElementById(view).style.display = "";
	Array.from(document.getElementsByClassName("menuBtn")).forEach(function(b) {b.classList.remove("active")});
	document.getElementById(view + "Btn").classList.add("active");
	closeMenu();
}

function enableMDNS() {
	var state = !document.getElementById("mdnsEnabled").checked;
	document.getElementById("mdnsName").disabled = state;
}
function enableMQTT() {
	var state = !document.getElementById("mqttEnabled").checked;
	document.getElementById("mqttName").disabled = state;
	document.getElementById("mqttHost").disabled = state;
	document.getElementById("mqttPort").disabled = state;
	document.getElementById("mqttTopic").disabled = state;
	document.getElementById("mqttReportEnabled").disabled = state;
	enableReport();
}
function enableReport() {
	var reportState = !document.getElementById("mqttReportEnabled").checked;
	var state = !document.getElementById("mqttEnabled").checked;
	document.getElementById("mqttReportTopic").disabled = reportState || (!reportState && state);
}
function mdnsNameChanged() {
	var mdnsName = document.getElementById("mdnsName").value
	document.getElementById("mdnsNameFull").value = mdnsName + ".local"
}
function mqttPortChanged() {
	document.getElementById("mqttPort").value = document.getElementById("mqttPort").value.replace(/[^0-9.]/g, '');
}

function showHelp() {
	console.log("show help");
	document.getElementById("helpPage").style.display = "block";
}

window.onclick = function(event) {
	if (event.target == document.getElementById("helpPage"))
		hideHelp();
}
function hideHelp() {
	console.log("hide help");
	document.getElementById("helpPage").style.display = "none";
}

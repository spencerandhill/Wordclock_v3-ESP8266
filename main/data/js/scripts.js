//Clock Picker Custom Code
$('.clockpicker').clockpicker({
  placement: 'bottom',
  align: 'left',
  autoclose: true,
  'default': 'now'
});

//Everything else
//var urlBase = "http://192.168.2.102/"; // used when hosting the site somewhere other than the ESP8266 (handy for testing without waiting forever to upload to SPIFFS)
var urlBase = ""; // used when hosting the site on the ESP8266

var brightnessLowTimer = {};
var brightnessHighTimer = {};
var sensorThresholdTimer = {};
var colorTimer = {};

var ignoreColorChange = true;
var inputUserTimeCollapsed = 0;
var confirmResetWiFiCollapsed = 0;

var allData;

$(document).ready( function() {
  $("#inputColor").minicolors(
    {
      theme: "bootstrap",
      changeDelay: 100,
      control: "wheel",
      format: "rgb",
      inline: true
    });
	
  $("#status").html("Connecting...");
  getAll();
});

$("#btnRefresh").click(function() {
  getAll();
});

$("#btnPowerOn").click(function() {
  setPower(1);
});

$("#btnPowerOff").click(function() {
  setPower(0);
});

$("#inputBrightnessLow").on("change mousemove", function() {
   $("#spanBrightnessLow").html($(this).val());
});

$("#inputBrightnessLow").on("change", function() {
   $("#spanBrightnessLow").html($(this).val());
   delaySetBrightnessLow();
});

$("#inputBrightnessHigh").on("change mousemove", function() {
  $("#spanBrightnessHigh").html($(this).val());
});

$("#inputBrightnessHigh").on("change", function() {
  $("#spanBrightnessHigh").html($(this).val());
  delaySetBrightnessHigh();
});

$("#inputSensorThreshold").on("change mousemove", function() {
  $("#spanSensorThreshold").html($(this).val());
});

$("#inputSensorThreshold").on("change", function() {
  $("#spanSensorThreshold").html($(this).val());
  delaySetSensorThreshold();
});

$("#inputTimeServer").change(function() {
   setTimeServer($("#inputTimeServer option:selected").index());
});

$("#inputUserTime").change(function() {
	setUserTime($("#inputUserTime").val());
});

$("#btnManualTime").click(function() {
	if(inputUserTimeCollapsed == 0) {
		inputUserTimeCollapsed = 1;
	}
	else {
		inputUserTimeCollapsed = 0;
	}

	setManual(inputUserTimeCollapsed);
});

$("#btnResetWifi").click(function() {
  if(confirmResetWiFiCollapsed == 0) {
    $("#btnConfirmResetWifi").attr("class", "btn btn-large btn-danger show");
    $("#btnResetWifi").html("Abort");
    confirmResetWiFiCollapsed = 1;
  }
  else {
    confirmResetWiFiCollapsed = 0;
    $("#btnConfirmResetWifi").attr("class", "btn btn-large btn-danger hidden");
    $("#btnResetWifi").html("Reset WiFi");
  }
});

$("#btnConfirmResetWifi").click(function() {
console.log("triggered");
  $.get(urlBase + "resetWifi", null);
  console.log("worked");

  /* function(data) {
    $("#status").attr("class", "alert alert-warning");
    $("#status").html("Wifi resetted");
    console.log("worked");
  });*/
});

$("#inputColor").change(function() {
  if(ignoreColorChange) return;

  var rgb = $("#inputColor").minicolors('rgbObject');
  delaySetColor(rgb);
});

$(".btn-color").click(function() {
  if(ignoreColorChange) return;

  var rgb = $(this).css('backgroundColor');
  var components = rgbToComponents(rgb);
  delaySetColor(components);

  var hexString = rgbToHex(components.r, components.g, components.b);
  ignoreColorChange = true;
  $("#inputColor").minicolors('value', hexString);
  ignoreColorChange = false;
});

function getAll() {
  $.get(urlBase + "all", function(data) {
    allData = data;
	  console.log("allData: " + allData)

    $("#inputBrightnessLow").val(data.brightness);
    $("#spanBrightnessLow").html(data.brightness);
    console.log("Brightness read")

    $("#inputSensorThreshold").val(data.sensorThreshold);
    $("#spanSensorThreshold").html(data.sensorThreshold);
    console.log("Brightness read")

    var hexString = rgbToHex(data.solidColor.r, data.solidColor.g, data.solidColor.b);
    ignoreColorChange = true;
    $("#inputColor").minicolors('value', hexString);
    ignoreColorChange = false;
    console.log("Color read")

    updatePowerButtons(data.power);
    console.log("Power read")
    
    updateManualUserTime(data.manualTime);
    console.log("ManualUserTime read")

    updateUserTime(data.userTime.hours, data.userTime.minutes);
    console.log("UserTime read")
	
    // clear timeServers list
    $("#inputTimeServer").find("option").remove();
	  console.log("timeServers read")
	
    // load timeServers list
    for(var i = 0; i < data.timeServers.length; i++) {
      var timeServer = data.timeServers[i];
      $("#inputTimeServer").append("<option value='" + i + "'>" + timeServer + "</option>");
    }
	  console.log("timeServers read")

    // select the current timeServer
    $("#inputTimeServer").val(data.currentTimeServer.index);
	  console.log("timeServer set")

    $("#status").html("Ready");
  });
}

function updatePowerButtons(value) {
  if(value == 0) {
    $("#btnPowerOn").attr("class", "btn btn-default");
    $("#btnPowerOff").attr("class", "btn btn-primary");
  } else {
    $("#btnPowerOn").attr("class", "btn btn-primary");
    $("#btnPowerOff").attr("class", "btn btn-default");
  }
}

function setPower(value) {
  $.post(urlBase + "power?value=" + value, function(data) {
    updatePowerButtons(data);
    $("#status").html("Set Power: " + data);
  });
}

function delaySetBrightnessLow() {
  clearTimeout(brightnessLowTimer);
  brightnessLowTimer = setTimeout(function() {
    setBrightness($("#inputBrightnessLow").val(), "brightnessLow");
  }, 300);
}

function delaySetBrightnessHigh() {
  clearTimeout(brightnessHighTimer);
  brightnessHighTimer = setTimeout(function() {
    setBrightness($("#inputBrightnessHigh").val(), "brightnessHigh");
  }, 300);
}

function delaySetSensorThreshold() {
  clearTimeout(sensorThresholdTimer);
  sensorThresholdTimer = setTimeout(function() {
    setSensorThreshold($("#inputSensorThreshold").val());
  }, 300);
}

function setBrightness(value, url) {
  $.post(urlBase + url + "?value=" + value, function(data) {
    $("#status").html("Set " + url + ": " + data);
  });
}

function setSensorThreshold(value) {
  $.post(urlBase + "sensorThreshold?value=" + value, function(data) {
    $("#status").html("Set SensorThreshold: " + data);
  });
}

function setTimeServer(value) {
  $.post(urlBase + "timeServer?value=" + value, function(data) {
    $("#status").html("Set TimeServer: " + data.name);
  });
}

function updateManualUserTime(value) {
	inputUserTimeCollapsed = value;

  if(value == 0) {
    $("#inputTimeServer").attr("disabled", false);			//no Manual Time => use NTP-Time
	$("#inputUserTimeGroup").attr("class", "form-group collapse");
    $("#btnManualTime").attr("class", "btn btn-danger");
    $("#btnManualTime").html("Manual");
  } else {
	$("#inputTimeServer").attr("disabled", true);			//no NTP-Time => use Manual Time
	$("#inputUserTimeGroup").attr("class", "form-group collapse in");
	$("#btnManualTime").attr("class", "btn btn-success");
	$("#btnManualTime").html("Automatic");
  }
}

function updateUserTime(hours, minutes) {
    $("#inputUserTime").attr("value", oneLeadingZero(hours) + ":" + oneLeadingZero(minutes));
    $("#timeField").attr("value", oneLeadingZero(hours) + ":" + oneLeadingZero(minutes));
}

function updateSensorValue(sensorValue) {
  $("#currentSensorField").attr("value", sensorValue);
}

function oneLeadingZero(num) {
	if(num < 10) {
		return "0" + num;
	}
	else return num;
}

function setUserTime(value) {
	var splittedHoursAndMinutes = value.split(':'); // split it at the colons
	var hours = splittedHoursAndMinutes[0];
	var minutes = splittedHoursAndMinutes[1];

  $.post(urlBase + "userTime?hours=" + hours + "&minutes=" + minutes + "&manualTime=" + inputUserTimeCollapsed, function(data) {
    $("#status").html("Set Time: " + oneLeadingZero(data.hours) + ":" + oneLeadingZero(data.minutes) + " ManualTime:" + data.manualTime);
  });
}

function setManual(value) {
  $.post(urlBase + "manualTime?value=" + value, function(data) {
    updateManualUserTime(data);
    $("#status").html("Set Manual: " + data);
  });
}

function delaySetColor(value) {
  clearTimeout(colorTimer);
  colorTimer = setTimeout(function() {
    setColor(value);
  }, 60);
}

function setColor(value) {
  $.post(urlBase + "solidColor?r=" + value.r + "&g=" + value.g + "&b=" + value.b, function(data) {
    $("#status").html("Set Color: rgb(" + data.r + ", " + data.g + ", " + data.b + ")");
  });
}

function componentToHex(c) {
    var hex = c.toString(16);
    return hex.length == 1 ? "0" + hex : hex;
}

function rgbToHex(r, g, b) {
    return "#" + componentToHex(r) + componentToHex(g) + componentToHex(b);
}

function rgbToComponents(rgb){
  var components = {};

   rgb = rgb.match(/^rgb\((\d+),\s*(\d+),\s*(\d+)\)$/);
   components.r = parseInt(rgb[1]);
   components.g = parseInt(rgb[2]);
   components.b = parseInt(rgb[3]);

   return components;
}

//check and update the time every second 
setInterval(function(){
  $.get(urlBase + "userTimeAndSensorValue", function(data) {
  console.log("time: " + data.hours + ":" + data.minutes);
  console.log("sensorValue: " + data.sensorValue);
  updateUserTime(data.hours, data.minutes);
  updateSensorValue(data.sensorValue);
  });
}, 800);
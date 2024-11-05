var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function locationSuccess(pos) {
  if (localStorage.getItem("var_mms_server")) {
    var i = (localStorage.getItem("var_mms_server") + '').indexOf("openweathermap", 0);
    if (i === -1 ? false : true) {
      openweathermap(pos);
    }
    else {
      getLocation(pos);
    }
  }
  else {
    getLocation(pos);
  }
}

function getLocation(pos) {
  if (localStorage.getItem("var_mms_custom_location")) {
      yahooweather(localStorage.getItem("var_mms_custom_location"), pos);
  }
  else {
    var url = 'http://nominatim.openstreetmap.org/reverse?format=json&lat=' + pos.coords.latitude + '&lon=' + pos.coords.longitude;
  
    // Send request to YahooWeather
    xhrRequest(url, 'GET', function(responseText) {
      if (typeof(responseText) == "undefined") {
        console.log(new Date() + " - No response from weather server. One more try.");
        openweathermap(pos);
      }
      else {
        // responseText contains a JSON object with weather info
        var json = JSON.parse(responseText);
        yahooweather(json.display_name, pos);
      }
    });
  }
}

function yahooweather(location, pos) {
  console.log("Getting weather from YahooWeather");
  var temperature_format = "c";
  if (localStorage.getItem("var_mms_temperature_scale")) {
    var i = (localStorage.getItem("var_mms_temperature_scale") + '').indexOf("fahrenheit", 0);
    if (i === -1 ? false : true) {
      temperature_format = "f";
    }
  }
  
  // Construct URL
  var url = 'https://query.yahooapis.com/v1/public/yql?format=json&q=' +
            encodeURIComponent('select item.condition, atmosphere, item.lat, item.long, units, location from weather.forecast ') +
            encodeURIComponent('where woeid in (select woeid from geo.places(1) where text="' + location + '") and u="'+temperature_format+'"');
  
  // Send request to YahooWeather
  xhrRequest(url, 'GET', function(responseText) {
    if (typeof(responseText) == "undefined" || responseText.indexOf("failed to connect") >= 0) {
      console.log(new Date() + " - No response from weather server. One more try.");
      openweathermap(pos);
    }
    else {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);
      
      if (typeof(json.query.results) === null || typeof(json.query.results) == "undefined" || typeof(json.query.results.channel) !== 'object' || 3200 == json.query.results.channel.item.condition.code) {
        console.log(new Date() + " - Incorrect response from weather server. One more try.");
        openweathermap(pos);
      }
      else {
        var conditions_id = 0;
        var temperature = "N/A";
        var humidity = "N/A";

        if (json.query.count>0) {
          temperature = json.query.results.channel.item.condition.temp+"°";
          // Conditions
          conditions_id = yahooConditionToOpenWeatherMap(json.query.results.channel.item.condition.code);
          
          humidity = parseInt(json.query.results.channel.atmosphere.humidity);
        }

        // Assemble dictionary using our keys
        var dictionary = {
          "TEMPERATURE": temperature,
          "CONDITIONS": conditions_id,
          "HUMIDITY": humidity,
        };

        // Send to Pebble
        Pebble.sendAppMessage(
          dictionary,
          function(e) {
            console.log("Weather info sent to Pebble successfully!");
          },
          function(e) {
            console.log("Error sending weather info to Pebble!");
          }
        );
      }
    }
  });
}

function openweathermap(pos) {
  console.log("Getting weather from OpenWeatherMap");
  var apikey = localStorage.getItem("var_mms_openweathermap_api_key");
  if (apikey && apikey != "undefined" && '' !== apikey) {
    openweathermapWithKey(pos, apikey);
  }
  else {
    var url = "http://pebble.itigor.com/make-me-smile/tools.php?get_openweathermap_apikey";

    xhrRequest(url, 'GET', function(responseText) {
      var apikey = false;
      // responseText contains a JSON object
      var json = JSON.parse(responseText);

      if (json.apikey) {
        apikey = json.apikey;
      }
      
      openweathermapWithKey(pos, apikey);
    });
  }
}
    
function openweathermapWithKey(pos, apikey) {
  var temperature_format = "metric";
  if (localStorage.getItem("var_mms_temperature_scale")) {
    var i = (localStorage.getItem("var_mms_temperature_scale") + '').indexOf("fahrenheit", 0);
    if (i === -1 ? false : true) {
      temperature_format = "imperial";
    }
  }

  // Construct URL
  var url = "",
      urlMain = "http://api.openweathermap.org/data/2.5/";
  if (localStorage.getItem("var_mms_custom_location")) {
    url = urlMain+"weather?q=" + encodeURIComponent(localStorage.getItem("var_mms_custom_location"));
  }
  else {
    url = urlMain+"weather?lat=" + pos.coords.latitude + "&lon=" + pos.coords.longitude;
  }
  url += '&units='+temperature_format;

  if (apikey) {
    url += '&APPID='+apikey;
  }

  // Send request to OpenWeatherMap
  xhrRequest(url, 'GET', function(responseText) {
    if (typeof(responseText) == "undefined" || responseText.indexOf("failed to connect") >= 0) {
      console.log(new Date() + " - No response from weather server. One more try.");
      getLocation(pos);
    }
    else {
      // responseText contains a JSON object with weather info
      var json = JSON.parse(responseText);

      var conditions_id = 0;
      var temperature = "N/A";
      var humidity = "N/A";

      if (parseInt(json.cod) != 404) {
        temperature = Math.round(json.main.temp)+"°";
        humidity = Math.round(json.main.humidity);

        // Conditions
        conditions_id = getConditionsId(json.weather[0].icon, json.weather[0].id);
      }

      // Assemble dictionary using our keys
      var dictionary = {
        "TEMPERATURE": temperature,
        "CONDITIONS": conditions_id,
        "HUMIDITY": humidity,
      };

      // Send to Pebble
      Pebble.sendAppMessage(
        dictionary,
        function(e) {
          console.log("Weather info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending weather info to Pebble!");
        }
      );
    }
  });
}

function getConditionsId(icon, conditions_id) {
  if (!icon.localeCompare("01d") || conditions_id == 904) {
    conditions_id = 1010;
  }
  else if (!icon.localeCompare("01n")) {
    conditions_id = 1020;
  }
  else if (!icon.localeCompare("02d")) {
    conditions_id = 1110;
  }
  else if (!icon.localeCompare("02n")) {
    conditions_id = 1120;
  }
  
  return conditions_id;
}

function yahooConditionToOpenWeatherMap(code) {
  var returnCode = 0;
  if (code === 0) { //Tornado
    returnCode = 900;
  }
  else if (code == 1) { //tropical storm
    returnCode = 901;
  }
  else if (code == 2) { //Hurricane
    returnCode = 902;
  }
  else if (code == 3) { //severe thunderstorms
    returnCode = 212;
  }
  else if (code == 4) { //Thunderstorm
    returnCode = 211;
  }
  else if (code >= 5 && code <= 7) { //mixed rain and snow
    returnCode = 616;
  }
  else if (code >= 8 && code <= 9) { //Drizzle
    returnCode = 300;
  }
  else if ((code >= 10 && code <= 12) || code == 40) { //Rain
    returnCode = 313;
  }
  else if ((code >= 13 && code <= 16) || (code >= 41 && code <= 43) || code == 46) { //Snow
    returnCode = 601;
  }
  else if (code == 17) { //Hail
    returnCode = 906;
  }
  else if (code == 18) { //sleet
    returnCode = 611;
  }
  else if (code == 19) { //dust
    returnCode = 761;
  }
  else if (code == 20) { //foggy
    returnCode = 741;
  }
  else if (code == 21) { //haze
    returnCode = 721;
  }
  else if (code == 22) { //smoky
    returnCode = 711;
  }
  else if (code >= 23 && code <= 24) { //windy
    returnCode = 905;
  }
  else if (code == 25) { //cold
    returnCode = 903;
  }
  else if ((code >= 26 && code <= 28) || code == 44) { //cloudy
    returnCode = 802;
  }
  else if (code == 29) { //partly cloudy (night)
    returnCode = 1120;
  }
  else if (code == 30) { //partly cloudy (day)
    returnCode = 1110;
  }
  else if (code == 31 || code == 33) { //clear (night)
    returnCode = 1020;
  }
  else if (code == 32 || code == 34) { //clear (day)
    returnCode = 1010;
  }
  else if (code == 35) { //mixed rain and hail
    returnCode = 201;
  }
  else if (code == 36) { //hot
    returnCode = 904;
  }
  else if ((code >= 37 && code <= 39) || code == 45 || code == 47) { //thunderstorms
    returnCode = 201;
  }
  
  return returnCode;
}

function locationError(err) {
  console.log("Error requesting location!");
  console.warn('location error: ' + err.code + ' - ' + err.message);
}

function getWeather() {
  if (localStorage.getItem("var_mms_custom_location")) {
    locationSuccess(false);
  }
  else {
    navigator.geolocation.getCurrentPosition(
      locationSuccess,
      locationError,
      {timeout: 15000, maximumAge: 2100000}
    );
  }
}

function getPlatform() {
  var platform = 'aplite';
  if(Pebble.getActiveWatchInfo) {
    // Available for use!
    var watch = Pebble.getActiveWatchInfo();
    platform = watch.platform;
  }
  
  return platform;
}

// Listen for when the watchface is opened
Pebble.addEventListener('ready', function(e) {
  console.log("PebbleKit JS ready!");

  // Get the initial weather
  getWeather();
});

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage', function(e) {
  console.log("AppMessage received!");
  getWeather();
});

Pebble.addEventListener('showConfiguration', function(e) {
  var platform = getPlatform();
  
  // Show config page
  var location = localStorage.getItem("var_mms_custom_location") ? localStorage.getItem("var_mms_custom_location") : "";
  var temperature_scale = localStorage.getItem("var_mms_temperature_scale") ? localStorage.getItem("var_mms_temperature_scale") : "";
  var bluetooth = localStorage.getItem("var_mms_bluetooth") ? localStorage.getItem("var_mms_bluetooth") : "";
  var battery = localStorage.getItem("var_mms_battery") ? localStorage.getItem("var_mms_battery") : "";
  var server = localStorage.getItem("var_mms_server") ? localStorage.getItem("var_mms_server") : "";
  var screen_color = localStorage.getItem("var_mms_screen_color_"+platform) ? localStorage.getItem("var_mms_screen_color_"+platform) : "";
  var hourly_beep = localStorage.getItem("var_mms_hourly_beep") ? localStorage.getItem("var_mms_hourly_beep") : "";
  var dnd = localStorage.getItem("var_mms_dnd") ? localStorage.getItem("var_mms_dnd") : "";
  var dnd_start = localStorage.getItem("var_mms_dnd_start") ? localStorage.getItem("var_mms_dnd_start") : "";
  var dnd_end = localStorage.getItem("var_mms_dnd_end") ? localStorage.getItem("var_mms_dnd_end") : "";
  var hour_lead_zero = localStorage.getItem("var_mms_hour_lead_zero") ? localStorage.getItem("var_mms_hour_lead_zero") : "";
  var day_of_week_humidity = localStorage.getItem("var_mms_day_of_week_humidity") ? localStorage.getItem("var_mms_day_of_week_humidity") : "";
  var openweathermap_api_key = localStorage.getItem("var_mms_openweathermap_api_key") ? localStorage.getItem("var_mms_openweathermap_api_key") : "";

  Pebble.openURL('http://pebble.itigor.com/make-me-smile/configurations.php?version=3.0' + //appinfo.versionCode +
                 '&platform=' + encodeURIComponent(platform) +
                 '&location=' + encodeURIComponent(location) +
                 '&temperature_scale=' + encodeURIComponent(temperature_scale) +
                 '&bluetooth=' + encodeURIComponent(bluetooth) +
                 '&battery=' + encodeURIComponent(battery) +
                 '&server=' + encodeURIComponent(server) +
                 '&screen_color=' + encodeURIComponent(screen_color) +
                 '&hourly_beep=' + encodeURIComponent(hourly_beep) +
                 '&dnd=' + encodeURIComponent(dnd) +
                 '&dnd_start=' + encodeURIComponent(dnd_start) +
                 '&dnd_end=' + encodeURIComponent(dnd_end) +
                 '&hour_lead_zero=' + encodeURIComponent(hour_lead_zero) +
                 '&day_of_week_humidity=' + encodeURIComponent(day_of_week_humidity) +
                 '&openweathermap_api_key=' + encodeURIComponent(openweathermap_api_key));
});

Pebble.addEventListener('webviewclosed', function(e) {
  if (e.response) {
    var platform = getPlatform();
    var configurations = JSON.parse(decodeURIComponent(e.response));

    localStorage.setItem("var_mms_custom_location", configurations.location);
    localStorage.setItem("var_mms_temperature_scale", configurations.temperature_scale);
    localStorage.setItem("var_mms_bluetooth", configurations.bluetooth);
    localStorage.setItem("var_mms_battery", configurations.battery);
    localStorage.setItem("var_mms_server", configurations.server);
    localStorage.setItem("var_mms_screen_color_"+platform, configurations.screen_color);
    localStorage.setItem("var_mms_hourly_beep", configurations.hourly_beep);
    localStorage.setItem("var_mms_dnd", configurations.dnd);
    localStorage.setItem("var_mms_dnd_start", configurations.dnd_start);
    localStorage.setItem("var_mms_dnd_end", configurations.dnd_end);
    localStorage.setItem("var_mms_hour_lead_zero", configurations.hour_lead_zero);
    localStorage.setItem("var_mms_day_of_week_humidity", configurations.day_of_week_humidity);
    localStorage.setItem("var_mms_openweathermap_api_key", configurations.openweathermap_api_key);

    var settings = {
      "BLUETOOTH"      : configurations.bluetooth,
      "BATTERY"        : (configurations.battery.localeCompare("battery_always") === 0) ? true : false,
      "SCREEN_COLOR"   : configurations.screen_color,
      "HOURLY_BEEP"    : (configurations.hourly_beep.localeCompare("beep") === 0) ? true : false,
      "DND"            : (configurations.dnd.localeCompare("dnd_on") === 0) ? true : false,
      "HOUR_LEAD_ZERO" : (configurations.hour_lead_zero.localeCompare("show_zero_in_hours") === 0) ? true : false,
      "SHOW_HUMIDITY"  : (configurations.day_of_week_humidity.localeCompare("show_humidity") === 0) ? true : false,
    };

    var dnd_start = parseInt(configurations.dnd_start);
    var dnd_end = parseInt(configurations.dnd_end);
    if (!configurations.dnd.localeCompare("dnd_on")) {
      if (isNaN(dnd_start) || isNaN(dnd_end)) {
        dnd_start = 0;
        dnd_end = 0;
      }
      settings.DND_START = dnd_start;
      settings.DND_END = dnd_end;
    }

    Pebble.sendAppMessage(
      settings,
      function(e) {
        console.log("Configurations sent to Pebble successfully!");
      },
      function(e) {
        console.log("Error sending configurations to Pebble!");
      }
    );
  }
  else {
    console.log('Configuration canceled');
  }
});
/*************************** Weather library start ****************************/

var owmWeatherAPIKey = '';

function owmWeatherXHR(url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
}


function owmWeatherSendToPebble(json) {
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyReply': 1,
    'OWMWeatherAppMessageKeyWind': json.wind.speed,
    'OWMWeatherAppMessageKeyIcon': json.weather[0].icon,
    'OWMWeatherAppMessageKeyHum': json.weather[0].humidity,
    'OWMWeatherAppMessageKeyDescriptionShort': json.weather[0].main,
    'OWMWeatherAppMessageKeyTempK': json.main.temp_max,
    'OWMWeatherAppMessageKeyName': json.name,
    'OWMWeatherAppMessageKeySunrise': json.sys.sunrise,
    'OWMWeatherAppMessageKeySunset': json.sys.sunset
  });
}

function owmWeatherLocationSuccess(pos) {
  var url = 'http://api.openweathermap.org/data/2.5/weather?lat=' + 
    pos.coords.latitude + '&lon=' + pos.coords.longitude + '&appid=' + owmWeatherAPIKey;
  //console.log('owm-weather: Location success. Contacting OpenWeatherMap.org...');
  //console.log(url);

  owmWeatherXHR(url, 'GET', function(responseText) {
    //console.log('owm-weather: Got API response!');
    if(responseText.length > 100) {
      owmWeatherSendToPebble(JSON.parse(responseText));
    } else {
      //console.log('owm-weather: API response was bad. Wrong API key?');
      Pebble.sendAppMessage({
        'OWMWeatherAppMessageKeyBadKey': 1
      });
    }
  });
}

function owmWeatherLocationError(err) {
  console.log('owm-weather: Location error');
  Pebble.sendAppMessage({
    'OWMWeatherAppMessageKeyLocationUnavailable': 1
  });
}

function owmWeatherHandler(dict) {
  if(dict.payload.OWMWeatherAppMessageKeyRequest) {
    owmWeatherAPIKey = dict.payload.OWMWeatherAppMessageKeyRequest;
    //console.log('owm-weather: Got fetch request from C app');
    
    navigator.geolocation.getCurrentPosition(owmWeatherLocationSuccess, owmWeatherLocationError, {
      timeout: 15000,
      maximumAge: 60000
    });
  }
}

/**************************** Weather library end *****************************/

Pebble.addEventListener('ready', function (e) {
  console.log('connect!' + e.ready);
});

Pebble.addEventListener('appmessage', function(e) {
  //console.log('appmessage: ' + JSON.stringify(e.payload));
  owmWeatherHandler(e);
});

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://pebble.nikolai-rinas.de/pebbleConfig.html';

  Pebble.openURL(url);
});

Pebble.addEventListener('webviewclosed', function(e) {
  // Decode the user's preferences
  var configData = JSON.parse(decodeURIComponent(e.response));

  //console.log('Configuration page returned: ' + JSON.stringify(configData));

  // Send to the watchapp via AppMessage
  var dict = {
    'DIGIMEMessageKeySelTime': configData.select_time_format,
    'DIGIMEMessageKeySelTemp': configData.select_temp_format,
    'DIGIMEMessageKeySelAPIKEY': configData.set_api_key,
    'DIGIMEMessageKeySelLan': configData.select_language
  };
  
  // Send to the watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Config data sent successfully!');
  }, function(e) {
    console.log('Error sending config data!');
  });
});




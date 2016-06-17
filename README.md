# DigiMe
Another nice pebble watchface

![DigiMe-Sunshine](https://raw.githubusercontent.com/workinghard/DigiMe/master/img/DIGIMI-Sunshine.png)
![DigiMe-Moonshine](https://raw.githubusercontent.com/workinghard/DigiMe/master/img/DIGIMI-Moonshine2.png)

## My Intention
I wanted to have one code base for a pebble watch face which is easy to modify and open source. This is a collection of most of the pebble developer tutorials. It includes:
 * Get weather data from [OpenWeatherMap](http://openweathermap.org)
 * Display battery status
 * Use of a font (for the moon phase)
 * Draw graphics (sun shine)
 * And of course diplay current time/date
  
I also thought about make some things configurable (like time format or temperature unit). But currently i don't like how it is implemented in pebble. You need to host an external website for this. I would need to create a dependency what i currently don't want. Perhaps later ... If you would like to have something different, feel free to post a feature request or just clone this project ...

This watchface is also designed to be responsive and drain not too much battery. Even with so many features my battery lasts for 5 days. As usual, it's highly usage dependant. 

All the features are separated into different source files. It should be easy to understand and modify it to your own needs and create your unique watch face.

### Version 1.3 update
It's now possible to configure temperature unit and provide your own API key. Time format is the same like you have on your phone.

### Version 1.4 update
You can now decide in which language the date should be displayed.
Currently supported: 
 * English
 * Spanish
 * German
Send me translations for more, if you like ...


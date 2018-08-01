                   .:                     :,                                          
,:::::::: ::`      :::                   :::                                          
,:::::::: ::`      :::                   :::                                          
.,,:::,,, ::`.:,   ... .. .:,     .:. ..`... ..`   ..   .:,    .. ::  .::,     .:,`   
   ,::    :::::::  ::, :::::::  `:::::::.,:: :::  ::: .::::::  ::::: ::::::  .::::::  
   ,::    :::::::: ::, :::::::: ::::::::.,:: :::  ::: :::,:::, ::::: ::::::, :::::::: 
   ,::    :::  ::: ::, :::  :::`::.  :::.,::  ::,`::`:::   ::: :::  `::,`   :::   ::: 
   ,::    ::.  ::: ::, ::`  :::.::    ::.,::  :::::: ::::::::: ::`   :::::: ::::::::: 
   ,::    ::.  ::: ::, ::`  :::.::    ::.,::  .::::: ::::::::: ::`    ::::::::::::::: 
   ,::    ::.  ::: ::, ::`  ::: ::: `:::.,::   ::::  :::`  ,,, ::`  .::  :::.::.  ,,, 
   ,::    ::.  ::: ::, ::`  ::: ::::::::.,::   ::::   :::::::` ::`   ::::::: :::::::. 
   ,::    ::.  ::: ::, ::`  :::  :::::::`,::    ::.    :::::`  ::`   ::::::   :::::.  
                                ::,  ,::                               ``             
                                ::::::::                                              
                                 ::::::                                               
                                  `,,`


https://www.thingiverse.com/thing:2821592
Luftdaten Stevensson screen by Naesstrom is licensed under the Creative Commons - Attribution license.
http://creativecommons.org/licenses/by/3.0/

# Summary

#Luftdata?
I'm part of a Swedish Citizen science projekt [luftdata.se](http://www.luftdata.se) measuring 
air particles using a sds011 sensor. It's a branch from the original german projekt [luftdaten.info](http://www.luftdaten.info) where you can read build instructions etc in English.

__(You can of course also use the model to measure to other things then particle measurements, I've printed one identical where I have a BME280, BH1750 and a external tipping bucket for rain thus making my own weather station!)__

You can see a live map of all the particle readings [here](http://deutschland.maps.luftdaten.info/#4/51.67/20.23)
<img src="https://i.imgur.com/gyZbaXX.png" /> <br />
The original case is made of 2 pieces 87° sewer pipes and that's a cheap and sturdy solution but I wanted something that looked a bit better on my wall so I designed this stevensson screen sensor holder. It fits the sds011 sensor and a DHT22/BME280 besides the mandatory nodeMCU/WemosD1

# Stevensson what?
A Stevenson screen or instrument shelter is a shelter or an enclosure to shield meteorological instruments against precipitation and direct heat radiation from outside sources, while still allowing air to circulate freely around them. You have probably seen some around in the world without thinking about it but it was first designed at the end of 1880!
<img src="https://cdn.shopify.com/s/files/1/1256/7113/products/M11_2.jpg?v=1492110317" />
# Parts
<img src="https://i.imgur.com/FupjeDf.jpg" />
It's enough with 2 bolts to keep it together but if you want to be on the really super safe side it's designed to use 4 bolts.
I recommend that you get stainless bolts and nuts since it'll be placed outside.
## Printed parts
- 6x middle part
- 1x wall bottom part
- 1x top part
- 1x sds011 holder

## Bought parts
- 2-4x M8x100mm bolts
- 2-4x M8 nuts
- 2x wood screws

# Print Settings

Printer: RepRapPro Mono Mendel
Rafts: No
Supports: No
Resolution: 0.2

Notes: 
I used a rather high percentage (60%) for the bottom part since it needs to carry the rest of the parts and keeping it stable.
The other parts are just printed with 25% infill since that's as low as I usually go.

Don't use any support, if you turn all the parts in the right direction it won't be needed.

# Post-Printing

## Step 1: Clean and Count

Clean all parts from any stringing and make sure you have printed the right number of all the parts.

## Step 2: Build the sensor

Ah yeah, you need a sensor to so head over to [luftdata.se](http://luftdata.se/bygg/) for the Swedish instructions or [luftdaten.info](https://luftdaten.info/en/construction-manual/) for the English instructions.
They also have instructions in a couple of other languages on their site.

## Step 3: Attach sensors to the sds011 holder

Use cable ties to attach the sds011, DHT22/BME280 and NodeMCU/Wemos to the inside of the mesh part. Try to get the ties situated in 4 different corners for making the insertion easier.

## Step 4: Fit the nuts to the top part

My suggstion is to heat up the nuts some and melt them down into the correct places in the top part making sure to get them as straight as possible.

## Step 5: Attach the bottom part to a wall

Use 2 wood screws (max 3,5mm diameter) to fasten the bottom part to a wall, remember that you need to have access to a USB cable to power the unit.

## Step 6: Stack the parts

This part can be a bit tricky so you might want to get help from someone to hand you the pieces.
Start with one bolt from the bottom and while holding it with one hand thread the other _6 middle parts_ onto it. After that insert the mesh holder for the sensor and finally put on the top part and attach the bolt to the nut.
Before you tighten it up to much insert the other bolt(s) and make sure that you manage to line up the holes correctly.
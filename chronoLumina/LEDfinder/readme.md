# Lumina Selector
A device for publishing messages via MQTT to Chrono Lumina

The final prototype uses the code in the folder [LEDfinder-esp32c3](LEDfinder-esp32c3/). The other folders are either code to run on a MKR1010 or test units of the code to get individual components working.

## Overview

The aim of this project was to build a controller that would allow me select 1 of 50 Chrono Lumina NeoPixel LED rings and send a message over MQTT to flash the lights white. My goal was to create an easy way to identify an LED ring by its device number.

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/53949301905/in/album-72177720320549464" title="Lumina"><img src="https://live.staticflickr.com/65535/53949301905_bea9aff795_c.jpg" width="100%" alt="Lumina"/></a>

The project started with the decision to use a rotary encoder to make an intuitive controller for selecting between 1 and 50 devices. I chose one with a built in push button but also looked at using a seperate button controller. The third component selected was a small OLED display to show which number was currently selected.  

Given these three constraints, and after some prototyping to make sure the parts would work together, I started to sketch out some ideas for how someone would hold and use the device.

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/53949107393/in/album-72177720320549464" title="Lumina Sketches"><img src="https://live.staticflickr.com/65535/53949107393_312de9388c_c.jpg" width="100%" alt="Lumina Sketches"/></a>

This informed a prototype one sketch of the circuit needed.

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/53949106988/in/album-72177720320549464" title="Lumina circuit"><img src="https://live.staticflickr.com/65535/53949106988_4eb3781fb6_c.jpg" width="100%" alt="Lumina circuit"/></a>

Which then allowed me to start iterating through designs in Fusion 360 for an enclosure. After 4 iterations and some trial and error prints the device started to look functional.

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/53973103941/in/album-72177720320549464" title="Lumina4"><img src="https://live.staticflickr.com/65535/53973103941_b22819c122_c.jpg" width="100%" alt="Lumina4"/></a>

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/53973531075/in/album-72177720320549464" title="Trying to 3D print"><img src="https://live.staticflickr.com/65535/53973531075_52dcaf569b_c.jpg" width="100%" alt="Trying to 3D print"/></a>

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/54016480715/in/album-72177720320549464" title="flashing all lights"><img src="https://live.staticflickr.com/31337/54016480715_1586592ee5_c.jpg" width="100%" alt="flashing all lights"/></a>

A video of the final operational prototype is on flickr:

<a data-flickr-embed="true" href="https://www.flickr.com/photos/pseudonomad/54031574406/in/album-72177720321469614" title="Lumina Selector - Final Prototype"><img src="https://live.staticflickr.com/31337/54031574406_09f7017604_c.jpg" width="800" height="450" alt="Lumina Selector - Final Prototype"/></a>

Development photos are on: [Flickr in an Album called Lumina Selector](https://www.flickr.com/photos/pseudonomad/albums/72177720321469614/)
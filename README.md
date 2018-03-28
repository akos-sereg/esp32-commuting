# ESP 32 Commuting

The purpose of this esp 32 microcontroller based device is to give me information quickly (by pushing one button only) about the next tram's arrival that stops in front of my house. It is handy at winter time if I do not want to wait long minutes outside at the tram stop until I freeze.

An obvious alternative of this purpose is checking the next tram's arrival on my smartphone but that's too mainstream. Also, doing such IoT thing with ESP 32 is more fun!

### How it works

1. Switch on with the toggle button
2. After esp32 boots up, it automatically connects to my home WiFi
3. Then it loads a pre-configured REST service - hosted by me - that returns a number: how many seconds do I have until the next tram arrives.
4. Device converts it to minutes and displays it on 2x7 segment display
5. Device continues polling the REST endpoint in every 10 seconds and refreshes the 2x7 segment display

### Device Setup

Ingredients:
- 15 x 110 Ohm resistors
- Toggle button (ON - OFF)
- ESP 32 module
- 2x7 segment display: i am using DA08-11EWA

![Circuit](https://github.com/akos-sereg/esp32-commuting/blob/master/doc/circuit.png?raw=true)

it looks like this: 

![Board](https://github.com/akos-sereg/esp32-commuting/blob/master/doc/board.jpg?raw=true)


### Configuration

In order to make this work for you, you will need to 
1. Host a REST service that connects to your local commuting service (in my example it's "BKK futar" in Budapest) and responds a number: how many seconds left until bus/tram/train/etc arrives. Once you have this REST service deployed, make sure you updated esp32/main/main.c WEB_SERVER, WEB_PORT and WEB_URL parameters.
2. Configure ESP 32 WiFi under esp32/main/main.c EXAMPLE_WIFI_SSID and EXAMPLE_WIFI_PASS - yes, this comes from an esp32-idf example :)

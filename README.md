# wakaama-mbed-application-board
This is an example of [Wakaama](https://github.com/eclipse/wakaama/) on [mbed LPC1768](https://developer.mbed.org/platforms/mbed-LPC1768/#features) with [application board](https://developer.mbed.org/components/mbed-Application-Board/).

![alt text][lpc1768] ![alt text][app_board]

[lpc1768]: https://developer.mbed.org/media/cache/platforms/lpc1768_3.jpg.250x250_q85.jpg "mbed LPC1768"
[app_board]: https://developer.mbed.org/media/uploads/chris/xapp_board_front_small_map1.png.pagespeed.ic.YiEqKsL82J.jpg "mbed application board"

Wakaama is used to make the mbed LPC1768, [lightweight M2M](http://technical.openmobilealliance.org/Technical/technical-information/release-program/current-releases/oma-lightweightm2m-v1-0) capable.

RGB Led, accelerometer and temperature sensor is mapped on [IPSO Smart Objects](http://technical.openmobilealliance.org/Technical/technical-information/omna/lightweight-m2m-lwm2m-object-registry).


# Features
The LCD screen shows times and temperature.
You can use joystick to switch on/off the light (left:on, right:off)

* You can write/read/observe time via _Lwm2m Device object_ (3).
* You can switch on/off the light (write/read/observe) or choose the color (read/write) via _Light Control object_ (3311).
* You can read/observe temperature via _Temperature Sensor object_ (3303).
* You can read/observe 1-3 axis position via _Accelerometer Sensor object_ (3313).

# Compile and try it
To compile it you need the [ARM GNU toolschains](https://launchpad.net/gcc-arm-embedded), 
GCC ARM Embedded can be installed freely using instructions found [here](http://gnuarmeclipse.livius.net/blog/toolchain-install/).

Then run the classic :
```
make
```
by default the device will try to connect to the [leshan](https://github.com/eclipse/leshan) [sandbox](http://leshan.eclipse.org/#/clients) with the client endpont name `lcp1768`
You could change the endpoint name and the server target with `make` variables :
```
make clean
make ENDPOINT_NAME=mydevice SERVER_URI=coap://10.0.0.1:5683
```

To compile in debug mode and get some logs on [serial port](https://developer.mbed.org/handbook/SerialPC#host-interface-and-terminal-application) : 
```
make clean
make DEBUG=1
```

You could also change ethernet timeout for each loop with `LOOP_TIMEOUT`, the default is 1000ms :
```
make clean
make LOOP_TIMEOUT=200
```
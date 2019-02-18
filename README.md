SPI-Py: Hardware SPI as a C Extension for Python
======

COPYRIGHT (C) 2012 Louis Thiery. All rights reserved. Further work by Connor Wolf.

Forked in 2019 by Nathan Leefer to fix memory handling in the C extension.

This program is free software; you can redistribute it and/or modify it under the terms of the GNU General Public License V2 as published by the Free Software Foundation.

LIABILITY  
This program is distributed for educational purposes only and is no way suitable for any particular application, especially commercial. There is no implied suitability so use at your own risk!

## Instructions

1. Clone or download this repository, navigate to the SPI-Py directory, and install the library using the following command. Use python3 if installing for Python 3.
<pre>
> sudo python setup.py install
</pre>

2. Make sure the SPI interface is enabled for your Raspberry Pi. This can be done using the raspi-config utility.
<pre>
> sudo raspi-config
</pre>

3. This module provides three functions for communicating with SPI devices:
<pre>
dev_dictionary = spi.openSPI(kwargs)
data_in = spi.transfer(dev_dictionary, data_out)
spi.closeSPI(dev_dictionary)
</pre>

The next section covers these in detail.

## Example usage

The below commands can be found in the [test_script.py](test_script.py) file.

### After installing the library, import the spi module to your Python code via :
<pre>
import spi
</pre>

### Open the file descriptor to the SPI device with one of two chip selects:
<pre>
device_0 = spi.openSPI(device="/dev/spidev0.0",mode=0,speed=500000,bits=8,delay=0)
</pre>
The device keyword can be either "/dev/spidev0.0" or "/dev/spidev0.1". The difference refers to which chip select pin is used by the SPI device driver. The mode keyword can be 0,1,2, or 3, and many SPI devices can operate up to 8000000 Hz speed, however it is recommended to check your data sheet. See the [Raspberry Pi docs](https://www.raspberrypi.org/documentation/hardware/raspberrypi/spi/README.md) for a detailed explanation of these and other parameters.

### Use the returned device handle to conduct an SPI transaction
<pre>
data_out = (0xFF,0x00,0xFF)
data_in = (0x00, 0x00, 0x00)
data_in = spi.transfer(device_0, data_out)
</pre>

The above would write the 3 bytes contained in data_out and copy the received data to data_in. Note that data_in will always be a tuple the same length as data_out, and will simply reflect the state of the MISO pin throughout the transaction. It is up to the user to understand the device behavior connected to the SPI pins.

To verify that this works connect GPIO 10 (MOSI, physical pin 19) to GPIO 9 (MISO, physical pin 21) in a loop back. You should see that data_out now equals data_in.

### Close the file descriptor for your SPI device
<pre>
spi.closeSPI(device_0)
</pre>

## Memory leak

The [memory_leak.py](/memory_leak.py) script continuously executes a simple transaction on /dev/spidev0.0. There does not appear to be a memory leak in this use case.

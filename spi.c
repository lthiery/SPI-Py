/* SPI testing utility (see copyright beow)
 *	adapted for use in Python
 * by Louis Thiery
 *
 * compile for Python using: "python setup.py build"
 * compiled module will be in "./build/lib.linux-armv6l-2.7/spi.so" 
 *
 * SPI testing utility (using spidev driver)
 *
 * Copyright (c) 2007  MontaVista Software, Inc.
 * Copyright (c) 2007  Anton Vorontsov <avorontsov@ru.mvista.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License.
 *
 */

#include <Python.h>
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

static void pabort(const char *s)
{
	perror(s);
	abort();
}

static const char *device = "/dev/spidev0.0";
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 500000;
static uint16_t delay;
static uint32_t bytesPerMessage=8;

int ret = 0;
int fd;

static PyObject* initialize(PyObject* self, PyObject* args)
{

	/*want to overload initialize call so that we can do:
		initialize()
		initialize("mode-string")
		initialize(bits,speed))
		initialize( ("mode-string"),bits,speed,delay)
	 Tried or'ing all of these but it didn't work:
		char *modeString;
		if(!PyArg_ParseTuple(args,"s",&modeString))
		if(!PyArg_ParseTuple(args,"ii",&bits,&speed))
		if(!PyArg_ParseTuple(args, "siii", &modeString, &bits, &speed, &delay))
	*/
	//this works at least but I don't know what the different modes are by integer

	if( !(PyArg_ParseTuple(args,"") || PyArg_ParseTuple(args,"iiii", &mode, &bytesPerMessage, &speed, &delay)) )
		return NULL;
	PyErr_Clear();
	/*
	uint8_t i;
	for(i=0;i<sizeof(modeString) / sizeof(char);i++){
		switch(modeString[i]){
			case 'l':
				mode |= SPI_LOOP;
				break;
			case 'H':
				mode |= SPI_CPHA;
				break;
			case 'O':
				mode |= SPI_CPOL;
				break;
			case 'L':
				mode |= SPI_LSB_FIRST;
				break;
			case 'C':
				mode |= SPI_CS_HIGH;
				break;
			case '3':
				mode |= SPI_3WIRE;
				break;
			case 'N':
				mode |= SPI_NO_CS;
				break;
			case 'R':
				mode |= SPI_READY;
				break;
			default:
				break;
		}
	}
	*/
	fd = open(device, O_RDWR);
        if (fd < 0)
                pabort("can't open device");
	/*
         * spi mode
         */
        ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
        if (ret == -1)
                pabort("can't set spi mode");

        ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
        if (ret == -1)
                pabort("can't get spi mode");

        /*
         * bits per word
         */
        ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
        if (ret == -1)
                pabort("can't set bits per word");

        ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
        if (ret == -1)
                pabort("can't get bits per word");

        /*
         * max speed hz
         */
        ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
        if (ret == -1)
                pabort("can't set max speed hz");

        ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
        if (ret == -1)
                pabort("can't get max speed hz");

        printf("spi mode: %d\n", mode);
        printf("bits per word: %d\n", bits);
        printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);
	printf("delay %d\n",delay);

	Py_RETURN_NONE;
}



static PyObject* transfer(PyObject* self, PyObject* args)
{
	PyObject* pyObj;

	if(!PyArg_ParseTuple(args, "O", &pyObj))
		return NULL;

	uint8_t tupleSize = PyTuple_Size(pyObj);

	uint8_t tx[bytesPerMessage];
	uint8_t rx[bytesPerMessage];
	PyObject* item;

	uint8_t i=0;

	while(i<bytesPerMessage){
		if(i<tupleSize){
			item = PyTuple_GetItem(pyObj, i);
			if(!PyInt_Check(item)){
				printf("non-integer contained in tuple");
				exit(1);
			}
			tx[i] = PyFloat_AsDouble(item);
		}
		else
			tx[i] = 0;

		printf("%d ", tx[i++]);
	}
	puts("");

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = bytesPerMessage,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1)
		pabort("can't send spi message");

/*
	printf("Received: \n");
	for (ret = 0; ret < ARRAY_SIZE; ret++) {
		if (!(ret % 4) && ret!=0)
			puts("");
		printf("%.2X ", rx[ret]);
	}
	puts("");
*/

	pyObj = PyTuple_New(bytesPerMessage);
	for(i=0;i<bytesPerMessage;i++)
		PyTuple_SetItem(pyObj, i, Py_BuildValue("i",rx[i]));

	return pyObj;
}


static PyObject* end(PyObject* self,PyObject* args)
{
	close(fd);
	Py_RETURN_NONE;
}

static PyMethodDef SpiMethods[] =
{
	{"initialize", initialize, METH_VARARGS, "Initializing"},
	{"transfer", transfer, METH_VARARGS, "Sending."},
	{"end", end, METH_NOARGS, "End."},
	{NULL, NULL, 0, NULL}
};

PyMODINIT_FUNC

initspi(void)
{
     (void) Py_InitModule("spi", SpiMethods);
}

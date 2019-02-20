/* SPI testing utility (see copyright beow)
 *	adapted for use in Python
 * 	by Louis Thiery
 * 	Lots more flexibility and cleanup by Connor Wolf (imaginaryindustries.com)
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

static PyObject* openSPI(PyObject *self, PyObject *args, PyObject *kwargs) {

    // Default parameters
    char *device = "/dev/spidev0.0";
    uint8_t mode=0;
    uint8_t bits = 8;
    uint32_t speed = 500000;
    uint16_t delay=0;

    int ret = 0;
    int fd;

    static char* kwlist[] = {"device", "mode", "bits", "speed", "delay", NULL};

    // Adding some sort of mode parsing would probably be a nice idea for the future, so you don't have to specify it as a bitfield
    // stuffed into an int.
    // For the moment the default mode ("0"), will probably work for 99% of people who need a SPI interface, so I'm not working on that
    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "|siiii:keywords", kwlist, &device, &mode, &bits, &speed, &delay)) {
        return NULL;
    }

    //
    // Parse mode of SPI device, option 0, 1, 2, 3
    //
    uint8_t spi_mode;
    switch(mode) {
        case 0:
            spi_mode = SPI_MODE_0;
            break;
        case 1:
            spi_mode = SPI_MODE_1;
            break;
        case 2:
            spi_mode = SPI_MODE_2;
            break;
        case 3:
            spi_mode = SPI_MODE_3;
            break;
        default:
            spi_mode = SPI_MODE_0;
    }

    // It's not clearly documented, but it seems that PyArg_ParseTupleAndKeywords basically only modifies the values passed to it if the
    // keyword pertaining to that value is passed to the function. As such, the defaults specified by the variable definition are used
    // unless you pass a kwd argument.
    // Note that there isn't any proper bounds-checking, so if you pass a value that exceeds the variable size, it's just truncated before
    // being stuffed into  the avasilable space. For example, passing a bits-per-word of 500 gets truncated to 244. Unfortunately, the
    // PyArg_ParseTupleAndKeywords function only seems to support ints of 32 bits.

    PyErr_Clear();

    // printf("Mode: %i, Bits: %i, Speed: %i, Delay: %i\n", mode, bits, speed, delay);

    fd = open(device, O_RDWR);
    if (fd < 0) {
        pabort("can't open device");
    }

    // Set mode of SPI device
    ret = ioctl(fd, SPI_IOC_WR_MODE, &spi_mode);
    if (ret == -1) {
        pabort("can't set spi mode");
    }

    ret = ioctl(fd, SPI_IOC_RD_MODE, &spi_mode);
    if (ret == -1) {
        pabort("can't get spi mode");
    }

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

    // Stuff the various initilization parameters into a dict, and return that.
    // Note that the returned values may not be completely real. It seems that, at least for the speed value,
    // the hardware only has several possible settings (250000, 500000, 1000000, etc...) Strangely enough, the
    // ioctl for setting the speed *returns the speed you specify*. However, the hardware seems to default to the
    // closest avalable value *below* the specified rate. (i.e. you will never get a speed faster then you spec),
    // but you may get a slower value.

    //It would probably be a good idea to bin-down the passed arguement to the available values, and return
    // that.

#if PY_MAJOR_VERSION >= 3
    PyObject* key_mode = PyBytes_FromString("mode");
    PyObject* key_bits = PyBytes_FromString("bits");
    PyObject* key_speed = PyBytes_FromString("speed");
    PyObject* key_delay = PyBytes_FromString("delay");
    PyObject* key_fd = PyBytes_FromString("fd");

    PyObject* val_mode = PyLong_FromLong((long)spi_mode);
    PyObject* val_bits = PyLong_FromLong((long)bits);
    PyObject* val_speed = PyLong_FromLong((long)speed);
    PyObject* val_delay = PyLong_FromLong((long)delay);
    PyObject* val_fd = PyLong_FromLong((long)fd);
#else
    PyObject* key_mode = PyString_FromString("mode");
    PyObject* key_bits = PyString_FromString("bits");
    PyObject* key_speed = PyString_FromString("speed");
    PyObject* key_delay = PyString_FromString("delay");
    PyObject* key_fd = PyString_FromString("fd");

    PyObject* val_mode = PyInt_FromLong((long)spi_mode);
    PyObject* val_bits = PyInt_FromLong((long)bits);
    PyObject* val_speed = PyInt_FromLong((long)speed);
    PyObject* val_delay = PyInt_FromLong((long)delay);
    PyObject* val_fd = PyInt_FromLong((long)fd);
#endif

    PyObject* retDict;
    retDict = PyDict_New();

    PyDict_SetItem(retDict, key_mode, val_mode);
    PyDict_SetItem(retDict, key_bits, val_bits);
    PyDict_SetItem(retDict, key_speed, val_speed);
    PyDict_SetItem(retDict, key_delay, val_delay);
    PyDict_SetItem(retDict, key_fd, val_fd);

    Py_XDECREF(key_mode);Py_XDECREF(val_mode);
    Py_XDECREF(key_bits);Py_XDECREF(val_bits);
    Py_XDECREF(key_speed);Py_XDECREF(val_speed);
    Py_XDECREF(key_delay);Py_XDECREF(val_delay);
    Py_XDECREF(key_fd);Py_XDECREF(val_fd);

    return retDict;
}



static PyObject* transfer(PyObject* self, PyObject* args) {

    uint8_t bits = 8;
    uint32_t speed = 500000;
    uint16_t delay;

    int ret = 0;
    int fd;
    
    PyObject* dict;
    PyObject* transferTuple;

    // "O" - Gets non-NULL borrowed reference to Python argument.
    // As far as I can tell, it's mostly just copying arg[0] into transferTuple
    // and making sure at least one arg has been passed (I think)
    if(!PyArg_ParseTuple(args, "OO", &dict, &transferTuple)) {		
        return NULL;												
    }									

    // Check that dictionary was parsed correctly
    if(!PyDict_Check(dict)) {
        pabort("First argument must be a valid dictionary.");
    }
    // Check that transfer tuple was parsed correctly.
    if(!PyTuple_Check(transferTuple)) {			
        pabort("Second argument must be a tuple of bytes.\n");
    }
   
   // Declare these variable separately so we can manually decrease reference
   // counts when finished and free up memory
#if PY_MAJOR_VERSION >= 3
    PyObject* p_bits = PyBytes_FromString("bits");
    PyObject* p_speed = PyBytes_FromString("speed");
    PyObject* p_delay = PyBytes_FromString("delay");
    PyObject* p_fd = PyBytes_FromString("fd");
#else
    PyObject* p_bits = PyString_FromString("bits");
    PyObject* p_speed = PyString_FromString("speed");
    PyObject* p_delay = PyString_FromString("delay");
    PyObject* p_fd = PyString_FromString("fd");
#endif

    // Get SPI device parameters to use
#if PY_MAJOR_VERSION >= 3
    bits = (uint8_t) PyLong_AsUnsignedLong(PyDict_GetItem( dict, p_bits ));
    speed = (uint32_t) PyLong_AsUnsignedLong(PyDict_GetItem( dict, p_speed ));
    delay = (uint16_t) PyLong_AsUnsignedLong(PyDict_GetItem( dict, p_delay ));
    fd = (int) PyLong_AsLong(PyDict_GetItem( dict, p_fd ));
#else
    bits = (uint8_t) PyInt_AsUnsignedLongMask(PyDict_GetItem( dict, p_bits ));
    speed = (uint32_t) PyInt_AsUnsignedLongMask(PyDict_GetItem( dict, p_speed ));
    delay = (uint16_t) PyInt_AsUnsignedLongMask(PyDict_GetItem( dict, p_delay ));
    fd = (int) PyInt_AsLong(PyDict_GetItem( dict, p_fd ));
#endif

    // Decrease reference counts for tempoerary variables, freeing memory
    Py_XDECREF(p_bits);
    Py_XDECREF(p_speed);
    Py_XDECREF(p_delay);
    Py_XDECREF(p_fd);


//	 printf("Mode: %i, Bits: %i, Speed: %i, Delay: %i\n", mode, bits, speed, delay);
    uint32_t tupleSize = PyTuple_Size(transferTuple);

    uint8_t tx[tupleSize];
    uint8_t rx[tupleSize];
    PyObject* tempItem;

    uint16_t i=0;

    while(i < tupleSize) {
        tempItem = PyTuple_GetItem(transferTuple, i);		//
#if PY_MAJOR_VERSION >= 3
        if(!PyLong_Check(tempItem)) {
            pabort("non-integer contained in tuple\n");
        }
#else
        if(!PyInt_Check(tempItem)) {
            pabort("non-integer contained in tuple\n");
        }
#endif

#if PY_MAJOR_VERSION >= 3
        tx[i] = (uint8_t)PyLong_AsSsize_t(tempItem);
#else
        tx[i] = (uint8_t)PyInt_AsSsize_t(tempItem);
#endif

        i++;
    }

    struct spi_ioc_transfer tr = {
        .tx_buf = (unsigned long)tx,
        .rx_buf = (unsigned long)rx,
        .len = tupleSize,
        .delay_usecs = delay,
        .speed_hz = speed,
        .bits_per_word = bits,
                .cs_change = 0,
    };

    ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
    if (ret < 1) {
        pabort("can't send spi message");
    }

    // This explains why the input data are not overwritten by the received data
    // If you delete this line you could have the function overwrite the input
    // data with the received data. (Works in Python 3, not Python 2)
    transferTuple = PyTuple_New(tupleSize);
    for(i=0;i<tupleSize;i++) {
        if(PyTuple_SetItem(transferTuple, i, Py_BuildValue("i",rx[i]))) {
            printf("spi: can't set value in data tuple.\n");
        }
    }

    return transferTuple;
}


static PyObject* closeSPI(PyObject* self,PyObject* args) {
    PyObject* dict;

    if(!PyArg_ParseTuple(args, "O", &dict)) {		// "O" - Gets non-NULL borrowed reference to Python argument.
        return NULL;					// As far as I can tell, it's mostly just copying arg[0] into transferTuple
    }

#if PY_MAJOR_VERSION >= 3
    PyObject* p_fd = PyBytes_FromString("fd"); 
#else
    PyObject* p_fd = PyString_FromString("fd"); 
#endif

#if PY_MAJOR_VERSION >= 3
    int fd = (int) PyLong_AsLong(PyDict_GetItem( dict, p_fd ));
#else
    int fd = (int) PyInt_AsLong(PyDict_GetItem( dict, p_fd ));
#endif
    
    close(fd);
    Py_XDECREF(p_fd);
    Py_RETURN_NONE;
}

static PyMethodDef SpiMethods[] =
{
    {"openSPI", (PyCFunction)openSPI, METH_VARARGS | METH_KEYWORDS, "Open SPI Port."},
    {"transfer", (PyCFunction)transfer, METH_VARARGS, "Transfer data."},
    {"closeSPI", (PyCFunction)closeSPI, METH_VARARGS, "Close SPI port."},
    {NULL, NULL, 0, NULL}
};

#if PY_MAJOR_VERSION >= 3
    static struct PyModuleDef moduledef = {
        PyModuleDef_HEAD_INIT,
        "spi",     /* m_name */
        "spi library",  /* m_doc */
        -1,                  /* m_size */
        SpiMethods,    /* m_methods */
        NULL,                /* m_reload */
        NULL,                /* m_traverse */
        NULL,                /* m_clear */
        NULL,                /* m_free */
    };
#endif

PyMODINIT_FUNC

#if PY_MAJOR_VERSION >= 3
PyInit_spi(void)
#else
initspi(void)
#endif
{
#if PY_MAJOR_VERSION >= 3
        PyObject *module = PyModule_Create(&moduledef);
#else
    (void) Py_InitModule("spi", SpiMethods);
#endif

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}

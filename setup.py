from distutils.core import setup, Extension

module1 = Extension('spi', sources = ['spi.c'])

setup (
    name = 'SPI-Py',
    author='Louis Thiery',
    url='https://github.com/lthiery/SPI-Py',
    download_url='https://github.com/lthiery/SPI-Py/archive/master.zip',
    version = '1.0',
    description = 'SPI-Py: Hardware SPI as a C Extension for Python',
    license='GPL-v2',
    platforms=['Linux'],
    ext_modules = [module1]
)

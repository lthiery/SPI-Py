import spi

def main():
    # Open file descriptor for 
    # spi device 0 using the CE0 pin for chip select
    device_0 =  spi.openSPI(device="/dev/spidev0.0",
                            mode=0,
                            speed=1000000)

    # Open file descriptor for
    # spi device 0 using the CE1 pin for chip select
    device_1 = spi.openSPI(device="/dev/spidev0.1",
                        mode=0,
                        speed=1000000)

    # Transact data
    data_out = (0xFF, 0x00, 0xFF)

    # This is not necessary, not just demonstrate loop-back
    data_in = (0x00, 0x00, 0x00)
    data_in = spi.transfer(device_0, data_out)
    print("Received from device 0:")
    print(data_in)

    data_in = (0x00, 0x00, 0x00)
    data_in = spi.transfer(device_1, data_out)
    print("Received from device 1:")
    print(data_in)

    # Close file descriptors
    spi.closeSPI(device_0)
    spi.closeSPI(device_1)


if __name__ == "__main__":
    main()
    
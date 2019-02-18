import spi

def transact():

	data_out = (0xFF,0x00,0xFA)
	# Open file descriptor for 
	# spi device 0 using the CE0 pin for chip select
	device_0 =  spi.openSPI(device="/dev/spidev0.0",
							mode=0,
							speed=1000000)
							
	print("**")
	print(device_0)

    # This is not necessary, not just demonstrate loop-back
	data_in = (0x00, 0x00, 0x00)
	data_in = spi.transfer(device_0, data_out)
	print("Received from device 0:")
	print(data_in)

	spi.closeSPI(device_0)
	print(device_0)
	print("**")


def main():

	g_run = True

	while g_run:
		try:
			transact()

		except KeyboardInterrupt:
			g_run = False

if __name__ == "__main__":
	main()

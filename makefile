first: start.c
	cl -c start.c user32.lib Ws2_32.lib
	cl -c network.c user32.lib Ws2_32.lib
	cl -o start.exe start.obj network.obj user32.lib Ws2_32.lib
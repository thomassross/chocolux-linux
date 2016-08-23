all:
	g++ chocolux_linux.cpp -o chocolux_linux `pkg-config --cflags --libs x11 gl` -s -Os

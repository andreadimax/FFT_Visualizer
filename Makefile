compile:
	g++ ./src/main.cpp -o audio -lportaudio -L/usr/local/lib/libportaudio.a -pthread

rm:
	rm -rf audio
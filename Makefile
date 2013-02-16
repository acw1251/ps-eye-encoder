CC = g++
CFLAGS = -I/usr/include/opencv2
LIBLINKS = -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy

all: encoder

encoder: encoder.o arduinoUart.o
	$(CC) encoder.o arduinoUart.o $(LIBLINKS) -o encoder

encoder.o: encoder.cpp
	$(CC) $(CFLAGS) -c encoder.cpp -o encoder.o

arduinoUart.o: arduinoUart.cpp
	$(CC) -c arduinoUart.cpp -o arduinoUart.o


hsvGUI: hsvGUI.cpp
	$(CC) $(CFLAGS) hsvGUI.cpp $(LIBLINKS) -o hsvGUI

andyGUI: andyGUI.cpp
	$(CC) $(CFLAGS) andyGUI.cpp $(LIBLINKS) -o andyGUI


clean:
	rm -rf encoder
	rm -rf *.o

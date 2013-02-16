CC = g++
CFLAGS = -I/usr/include/opencv2
LIBLINKS = -lopencv_core -lopencv_imgproc -lopencv_calib3d -lopencv_video -lopencv_features2d -lopencv_ml -lopencv_highgui -lopencv_objdetect -lopencv_contrib -lopencv_legacy

all: hsvGUI hsvMatchGUI

hsvGUI: hsvGUI.cpp
	$(CC) $(CFLAGS) hsvGUI.cpp $(LIBLINKS) -o hsvGUI

hsvMatchGUI: hsvMatchGUI.cpp
	$(CC) $(CFLAGS) hsvMatchGUI.cpp $(LIBLINKS) -o hsvMatchGUI

clean:
	rm -rf hsvGUI hsvMatchGUI

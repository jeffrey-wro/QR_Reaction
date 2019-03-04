#include <vector>
#include <iostream>

#include <stdio.h>

#include "MyRio.h"
#include "I2C.h"
#include "Motor_Controller.h"
#include "Utils.h"

#include "opencv2/core.hpp"
#include "opencv2/opencv.hpp"
#include "opencv2/objdetect.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/operations.hpp"

#include "ImageSender.h"

using namespace std;
using namespace cv;

#define ENABLE_SERVER 1

extern NiFpga_Session myrio_session;
NiFpga_Status status;

int main(int argc, char **argv) {

	VideoCapture cam(0); // open the default camera
	if (!cam.isOpened())  // check if we succeeded
		return -1;

	status = MyRio_Open();
	if (MyRio_IsNotSuccess(status)) {
		return status;
	}

	MyRio_I2c i2c;
	status = Utils::setupI2CB(&myrio_session, &i2c);

	Motor_Controller mc = Motor_Controller(&i2c);
	mc.controllerEnable(DC);

	int volt = mc.readBatteryVoltage(1);
	printf("%d\n\n", volt);

	string data;
    Mat display, frame, pts;
	
	for(;;)
    {
		cam >> frame; //get new frame
		cvtColor(frame, display, COLOR_BGR2GRAY);
		
		QRCodeDetector qrDecoder = QRCodeDetector();
		if(qrDecoder.detect(display, pts))
		{
			//set data to decoded qr code
			data = qrDecoder.detectAndDecode(display, pts);
			
			if(data == "Ver1")
			{
				//set motor speed
				mc.setMotorSpeeds(DC, 50, -50);
				//wait for 2 seconds
				Utils::waitFor(2);
				//set speed 0
				mc.setMotorSpeeds(DC, 0, 0);
				break;
			}
		}

	}

	Utils::waitFor(2);

	mc.controllerReset(DC);

	status = MyRio_Close();

	return status;
}
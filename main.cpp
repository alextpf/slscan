#include <stdlib.h>
#include <iostream>
#include <conio.h>
#include <windows.h> // WinApi header

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>

#include "VideoProcessor.h"

using namespace cv;
using namespace std;

enum OPERATION {
	CAPTURE_CALI_LEFT = 0,
	CAPTURE_CALI_RIGHT,
	CAPTURE_CALI_LEFT_RIGHT,
	CALIBRATE_LEFT,
	CALIBRATE_RIGHT,
	CALIBRATE_LEFT_RIGHT,
	MANUAL_SCAN,
	AUTO_SCAN,
	EXIT
};

OPERATION MainMenu()
{
	system( "CLS" ); // clear prompt command
	cout << "Choose from the following options:\n";
	cout << "=============================================================================\n";
	cout << "[1]: capture calibration patterns for left camera ( Esc to terminate )\n";
	cout << "[2]: capture calibration patterns for right camera  ( Esc to terminate )\n";
	cout << "[3]: capture calibration patterns for both left & right camera  ( Esc to terminate )\n";
	cout << "[4]: calibrate left camera\n";
	cout << "[5]: calibrate right camera\n";
	cout << "[6]: calibrate both left & right camera\n";
	cout << "[7]: Manual Scan ( hit 'c' to capture one scan ) \n";
	cout << "[8]: Auto Scan with turntable \n";
	cout << "[9]: Exit \n\n";
	cout << "Answer:";

	char c;
	cin << c;

	return static_cast<OPERATION>( static_cast<int>( c ) - 48);

} // MainMenu

//========================================
int main()
{
	OPERATION c( CAPTURE_CALI_LEFT );
	while ( c != EXIT )
	{
		c = MainMenu();
	}

	//////////////////
	// variables
	//////////////////

	//const char inPath[]		= "C:/tmp/data3/";
	const char inPath[]		= "C:/Users/alex_/Documents/target_recognition/target/data/";
	//const char inPath[]		= "C:/Users/alex_/Documents/Arduino/aidenbot/v2/aidenbot/data/webcam/";
	//const char outPath[]	= "C:/tmp/data3/";
	const char outPath[] = "C:/Users/alex_/Documents/target_recognition/target/data/";
	const char filename[]	= "3";

	const int webCamId		= 1; // 0: default (laptop's camera), 1: external connected cam
	const int startFrame    = 0;// frame number we want to start at
	const int endFrame		= 837;

	int inputType = 0;
	int outputType = 0;
	VideoProcessor processor;
	bool showInputImg = true;
	bool showOutputImg = true;
	FrameProcessor * proc = NULL;
	int delay = 1;

	//cout << '\a';
	Beep( 523, 500 ); // 523 hertz (C5) for 500 milliseconds
	cin.get(); // wait
	return 0;

	/////////////////////////////////////////////////////
	// Input
	/////////////////////////////////////////////////////
	switch (inputType)
	{
		case 0:
		{
			/////////////////////////
			// input: images
			/////////////////////////
			std::vector<std::string> imgs;

			for (int i = 0; i < endFrame; i++)
			{
				char buffer[100];
				sprintf_s(buffer, "%s%s%03i.jpg", inPath, filename,i);

				std::string name = buffer;
				imgs.push_back(name);
			}

			processor.SetInput(imgs);
		}
		break;

		case 1:
		{
			/////////////////////////
			// input: video
			/////////////////////////
			char buffer[100];
			sprintf_s(buffer, "%s%s.mp4", inPath, filename);

			std::string name = buffer;
			if (!processor.SetInput(name))
			{
				std::cout << "open file error" << std::endl;
			}
		}
		break;

		case 2:
		{
			/////////////////////////
			// input: webcam
			/////////////////////////
			processor.SetInput( webCamId ); //webcam
		}
		break;

		default:
			break;
	} //switch (inputType)

	/////////////////////////////////////////////////////
	// Output
	/////////////////////////////////////////////////////
	switch (outputType)
	{
		case 0:
		{
			/////////////////////////
			// output: images
			/////////////////////////
			char buffer[100];
			sprintf_s(buffer, "%s%s", outPath, filename);

			processor.SetOutput(buffer, ".jpg");
		}
		break;

		case 1:
		{
			/////////////////////////
			// output: video
			/////////////////////////
			char buffer[100];
			sprintf_s(buffer, "%s%s.mp4", outPath, filename );

			int codec = CV_FOURCC( 'D', 'I', 'V', 'X' );
			int fps = 30;
			//int codec = CV_FOURCC( 'P', 'I', 'M', '1' );

			processor.SetOutput( buffer, codec, fps );
		}
		break;

		// case 2:// no output

		default:
			break;
	}//switch (outputType)


	processor.SetFrameProcessor( proc );

	// Declare a window to display the video
	if ( showOutputImg )
	{
		processor.DisplayOutput( "Test Output" );
	}

	// Declare a window to display the input
	if ( showInputImg )
	{
		processor.DisplayInput( "Input" );
	}

	if (!processor.SetFrameNumber(startFrame))
	{
		std::cout << "err";
		return -1;
	}

	processor.SetDelay(delay);

	// Start the Process
	processor.Run();

	return 0;
}
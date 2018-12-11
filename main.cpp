#include <stdlib.h>
#include <iostream>
#include <conio.h>
#include <windows.h> // WinApi header

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>

#include "LiveViewProcessor.h"

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

    int c;
    std::cin >> c;

    return static_cast<OPERATION>( c );

} // MainMenu

//========================================
int main()
{
    char inPath[256];
    char outPath[256];
    char filename[256];
    int webCamId; // 0: default (laptop's camera), 1: external connected cam

    int inputType( 0 ); // 0: imgs, 1: video, 2: webcam
    int outputType( 0 ); // 0: imgs, 1: video, 2: webcam
    int delay( 0 );
    bool showOutputImg( false );
    bool showInputImg( false );

    LiveViewProcessor processor;

	OPERATION op( CAPTURE_CALI_LEFT );
	while ( op != EXIT )
	{
        op = MainMenu();
        switch( op )
        {
        case CAPTURE_CALI_LEFT:
        {
            // hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate
            sprintf_s( inPath, "../cali_data/left/" );
            sprintf_s( outPath, "../cali_data/left/" );
            sprintf_s( filename, "cali_left" );
            inputType = 2; // webcam
            outputType = 0; // imgs
            delay = 0; // wait indefinitely, untill any of the above charaters is hit ('c','a','r','Esc')
            webCamId = 1;

            showOutputImg = true;
            showInputImg = true;
        }
        break;

        case CAPTURE_CALI_RIGHT:
        {
            // hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate
            sprintf_s( inPath, "../cali_data/right/" );
            sprintf_s( outPath, "../cali_data/right/" );
            sprintf_s( filename, "cali_right" );
            inputType = 2; // webcam
            outputType = 0; // imgs
            delay = 0; // wait indefinitely, untill any of the above charaters is hit ('c','a','r','Esc')
            webCamId = 2;

            showOutputImg = true;
            showInputImg = true;
        }
        break;

        case CAPTURE_CALI_LEFT_RIGHT:
        {}
        break;

        case CALIBRATE_LEFT:
        {}
        break;

        case CALIBRATE_RIGHT:
        {}
        break;

        case CALIBRATE_LEFT_RIGHT:
        {}
        break;

        case MANUAL_SCAN:
        {}
        break;

        case AUTO_SCAN:
        {}
        break;

        default:
            break;


        }
	}

	//////////////////
	// variables
	//////////////////

	const int startFrame    = 0;// frame number we want to start at
	const int endFrame		= 837;
    FrameProcessor * proc = NULL;

    //cout << '\a';
    //Beep( 523, 500 ); // 523 hertz (C5) for 500 milliseconds
    //cin.get(); // wait
    //return 0;

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
#include <iostream>
#include <stdlib.h>
#include <conio.h>
#include <windows.h> // WinApi header
#include <memory>
#include <fstream>      // std::ifstream

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>

#include "LiveViewProcessor.h"
#include "Calibrator.h"

using namespace cv;
using namespace std;

//======================================
enum OPERATION {
	CAPTURE_CALI_LEFT = 1,
	CAPTURE_CALI_RIGHT,
	CAPTURE_CALI_LEFT_RIGHT,
	CALIBRATE_LEFT,
	CALIBRATE_RIGHT,
	CALIBRATE_LEFT_RIGHT,
	MANUAL_SCAN,
	AUTO_SCAN,
	WRITE_CONFIG,
	EXIT
};

enum SOURCE_TYPE {
    IMG = 0,
    VIDEO,
    WEBCAM,
    NONE
};

enum WEB_CAM_ID {
	DEFAULT,
	LEFT,
	RIGHT
};
//======================================
//fwd declare
void SetParams();
OPERATION MainMenu();
void WriteConfig();
void ReadConfig();
//======================================
//globals
char inPath[256];
char outPath[256];
char filename[256];
int webCamId( DEFAULT ); // 0: default (laptop's camera), 1: external connected cam

SOURCE_TYPE inputType( WEBCAM ); // 0: imgs, 1: video, 2: webcam
SOURCE_TYPE outputType( IMG ); // 0: imgs, 1: video, 2: webcam
int delay( 0 );
bool showOutputImg( false );
bool showInputImg( false );

OPERATION op( CAPTURE_CALI_LEFT );
int w; // cali pattern width
int h; // cali pattern height
float blockSize; // physical size of a chessboard block, in mm
//========================================
int main()
{


	while ( op != EXIT )
	{
        op = MainMenu();
        switch( op )
        {
			case WRITE_CONFIG:
			{
				WriteConfig();
			}
			break;

            case CAPTURE_CALI_LEFT:
            {
                //////////////////////
                // Read from config
                //////////////////////
				ReadConfig();

                // hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate
                sprintf_s( inPath, "../cali_data/left/" );
                sprintf_s( outPath, "../cali_data/left/" );
                sprintf_s( filename, "cali_left" );
                inputType = WEBCAM;
                outputType = IMG;

                delay = 1; // ms
                webCamId = LEFT; // 0:
				Calibrator processor;

			    processor.SetNumSource( 1 ); // only 1 camera
                processor.SetDelay( delay );
                processor.SetWidth( w );
                processor.SetHeight( h );
				processor.SetBlockSize( blockSize );
				processor.SetFileName( filename );
				processor.SetFileNameNumDigits( 2 );

                string title = "Left Cam";
                vector<string> wn;
                wn.push_back( title );
                processor.DisplayOutput( wn );

                //====================================
                switch( inputType )
                {
                    case IMG:
                    {
                        const int startFrame = 0;// frame number we want to start at
                        const int endFrame = processor.GetNumCaliImgs();

                        vector<vector<string>> imgs;

                        vector<string> tmp;

                        for( int i = 0; i < endFrame; i++ )
                        {
                            char buffer[100];
                            sprintf_s( buffer, "%s%s%02i.jpg", inPath, filename, i );

                            string name = buffer;
                            tmp.push_back( name );
                        }
                        imgs.push_back( tmp );

                        processor.SetInput( imgs );
                    } // case IMG
                    break;

                    case WEBCAM:
                    {
                        vector<int> id;
                        id.push_back(webCamId);

                        bool ok = processor.SetInput( id ); //webcam
                        if( !ok )
                        {
                            cout << "Can't open webcam/n";
                            return -1;
                        }
                    }// case WEBCAM
                    break;

                    default:
                        break;
                }//switch inputtype

				switch ( outputType )
				{
					case IMG:
					{
						/////////////////////////
						// output: images
						/////////////////////////
						char buffer[100];
						sprintf_s( buffer, "%s%s", outPath, filename );
						vector<string> s;
						s.push_back( buffer );

						processor.SetOutput( s, ".jpg" );
					}
					break;

					default:
						break;
				}//switch ( outputType )

				// Start the Process
                processor.Run();
            }//case CAPTURE_CALI_LEFT:
            break;

        case CAPTURE_CALI_RIGHT:
        {
            // hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate
            sprintf_s( inPath, "../cali_data/right/" );
            sprintf_s( outPath, "../cali_data/right/" );
            sprintf_s( filename, "cali_right" );
            inputType = WEBCAM;
            outputType = NONE;

            delay = 1;
            webCamId = RIGHT;

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


        }// switch

	} // while ( op != EXIT )

	return 0;
}//main

//================================================================
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
	cout << "[9]: Write Config file \n";
	cout << "[10]: Exit \n\n";
	cout << "Answer: ";

	int c;
	std::cin >> c;

	return static_cast<OPERATION>( c );

} // MainMenu

//=======================================================
void SetParams()
{
	//////////////////
	// variables
	//////////////////

	//cout << '\a';
	//Beep( 523, 500 ); // 523 hertz (C5) for 500 milliseconds
	//cin.get(); // wait
	//return 0;

	/*if ( !processor.SetFrameNumber( startFrame ) )
	{
		std::cout << "err";
		return ;
	}
*/

}//SetParams

//=======================================================
void WriteConfig()
{
	int board_width = 11;
	int board_height = 8;
	float block_size = 50.0f; //mm

	cv::FileStorage fs( "config.xml", cv::FileStorage::WRITE );
	fs << "board_width" << board_width << "board_height" << board_height
	   << "block_size" << block_size;
	fs.release();
}//WriteConfig

//=========================================
void ReadConfig()
{
	cv::FileStorage fs;
	fs.open( "config.xml", cv::FileStorage::READ );

	cout << "\nboard_width: " << static_cast<int>( fs["board_width"] );
	cout << "\nboard_height: " << static_cast<int>( fs["board_height"] );
	cout << "\nblock_size: " << static_cast<float>( fs["block_size"] );

	fs["board_width"] >> w;
	fs["board_height"] >> h;
	fs["block_size"] >> blockSize;
}
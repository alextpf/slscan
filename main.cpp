#include <iostream>
#include <stdlib.h>
#include <conio.h>
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
	CAPTURE_CALI_LEFT_AND_RIGHT,
	CALIBRATE_LEFT,
	CALIBRATE_RIGHT,
	CALIBRATE_LEFT_AND_RIGHT,
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

//======================================
//fwd declare
OPERATION MainMenu();
void WriteConfig();
void ReadConfig();
bool CaptureAndOrCali(
	string in,
	string out,
    vector<string> name,
	SOURCE_TYPE inType,
	SOURCE_TYPE outType,
	vector<LiveViewProcessor::WEB_CAM_ID> webCamId,
    vector<string> title
	);

//======================================
//globals
char inPath[256];
char outPath[256];
char filename[256];
LiveViewProcessor::WEB_CAM_ID webCamId( LiveViewProcessor::DEFAULT_CAM ); // 0: default (laptop's camera), 1: external connected cam

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
				// hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate
                vector<LiveViewProcessor::WEB_CAM_ID> ids;
                ids.push_back( LiveViewProcessor::LEFT_CAM );
                vector<string> title;
                title.push_back( "Left Cam" );
                vector<string> fileName;
                fileName.push_back( "cali_left" );

                const bool ok = CaptureAndOrCali(
                    "cali_data/left/",
                    "cali_data/left/",
                    fileName,
                    WEBCAM,
                    IMG,
                    ids,
					title );
				if ( !ok )
				{
					return -1;
				}
            }//case CAPTURE_CALI_LEFT:
            break;

        case CAPTURE_CALI_RIGHT:
        {
			// hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate
            vector<LiveViewProcessor::WEB_CAM_ID> ids;
            ids.push_back( LiveViewProcessor::RIGHT_CAM );
            vector<string> title;
            title.push_back( "Right Cam" );
            vector<string> fileName;
            fileName.push_back( "cali_right" );

			const bool ok = CaptureAndOrCali(
				"cali_data/right/",
				"cali_data/right/",
                fileName,
				WEBCAM,
				IMG,
				ids,
				title );
			if ( !ok )
			{
				return -1;
			}
		}
        break;

        case CAPTURE_CALI_LEFT_AND_RIGHT:
        {
            // hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate
            vector<LiveViewProcessor::WEB_CAM_ID> ids;
            ids.push_back( LiveViewProcessor::DEFAULT_CAM );
            ids.push_back( LiveViewProcessor::LEFT_CAM );
            //ids.push_back( LiveViewProcessor::RIGHT_CAM );

            vector<string> title;
            title.push_back( "Left Cam" );
            title.push_back( "Right Cam" );
            vector<string> fileName;
            fileName.push_back( "cali_left" );
            fileName.push_back( "cali_right" );

            const bool ok = CaptureAndOrCali(
                "cali_data/leftAndRight/",
                "cali_data/leftAndRight/",
                fileName,
                WEBCAM,
                IMG,
                ids,
                title );
            if( !ok )
            {
                return -1;
            }
        }
        break;

        case CALIBRATE_LEFT:
        {
            vector<LiveViewProcessor::WEB_CAM_ID> ids;
            ids.push_back( LiveViewProcessor::LEFT_CAM );
            vector<string> title;
            title.push_back( "Left Cam" );
            vector<string> fileName;
            fileName.push_back( "cali_left" );

			const bool ok = CaptureAndOrCali(
				"cali_data/left/",
				"cali_data/left/",
                fileName,
				IMG,
				IMG,
				ids,
				title );
			if ( !ok )
			{
				return -1;
			}
		}
        break;

        case CALIBRATE_RIGHT:
        {
            vector<LiveViewProcessor::WEB_CAM_ID> ids;
            ids.push_back( LiveViewProcessor::RIGHT_CAM );
            vector<string> title;
            title.push_back( "Right Cam" );
            vector<string> fileName;
            fileName.push_back( "cali_right" );

			const bool ok = CaptureAndOrCali(
				"cali_data/right/",
				"cali_data/right/",
                fileName,
				IMG,
				IMG,
				ids,
				title );
			if ( !ok )
			{
				return -1;
			}
		}
        break;

        case CALIBRATE_LEFT_AND_RIGHT:
        {
            vector<LiveViewProcessor::WEB_CAM_ID> ids;
            ids.push_back( LiveViewProcessor::DEFAULT_CAM );
            ids.push_back( LiveViewProcessor::LEFT_CAM );
            //ids.push_back( LiveViewProcessor::RIGHT_CAM );

            vector<string> title;
            title.push_back( "Left Cam" );
            title.push_back( "Right Cam" );

            vector<string> fileName;
            fileName.push_back( "cali_left" );
            fileName.push_back( "cali_right" );

            const bool ok = CaptureAndOrCali(
                "cali_data/leftAndRight/",
                "cali_data/leftAndRight/",
                fileName,
                IMG,
                IMG,
                ids,
                title );
            if( !ok )
            {
                return -1;
            }
        }
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
void WriteConfig()
{
	int board_width = 10;
	int board_height = 7;
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
}//ReadConfig

//=========================
bool CaptureAndOrCali(
	string in,
	string out,
    vector<string> name,
	SOURCE_TYPE inType,
	SOURCE_TYPE outType,
	vector<LiveViewProcessor::WEB_CAM_ID> webCamId,
    vector<string> title
)
{
	//////////////////////
	// Read from config
	//////////////////////
	ReadConfig();

	// hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate

	inputType = inType;
	outputType = outType;

	delay = 1; // ms
	const bool captureAndCali = false; // capture only, don't cali
    const int numDigit = 2;
    const int numSource = static_cast<int>( webCamId.size() );

	Calibrator processor;

	processor.SetNumSource( numSource ); // num of camera
	processor.SetDelay( delay );
	processor.SetWidth( w );
	processor.SetHeight( h );
	processor.SetBlockSize( blockSize );
	processor.SetFileName( name );
	processor.SetFileNameNumDigits( numDigit );
	processor.SetCaptureAndCali( captureAndCali );
    processor.SetScaleFactorForShow( 0.5f );
	processor.SetPath( in );
	processor.DisplayOutput( title );

	//====================================
	switch ( inType )
	{
	case IMG:
	{
		processor.SetDoCali( true );
		processor.ReadNumCaliImgs(); // assuming previously we have captured the cali imgs

		const int startFrame = 0;// frame number we want to start at
		const int endFrame = processor.GetNumCaliImgs();
		processor.SetNumCaliImgs( 0 ); // reset

		vector<vector<string>> imgs;

		vector<string> tmp;
        for( int j = 0; j < numSource; j++ )
        {
            for( int i = 0; i < endFrame; i++ )
            {
                char buffer[100];
                sprintf_s( buffer, "%s%s%02i.jpg", in.c_str(), name[j].c_str(), i );

                string imgName = buffer;
                tmp.push_back( imgName );
            }
            imgs.push_back( tmp );
        }

		processor.SetInput( imgs );
	} // case IMG
	break;

	case WEBCAM:
	{
		bool ok = processor.SetInput( webCamId ); //webcam
		if ( !ok )
		{
			cout << "Can't open webcam/n";
			return false;
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
        vector<string> s;
        for( int i = 0; i < numSource; i++ )
        {
            char buffer[100];
            sprintf_s( buffer, "%s%s", out.c_str(), name[i].c_str() );

            s.push_back( buffer );
        }

        processor.SetOutput( s, ".jpg", numDigit );
	}
	break;

	default:
		break;
	}//switch ( outputType )

	 // Start the Process
	processor.Run();

	return true;
}//CaptureAndOrCali
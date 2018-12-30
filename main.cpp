//========================================================
// Alex Chen, 2018
// alextpf@gmail.com
//
// note:
// - object has to be located at ~ m from the projector (mine is a VANKYO Leisure 510 )
// - the extented screen (for the projector) has to be set at the resolution of 1280 x 768, with no scaling
// - the scanner has a depth of XX m, and width, height of XX m
// - to run off auto focus of Logitech Webcam C920, download its LogiCameraSettings_2.5.17.exe or LGS to run it off
//========================================================
#include <iostream>
#include <string>
#include <stdlib.h>
#include <conio.h>
#include <memory>
#include <fstream>      // std::ifstream
#include <direct.h> //for mkdir
#include <sys/types.h>
#include <sys/stat.h>

#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>

#include "LiveViewProcessor.h"
#include "Calibrator.h"
#include "SerialPort.h"

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
	MANUAL_SCAN_ONE_VIEW,
	GENERATE_3D_ONE_VIEW,
	AUTO_SCAN,
	TURN_TABLE,
	WRITE_CONFIG,
	EXIT
};

enum SOURCE_TYPE {
    IMG = 0,
    VIDEO,
    WEBCAM,
    NONE
};

enum DIR_STATUS {
    EXISTS,
    CANT_ACCESS,
    NO_EXIST
};
//======================================
//fwd declare
OPERATION MainMenu();
void WriteConfig();
void ReadConfig();
bool CaptureAndOrCali(
	string in,
	string out,
    vector<string> inName,
	vector<string> outName,
	SOURCE_TYPE inType,
	SOURCE_TYPE outType,
	vector<LiveViewProcessor::WEB_CAM_ID> webCamId,
	vector<string> title );

bool Calculate3DPrepare(
	string in,
	string out,
	vector<string> inName,
	vector<string> outName,
	SOURCE_TYPE inType,
	SOURCE_TYPE outType,
	vector<string> title );

bool CaptureLeft();
bool CaptureRight();
bool CaptureLeftAndRight();
bool CalibrateLeft();
bool CalibrateRight();
bool CalibrateLeftAndRight();
bool ScanOneView( string path );
bool Generate3DForOneView( string path );
bool OneTurn( const int curDeg, const int speed );

// utility
DIR_STATUS DirExists( const char *path );

//======================================
//globals
Calibrator processor;
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
int projW; // projector width
int projH; // projector height
int com; // com port
int speed; // turn table speed
int duration; // duration of each turn of turntable
float blockSize; // physical size of a chessboard block, in mm
shared_ptr<SerialPort> serialPort;
//========================================
int main()
{
	//====================================
	//////////////////////
	// Read from config
	//////////////////////
	ReadConfig();

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
				if ( !CaptureLeft() )
				{
					return -1;
				}
            }//case CAPTURE_CALI_LEFT:
            break;

			case CAPTURE_CALI_RIGHT:
			{
				if ( !CaptureRight() )
				{
					return -1;
				}
			}
			break;

			case CAPTURE_CALI_LEFT_AND_RIGHT:
			{
				if ( !CaptureLeftAndRight() )
				{
					return -1;
				}
			}
			break;

			case CALIBRATE_LEFT:
			{
				if ( !CalibrateLeft() )
				{
					return -1;
				}
			}
			break;

			case CALIBRATE_RIGHT:
			{
				if ( !CalibrateRight() )
				{
					return -1;
				}
			}
			break;

			case CALIBRATE_LEFT_AND_RIGHT:
			{
				if ( !CalibrateLeftAndRight() )
				{
					return -1;
				}
			}
			break;

			case MANUAL_SCAN_ONE_VIEW:
			{
				char name[256];

				system( "CLS" ); // clear prompt command
				std::cout << "Please enter the project name: ";
				std::cin.getline( name, 256 );

				std::stringstream dir;
				dir << name << "/";
				_mkdir( dir.str().c_str() );

				if ( !ScanOneView( dir.str() ) )
				{
					return -1;
				}
			}
			break;

			case GENERATE_3D_ONE_VIEW:
			{
                string path = "scan_data/";

				if ( !Generate3DForOneView( path ) )
				{
					return -1;
				}
			}
			break;

			case AUTO_SCAN:
			{
				char name[256];

				system( "CLS" ); // clear prompt command
				std::cout << "Please enter the project name: ";
				std::cin.getline( name, 256 );

				int fullCircle = 360; // degree, full circle
				int delta = 30; // one turn
				int steps = fullCircle / delta;

				if ( !serialPort )
				{
					char comPort[20];
					sprintf_s( comPort, "\\\\.\\COM%d", com );
					serialPort = make_shared<SerialPort>( comPort );
					if ( !serialPort )
					{
						cout << "serial communication error\n";
						return -1;
					}
				}

				if ( !serialPort->IsConnected() )
				{
					cout << "serial not connected\n";
					return -1;
				}

				// each step capture
				for ( int curDeg = 0; curDeg < fullCircle; curDeg += delta )
				{
					std::stringstream dir;
					dir << name << curDeg << "/";
					_mkdir( dir.str().c_str() );
					if ( !ScanOneView( dir.str() ) )
					{
						return -1;
					}//if

                    // now rotate turn table
                    if( !OneTurn( curDeg, speed ) )
                    {
                        cout << "turn error\n";
                        return -1;
                    }

					cv::waitKey(duration);
				}//for i

				// now generate
				for ( int curDeg = 0; curDeg < fullCircle; curDeg += delta )
				{
					std::stringstream dir;
					dir << name << curDeg << "/";
					if ( !Generate3DForOneView( dir.str() ) )
					{
						return -1;
					}
				}//for i
			}
			break;

			case TURN_TABLE:
			{
				char pos[256];

				system( "CLS" ); // clear prompt command
				std::cout << "Please enter table position in degree: ";
				std::cin.getline( pos, 256 );
				stringstream stream;
				stream.str( pos );
				int deg;
				stream >> deg;

				if ( !serialPort )
				{
					char comPort[20];
					sprintf_s( comPort, "\\\\.\\COM%d", com );
					serialPort = make_shared<SerialPort>( comPort );
					if ( !serialPort )
					{
						cout << "serial communication error\n";
						return -1;
					}
				}

				if ( !serialPort->IsConnected() )
				{
					cout << "serial not connected\n";
					return -1;
				}

				// now rotate turn table
				if ( !OneTurn( deg, speed ) )
				{
					cout << "turn error\n";
					return -1;
				}
			}
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
	cout << "[7]: Manual Scan one view \n";
	cout << "[8]: Generate 3D of scanned one view \n";
	cout << "[9]: Auto Scan with turntable \n";
	cout << "[10]: Turn table \n";
	cout << "[11]: Write Config file \n";
	cout << "[12]: Exit \n\n";
	cout << "Answer: ";

	char name[256];

	std::cin.getline( name, 256 );
	int c = atoi( name );

	return static_cast<OPERATION>( c );

} // MainMenu

//=======================================================
void WriteConfig()
{
	int board_width = 10;
	int board_height = 7;
	float block_size = 50.0f; //mm
    int projectorWidth = 1280; // VANKYO Leisure 510
    int projectorHeight = 768;
    int com = 4; // com port
    int speed = 50; // speed
	int duration = 5000;//ms

	cv::FileStorage fs( "config.xml", cv::FileStorage::WRITE );
    fs << "board_width" << board_width << "board_height" << board_height
        << "block_size" << block_size
        << "projector_width" << projectorWidth
        << "projector_height" << projectorHeight
        << "com" << com
        << "speed" << speed
		<< "duration" << duration;

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
	fs["projector_width"] >> projW;
	fs["projector_height"] >> projH;
    fs["com"] >> com;
	fs["speed"] >> speed;
	fs["duration"] >> duration;

	fs.release();
}//ReadConfig

//=========================
bool CaptureAndOrCali(
	string in,
	string out,
    vector<string> inName,
	vector<string> outName,
	SOURCE_TYPE inType,
	SOURCE_TYPE outType,
	vector<LiveViewProcessor::WEB_CAM_ID> webCamId,
    vector<string> title )
{
	// hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate

	delay = 1; // ms
	const bool captureAndCali = false; // capture only, don't cali
    const int numDigit = 2;
    const int numSource = static_cast<int>( webCamId.size() );
	const float scaleFactor = 0.4f;

	processor.SetNumSource( numSource ); // num of camera
	processor.SetDelay( delay );
	processor.SetCaliPatternWidth( w );
	processor.SetCaliPatternHeight( h );
	processor.SetBlockSize( blockSize );
	processor.SetInputFileName( inName );
	processor.SetOutputFileName( outName );
	processor.SetFileNameNumDigits( numDigit );
	processor.SetCaptureAndCali( captureAndCali );
    processor.SetScaleFactorForShow( scaleFactor );
	processor.SetPath( in );
	processor.DisplayOutput( title );

	//====================================
	switch ( inType )
	{
	case IMG:
	{
		processor.SetDoCali( true );

		const int startFrame = 0;// frame number we want to start at
		processor.ReadNumCaliImgs(); // assuming previously we have captured the cali imgs
		const int endFrame = processor.GetNumCaliImgs();
		processor.SetNumCaliImgs( 0 ); // reset

		vector<vector<string>> imgs;

        for( int j = 0; j < numSource; j++ )
        {
			vector<string> tmp;
            for( int i = 0; i < endFrame; i++ )
            {
                char buffer[100];
                sprintf_s( buffer, "%s%s%02i.jpg", in.c_str(), inName[j].c_str(), i );

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
            sprintf_s( buffer, "%s%s", out.c_str(), outName[i].c_str() );

            s.push_back( buffer );
        }

        processor.SetOutput( s, ".jpg", numDigit );
	}
	break;

	default:
		break;
	}//switch ( outputType )

	return true;
}//CaptureAndOrCali

//=========================
bool Calculate3DPrepare(
	string in,
	string out,
	vector<string> inName,
	vector<string> outName,
	SOURCE_TYPE inType,
	SOURCE_TYPE outType,
	vector<string> title )
{
	// hit 'c' to capture, 'a' to accept, 'r' to reject, and 'Esc' to terminate

	delay = 1; // ms
	const int numDigit = 2;
	const int numSource = 2;
	const float scaleFactor = 0.4f;

	processor.SetNumSource( numSource ); // num of camera
	processor.SetDelay( delay );
	processor.SetInputFileName( inName );
	processor.SetOutputFileName( outName );
	processor.SetFileNameNumDigits( numDigit );
	processor.SetPath( in );
	processor.DisplayOutput( title );
	processor.SetScaleFactorForShow( scaleFactor );

	//====================================
	switch ( inType )
	{
	case IMG:
	{
		processor.SetDoCali( true );

		const int startFrame = 0;// frame number we want to start at
		const int endFrame = processor.ReadNumPatterns(); // assuming previously we have captured the cali imgs

		vector<vector<string>> imgs;

		for ( int j = 0; j < numSource; j++ )
		{
			vector<string> tmp;
			for ( int i = 0; i < endFrame; i++ )
			{
				char buffer[100];
				sprintf_s( buffer, "%s%s%02i.jpg", in.c_str(), inName[j].c_str(), i );

				string imgName = buffer;
				tmp.push_back( imgName );
			}
			imgs.push_back( tmp );
		}

		processor.SetInput( imgs );
	} // case IMG
	break;
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
		for ( int i = 0; i < numSource; i++ )
		{
			char buffer[100];
			sprintf_s( buffer, "%s%s", out.c_str(), outName[i].c_str() );

			s.push_back( buffer );
		}

		processor.SetOutput( s, ".jpg", numDigit );
	}
	break;

	default:
		break;
	}//switch ( outputType )

	processor.Generate3D();

	return true;
}//Calculate3DPrepare

//===========================
bool CaptureLeft()
{
    string path = "cali_data/left/";
    _mkdir( path.c_str() );

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	ids.push_back( LiveViewProcessor::LEFT_CAM );
	vector<string> title;
	title.push_back( "Left Cam" );
	vector<string> fileName;
	fileName.push_back( "cali_left" );

	const bool ok = CaptureAndOrCali(
        path,
        path,
		fileName, // input name, not used
		fileName, // output name
		WEBCAM,
		IMG,
		ids,
		title );
	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.CaptureAndClibrate();

	return true;
}//CaptureLeft

//==========================
bool CaptureRight()
{
    string path = "cali_data/right/";
    _mkdir( path.c_str() );

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	ids.push_back( LiveViewProcessor::RIGHT_CAM );
	vector<string> title;
	title.push_back( "Right Cam" );
	vector<string> fileName;
	fileName.push_back( "cali_right" );

	const bool ok = CaptureAndOrCali(
        path,
        path,
		fileName, // input name, not used
		fileName, // output name
		WEBCAM,
		IMG,
		ids,
		title );

	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.CaptureAndClibrate();
	return true;
} // CaptureRight

//=====================
bool CaptureLeftAndRight()
{
    string path = "cali_data/leftAndRight/";
    _mkdir( path.c_str() );

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	//ids.push_back( LiveViewProcessor::DEFAULT_CAM );
	ids.push_back( LiveViewProcessor::LEFT_CAM );
	ids.push_back( LiveViewProcessor::RIGHT_CAM );

	vector<string> title;
	title.push_back( "Left Cam" );
	title.push_back( "Right Cam" );
	vector<string> fileName;
	fileName.push_back( "cali_left" );
	fileName.push_back( "cali_right" );

	const bool ok = CaptureAndOrCali(
		path,
        path,
		fileName, // input name, not used
		fileName, // output name
		WEBCAM,
		IMG,
		ids,
		title );
	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.CaptureAndClibrate();
	return true;
}//CaptureLeftAndRight

//==============================
bool CalibrateLeft()
{
    string path = "cali_data/left/";
    if( DirExists( path.c_str() ) != EXISTS )
    {
        cout << "Need to scan left cam data first!/n";
        return false;
    }

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	ids.push_back( LiveViewProcessor::LEFT_CAM );
	vector<string> title;
	title.push_back( "Left Cam" );
	vector<string> fileName;
	fileName.push_back( "cali_left" );

	const bool ok = CaptureAndOrCali(
        path,
        path,
		fileName, // input name
		fileName, // output name
		IMG,
		IMG,
		ids,
		title );

	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.CaptureAndClibrate();
	return true;
}//CalibrateLeft

//=====================
bool CalibrateRight()
{
    string path = "cali_data/right/";
    if( DirExists( path.c_str() ) != EXISTS )
    {
        cout << "Need to scan Right cam data first!/n";
        return false;
    }

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	ids.push_back( LiveViewProcessor::RIGHT_CAM );
	vector<string> title;
	title.push_back( "Right Cam" );
	vector<string> fileName;
	fileName.push_back( "cali_right" );

	const bool ok = CaptureAndOrCali(
        path,
        path,
		fileName,
		fileName,
		IMG,
		IMG,
		ids,
		title );
	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.CaptureAndClibrate();
	return true;
} // CalibrateRight

//============================
bool CalibrateLeftAndRight()
{
    string path = "cali_data/leftAndRight/";
    if( DirExists( path.c_str() ) != EXISTS )
    {
        cout << "Need to scan left+right cam data first!/n";
        return false;
    }

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	//ids.push_back( LiveViewProcessor::DEFAULT_CAM );
	ids.push_back( LiveViewProcessor::LEFT_CAM );
	ids.push_back( LiveViewProcessor::RIGHT_CAM );

	vector<string> title;
	title.push_back( "Left Cam" );
	title.push_back( "Right Cam" );

	vector<string> fileName;
	fileName.push_back( "cali_left" );
	fileName.push_back( "cali_right" );

	const bool ok = CaptureAndOrCali(
        path,
        path,
		fileName,
		fileName,
		IMG,
		IMG,
		ids,
		title );

	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.CaptureAndClibrate();
	return true;
}//CalibrateLeftAndRight

//==========================
bool ScanOneView( string path )
{
    // create dir if not already existed. If already existed, will overwrite its contents!
    _mkdir( path.c_str() );

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	//ids.push_back( LiveViewProcessor::DEFAULT_CAM );
	ids.push_back( LiveViewProcessor::LEFT_CAM );
	ids.push_back( LiveViewProcessor::RIGHT_CAM );

	vector<string> title;
	title.push_back( "Left Cam" );
	title.push_back( "Right Cam" );

	vector<string> fileName;
	fileName.push_back( "scan_left" );
	fileName.push_back( "scan_right" );

	processor.SetProjectorDimension( projW, projH );
	processor.SetProjWinName( "Pattern Window" );

	bool ok = processor.GeneratePattern();

	ok = ok && CaptureAndOrCali(
		path,
		path,
		fileName,
		fileName,
		WEBCAM,
		IMG,
		ids,
		title );

	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.Scan();
	return true;
} // ScanOneView

//=========================
bool Generate3DForOneView( string path )
{
    // check if the dir exist or not, if not, need to scan data first!
    if( DirExists( path.c_str() ) != EXISTS )
    {
        cout << "Need to scan data first!/n";
        return false;
    }

	vector<string> inputFileName;
	inputFileName.push_back( "scan_left" );
	inputFileName.push_back( "scan_right" );

	vector<string> outputFileName;
	outputFileName.push_back( "scan_left_rectified" );
	outputFileName.push_back( "scan_right_rectified" );

	vector<string> title;
	title.push_back( "Left Cam" );
	title.push_back( "Right Cam" );

	processor.SetProjectorDimension( projW, projH );
	if ( !processor.GeneratePattern() )
	{
		return false;
	}

	const bool ok = Calculate3DPrepare(
		path,
		path,
		inputFileName,
		outputFileName,
		IMG,
		NONE,
		title );

	return ok;
}//Generate3DForOneView

//======================================
DIR_STATUS DirExists( const char *path )
{
    struct stat info;

    if( stat( path, &info ) != 0 )
    {
        return CANT_ACCESS;
    }
    else if( info.st_mode & S_IFDIR )
    {
        return EXISTS;
    }
    else
    {
        return NO_EXIST;
    }
} // DirExists

bool OneTurn( const int curDeg, const int speed )
{
    // send messeage over the serial port

    // message lay out :
    // 0, 1: Initial sync markers
    // 2, 3: desired table degree
    // 4, 5: desired table speed
    BYTE message[6];

    // Initial sync markers
    message[0] = 0x7F;
    message[1] = 0x7F;

    // desired table degree
    message[2] = ( curDeg >> 8 ) & 0xFF;
    message[3] = curDeg & 0xFF;

    // desired table speed
    message[4] = ( speed >> 8 ) & 0xFF;
    message[5] = speed & 0xFF;

    bool ok = serialPort->WriteSerialPort<BYTE>( message, 6 );

    if( !ok )
    {
        return false;
    }

    // wait until the turn is done
	// TODO: this is not working for now
    /*bool done( false );

    while( !done )
    {
        ok = serialPort->ReadSerialPort<BYTE>( message, 1 );
        if( !ok )
        {
            return false;
        }
		cout << message[0] << endl;

        done = message[0] == 0x01;
    }*/

    return ok;
}//OneTurn
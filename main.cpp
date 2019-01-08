//========================================================
// Alex Chen, 2018
// alextpf@gmail.com
//
// note:
// - object has to be located at ~ m from the projector (mine is a VANKYO Leisure 510 )
// - the extented screen (for the projector) has to be set at the resolution of 1280 x 768, with no scaling
// - the scanner has a depth of 56"+-3", and width, height of XX m
// - to run off auto focus of Logitech Webcam C920, download its LogiCameraSettings_2.5.17.exe or LGS to run it off
// - for PCL, download pre-compiled version, ex: PCL-1.8.1-AllInOne-msvc2015-win64 ,from
//   https://github.com/PointCloudLibrary/pcl/releases
//   the 3rd party will then be installed in C:\Program Files\PCL 1.8.1\3rdParty
//   we will just need to add "include", "lib" to project path, and "bin" to env var "path";
//   *.dlls to the additional dependancies
// - for the 2 cameras, the more they direct torward to each other (i.e. more angle, not parallel ), the less depth results we get
// - The way to create a 3D model from a single view 3D shot:
// 1. in meshlab, delete unwanted points
// 2. downsample points by: filter -> sampling -> poisson disk sampling
// 3. calculate normals by: filter -> point sets -> compute normals for point sets
// 4. meshing point clouds by: filter -> Screen Poisson Surface Reconstruction, the higher value of Reconstruction Depth and Interpolation Weight are, the more faithful the surface is to the points
// 5. edit mesh in MeshMixer, where it has Lasso tool in "Select"
// 6. smooth the trimmed mesh in MeshLab: Filter -> Smooth, meshing .... -> either 1. Depth Smooth, or 2. Laplacian Smooth, or both
//    Note: when doing so you may need to invert the surface normal to see the rendering results correctly: Filter -> Normal.... -> Invert Face Normal
// 7. in MeshMixer, "Shell" the surface by: 1. select the whole surface, then 2. Edit -> Offset
//========================================================
#include <iostream>
#include <string>
#include <stdlib.h>
#include <conio.h>
#include <memory>
#include <fstream> // std::ifstream
#include <direct.h> //for mkdir

// CV
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/video.hpp>
#include <opencv2/imgproc.hpp>

// PCL
//#include <pcl/io/pcd_io.h>
//#include <pcl/point_types.h>
//#include <pcl/registration/icp.h>

#include "LiveViewProcessor.h"
#include "Calibrator.h"
#include "SerialPort.h"

using namespace cv;
using namespace std;

//======================================
enum OPERATION {
	LIVE_VIEW = 1,
	CAPTURE_CALI_LEFT,
	CAPTURE_CALI_RIGHT,
	CAPTURE_CALI_LEFT_AND_RIGHT,
	MANUAL_SCAN_ONE_VIEW,
	AUTO_SCAN,
	TURN_TABLE,
	WRITE_CONFIG,
    REGENERATE_PLY_WITH_TEXTURE,
	EXIT,
	CAPTURE_LEFT, // obselete. still works but hidden right now
	CAPTURE_RIGHT, // obselete. still works but hidden right now
	CAPTURE_LEFT_AND_RIGHT, // obselete. still works but hidden right now
    CALIBRATE_LEFT, // obselete. still works but hidden right now
    CALIBRATE_RIGHT, // obselete. still works but hidden right now
    CALIBRATE_LEFT_AND_RIGHT, // obselete. still works but hidden right now
    GENERATE_3D_ONE_VIEW, // obselete. still works but hidden right now
    AUTO_GENERATE_3D_VIEWS,
    DEBUG_DECODING,
    CAPTURE_ROI_FOR_DATA
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
//void TestPcl();
//void ICPTest();
bool CaptureROIForData( string path );
bool LiveView();
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
	vector<string> title);

bool Calculate3DPrepare(
	string in,
	string out,
	vector<string> inName,
	vector<string> outName,
	SOURCE_TYPE inType,
	SOURCE_TYPE outType,
	vector<string> title,
    const bool debug );

bool CaptureLeft();
bool CaptureRight();
bool CaptureLeftAndRight();
bool CalibrateLeft();
bool CalibrateRight();
bool CalibrateLeftAndRight();
bool ScanOneView( string path, const bool captureRoi );
bool Generate3DForOneView( string path, const bool debug );
bool OneTurn( const int curDeg, const int speed );

// utility
DIR_STATUS DirExists( const char *path );

//======================================
//globals
bool colImgOnly = false;
Calibrator processor;
char inPath[256];
char outPath[256];
char filename[256];
LiveViewProcessor::WEB_CAM_ID webCamId( LiveViewProcessor::DEFAULT_CAM ); // 0: default (laptop's camera), 1: external connected cam

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
    /*ICPTest();
    return -1;*/
	/*TestPcl();
	return -1;*/
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
			case LIVE_VIEW:
			{
				LiveView();
			}
			break;

            case CAPTURE_CALI_LEFT:
			{
                // capture
				if ( !CaptureLeft() )
				{
					return -1;
				}

				if ( processor.GetNumCaliImgs() <= 0 )
				{
					return true;
				}

                //calibrate
                if( !CalibrateLeft() )
                {
                    return -1;
                }
            }//case CAPTURE_CALI_LEFT:
            break;

			case CAPTURE_CALI_RIGHT:
			{
                // capture
				if ( !CaptureRight() )
				{
					return -1;
				}

				if ( processor.GetNumCaliImgs() <= 0 )
				{
					return true;
				}

                //calibrate
                if( !CalibrateRight() )
                {
                    return -1;
                }
			}
			break;

			case CAPTURE_CALI_LEFT_AND_RIGHT:
			{
                // capture
				if ( !CaptureLeftAndRight() )
				{
					return -1;
				}

				if ( processor.GetNumCaliImgs() <= 0 )
				{
					return true;
				}

                //calibrate
                if( !CalibrateLeftAndRight() )
                {
                    return -1;
                }
			}
			break;

			case MANUAL_SCAN_ONE_VIEW:
			{
				char name[256];

				std::cout << "Please enter the project name: ";
				std::cin.getline( name, 256 );

				std::stringstream dir;
				dir << name << "/";
				_mkdir( dir.str().c_str() );

                // capture
				bool captureRoi = false;
				if ( !ScanOneView( dir.str(), captureRoi ) )
				{
					return -1;
				}

                //// calculate
                //const bool debug = false;
                //if( !Generate3DForOneView( dir.str(), debug ) )
                //{
                //    return -1;
                //}
			}
			break;

			case AUTO_SCAN:
			{
				char name[256];

				std::cout << "Please enter the project name: ";
				std::cin.getline( name, 256 );

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

                int fullCircle = 360; // degree, full circle
                int delta = 30; // one turn
                int steps = fullCircle / delta;

				// each step capture
				bool captureRoi = false;

                for( int i = 0, curDeg = 0; i < steps; ++i, curDeg += delta )
				{
                    cout << "================================ \n";
                    cout << "= Scanning " << i + 1 << "/" << steps << " view ... \n";
                    cout << "================================ \n";

					std::stringstream dir;
					dir << name << curDeg << "/";
					_mkdir( dir.str().c_str() );

					captureRoi = i == 0;

					if ( !ScanOneView( dir.str(), captureRoi ) )
					{
						return -1;
					}//if

                    // now rotate turn table
                    if( !OneTurn( delta, speed ) )
                    {
                        cout << "turn error\n";
                        return -1;
                    }

					// pause some time for the table to start turning
					//cv::waitKey( 0 );

					// wait until the turn is done
					// TODO: this is not working for now??
					bool done( false );
					string isDone = "IsDone";
					while ( !done )
					{
						char data[7];
						int bytesRead = serialPort->ReadSerialPort<char>( data, 7 );
						if ( bytesRead > 0 )
						{
							done = true;
							//debug
							cout << data << endl;
						}
					}

                    // alternative method; blindly wait for some period
					//cv::waitKey(duration);

				}//for i

				//// now generate
    //            for( int i = 0, curDeg = 0; i < steps; ++i, curDeg += delta )
    //            {
    //                cout << "Generating " << i + 1 << "/" << steps << " view ... \n";

				//	std::stringstream dir;
				//	dir << name << curDeg << "/";
                //const bool debug = false;
                //	if ( !Generate3DForOneView( dir.str(),debug ) )
				//	{
				//		return -1;
				//	}
				//}//for i
			}
			break;

            case AUTO_GENERATE_3D_VIEWS:
            {
                char name[256];

                std::cout << "Please enter the project name: ";
                std::cin.getline( name, 256 );

                int fullCircle = 360; // degree, full circle
                int delta = 20; // one turn
                int steps = fullCircle / delta;

                // now generate
                for( int i = 0, curDeg = 0; i < steps; ++i, curDeg += delta )
                {
                    cout << "================================ \n";
                    cout << "= Generating " << i + 1 << "/" << steps << " view ... \n";
                    cout << "================================ \n";

                    std::stringstream dir;
                    dir << name << curDeg << "/";
                    const bool debug = false;
                    if( !Generate3DForOneView( dir.str(), debug ) )
                    {
                        return -1;
                    }
                }//for i
            }
            break;

            case REGENERATE_PLY_WITH_TEXTURE:
            {
				char name[256];

				std::cout << "Please enter the project name: ";
				std::cin.getline( name, 256 );

				stringstream stream;
				stream.str( name );

				string xyz = stream.str() + "/results.xyz";
                string texture = stream.str() + "/texture.jpg";
                string ply = stream.str() + "/newResult.ply";
                Exporter::GeneratePlyFromXYZ( xyz, texture, ply );
            }
            break;

			case TURN_TABLE:
			{
				char pos[256];

				std::cout << "Please enter how many degrees you want the table to turn: ";
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

				// pause some time for the table to start turning
				cv::waitKey(2000);

				// wait until the turn is done
				// TODO: this is not working for now??
				bool done( false );
				string isDone = "IsDone";
				while ( !done )
				{
					char data[7];
					int bytesRead = serialPort->ReadSerialPort<char>( data, 7 );
					if ( bytesRead > 0 )
					{
						done = true;
						//debug
						cout << data << endl;
					}
				}
			}
			break;

			case CAPTURE_LEFT:
			{
				// capture
				if ( !CaptureLeft() )
				{
					return -1;
				}
			}
			break;

			case CAPTURE_RIGHT:
			{
				// capture
				if ( !CaptureRight() )
				{
					return -1;
				}
			}
			break;

			case CAPTURE_LEFT_AND_RIGHT:
			{
				// capture
				if ( !CaptureLeftAndRight() )
				{
					return -1;
				}
			}
			break;

            case CAPTURE_ROI_FOR_DATA:
            {
                char name[256];
                std::cout << "Please enter the project name: ";
                std::cin.getline( name, 256 );

                std::stringstream dir;
                dir << name << "/";

                CaptureROIForData( dir.str() );
            }
            break;

            case CALIBRATE_LEFT:
            {
                if( !CalibrateLeft() )
                {
                    return -1;
                }
            }
            break;

            case CALIBRATE_RIGHT:
            {
                if( !CalibrateRight() )
                {
                    return -1;
                }
            }
            break;

            case CALIBRATE_LEFT_AND_RIGHT:
            {
                if( !CalibrateLeftAndRight() )
                {
                    return -1;
                }
            }
            break;

            case GENERATE_3D_ONE_VIEW:
            {
                char name[256];
                std::cout << "Please enter the project name: ";
                std::cin.getline( name, 256 );

                std::stringstream dir;
                dir << name << "/";

                const bool debug = false;
                if( !Generate3DForOneView( dir.str(), debug ) )
                {
                    return -1;
                }
            }
            break;

            case DEBUG_DECODING:
            {
                char name[256];
                std::cout << "Please enter the project name: ";
                std::cin.getline( name, 256 );

                std::stringstream dir;
                dir << name << "/";

                const bool debug = true;

                if( !Generate3DForOneView( dir.str(), debug ) )
                {
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
	cout << "Choose from the following options:\n";
	cout << "=============================================================================\n";
	cout << "[1]: Live View\n";
	cout << "[2]: capture & calibrate left camera ( Esc to terminate )\n";
	cout << "[3]: capture & calibrate right camera  ( Esc to terminate )\n";
	cout << "[4]: capture & calibrate both left & right camera  ( Esc to terminate )\n";
	cout << "[5]: Manual Scan one view \n";
	cout << "[6]: Auto Scan with turntable \n";
	cout << "[7]: Turn table \n";
	cout << "[8]: Write Config file \n";
    cout << "[9]: Regenerate Ply with texture file \n";
	cout << "[10]: Exit \n\n";
	cout << "below hidden but still works \n";
	cout << "( [11]: capture left camera\n"; // obsolete
	cout << "  [12]: capture right camera\n"; // obsolete
	cout << "  [13]: capture left & right camera\n"; // obsolete
    cout << "  [14]: calibrate left camera\n"; // obsolete
    cout << "  [15]: calibrate right camera\n"; // obsolete
    cout << "  [16]: calibrate both left & right camera\n"; // obsolete
    cout << "  [17]: Generate 3D of scanned one view ) \n"; // obsolete
    cout << "  [18]: Auto-generate 3D of scanned views ) \n";
    cout << "  [19]: Debug decoding process \n";
    cout << "  [20]: Create ROI for scanned data \n\n";

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
    int com = 6; // com port
    int speed = 1500; // speed
	int duration = 1500;//ms

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
	processor.Init();

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
	processor.ReadRoi();

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
	}//switch intype

	switch ( outType )
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
	}//switch ( outType )

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
	vector<string> title,
    const bool debug )
{
	processor.Init();

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
	default:
		break;
	}//switch inType

	switch ( outType )
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
	}//switch ( outType )

	processor.ReadRoi();

    processor.Generate3D( debug );

	return true;
}//Calculate3DPrepare

//===========================
bool CaptureLeft()
{
    string path0 = "cali_data";
    _mkdir( path0.c_str() );

	string path = "cali_data/left/";
	_mkdir( path.c_str() );

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	ids.push_back( LiveViewProcessor::LEFT_CAM );
	vector<string> title;
	title.push_back( "Left Cam" );
	vector<string> fileName;
	fileName.push_back( "cali_left" );

	processor.SetNumCaliImgs( 0 );// reset

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
	const bool captureRoi = false;
	// Start the Process
	processor.CaptureAndClibrate( captureRoi );

	return true;
}//CaptureLeft

//==========================
bool CaptureRight()
{
	string path0 = "cali_data";
	_mkdir( path0.c_str() );

    string path = "cali_data/right/";
    _mkdir( path.c_str() );

	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	ids.push_back( LiveViewProcessor::RIGHT_CAM );
	vector<string> title;
	title.push_back( "Right Cam" );
	vector<string> fileName;
	fileName.push_back( "cali_right" );

	processor.SetNumCaliImgs( 0 );// reset

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

	const bool captureRoi = false;

	// Start the Process
	processor.CaptureAndClibrate( captureRoi );
	return true;
} // CaptureRight

//=====================
bool CaptureLeftAndRight()
{
	string path0 = "cali_data";
	_mkdir( path0.c_str() );

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

	processor.SetNumCaliImgs( 0 );// reset

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

	const bool captureRoi = false;

	// Start the Process
	processor.CaptureAndClibrate( captureRoi );
	return true;
}//CaptureLeftAndRight

//-=================================
bool LiveView()
{
	vector<LiveViewProcessor::WEB_CAM_ID> ids;
	//ids.push_back( LiveViewProcessor::DEFAULT_CAM );
	ids.push_back( LiveViewProcessor::LEFT_CAM );
	ids.push_back( LiveViewProcessor::RIGHT_CAM );

	vector<string> title;
	title.push_back( "Left Cam" );
	title.push_back( "Right Cam" );
	vector<string> name;
	const bool ok = CaptureAndOrCali(
		"",
		"",
		name,
		name,
		WEBCAM,
		NONE,
		ids,
		title );
	if ( !ok )
	{
		return false;
	}

	// Start the Process
	processor.LiveView();
	return true;
}//LiveView

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

	vector<string> inFileName;
	inFileName.push_back( "cali_left" );

	vector<string> outFileName;
	outFileName.push_back( "cali_left" );

	const bool ok = CaptureAndOrCali(
        path,
        path,
		inFileName, // input name
		outFileName, // output name
		IMG,
		IMG,
		ids,
		title);

	if ( !ok )
	{
		return false;
	}

	const bool captureRoi = false;

	// Start the Process
	processor.CaptureAndClibrate( captureRoi );
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

	vector<string> inFileName;
	inFileName.push_back( "cali_right" );

	vector<string> outFileName;
	outFileName.push_back( "cali_right" );

	const bool ok = CaptureAndOrCali(
        path,
        path,
		inFileName,
		outFileName,
		IMG,
		IMG,
		ids,
		title);
	if ( !ok )
	{
		return false;
	}

	const bool captureRoi = false;

	// Start the Process
	processor.CaptureAndClibrate( captureRoi );
	return true;
} // CalibrateRight

  //==============================
bool CaptureROIForData( string path )
{
    if( DirExists( path.c_str() ) != EXISTS )
    {
        cout << "Need to scan left cam data first!/n";
        return false;
    }

    vector<string> inputFileName;
    inputFileName.push_back( "scan_left" );
    inputFileName.push_back( "scan_right" );

    vector<string> title;
    title.push_back( "Left Cam" );
    title.push_back( "Right Cam" );

    processor.Init();

    delay = 1; // ms
    const int numDigit = 2;
    const int numSource = 2;
    const float scaleFactor = 0.4f;

    processor.SetNumSource( numSource ); // num of camera
    processor.SetDelay( delay );
    processor.SetInputFileName( inputFileName );
    processor.SetFileNameNumDigits( numDigit );
    processor.SetPath( path );
    processor.DisplayOutput( title );
    processor.SetScaleFactorForShow( scaleFactor );

    const int startFrame = 0;// frame number we want to start at
    const int endFrame = processor.ReadNumPatterns(); // assuming previously we have captured the cali imgs

    vector<vector<string>> imgs;

    for( int j = 0; j < numSource; j++ )
    {
        vector<string> tmp;
        for( int i = 0; i < endFrame; i++ )
        {
            char buffer[100];
            sprintf_s( buffer, "%s%s%02i.jpg", path.c_str(), inputFileName[j].c_str(), i );

            string imgName = buffer;
            tmp.push_back( imgName );
        }
        imgs.push_back( tmp );
    }

    processor.SetInput( imgs );

    const bool captureRoi = true;

    // Start the Process
    processor.CaptureAndClibrate( captureRoi );

    //==============
    return true;
}//CaptureROIForData

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
		title);

	if ( !ok )
	{
		return false;
	}

	const bool captureRoi = false;

	// Start the Process
	processor.CaptureAndClibrate( captureRoi );
	return true;
}//CalibrateLeftAndRight

//==========================
bool ScanOneView( string path, const bool captureRoi )
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

	processor.SetColImgOnly( colImgOnly );
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
	processor.Scan( captureRoi );
	return true;
} // ScanOneView

//=========================
bool Generate3DForOneView( string path, const bool debug )
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

	processor.Init();
	processor.SetColImgOnly( colImgOnly );
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
		IMG,
		title,
        debug );

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

    return ok;
}//OneTurn
//
//void TestPcl()
//{
//	pcl::PointCloud<pcl::PointXYZ> cloud;
//
//	// Fill in the cloud data
//	cloud.width = 5;
//	cloud.height = 1;
//	cloud.is_dense = false;
//	cloud.points.resize( cloud.width * cloud.height );
//
//	for ( size_t i = 0; i < cloud.points.size(); ++i )
//	{
//		cloud.points[i].x = 1024 * rand() / ( RAND_MAX + 1.0f );
//		cloud.points[i].y = 1024 * rand() / ( RAND_MAX + 1.0f );
//		cloud.points[i].z = 1024 * rand() / ( RAND_MAX + 1.0f );
//	}
//
//	pcl::io::savePCDFileASCII( "test_pcd.pcd", cloud );
//	std::cerr << "Saved " << cloud.points.size() << " data points to test_pcd.pcd." << std::endl;
//
//	for ( size_t i = 0; i < cloud.points.size(); ++i )
//		std::cerr << "    " << cloud.points[i].x << " " << cloud.points[i].y << " " << cloud.points[i].z << std::endl;
//
//}
//
////====================
//void ICPTest()
//{
//    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_in( new pcl::PointCloud<pcl::PointXYZ> );
//    pcl::PointCloud<pcl::PointXYZ>::Ptr cloud_out( new pcl::PointCloud<pcl::PointXYZ> );
//
//    // Fill in the CloudIn data
//    cloud_in->width = 5;
//    cloud_in->height = 1;
//    cloud_in->is_dense = false;
//    cloud_in->points.resize( cloud_in->width * cloud_in->height );
//    for( size_t i = 0; i < cloud_in->points.size(); ++i )
//    {
//        cloud_in->points[i].x = 1024 * rand() / ( RAND_MAX + 1.0f );
//        cloud_in->points[i].y = 1024 * rand() / ( RAND_MAX + 1.0f );
//        cloud_in->points[i].z = 1024 * rand() / ( RAND_MAX + 1.0f );
//    }
//    std::cout << "Saved " << cloud_in->points.size() << " data points to input:"
//        << std::endl;
//
//    for( size_t i = 0; i < cloud_in->points.size(); ++i )
//    {
//        std::cout << "    " <<
//            cloud_in->points[i].x << " " << cloud_in->points[i].y << " " <<
//            cloud_in->points[i].z << std::endl;
//    }
//
//    *cloud_out = *cloud_in;
//    std::cout << "size:" << cloud_out->points.size() << std::endl;
//
//    for( size_t i = 0; i < cloud_in->points.size(); ++i )
//    {
//        cloud_out->points[i].x = cloud_in->points[i].x + 0.7f;
//    }
//
//    std::cout << "Transformed " << cloud_in->points.size() << " data points:"
//        << std::endl;
//
//    for( size_t i = 0; i < cloud_out->points.size(); ++i )
//    {
//        std::cout << "    " << cloud_out->points[i].x << " " <<
//            cloud_out->points[i].y << " " << cloud_out->points[i].z << std::endl;
//    }
//
//    pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
//    icp.setInputSource( cloud_in );
//    icp.setInputTarget( cloud_out );
//    pcl::PointCloud<pcl::PointXYZ> Final;
//    icp.align( Final );
//    std::cout << "has converged:" << icp.hasConverged() << " score: " <<
//        icp.getFitnessScore() << std::endl;
//    std::cout << icp.getFinalTransformation() << std::endl;
//
//}
#include <windows.h> // WinApi header
#include <opencv2/opencv.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/calib3d.hpp>
#include <opencv2/core.hpp>

#include "Calibrator.h"
//=======================================================================
// helper function to show type
std::string type2str( int type )
{
	std::string r;

	uchar depth = type & CV_MAT_DEPTH_MASK;
	uchar chans = 1 + ( type >> CV_CN_SHIFT );

	switch ( depth )
	{
	case CV_8U:  r = "8U"; break;
	case CV_8S:  r = "8S"; break;
	case CV_16U: r = "16U"; break;
	case CV_16S: r = "16S"; break;
	case CV_32S: r = "32S"; break;
	case CV_32F: r = "32F"; break;
	case CV_64F: r = "64F"; break;
	default:     r = "User"; break;
	}

	r += "C";
	r += ( chans + '0' );

	return r;
}//std::string type2str(int type)

//=======================================================================
Calibrator::Calibrator()
	: LiveViewProcessor()
	, m_NumCaliImgs(0)
	, m_CalibPatternWidth( 0 )
	, m_CalibPatternHeight( 0 )
	, m_CaptureAndCali( false )
	, m_DoCali( false )
{}

//=======================================================================
void Calibrator::LiveView()
{
	// current frame
	vector<cv::Mat> frame;

	// output frame
	vector<cv::Mat> output;

	// if no m_Capture device has been set
	if ( !IsOpened() )
	{
		return;
	}

	for ( int i = 0; i < m_NumSource; i++ )
	{
		frame.push_back( cv::Mat() );
		output.push_back( cv::Mat() );
	}

	m_Stop = false;
	bool ok( true );

	while ( !IsStopped() )
	{
		// read next frame if any
		ok = ok && ReadNextFrame( frame );
		if ( !ok )
		{
			break;
		}

		DisplayFrame( m_WindowNameInput, frame );

		output = frame;

		DisplayFrame( m_WindowNameOutput, output );

		// check if we should stop
		if ( m_FrameToStop >= 0 && GetFrameNumber() == m_FrameToStop )
		{
			StopIt();
		}
		int key = cv::waitKey( 1 );
		if ( key == 27 )
		{
			StopIt();
		}
	} // while ( !IsStopped() )

	// after all the images are processed
	// close window
	for ( int i = 0; i < m_NumSource; i++ )
	{
		cv::destroyWindow( m_WindowNameOutput[i] );
	}

} // LiveView

//=======================================================================
void Calibrator::CaptureAndClibrate()
{
	// current frame
	vector<cv::Mat> frame;

	// output frame
	vector<cv::Mat> output;

	// if no m_Capture device has been set
	if ( !IsOpened() )
	{
		return;
	}

    for( int i = 0; i < m_NumSource; i++ )
    {
        frame.push_back( cv::Mat() );
        output.push_back( cv::Mat() );
    }

	m_Stop = false;
	bool ok( true );

	while ( !IsStopped() )
	{
		// read next frame if any
		ok = ok && ReadNextFrame( frame );
		if ( !ok )
		{
			break;
		}

        DisplayFrame( m_WindowNameInput, frame );

		// check whether capture or start calibration
		if ( !CaptureOptions( frame, output ) )
		{
			return;
		}

		output = frame;

        DisplayFrame( m_WindowNameOutput, output );

		// check if we should stop
		if ( m_FrameToStop >= 0 && GetFrameNumber() == m_FrameToStop )
		{
			StopIt();
		}
	} // while ( !IsStopped() )

	// after all the images are processed
	// close window
	for ( int i = 0; i < m_NumSource; i++ )
	{
		cv::destroyWindow( m_WindowNameOutput[i] );
	}

	if ( ok && m_DoCali )
	{
        Calibrate();
        WriteNumCaliImgs();
	}
} // CaptureAndClibrate

//=======================================================================
void Calibrator::Generate3D( const bool debug )
{
	// read calibration result of each camera
	string leftPath = "cali_data/left/";
	string rightPath = "cali_data/right/";
	cv::Mat M1, M2, D1, D2;

	ReadSingleCamCaliResults( leftPath, M1, D1 );
	ReadSingleCamCaliResults( rightPath, M2, D2 );

	// read calibration result of the stereo rig
	string seteroPath = "cali_data/leftAndRight/";
	cv::Mat R, T, E, F;

	ReadStereoCamCaliResults( seteroPath, R, T, E, F );

	// read one picture to get image size
	cv::Mat tmp = cv::imread( *( m_ItImg[0] ) );
	m_ImgSiz = tmp.size();

	// Stereo Rectify
	cv::Mat R1, R2, P1, P2, Q;
	cv::Rect validRoi[2];

	cv::stereoRectify( M1, D1, M2, D2, m_ImgSiz, R, T, R1, R2, P1, P2, Q, 0,
		-1, m_ImgSiz, &validRoi[0], &validRoi[1] );

	if ( false )
	{
		// get mat type
		std::string typeName = type2str( Q.type() );
		cout << typeName.c_str() << endl; // 64FC1
	}

	cv::Mat map1x, map1y, map2x, map2y;
	initUndistortRectifyMap( M1, D1, R1, P1, m_ImgSiz, CV_32FC1, map1x, map1y );
	initUndistortRectifyMap( M2, D2, R2, P2, m_ImgSiz, CV_32FC1, map2x, map2y );

	//==============================================================
	// Now process each of the captured pattern, by rectifying them

	// current frame
	vector<cv::Mat> frame;

	// output frame
	vector<cv::Mat> output;

	for ( int i = 0; i < m_NumSource; i++ )
	{
		frame.push_back( cv::Mat() );
		output.push_back( cv::Mat() );
	}

	m_Stop = false;
	bool ok( true );

	vector<cv::Mat> left;
	vector<cv::Mat> right;
	string rectifyWinName = "Rectified";

	cv::Mat colrImg;
	int counter = 0;

	while ( !IsStopped() )
	{
		// read next frame if any
		ok = ok && ReadNextFrame( frame );
		if ( !ok )
		{
			break;
		}

		if ( false ) // no need to display input images
		{
			DisplayFrame( m_WindowNameInput, frame );
		}

		// rectify images
		cv::remap( frame[0], output[0], map1x, map1y, cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar() );
		cv::remap( frame[1], output[1], map2x, map2y, cv::INTER_NEAREST, cv::BORDER_CONSTANT, cv::Scalar() );

		if ( counter == m_GrayCode.GetNumPatternImgs() )
		{
			//save the "white" image
			colrImg = output[0].clone(); // 8UC3

            std::stringstream texturePath;
            texturePath << m_Path << "texture" << m_Extension;
            cv::imwrite( texturePath.str(), colrImg );
		}

		// save tmp for composite purpose
		cv::Mat tmpLeft = output[0].clone();
		cv::Mat tmpRight = output[1].clone();

		// convert to gray scale
		cv::cvtColor( output[0], output[0], cv::COLOR_BGRA2GRAY );
		cv::cvtColor( output[1], output[1], cv::COLOR_BGRA2GRAY );

		if ( false )
		{
			// get mat type
			std::string typeName = type2str( output[0].type() );
			cout << typeName.c_str() << endl;
		}

		left.push_back( output[0].clone() ); // left
		right.push_back( output[1].clone() ); // right

		if ( false )//no need to individually display rectified imgs
		{
			DisplayFrame( m_WindowNameOutput, output );
		}

		if ( true )
		{
			DisplayAndSaveComposite( rectifyWinName, tmpLeft/*8UC3*/, tmpRight );
		}

		// check if we should stop
		if ( m_ItImg[0] == m_Images[0].end() )
		{
			StopIt();
		}

		counter++;
	} // while ( !IsStopped() )

	// after all the images are processed
	// close window
	for ( int i = 0; i < m_NumSource; i++ )
	{
		cv::destroyWindow( m_WindowNameOutput[i] );
	}
	cv::destroyWindow( rectifyWinName );

	//sanity check
	int patternSize = static_cast<int>( left.size() );
	patternSize -= 2;

	if ( patternSize != m_GrayCode.GetNumPatternImgs() )
	{
		cout << "number pattern error/n";
		return;
	}

	// last 2 imgs are white & black img, for shadow calculation use
	vector<cv::Mat> white;
	white.push_back( left[patternSize] );
	white.push_back( right[patternSize] );

	vector<cv::Mat> black;
	black.push_back( left[patternSize + 1] );
	black.push_back( right[patternSize + 1] );

	//debug
	if ( false )
	{
		cv::imshow( "white left", left[patternSize] );
		cv::imshow( "white right", right[patternSize] );
		cv::imshow( "black left", left[patternSize + 1] );
		cv::imshow( "black right", right[patternSize + 1] );
	}
	//=========

	left.pop_back();	left.pop_back();
	right.pop_back();	right.pop_back();

	vector<vector<cv::Mat>> captured;
	captured.push_back( left );
	captured.push_back( right );

	//restructure captured such that its size is like 2 (left & right) x NumPatternImgs x images
	m_GrayCode.Decode( captured, white/*8UC1*/, black, debug );

	vector<cv::Point3d> pointcloud;
	vector<cv::Point3i> colors;
    vector<cv::Vec2f> textureCoord;

	cv::Mat disp = m_GrayCode.GetDisparityMap(); // CV_32S

	ReprojectImageTo3D( colrImg, disp, Q, pointcloud, colors, textureCoord ); // Q is 64FC1

	// export

    std::stringstream objPath;
    objPath << m_Path << "results.obj";
    std::stringstream plyPath;
    plyPath << m_Path << "results.ply";

    // don't export wrl. No mesh yet!
    std::stringstream xyzPath;
	xyzPath << m_Path << "results.xyz";

	Exporter::ExportToObj( pointcloud, objPath.str() );
	Exporter::ExportToPly( pointcloud, colors, plyPath.str() );
    Exporter::SaveXYZAndTexture( pointcloud, textureCoord, xyzPath.str() );
} // Generate3D

//=======================================================================
void Calibrator::ReprojectImageTo3D(
	const cv::Mat& colorImg,
	const cv::Mat& disp , // CV_32S
	const cv::Mat& Q, // 4x4, 64FC1
	vector<cv::Point3d>& pointcloud,
	vector<cv::Point3i>& colors,
    vector<cv::Vec2f>& textureCoord )
{
	cv::Mat tmp = cv::Mat( 4, 1, CV_64F );

	for ( int r = 0; r < disp.rows; ++r )
	{
		for ( int c = 0; c < disp.cols; ++c )
		{
			if ( disp.at<int>( r, c ) == 0 )
			{
				continue;
			}

			tmp.at<double>( 0, 0 ) = static_cast<double>( c );
			tmp.at<double>( 1, 0 ) = static_cast<double>( r );
			tmp.at<double>( 2, 0 ) = static_cast<double>( disp.at<int>( r, c ) );
			tmp.at<double>( 3, 0 ) = 1.0;

			tmp = Q * tmp;

			tmp /= tmp.at<double>( 3, 0 );

            // geometry info
			cv::Vec3d point;
			point[0] = tmp.at<double>( 0, 0 );
			point[1] = -tmp.at<double>( 1, 0 ); // flip y
			point[2] = tmp.at<double>( 2, 0 );

			pointcloud.push_back( point );

            // color info
			cv::Point3i color;
			color.x = int( colorImg.at<cv::Vec3b>( r, c )[2] );//r
			color.y = int( colorImg.at<cv::Vec3b>( r, c )[1] );//g
			color.z = int( colorImg.at<cv::Vec3b>( r, c )[0] );//b
			colors.push_back( color );

            // texture coordinate
            cv::Vec2f coord;
            float s = static_cast<float>( c ) / static_cast<float>( disp.cols - 1.0f );
            float t = 1.0f - static_cast<float>( r ) / static_cast<float>( disp.rows - 1.0f );
            coord[0] = s;
            coord[1] = t;

            textureCoord.push_back( coord );
		}//for c
	}//for r
}//ReprojectImageTo3D

//=======================================================================
void Calibrator::Scan()
{
	// Setting pattern window on second monitor (the projector's one)
	cv::namedWindow( m_ProjWinName, cv::WINDOW_NORMAL );
	cv::resizeWindow( m_ProjWinName, m_GrayCode.GetWidth(), m_GrayCode.GetHeight() );
	int mainScrnWidth = 1920;
    int yOffset = -35; // window top bar height
	cv::moveWindow( m_ProjWinName, mainScrnWidth, yOffset );
	cv::setWindowProperty( m_ProjWinName, cv::WND_PROP_FULLSCREEN, cv::WINDOW_FULLSCREEN );

	const vector<cv::Mat>& pattern = m_GrayCode.GetPattern();

    int patSiz = static_cast<int>( pattern.size() );

    //for debug; save the patterns
    const bool debug = false;
    if( debug )
    {
        for( int i = 0; i < patSiz; i++ )
        {
            WriteImg( "pattern", pattern[i], i );
        }
    }
    //====================

    for( int i = 0; i < patSiz; i++ )
	{
		// show the projector pattern
		imshow( m_ProjWinName, pattern[i] );

		vector<cv::Mat> frame;
		for ( int j = 0; j < m_NumSource; j++ )
		{
			frame.push_back( cv::Mat() );
		}

        // pause a bit for the pattern to be shown
        int pauseMs = 200; // in ms
        cv::waitKey( pauseMs );

		bool ok = ReadNextFrame( frame );
		if ( !ok )
		{
			cout << "can't read video/n";
			break;
		}

        DisplayFrame( m_WindowNameOutput, frame );

		bool manualAccept = false;
		if ( manualAccept )
		{
			cout << "Waiting to save image number " << i + 1 << endl << "Press any key to acquire the photo" << endl;

			int key = cv::waitKey( 0 );
			// Pressing enter, it saves the output
			if ( key == 13 )
			{
				for ( int j = 0; j < m_NumSource; j++ )
				{
                    WriteImg( m_OutputFileName[j], frame[j], i );
				}
			}

			// Pressing escape, the program closes
			if ( key == 27 )
			{
				cout << "Closing program" << endl;
			}
		} // if ( manualAccept )
		else
		{
			//auto shoot
            for( int j = 0; j < m_NumSource; j++ )
            {
                WriteImg( m_OutputFileName[j], frame[j], i );
            }

		}//if ( manualAccept )

	} // for i

	// save number of patterns
	WriteNumPatterns();
}//Scan

//=======================================================================
bool Calibrator::CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output )
{
	m_ImgSiz = frame[0].size();

	int ret = cv::waitKey( m_Delay );
	if ( m_DoCali )
	{
        // frame is already the loaded from saved images, so we do cali
        const bool writeImg = false;
        if( !FindChessboard( frame, writeImg ) )
        {
            cout << *( m_ItImg[0] ) << ": can't find chessboard\n";
            cout << *( m_ItImg[1] ) << ": can't find chessboard\n";
        }

        if( m_ItImg[0] == m_Images[0].end() )
        {
            StopIt();
        }
	}
	else
	{
		if ( ret == 67/*'C'*/ || ret == 99 /*'c'*/ )
		{
			Beep( 523, 500 ); // 523 hertz (C5) for 500 milliseconds

            for( int i = 0; i < m_NumSource; i++ )
            {
                // copy to output first
                output[i] = frame[i].clone();
            }

            if( m_CaptureAndCali )
            {
                const bool writeImg = true;
                if( !FindChessboard( frame, writeImg ) )
                {
                    cout << *( m_ItImg[0] ) << ": can't find chessboard\n";
                    cout << *( m_ItImg[1] ) << ": can't find chessboard\n";
                }

				m_NumCaliImgs++;
            }//if ( m_CaptureAndCali )
            else
            {
                for( int i = 0; i < m_NumSource; i++ )
                {
                    WriteImg( m_OutputFileName[i], frame[i], m_NumCaliImgs );
                }

				m_NumCaliImgs++;
            }//if ( m_CaptureAndCali )
		}
		else if ( ret == 27/*ESC*/ )
		{
            // close window
            for( int i = 0; i < m_NumSource; i++ )
            {
                cv::destroyWindow( m_WindowNameOutput[i] );
            }

			if ( m_CaptureAndCali )
			{
				Calibrate();

			}//if ( m_CaptureAndCali )
			else
			{
				WriteNumCaliImgs();
			}//if ( m_CaptureAndCali )

			StopIt();
		}
	} // if ( !m_DoCali )

	return true;
} // CaptureOptions

//===============================================
void Calibrator::WriteSingleCamCaliResults( const double err )
{
	string s = m_Path + "CaliResult.xml";
	cv::FileStorage fs( s, cv::FileStorage::WRITE );

	fs << "ImageWidth" << m_ImgSiz.width << "ImageHeight" << m_ImgSiz.height
		<< "IntrinsicMatrix" << m_IntrinsicMat
		<< "DistortionCoeffs" << m_DistCoeff
		<< "NumCaliImgs" << m_NumCaliImgs
		<< "ReprojectionError" << err;

	fs.release();
}//WriteSingleCamCaliResults

//===============================================
void Calibrator::ReadSingleCamCaliResults(
	const string path,
	cv::Mat& intrinsicMat,
	cv::Mat& distCoeff )
{
	string s = path + "CaliResult.xml";
	cv::FileStorage fs( s, cv::FileStorage::READ );

	fs["IntrinsicMatrix"] >> intrinsicMat;
	fs["DistortionCoeffs"] >> distCoeff;

	fs.release();
}//ReadSingleCamCaliResults

//===============================================
void Calibrator::WriteStereoCamCaliResults( const double err )
{
	string s = m_Path + "StereoCaliResults.xml";
	cv::FileStorage fs( s, cv::FileStorage::WRITE );
	fs << "R" << m_R;
	fs << "T" << m_T;
	fs << "E" << m_E;
	fs << "F" << m_F;
	fs << "reprojectionErr" << err;
	fs.release();
}//WriteStereoCamCaliResults

//===============================================
void Calibrator::ReadStereoCamCaliResults(
	const string path,
	cv::Mat& R,
	cv::Mat& T,
	cv::Mat& E,
	cv::Mat& F )
{
	string s = path + "StereoCaliResults.xml";
	cv::FileStorage fs( s, cv::FileStorage::READ );

	fs["R"] >> R;
	fs["T"] >> T;
	fs["E"] >> E;
	fs["F"] >> F;

	fs.release();
} // ReadStereoCamCaliResults

//===============================================
void Calibrator::WriteNumPatterns()
{
	string s = m_Path + "NumPatterns.xml";
	cv::FileStorage fs( s, cv::FileStorage::WRITE );
	fs << "NumPatterns" << m_GrayCode.GetNumPatternImgs() + 2 /*black & white*/;
	fs.release();
}//WriteNumPatterns

//===============================================
int Calibrator::ReadNumPatterns()
{
	string s = m_Path + "NumPatterns.xml";
	cv::FileStorage fs( s, cv::FileStorage::READ );
	int m;
	fs["NumPatterns"] >> m;
	fs.release();

	return m;
}//WriteNumPatterns

//===============================================
void Calibrator::WriteNumCaliImgs()
{
	string s = m_Path + "NumCaliImgs.xml";
	cv::FileStorage fs( s, cv::FileStorage::WRITE );
	fs << "NumCaliImgs" << m_NumCaliImgs;
	fs.release();
}//WriteNumCaliImgs

//===============================================
void Calibrator::ReadNumCaliImgs()
{
	string s = m_Path + "NumCaliImgs.xml";
	cv::FileStorage fs( s, cv::FileStorage::READ );
	fs["NumCaliImgs"] >> m_NumCaliImgs;
	fs.release();
}//ReadNumCaliImgs

//===============================================
bool Calibrator::FindChessboard( const vector<cv::Mat>& imgs, const bool writeImg )
{
	cv::Size boardSize = cv::Size( m_CalibPatternWidth, m_CalibPatternHeight );

	vector<vector<cv::Point2f>> tmpCorners; // #source of #corerns in a img
	vector<cv::Mat> downImg; // down sampled img
	bool found( true );

    for( int i = 0; i < m_NumSource; i++ )
    {
		vector<cv::Point2f> corners;
		const cv::Mat& img = imgs[i];

        found = found && cv::findChessboardCorners( img, boardSize, corners );

        if( found )
        {
            cv::Mat gray;
            cv::cvtColor( img, gray, cv::COLOR_BGRA2GRAY );
            cv::cornerSubPix( gray, corners, cv::Size( 11, 11 ), cv::Size( -1, -1 ),
                cv::TermCriteria( cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1 ) );

			tmpCorners.push_back( corners );
            // show the corners on output window

            // show a downsampled img
            cv::Mat tmp = img.clone();
            cv::resize( tmp, tmp, cv::Size(), m_ScaleFactorForShow, m_ScaleFactorForShow, cv::INTER_LINEAR );

            //downsampled corners
            vector<cv::Point2f> downCorners;
            for( int i = 0; i < corners.size(); i++ )
            {
                cv::Point2f tmp(
                    static_cast<float>( corners[i].x * m_ScaleFactorForShow ),
                    static_cast<float>( corners[i].y * m_ScaleFactorForShow ) );

				downCorners.push_back( tmp );
            }

            // draw it
            cv::drawChessboardCorners( tmp, boardSize, downCorners, found );
            cv::imshow( m_WindowNameOutput[i], tmp );

			downImg.push_back( tmp );

        }//if ( found )
    } // for

	if ( found )
	{
        // cache image points
        m_ImagePoints.push_back( tmpCorners ); // #caliImgs of #source of #corerns in a img
        m_NumCaliImgs++;

        for( int i = 0; i < m_NumSource; i++ )
        {
            // we take it, now store the parameters and image

            if( writeImg )
            {
                WriteImg( m_OutputFileName[i], imgs[i], m_NumCaliImgs );
            }

            WriteCaliWithCirclesImg( m_OutputFileName[i], downImg[i] );
        }
	} // if ( found )

	return found;
}//FindChessboard

void Calibrator::Calibrate()
{
	// use all the captured imgs for calibration

	// create object points
	vector<vector<cv::Point3f>> objPt;
	const int boardSize = m_CalibPatternWidth * m_CalibPatternHeight;

	for ( int i = 0; i < m_NumCaliImgs; i++ )
	{
		vector<cv::Point3f> tmpPt;

		for ( int j = 0; j < boardSize; j++ )
		{
			cv::Point3f p(
				static_cast<float> ( j / m_CalibPatternWidth ),
				static_cast<float> ( j % m_CalibPatternWidth ), 0.0f );

			tmpPt.push_back( p );
		}

		objPt.push_back( tmpPt );
	}

	if ( m_NumSource == 1 )
	{
		//reorganize m_ImagePoints:  #caliImgs of #source of #corerns in a img
		vector<vector<cv::Point2f>> imgPts;
		int numImgs = static_cast<int>( m_ImagePoints.size() );

		for ( int i = 0; i < numImgs; i++ )
		{
			imgPts.push_back( m_ImagePoints[i][0] );
		}

		// calibrate the camera
		double err = cv::calibrateCamera(
			objPt, imgPts, m_ImgSiz, m_IntrinsicMat,
			m_DistCoeff, cv::noArray(), cv::noArray(),
			cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT );

		// SAVE THE INTRINSICS AND DISTORTIONS
		cout << " *** DONE!\n\nReprojection error is " << err
			<< "\nStoring caliResult.xml \n\n";

		WriteSingleCamCaliResults(err);
		//
		// Build the undistort map which we will use for all
		// subsequent frames.
		cv::Mat map1, map2;
		cv::initUndistortRectifyMap( m_IntrinsicMat, m_DistCoeff,
			cv::Mat(), m_IntrinsicMat, m_ImgSiz,
			CV_16SC2, map1, map2 );

		// undistort the images and save them
		for ( int i = 0; i < m_NumCaliImgs; i++ )
		{
			// read imgs
			std::stringstream ss;
			ss << m_Path << m_InputFileName[0]  << std::setfill( '0' ) << std::setw( m_Digits ) << i << m_Extension;
			cv::Mat orig = cv::imread( ss.str() );
			cv::Mat undistort;

			cv::remap( orig, undistort, map1, map2, cv::INTER_LINEAR,
				cv::BORDER_CONSTANT, cv::Scalar() );

			// save the undistorted imgs
			std::stringstream ss1;
			ss1 << m_Path << m_OutputFileName[0] << "_undist"<< std::setfill( '0' ) << std::setw( m_Digits ) << i << m_Extension;
			cv::imwrite( ss1.str(), undistort );
		} // for i
	}
	else
	{
		//reorganize m_ImagePoints:  #caliImgs of #source of #corerns in a img
		vector<vector<cv::Point2f> > imgPts[2];

		for ( int i = 0; i < m_NumCaliImgs; i++ )
		{
			imgPts[0].push_back( m_ImagePoints[i][0] ); // left
			imgPts[1].push_back( m_ImagePoints[i][1] ); // right
		}

		// stereo
		// read intrinsic matrix and distortion coeff of both cameras from perviously calibrated results
		string leftPath = "cali_data/left/";
		string rightPath = "cali_data/right/";

		cv::Mat M1, M2, D1, D2;

		ReadSingleCamCaliResults( leftPath, M1, D1 );
		ReadSingleCamCaliResults( rightPath, M2, D2 );

		double err = cv::stereoCalibrate(
			objPt, imgPts[0], imgPts[1], M1, D1, M2, D2, m_ImgSiz, m_R, m_T, m_E, m_F,
			cv::CALIB_FIX_INTRINSIC/*cv::CALIB_USE_INTRINSIC_GUESS*/,
			cv::TermCriteria( cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 100,
				1e-5 ) );

		WriteStereoCamCaliResults( err );

		// COMPUTE AND DISPLAY RECTIFICATION
		//
		const bool showUndistorted( true );

		if ( showUndistorted )
		{
			string undistWinName = "rectified";

			cv::Mat R1, R2, P1, P2, map11, map12, map21, map22;

			// IF BY CALIBRATED (BOUGUET'S METHOD)
			//

			cv::stereoRectify( M1, D1, M2, D2, m_ImgSiz, m_R, m_T, R1, R2, P1, P2,
				cv::noArray(), 0 );

			// Precompute maps for cvRemap()
			initUndistortRectifyMap( M1, D1, R1, P1, m_ImgSiz, CV_16SC2, map11,
				map12 );
			initUndistortRectifyMap( M2, D2, R2, P2, m_ImgSiz, CV_16SC2, map21,
				map22 );

			//=========
			// RECTIFY THE IMAGES AND FIND DISPARITY MAPS
			//
			cv::Mat pair;
			pair.create( m_ImgSiz.height, m_ImgSiz.width * 2, CV_8UC3 );

			// Setup for finding stereo corrrespondences
			//
			cv::Ptr<cv::StereoSGBM> stereo = cv::StereoSGBM::create(
				-64, 128, 11, 100, 1000, 32, 0, 15, 1000, 16, cv::StereoSGBM::MODE_HH );

			vector<cv::Mat> frame;
			for ( int j = 0; j < 2; j++ )
			{
				frame.push_back( cv::Mat() );
			}

			//reset img reader
			m_ItImg.clear();

			for ( int i = 0; i < m_NumSource; i++ )
			{
				// the input will be this vector of m_Images
				m_ItImg.push_back( m_Images[i].begin() );
			}

			for ( int i = 0; i < m_NumCaliImgs; i++ )
			{
				ReadNextFrame( frame );

				cv::Mat img1r, img2r;

				cv::Mat gray0, gray1;
				cv::cvtColor( frame[0], gray0, cv::COLOR_BGRA2GRAY );
				cv::cvtColor( frame[1], gray1, cv::COLOR_BGRA2GRAY );

				cv::remap( gray0, img1r, map11, map12, cv::INTER_LINEAR );
				cv::remap( gray1, img2r, map21, map22, cv::INTER_LINEAR );

				// When the stereo camera is oriented vertically,
				// Hartley method does not transpose the
				// image, so the epipolar lines in the rectified
				// images are vertical. Stereo correspondence
				// function does not support such a case.
				bool computeDisp = false;
				if ( computeDisp )
				{
                    cv::Mat disp, vdisp;
					stereo->compute( img1r, img2r, disp ); // warning: heavy lifting!

					cv::normalize( disp, vdisp, 0, 256, cv::NORM_MINMAX, CV_8U );
					cv::imshow( "disparity", vdisp );
				}

				cv::Mat part = pair.colRange( 0, m_ImgSiz.width );

				cv::cvtColor( img1r, part, cv::COLOR_GRAY2BGR );

				part = pair.colRange( m_ImgSiz.width, m_ImgSiz.width * 2 );

				cvtColor( img2r, part, cv::COLOR_GRAY2BGR );

				for ( int j = 0; j < m_ImgSiz.height; j += 16 )
				{
					cv::line( pair, cv::Point( 0, j ), cv::Point( m_ImgSiz.width * 2, j ),
						cv::Scalar( 0, 255, 0 ) );
				}

				cv::Mat downSize;
				cv::resize( pair, downSize, cv::Size(), m_ScaleFactorForShow, m_ScaleFactorForShow );

				cv::imshow( undistWinName, downSize );

				// save the image
				WriteImg( undistWinName, pair, i );

				cv::waitKey( 1 );
			}//for ( int i = 0; i < m_NumCaliImgs; i++ )
			cv::destroyWindow( undistWinName );

		}//if ( showUndistorted )
	}//if ( m_NumSource == 2 )
} // Calibrate

//=====================================================
void Calibrator::WriteImg(
    const string& fileName,
    const cv::Mat& img,
    const int idx )
{
    std::stringstream ss;
    ss << m_Path << fileName << std::setfill( '0' ) << std::setw( m_Digits ) << idx << m_Extension;
    cv::imwrite( ss.str(), img );
}//WriteImg

//=====================================================
void Calibrator::WriteCaliWithCirclesImg( const string& fileName, const cv::Mat& img )
{
    std::stringstream ss;
    ss << m_Path << fileName << "_pattern" << std::setfill( '0' ) << std::setw( m_Digits ) << m_NumCaliImgs << m_Extension;
    cv::imwrite( ss.str(), img );
}//WriteCaliWithCirclesImg

//=====================================================
void  Calibrator::DisplayFrame( const vector<string>& winName, const vector<cv::Mat>& output )
{
	// display output frame
	if ( winName.size() > 0 && !IsStopped() )
	{
		for ( int i = 0; i < m_NumSource; i++ )
		{
			if ( m_ScaleFactorForShow != 1.0f )
			{
				cv::Mat tmp;
				cv::resize( output[i], tmp, cv::Size(), m_ScaleFactorForShow, m_ScaleFactorForShow );
				cv::imshow( winName[i], tmp );
			}
			else
			{
				cv::imshow( winName[i], output[i] );
			}
			cv::moveWindow( winName[i], i * 750, 0 );
		}
	}
} // DisplayFrame

//=====================================================
void Calibrator::DisplayAndSaveComposite(
	const string& rectifyWinName,
	const cv::Mat& tmpLeft,
	const cv::Mat& tmpRight )
{
	//write a composite pair image
	cv::Mat pair;
	pair.create( m_ImgSiz.height, m_ImgSiz.width * 2, CV_8UC3 );

	cv::Mat part = pair.colRange( 0, m_ImgSiz.width );

	tmpLeft.copyTo( part ); // left

	part = pair.colRange( m_ImgSiz.width, m_ImgSiz.width * 2 );

	tmpRight.copyTo( part ); // right

	for ( int j = 0; j < m_ImgSiz.height; j += 16 )
	{
		cv::line( pair, cv::Point( 0, j ), cv::Point( m_ImgSiz.width * 2, j ),
			cv::Scalar( 0, 255, 0 ) );
	}

	if ( false )
	{
		cv::Mat downSize;
		cv::resize( pair, downSize, cv::Size(), m_ScaleFactorForShow, m_ScaleFactorForShow );

		cv::imshow( rectifyWinName, downSize );
		cv::moveWindow( rectifyWinName, 0, 0 );
	}

	// save the image
	WriteImg( rectifyWinName, pair, m_CurrentIndex[0] - 1 );
	m_CurrentIndex[0]++;

	//================================================
	cv::waitKey( m_Delay );

}//DisplayAndSaveComposite
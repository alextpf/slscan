#include <opencv2/opencv.hpp>
#include "Calibrator.h"

//=======================================================================
Calibrator::Calibrator()
	: LiveViewProcessor()
	, m_NumCaliImgs(0)
	, m_Width( 0 )
	, m_Height( 0 )
{}

//=======================================================================
void Calibrator::Run()
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

	while ( !IsStopped() )
	{
		// read next frame if any
		if ( !ReadNextFrame( frame ) )
		{
			break;
		}

		// display input frame
        if( m_WindowNameInput.size() > 0 )
        {
            for( int i = 0; i < m_NumSource; i++ )
            {
                cv::imshow( m_WindowNameInput[i], frame[i] );
            }
		}

		// check whether capture or start calibration
		CaptureOptions( frame, output );

		output = frame;

		// display output frame
        if( m_WindowNameOutput.size() > 0 )
        {
            for( int i = 0; i < m_NumSource; i++ )
            {
                cv::imshow( m_WindowNameOutput[i], output[i] );
            }
		}

		// check if we should stop
		if ( m_FrameToStop >= 0 && GetFrameNumber() == m_FrameToStop )
		{
			StopIt();
		}
	}
} // Run

//=======================================================================
void Calibrator::CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output )
{
	int ret = cv::waitKey( m_Delay );
	if ( ret == 27/*ESC*/ )
	{
		StopIt();
	}

	if ( ret == 67/*'C'*/ || ret == 99 /*'c'*/ )
	{
		// capture, and find chessboard corners
		cv::Mat& in = frame[0];
		cv::Mat& out = output[0];

		// copy to output first
		out = in.clone();

		//int board_n = board_w * board_h;
		cv::Size boardSize = cv::Size( m_Width, m_Height );

		vector<cv::Point2f> corners;
		const bool found = cv::findChessboardCorners( out, boardSize, corners );

		if ( found )
		{
			cv::cornerSubPix( out, corners, cv::Size( 11, 11 ), cv::Size( -1, -1 ),
				cv::TermCriteria( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ) );

			// show the corners on output window

			// show a downsampled img
			cv::Mat tmp = out.clone();
			const double scale = 0.5;
			cv::resize( tmp, tmp, cv::Size(), scale, scale, cv::INTER_LINEAR );

			//downsampled corners
			vector<cv::Point2f> tmpCorners;
			for ( int i = 0; i < corners.size(); i++ )
			{
				cv::Point2f tmp(
					static_cast<float>( corners[i].x * scale ),
					static_cast<float>( corners[i].y * scale ) );

				tmpCorners.push_back( tmp );
			}

			// draw it
			cv::drawChessboardCorners( tmp, boardSize, tmpCorners, found );
			cv::imshow( m_WindowNameOutput[0], tmp );

			bool needAnswer( true );
			while ( needAnswer )
			{
				int ans = cv::waitKey( 0 ); // wait indefinitely for an answer: "a" (accept) or "r" (reject)

				if ( ans == 65/*'A'*/ || ans == 97/*'a'*/ )
				{
					// we take it, now store the parameters and image

					// cache image points
					m_ImagePoints.push_back( corners );

					// write imgs
					std::stringstream ss;
					ss << m_FileName << std::setfill( '0' ) << std::setw( m_Digits ) << m_NumCaliImgs << m_Extension;
					cv::imwrite( ss.str(), out );

					m_NumCaliImgs++;

					needAnswer = false;
				}
				else if ( ans == 82/*'R'*/ || ans == 114/*'r'*/ )
				{
					needAnswer = false;
				}
			}
		}
	}
	else if ( ret == 27/*ESC*/ )
	{
		// use all the captured imgs for calibration

		// create object points
		vector<cv::Point3f> objPt;
		const int boardSize = m_Width * m_Height;

		for ( int i = 0; i < m_NumCaliImgs; i++ )
		{
			for ( int j = 0; j < boardSize; j++ )
			{
				cv::Point3f p(
					static_cast<float> ( j / m_Width ),
					static_cast<float> ( j % m_Width ), 0.0f );

				objPt.push_back( p );
			}
		}

		// calibrate camera
		cv::Mat intrinsicMatrix, distortionCoeffs;
		cv::Size siz = frame[0].size();

		double err = cv::calibrateCamera(
			objPt, m_ImagePoints, siz , intrinsicMatrix,
			distortionCoeffs, cv::noArray(), cv::noArray(),
			cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT );

		// SAVE THE INTRINSICS AND DISTORTIONS
		cout << " *** DONE!\n\nReprojection error is " << err
			<< "\nStoring caliResult.xml \n\n";

		cv::FileStorage fs( "caliResult.xml", cv::FileStorage::WRITE );
		fs << "image_width" << siz.width << "image_height" << siz.height
			<< "intrinsicMatrix" << intrinsicMatrix
			<< "distortionCoeffs" << distortionCoeffs
			<< "m_NumCaliImgs" << m_NumCaliImgs;

		fs.release();

		//
		// Build the undistort map which we will use for all
		// subsequent frames.
		cv::Mat map1, map2;
		cv::initUndistortRectifyMap( intrinsicMatrix, distortionCoeffs,
			cv::Mat(), intrinsicMatrix, siz,
			CV_16SC2, map1, map2 );

		// undistort the images and save them
		for ( int i = 0; i < m_NumCaliImgs; i++ )
		{
			// read imgs
			std::stringstream ss;
			ss << m_FileName << std::setfill( '0' ) << std::setw( m_Digits ) << i << m_Extension;
			cv::Mat orig = cv::imread( ss.str() );
			cv::Mat undistort;

			cv::remap( orig, undistort, map1, map2, cv::INTER_LINEAR,
				cv::BORDER_CONSTANT, cv::Scalar() );

			// save the undistorted imgs
			ss.seekg( 0 );
			ss << m_FileName << "_undist" << std::setfill( '0' ) << std::setw( m_Digits ) << i << m_Extension;
			cv::imwrite( ss.str(), undistort );
		}

		StopIt();
	}
} // CaptureOptions
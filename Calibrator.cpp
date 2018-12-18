#include <windows.h> // WinApi header
#include <opencv2/opencv.hpp>
#include "Calibrator.h"

//=======================================================================
Calibrator::Calibrator()
	: LiveViewProcessor()
	, m_NumCaliImgs(0)
	, m_Width( 0 )
	, m_Height( 0 )
	, m_CaptureAndCali( false )
	, m_DoCali( false )
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
	bool ok( true );

	while ( !IsStopped() )
	{
		// read next frame if any
		ok = ok && ReadNextFrame( frame );
		if ( !ok )
		{
			break;
		}

		// display input frame
        if( m_WindowNameInput.size() > 0 )
        {
            for( int i = 0; i < m_NumSource; i++ )
            {
                if( m_ScaleFactorForShow != 1.0f )
                {
                    cv::Mat tmp;
                    cv::resize( frame[i], tmp, cv::Size(), m_ScaleFactorForShow, m_ScaleFactorForShow );
                    cv::imshow( m_WindowNameInput[i], tmp );
                }
                else
                {
                    cv::imshow( m_WindowNameInput[i], frame[i] );
                }
            }
		}

		// check whether capture or start calibration
		CaptureOptions( frame, output );

		output = frame;

		// display output frame
        if( m_WindowNameOutput.size() > 0 && !IsStopped() )
        {
            for( int i = 0; i < m_NumSource; i++ )
            {
                if( m_ScaleFactorForShow != 1.0f )
                {
                    cv::Mat tmp;
                    cv::resize( output[i], tmp, cv::Size(), m_ScaleFactorForShow, m_ScaleFactorForShow );
                    cv::imshow( m_WindowNameOutput[i], tmp );
                }
                else
                {
                    cv::imshow( m_WindowNameOutput[i], output[i] );
                }
            }
		}

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
} // Run

//=======================================================================
void Calibrator::CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output )
{
	m_ImgSiz = frame[0].size();

	int ret = cv::waitKey( m_Delay );
	if ( m_DoCali )
	{
		// frame is already the loaded from saved images, so we do cali
		const bool writeImg = false;
		if ( !FindChessboard( frame[0], writeImg ) )
		{
			cout << "can't find chessboard/n";
			return;
		}

		if ( m_ItImg[0] == m_Images[0].end() )
		{
			StopIt();
		}
	}
	else
	{
		if ( ret == 67/*'C'*/ || ret == 99 /*'c'*/ )
		{
			Beep( 523, 500 ); // 523 hertz (C5) for 500 milliseconds

			// capture, and find chessboard corners
			cv::Mat& in = frame[0];
			cv::Mat& out = output[0];

			// copy to output first
			out = in.clone();
			if ( m_CaptureAndCali )
			{
				const bool writeImg = true;
				if ( !FindChessboard( out, writeImg ) )
				{
					cout << "can't find chessboard/n";
					return;
				}
			}//if ( m_CaptureAndCali )
			else
			{
				WriteCaliImg( in );
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

} // CaptureOptions

//===============================================
void Calibrator::WriteCaliResults()
{
	string s = m_Path + "CaliResult.xml";
	cv::FileStorage fs( s, cv::FileStorage::WRITE );

	fs << "ImageWidth" << m_ImgSiz.width << "ImageHeight" << m_ImgSiz.height
		<< "IntrinsicMatrix" << m_IntrinsicMat
		<< "DistortionCoeffs" << m_DistCoeff
		<< "NumCaliImgs" << m_NumCaliImgs;

	fs.release();
}//WriteCaliResults

//===============================================
void Calibrator::ReadCaliResults()
{
	cv::FileStorage fs( "CaliResult.xml", cv::FileStorage::READ );

	fs["ImageWidth"] >> m_ImgSiz.width;
	fs["ImageHeight"] >> m_ImgSiz.height;
	fs["IntrinsicMatrix"] >> m_IntrinsicMat;
	fs["DistortionCoeffs"] >> m_DistCoeff;
	fs["NumCaliImgs"] >> m_NumCaliImgs;

	fs.release();
}//ReadCaliResults

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
bool Calibrator::FindChessboard( const cv::Mat& img, const bool writeImg )
{
	cv::Size boardSize = cv::Size( m_Width, m_Height );

	vector<cv::Point2f> corners;
	const bool found = cv::findChessboardCorners( img, boardSize, corners );

	if ( found )
	{
        cv::Mat gray;
        cv::cvtColor( img, gray, cv::COLOR_BGRA2GRAY );
		cv::cornerSubPix( gray, corners, cv::Size( 11, 11 ), cv::Size( -1, -1 ),
			cv::TermCriteria( cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.1 ) );

		// show the corners on output window

		// show a downsampled img
		cv::Mat tmp = img.clone();
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
				Beep( 523, 500 ); // 523 hertz (C5) for 500 milliseconds

				// we take it, now store the parameters and image

				// cache image points
				m_ImagePoints.push_back( corners );

				if ( writeImg )
				{
					WriteCaliImg( img );
				}

				WriteCaliWithCirclesImg( tmp );

				m_NumCaliImgs++;

				needAnswer = false;
			}
			else if ( ans == 82/*'R'*/ || ans == 114/*'r'*/ )
			{
				Beep( 523, 500 ); // 523 hertz (C5) for 500 milliseconds

				needAnswer = false;
			}

		}//while ( needAnswer )
	}//if ( found )

	return found;
}//FindChessboard

void Calibrator::Calibrate()
{
	// use all the captured imgs for calibration

	// create object points
	vector<vector<cv::Point3f>> objPt;
	const int boardSize = m_Width * m_Height;

	for ( int i = 0; i < m_NumCaliImgs; i++ )
	{
		vector<cv::Point3f> tmpPt;

		for ( int j = 0; j < boardSize; j++ )
		{
			cv::Point3f p(
				static_cast<float> ( j / m_Width ),
				static_cast<float> ( j % m_Width ), 0.0f );

			tmpPt.push_back( p );
		}

		objPt.push_back( tmpPt );
	}

	// calibrate the camera
	double err = cv::calibrateCamera(
		objPt, m_ImagePoints, m_ImgSiz, m_IntrinsicMat,
		m_DistCoeff, cv::noArray(), cv::noArray(),
		cv::CALIB_ZERO_TANGENT_DIST | cv::CALIB_FIX_PRINCIPAL_POINT );

	// SAVE THE INTRINSICS AND DISTORTIONS
	cout << " *** DONE!\n\nReprojection error is " << err
		<< "\nStoring caliResult.xml \n\n";

	WriteCaliResults();
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
		ss << m_Path << m_FileName << std::setfill( '0' ) << std::setw( m_Digits ) << i << m_Extension;
		cv::Mat orig = cv::imread( ss.str() );
		cv::Mat undistort;

		cv::remap( orig, undistort, map1, map2, cv::INTER_LINEAR,
			cv::BORDER_CONSTANT, cv::Scalar() );

		// save the undistorted imgs
		std::stringstream ss1;
		ss1 << m_Path << m_FileName << "_undist" << std::setfill( '0' ) << std::setw( m_Digits ) << i << m_Extension;
		cv::imwrite( ss1.str(), undistort );
	}
} // Calibrate

//=====================================================
void Calibrator::WriteCaliImg( const cv::Mat& img )
{
	// write imgs
	std::stringstream ss;
	ss << m_Path << m_FileName << std::setfill( '0' ) << std::setw( m_Digits ) << m_NumCaliImgs << m_Extension;
	cv::imwrite( ss.str(), img );
}

//=====================================================
void Calibrator::WriteCaliWithCirclesImg( const cv::Mat& img )
{
	// write imgs
	std::stringstream ss;
	ss << m_Path << m_FileName << "_pattern" << std::setfill( '0' ) << std::setw( m_Digits ) << m_NumCaliImgs << m_Extension;
	cv::imwrite( ss.str(), img );
}
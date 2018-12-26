#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <map> // for multimap, in Decode
#include <iostream>
#include <fstream>

#include "GrayCode.h"
#define GREEN  cv::Scalar(   0, 255,   0 )

//======================================
GrayCode::GrayCode()
    : m_ProjectorWidth( 1280 ) // VANKYO Leisure 510
    , m_ProjectorHeight( 768 )
    , m_NumColImgs( 0 )
    , m_NumRowImgs( 0 )
    , m_BlackThresh( 40 )
    , m_WhiteThresh( 5 )
	, m_ImgWidth( 0 )
	, m_ImgHeight( 0 )
{}

//======================================
bool GrayCode::GeneratePattern()
{
	if ( m_NumPatternImgs <= 0 )
	{
		std::cout << "projector dimenssion is not set (check config.xml )/n";
		return false;
	}

	// allocate
	for ( int i = 0; i < m_NumPatternImgs; i++ )
	{
		cv::Mat tmp( m_ProjectorHeight, m_ProjectorWidth, CV_8U );
		m_Pattern.push_back( tmp );
	}

	uchar flag = 0;

	for ( int c = 0; c < m_ProjectorWidth; c++ )  // col loop
	{
		int rem = 0; // remain
		int num = c;
		int prevRem = c % 2;

		for ( int k = 0; k < m_NumColImgs; k++ )  // images loop
		{
			num = num / 2;
			rem = num % 2;

			if ( ( rem == 0 && prevRem == 1 ) || ( rem == 1 && prevRem == 0 ) )
			{
				flag = 1;
			}
			else
			{
				flag = 0;
			}

			for ( int r = 0; r < m_ProjectorHeight; r++ )  // rows loop
			{
				uchar pixel_color = (uchar)flag * 255;

				m_Pattern[2 * m_NumColImgs - 2 * k - 2].at<uchar>( r, c ) = pixel_color;

				if ( pixel_color > 0 )
				{
					pixel_color = (uchar)0;
				}
				else
				{
					pixel_color = (uchar)255;
				}

				m_Pattern[2 * m_NumColImgs - 2 * k - 1].at<uchar>( r, c ) = pixel_color;  // inverse
			} // for r

			prevRem = rem;

		} // for k
	}//for c

	//====================

	for ( int r = 0; r < m_ProjectorHeight; r++ )  // rows loop
	{
		int rem = 0;
		int num = r;
		int prevRem = r % 2;

		for ( int k = 0; k < m_NumRowImgs; k++ ) // img loop
		{
			num = num / 2;
			rem = num % 2;

			if ( ( rem == 0 && prevRem == 1 ) || ( rem == 1 && prevRem == 0 ) )
			{
				flag = 1;
			}
			else
			{
				flag = 0;
			}

			for ( int c = 0; c < m_ProjectorWidth; c++ ) // col loop
			{

				uchar pixel_color = (uchar)flag * 255;
				m_Pattern[2 * m_NumRowImgs - 2 * k + 2 * m_NumColImgs - 2].at<uchar>( r, c ) = pixel_color;

				if ( pixel_color > 0 )
				{
					pixel_color = (uchar)0;
				}
				else
				{
					pixel_color = (uchar)255;
				}

				m_Pattern[2 * m_NumRowImgs - 2 * k + 2 * m_NumColImgs - 1].at<uchar>( r, c ) = pixel_color;
			}//for c

			prevRem = rem;
		} // for k
	} // for r

	// generate black and white imgs
	m_WhiteImage = cv::Mat( m_ProjectorHeight, m_ProjectorWidth, CV_8U, cv::Scalar( 255 ) );
	m_BlackImage = cv::Mat( m_ProjectorHeight, m_ProjectorWidth, CV_8U, cv::Scalar( 0 ) );

	m_Pattern.push_back( m_WhiteImage );
	m_Pattern.push_back( m_BlackImage );

	return true;
}// GeneratePattern

//===========================================================================
void GrayCode::ComputeNumPatternImgs( const int w, const int h )
{
	SetProjectorWidth( w );
	SetProjectorHeight( h );

	m_NumColImgs = ceil( log( double( m_ProjectorWidth ) ) / log( 2.0 ) );
	m_NumRowImgs = ceil( log( double( m_ProjectorHeight ) ) / log( 2.0 ) );

	m_NumPatternImgs = 2 * m_NumColImgs + 2 * m_NumRowImgs;

}// ComputeNumPatternImgs

//===========================================================================
void GrayCode::GenerateShadowMask(
	const vector<cv::Mat>& whiteImg,
	const vector<cv::Mat>& blackImg )
{
    const int numSrc = 2;

    // allocate space
    for( int i = 0; i < numSrc; i++ )
    {
        m_ShadowMask.push_back( cv::Mat() );
    }

	m_ImgWidth = whiteImg[0].cols;
	m_ImgHeight = whiteImg[0].rows;
	int shadowCount = 0;
	int nonShadowCount = 0;

	bool loadFromSaved = false;

    for( int k = 0; k < numSrc; k++ )
    {
		m_ShadowMask[k] = cv::Mat( m_ImgHeight, m_ImgWidth, CV_8U );

		// build filename
		std::stringstream ss;
		ss << m_Path << "shadowMask" << k << ".txt";

		if( loadFromSaved )
		{
			std::ifstream logger;
			logger.open( ss.str(), std::ifstream::in );

			for ( int r = 0; r < m_ImgHeight; r++ ) // row
			{
				system( "CLS" ); // clear prompt command
				cout << "Calculating Shadow Mask ...: ";
				float percentage = float( r + k * m_ImgHeight ) / float( 2.0f * m_ImgHeight ) * 100.0f;

				cout << percentage << "%\n";

				for ( int c = 0; c < m_ImgWidth; c++ ) // col
				{
					string line;
					getline( logger, line );
					int val;

					stringstream stream;
					stream.str( line );
					stream >> val;
					if ( val == 0 )
					{
						shadowCount++;
					}
					else
					{
						nonShadowCount++;
					}

					m_ShadowMask[k].at<uchar>( r, c ) = (uchar)val; // is not shadow
				}//for c
			}//for r
			logger.close();
		}
		else
		{
			std::ofstream logger;
			logger.open( ss.str(), std::ios_base::out ); // fresh file
			//=============================================

			for ( int r = 0; r < m_ImgHeight; r++ ) // row
			{
				system( "CLS" ); // clear prompt command
				cout << "Calculating Shadow Mask ...: ";
				float percentage = float( r + k * m_ImgHeight ) / float( 2.0f * m_ImgHeight ) * 100.0f;

				cout << percentage << "%\n";

				for ( int c = 0; c < m_ImgWidth; c++ ) // col
				{
					int white = (int)whiteImg[k].at<uchar>( r, c );
					int black = (int)blackImg[k].at<uchar>( r, c );

					if ( abs( white - black ) > m_BlackThresh )
					{
						m_ShadowMask[k].at<uchar>( r, c ) = (uchar)255; // is not shadow
						nonShadowCount++;
						logger << 255 << endl;
					}
					else
					{
						m_ShadowMask[k].at<uchar>( r, c ) = (uchar)0; // is shadow
						shadowCount++;
						logger << 0 << endl;
					}
				}// for y
			}//for x

			logger.close();

		} // if( loadFromSaved )
    }// for k

	cout << "shadowCount= " << shadowCount << endl;
	cout << "nonShadowCount= " << nonShadowCount << endl;

	bool showAndSaveImg = true;
	if ( showAndSaveImg )
	{
		//debug; draw it
		cv::Mat leftImg, rightImg;

		cv::resize( m_ShadowMask[0], leftImg, cv::Size(), 0.4f, 0.4f, cv::INTER_LINEAR );
		cv::resize( m_ShadowMask[1], rightImg, cv::Size(), 0.4f, 0.4f, cv::INTER_LINEAR );

		string leftMaskName = "Left Shadow Mask";
		cv::imshow( leftMaskName, leftImg );
		cv::moveWindow( leftMaskName, 0, 0 );

		string rightMaskName = "Right Shadow Mask";
		cv::imshow( rightMaskName, rightImg );
		cv::moveWindow( rightMaskName, 755, 0 );
		cv::waitKey( 1000 );

		std::stringstream ss1;
		ss1 << m_Path << leftMaskName << ".jpg";
		cv::imwrite( ss1.str(), m_ShadowMask[0] );

		std::stringstream ss;
		ss << m_Path << rightMaskName << ".jpg";
		cv::imwrite( ss.str(), m_ShadowMask[1] );

		// destroy win
		cv::destroyWindow( leftMaskName );
		cv::destroyWindow( rightMaskName );
	}//if ( showAndSaveImg )
}//GenerateShadowMask


//===========================================================================
bool GrayCode::Decode(
    const vector<vector<cv::Mat>>& captured,
    const vector<cv::Mat>& whiteImages,
    const vector<cv::Mat>& blackImages )
{
    const int numSrc = static_cast<int>( captured.size() );

    GenerateShadowMask( whiteImages/*8UC1*/, blackImages );

	std::map     < int /*(r * ImgWidth + c) on the captured image */, int /*(r * ProjWidth + c) decoded decimal*/ > leftCam;
	std::multimap< int /*(r * ProjWidth + c) decoded decimal*/,	   int /*(r * ImgWidth + c) on the captured image */ > rightCam;

    std::stringstream leftPath;
    leftPath << m_Path << "leftCamMap.txt";

    std::stringstream rightPath;
    rightPath << m_Path << "rightCamMap.txt";

	bool loadFromSavedMap = false;
	if ( loadFromSavedMap )
	{
		std::ifstream logLeftCam;
        logLeftCam.open( leftPath.str(), std::ifstream::in );

		std::ifstream logRightCam;
		logRightCam.open( rightPath.str(), std::ifstream::in );

		string line;
		while ( getline( logLeftCam, line ) )
		{
			int imgLocIdx, decimalIdx;
			stringstream stream;
			stream.str( line );
			stream >> imgLocIdx;
			stream >> decimalIdx;

			leftCam.insert( std::make_pair( imgLocIdx, decimalIdx ) );

		}//while
		logLeftCam.close();

		while ( getline( logRightCam, line ) )
		{
			int imgLocIdx, decimalIdx;
			stringstream stream;
			stream.str( line );
			stream >> decimalIdx;
			stream >> imgLocIdx;

			rightCam.insert( std::make_pair( decimalIdx, imgLocIdx ) );
		}//while
		logRightCam.close();
	}
	else
	{
		// logger
		bool log = true;
		std::ofstream logLeftCam;
		logLeftCam.open( leftPath.str(), std::ios_base::out ); // fresh file
		logLeftCam.close();
		logLeftCam.open( leftPath.str(), std::ios_base::app ); // append mode

		std::ofstream logRightCam;
		logRightCam.open( rightPath.str(), std::ios_base::out ); // fresh file
		logRightCam.close();
		logRightCam.open( rightPath.str(), std::ios_base::app ); // append mode
		//=================

		for ( int k = 0; k < numSrc; k++ ) // number of source (2, cameras)
		{
			for ( int r = 0; r < m_ImgHeight; r++ ) // row
			{
				system( "CLS" ); // clear prompt command
				cout << "Creating Maps ...: ";
				float percentage = float( r + k * m_ImgHeight ) / float( 2.0f * m_ImgHeight ) * 100.0f;

				cout << percentage << "%\n";

				for ( int c = 0; c < m_ImgWidth; c++ ) // col
				{
					//if the pixel is not shadowed, reconstruct
					if ( int( m_ShadowMask[k].at<uchar>( r, c ) ) == 255 ) // not shadow
					{
						int rowDec, colDec; // decoded decimal number corresponding to to a (r,c) on the image

						//for a (r,c) pixel of the camera returns the corresponding projector pixel by calculating the decimal number
						bool ok = GetProjPixel( captured[k], r, c, rowDec, colDec );

						if ( !ok )
						{
							continue;
						}

						const int imgLocIdx = RowColToIdx( r, c, m_ImgWidth );
						const int decimalIdx = RowColToIdx( rowDec, colDec, m_ProjectorWidth );

						if ( k == 0 )
						{
							// left cam
							leftCam.insert( std::make_pair( imgLocIdx, decimalIdx ) );

							if ( log )
							{
								//save the maps
								logLeftCam << imgLocIdx << " " << decimalIdx << endl;
							}
						}
						else
						{
							// right cam
							rightCam.insert( std::make_pair( decimalIdx, imgLocIdx ) );

							if ( log )
							{
								//save the maps
								logRightCam << decimalIdx << " " << imgLocIdx << endl;
							}
						}// if k == 0
					}// if m_ShadowMask
				} // for c
			} // for r
		}// for k

		logLeftCam.close();
		logRightCam.close();
	}// if loadFromSavedMap

	m_DisparityMap = cv::Mat::zeros( m_ImgHeight, m_ImgWidth, CV_32S );

	int marker = 0; // running marker which indicate where the pix is on the right img

	unsigned int numMatchedPts = 0;

	//debug flat; show images
	bool showImgs = false;
	bool showMatched = false;
	bool saveDisp = true;

	bool loadDisp = false;

    std::stringstream dispPath;
    dispPath << m_Path << "Disparity.txt";

	if ( loadDisp )
	{
		std::ifstream disparity;
		disparity.open( dispPath.str(), std::ifstream::in );

		string line;
		while ( getline( disparity, line ) )
		{
			int r, c, d;
			stringstream stream;
			stream.str( line );
			stream >> r;
			stream >> c;
			stream >> d;

			m_DisparityMap.at<int>( r, c ) = d;
			numMatchedPts++;
		}//while
		disparity.close();
	}
	else
	{
		std::ofstream disp;
		disp.open( dispPath.str(), std::ios_base::out ); // fresh file

		cv::Mat leftImg; // white img
		cv::Mat rightImg; // white img
		float s = 0.4f;

		cv::resize( whiteImages[0], leftImg, cv::Size(), s, s, cv::INTER_LINEAR );
		cv::resize( whiteImages[1], rightImg, cv::Size(), s, s, cv::INTER_LINEAR );
		string leftName = "Left";
		string rightName = "Right";

		std::map      < int, int>::iterator itLeft;
		std::multimap < int, int>::iterator itRight;

		unsigned int total = m_ImgHeight * m_ImgWidth;

		int marker = 0; // running marker which indicate where the pix is on the right img
		int rightLimit;

		// scan line fashion to find the correspondance
		for ( int r = 0; r < m_ImgHeight; r++ ) // row
		{
			// show percentage
			system( "CLS" ); // clear prompt command
			cout << "Decoding ...: ";
			float percentage = float( r ) / float( m_ImgHeight ) * 100.0f;

			cout << percentage << "%\n";
			//===================================

			for ( int c = 0; c < m_ImgWidth; c++ ) // col
			{
				rightLimit = c;

				if ( showImgs )
				{
					int off = 2;
					int thick = 2;
					cv::Mat tmpLeft = leftImg.clone();
					cv::Point p1( c * s - off, r * s - off );
					cv::Point p2( c * s + off, r * s + off );
					cv::cvtColor( tmpLeft, tmpLeft, cv::COLOR_GRAY2BGR );
					cv::rectangle( tmpLeft, p1, p2, GREEN, thick );

					cv::imshow( leftName, tmpLeft );
					cv::moveWindow( leftName, 0, 0 );
					cv::waitKey( 1 );
				}

				// convert row col to idx
				const int leftPixIdx = RowColToIdx( r, c, m_ImgWidth );
				itLeft = leftCam.find( leftPixIdx ); // find the corresponding idx

				if ( itLeft != leftCam.end() )
				{
					int dec = itLeft->second;
					int rightPixIdx;

					bool ok( false );

					if ( false )
					{
						const int count = rightCam.count( dec );
						cout << "count= " << count << endl;
					}

					for ( itRight = rightCam.equal_range( dec ).first; itRight != rightCam.equal_range( dec ).second; ++itRight )
					{
						rightPixIdx = ( *itRight ).second;
						int row, col;
						IdxToRowCol( rightPixIdx, row, col, m_ImgWidth );

						if ( showImgs )
						{
							int off = 2;
							int thick = 2;

							cv::Mat tmpRight = rightImg.clone();
							cv::Point p3( col * s - off, row * s - off );
							cv::Point p4( col * s + off, row * s + off );
							cv::cvtColor( tmpRight, tmpRight, cv::COLOR_GRAY2BGR );
							cv::rectangle( tmpRight, p3, p4, GREEN, thick );

							cv::imshow( rightName, tmpRight );
							cv::moveWindow( rightName, 755, 0 );
							cv::waitKey( 1 );
						}

						// evaluate the legit of the found pix
						if ( row == r &&
							 col > marker/* "continuity constraint" */ &&
							 col < rightLimit /* "parallell constraint" */ )
						{
							if ( showMatched )
							{
								int off = 2;
								int thick = 2;

								cv::Mat tmpLeft = leftImg.clone();
								cv::Point p1( c * s - off, r * s - off );
								cv::Point p2( c * s + off, r * s + off );
								cv::cvtColor( tmpLeft, tmpLeft, cv::COLOR_GRAY2BGR );
								cv::rectangle( tmpLeft, p1, p2, GREEN, thick );

								cv::imshow( leftName, tmpLeft );
								cv::moveWindow( leftName, 0, 0 );
								cv::waitKey( 1 );

								cv::Mat tmpRight = rightImg.clone();
								cv::Point p3( col * s - off, row * s - off );
								cv::Point p4( col * s + off, row * s + off );
								cv::cvtColor( tmpRight, tmpRight, cv::COLOR_GRAY2BGR );
								cv::rectangle( tmpRight, p3, p4, GREEN, thick );

								cv::imshow( rightName, tmpRight );
								cv::moveWindow( rightName, 755, 0 );
								cv::waitKey( 1 );
							}

							ok = true;
							marker = col;
							numMatchedPts++;
							break;
						}
					}// for

					if ( ok )
					{
						m_DisparityMap.at<int>( r, c ) = marker - c;

						if ( saveDisp )
						{
							disp << r << " " << c << " " << marker - c << endl;
						}
					}
				}// if itLeft

			}// for c

			marker = 0; //reset the marker
		}// for r

		disp.close();
	} // if ( loadDisp )

	cout << "mumber of matched pts: " << numMatchedPts << endl;

    return true;

}//Decode

//===========================================================================
bool GrayCode::GetProjPixel(
    const vector<cv::Mat>& captured,
    const int r, // pixel, on the image
	const int c, // pixel, on the image
    int& rowDec,
	int& colDec )
{
    std::vector<uchar> grayCol;
    std::vector<uchar> grayRow;

    // process column images
    for( int count = 0; count < m_NumColImgs; count++ )
    {
        // get pixel intensity for regular pattern projection and its inverse
		int val1 = int( captured[count * 2    ].at<uchar>( r, c ) );
		int val2 = int( captured[count * 2 + 1].at<uchar>( r, c ) );

        // check if the intensity difference between the values of
        // the normal and its inverse projection image is in a valid range
        if( abs( val1 - val2 ) < m_WhiteThresh )
        {
            // TODO: recover method?
            return false;
        }

        // determine if projection pixel is on or off
        if( val1 > val2 )
        {
            grayCol.push_back( 1 );
        }
        else
        {
            grayCol.push_back( 0 );
        }
    }// for m_NumColImgs

    colDec = GrayToDec( grayCol );

    // process row images
    for( int count = 0; count < m_NumRowImgs; count++ )
    {
        // get pixel intensity for regular pattern projection and its inverse
		int val1 = int( captured[count * 2 + m_NumColImgs * 2    ].at<uchar>( r, c ) );
		int val2 = int( captured[count * 2 + m_NumColImgs * 2 + 1].at<uchar>( r, c ) );

        // check if the intensity difference between the values of the normal and its inverse projection image is in a valid range
        if( abs( val1 - val2 ) < m_WhiteThresh )
        {
            // TODO: recover method?
            return false;
        }

        // determine if projection pixel is on or off
        if( val1 > val2 )
        {
            grayRow.push_back( 1 );
        }
        else
        {
            grayRow.push_back( 0 );
        }
    }// for m_NumRowImgs

	rowDec = GrayToDec( grayRow );

    if( ( rowDec >= m_ProjectorHeight || colDec >= m_ProjectorWidth ) )
    {
        return false;
    }

    return true;

} // GetProjPixel

//===========================================================================
int GrayCode::GrayToDec( const std::vector<uchar>& gray ) const
{
    int dec = 0;

    int siz = static_cast<int>( gray.size() );

    uchar tmp = gray[0];

    if( tmp )
    {
        dec += (int)pow( 2.0 /*base*/, siz - 1 /*exp*/);
    }

    for( int i = 1; i < siz; i++ )
    {
        // XOR operation
        tmp = tmp ^ gray[i];
        if( tmp )
        {
            dec += (int)pow( 2.0 /*base*/, siz - i - 1 );
        }
    }

    return dec;
}//GrayToDec
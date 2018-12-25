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

    for( int k = 0; k < numSrc; k++ )
    {
        m_ShadowMask[k] = cv::Mat( m_ImgHeight, m_ImgWidth, CV_8U );

        for( int x = 0; x < m_ImgWidth; x++ ) // col
        {
            for( int y = 0; y < m_ImgHeight; y++ ) // row
            {
                double white = whiteImg[k].at<uchar>( cv::Point( x, y ) );
                double black = blackImg[k].at<uchar>( cv::Point( x, y ) );

                if( abs( white - black ) > m_BlackThresh )
                {
                    m_ShadowMask[k].at<uchar>( cv::Point( x, y ) ) = (uchar)1; // is not shadow
                }
                else
                {
                    m_ShadowMask[k].at<uchar>( cv::Point( x, y ) ) = (uchar)0; // is shadow
                }
            }// for y
        }//for x
    }// for k

}//GenerateShadowMask

//===========================================================================
bool GrayCode::Decode(
    const vector<vector<cv::Mat>>& captured,
    const vector<cv::Mat>& whiteImages,
    const vector<cv::Mat>& blackImages )
{
    const int numSrc = static_cast<int>( captured.size() );

    GenerateShadowMask( whiteImages, blackImages );

	//std::map     < int /*(x * ImgHeight + y) on the captured image */, int /*(x * ProjHeight + y) decoded decimal*/ > leftCam;
	//std::multimap< int /*(x * ProjHeight + y) decoded decimal*/,	   int /*(x * ImgHeight + y) on the captured image */ > rightCam;

	//bool loadFromSavedMap = false;
	//if ( loadFromSavedMap )
	//{
	//	std::ifstream logLeftCam;
	//	logLeftCam.open( "leftCamMap.txt", std::ifstream::in );

	//	std::ifstream logRightCam;
	//	logRightCam.open( "rightCamMap.txt", std::ifstream::in );

	//	string line;
	//	while ( getline( logLeftCam, line ) )
	//	{
	//		int imgLocIdx, decimalIdx;
	//		stringstream stream;
	//		stream.str( line );
	//		stream >> imgLocIdx;
	//		stream >> decimalIdx;

	//		leftCam.insert( std::make_pair( imgLocIdx, decimalIdx ) );

	//	}//while
	//	logLeftCam.close();

	//	while ( getline( logRightCam, line ) )
	//	{
	//		int imgLocIdx, decimalIdx;
	//		stringstream stream;
	//		stream.str( line );
	//		stream >> decimalIdx;
	//		stream >> imgLocIdx;

	//		rightCam.insert( std::make_pair( decimalIdx, imgLocIdx ) );
	//	}//while
	//	logRightCam.close();
	//}
	//else
	//{
	//	// logger
	//	bool log = true;
	//	std::ofstream logLeftCam;
	//	logLeftCam.open( "leftCamMap.txt", std::ios_base::out ); // fresh file
	//	logLeftCam.close();
	//	logLeftCam.open( "leftCamMap.txt", std::ios_base::app ); // append mode

	//	std::ofstream logRightCam;
	//	logRightCam.open( "rightCamMap.txt", std::ios_base::out ); // fresh file
	//	logRightCam.close();
	//	logRightCam.open( "rightCamMap.txt", std::ios_base::app ); // append mode
	//	//=================

	//	for ( int k = 0; k < numSrc; k++ ) // number of source (2, cameras)
	//	{
	//		for ( int x = 0; x < m_ImgWidth; x++ ) // col
	//		{
	//			for ( int y = 0; y < m_ImgHeight; y++ ) // row
	//			{
	//				//if the pixel is not shadowed, reconstruct
	//				if ( m_ShadowMask[k].at<uchar>( y, x ) == 0 ) // not shadow
	//				{
	//					cv::Point imgLoc( x, y );
	//					cv::Point decimal; // decoded decimal number corresponding to to a (x,y) on the image

	//					//for a (x,y) pixel of the camera returns the corresponding projector pixel by calculating the decimal number
	//					bool ok = GetProjPixel( captured[k], x, y, decimal );

	//					if ( !ok )
	//					{
	//						continue;
	//					}

	//					const int imgLocIdx = XYToIdx( imgLoc, m_ImgHeight );
	//					const int decimalIdx = XYToIdx( decimal, m_ProjectorHeight );

	//					if ( k == 0 )
	//					{
	//						// left cam
	//						leftCam.insert( std::make_pair( imgLocIdx, decimalIdx ) );

	//						if ( log )
	//						{
	//							//save the maps
	//							logLeftCam << imgLocIdx << " " << decimalIdx << endl;
	//						}
	//					}
	//					else
	//					{
	//						// right cam
	//						rightCam.insert( std::make_pair( decimalIdx, imgLocIdx ) );

	//						if ( log )
	//						{
	//							//save the maps
	//							logRightCam << decimalIdx << " " << imgLocIdx << endl;
	//						}
	//					}// if k == 0
	//				}
	//			} // for y
	//		} // for x
	//	}// for k

	//	logLeftCam.close();
	//	logRightCam.close();
	//}// if loadFromSavedMap

	m_DisparityMap = cv::Mat::zeros( m_ImgHeight, m_ImgWidth, CV_32S );

	int marker = 0; // running marker which indicate where the pix is on the right img

	unsigned int numMatchedPts = 0;

	//debug flat; show images
	bool showImgs = true;
	bool saveDisp = true;

	bool loadDisp = false;
	if ( loadDisp )
	{
		std::ifstream disparity;
		disparity.open( "Disparity.txt", std::ifstream::in );

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
		disp.open( "Disparity.txt", std::ios_base::out ); // fresh file

		cv::Mat leftImg; // white img
		cv::Mat rightImg; // white img

		cv::resize( whiteImages[0], leftImg, cv::Size(), 0.4f, 0.4f, cv::INTER_LINEAR );
		cv::resize( whiteImages[1], rightImg, cv::Size(), 0.4f, 0.4f, cv::INTER_LINEAR );
		string leftName = "Left";
		string rightName = "Right";

		unsigned int total = m_ImgHeight * m_ImgWidth;

		// scan line fashion to find the correspondance
		for ( int rl = 0; rl < m_ImgHeight; rl++ ) // row, left
		{
			for ( int cl = 0; cl < m_ImgWidth; cl++ ) // col, right
			{
				if ( showImgs )
				{
					int off = 2;
					int thick = 2;
					cv::Mat tmpLeft = leftImg.clone();
					cv::Point p1( cl - off, rl - off );
					cv::Point p2( cl + off, rl + off );
					cv::rectangle( tmpLeft, p1, p2, GREEN, thick );

					cv::imshow( leftName, tmpLeft );
					cv::moveWindow( leftName, 0, 0 );
					cv::waitKey( 1 );
				}

				unsigned int idx = rl * m_ImgWidth + cl;

				float percentage = float( idx ) / float( total ) * 100.0f;

				cout << percentage << "%\n";

				//if the pixel is not shadowed, reconstruct
				if ( m_ShadowMask[0].at<uchar>( cv::Point( cl, rl) ) ) // not shadow
				{
					cv::Point decimalLeft;

					//for a (x,y) pixel of the camera returns the corresponding projector pixel by calculating the decimal number
					bool ok = GetProjPixel( captured[0], cl, rl, decimalLeft );

					if ( !ok )
					{
						continue;
					}

					// search on the right image, the same row
					for ( int cr = marker; cr < cl; cr++ )
					{
						if ( showImgs )
						{
							int off = 2;
							int thick = 2;
							/*cv::Mat tmpLeft = leftImg.clone();
							cv::Point p1( cl - off, rl - off );
							cv::Point p2( cl + off, rl + off );
							cv::rectangle( tmpLeft, p1, p2, GREEN, thick );

							cv::imshow( leftName, tmpLeft );
							cv::moveWindow( leftName, 0, 0 );
*/
							cv::Mat tmpRight = rightImg.clone();
							cv::Point p3( cr - off, rl - off);
							cv::Point p4( cr + off, rl + off );
							cv::rectangle( tmpRight, p3, p4, GREEN, thick );

							cv::imshow( rightName, tmpRight );
							cv::moveWindow( rightName, 755, 0 );
							cv::waitKey( 1 );
						}

						cv::Point decimalRight;
						bool ok = GetProjPixel( captured[1], cr, rl, decimalRight );

						if ( !ok )
						{
							continue;
						}

						if ( decimalLeft == decimalRight &&
							m_ShadowMask[1].at<uchar>( cv::Point( cr, rl ) ) /*not shadow*/ ) // is a match!
						{
							marker = cr;
							m_DisparityMap.at<int>( rl, cl ) = cr - cl;
							numMatchedPts++;
							if ( saveDisp )
							{
								disp << rl << " " << cl << " " << cr - cl << endl;
							}
							break;
						}
					}// for cr
				} //if ( m_ShadowMask[0].at<uchar>( rl, cl ) == 0 ) // not shadow
			}// for cl

			marker = 0; //reset the marker
		}// for rl

		disp.close();
	} // if ( loadDisp )

	cout << "mumber of matched pts: " << numMatchedPts << endl;

	/*
	std::vector<cv::Point> cam1Pixs, cam2Pixs;

    for( int x = 0; x < m_ProjectorWidth; x++ )
    {
        for( int y = 0; y < m_ProjectorHeight; y++ )
        {
			cam1Pixs = camsPixels[0][ XYToIdx( x, y ) ];
            cam2Pixs = camsPixels[1][ XYToIdx( x, y ) ];

            const int cam1PixSiz = static_cast<int>( cam1Pixs.size() );
            const int cam2PixSiz = static_cast<int>( cam2Pixs.size() );

            if( cam1PixSiz == 0 || cam2PixSiz == 0 )
            {
                continue;
            }

            cv::Point p1;
            cv::Point p2;

            double sump1x = 0.0;
            double sump2x = 0.0;

            for( int c1 = 0; c1 < cam1PixSiz; c1++ )
            {
                p1 = cam1Pixs[c1];
                sump1x += p1.x;
            }

            for( int c2 = 0; c2 < cam2PixSiz; c2++ )
            {
                p2 = cam2Pixs[c2];
                sump2x += p2.x;
            }

            sump1x /= static_cast<double>( cam1PixSiz );
            sump2x /= static_cast<double>( cam2PixSiz );

            for( int c1 = 0; c1 < cam1PixSiz; c1++ )
            {
                p1 = cam1Pixs[c1];
                m_DisparityMap.at<double>( p1.y, p1.x ) = sump2x - sump1x;
            }

            sump2x = 0.0; // reset
            sump1x = 0.0;
        } // for height
    } // for width
	*/
    return true;

}//Decode

//===========================================================================
bool GrayCode::GetProjPixel(
    const vector<cv::Mat>& captured,
    int x, // pixel, on the image
    int y, // pixel, on the image
    cv::Point& projPix )
{
    int xDec, yDec;

    std::vector<uchar> grayCol;
    std::vector<uchar> grayRow;

    // process column images
    for( int count = 0; count < m_NumColImgs; count++ )
    {
        // get pixel intensity for regular pattern projection and its inverse
        double val1 = captured[count * 2].at<uchar>( cv::Point( x, y ) );
        double val2 = captured[count * 2 + 1].at<uchar>( cv::Point( x, y ) );

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

    xDec = GrayToDec( grayCol );

    // process row images
    for( int count = 0; count < m_NumRowImgs; count++ )
    {
        // get pixel intensity for regular pattern projection and its inverse
        double val1 = captured[count * 2 + m_NumColImgs * 2].at<uchar>( cv::Point( x, y ) );
        double val2 = captured[count * 2 + m_NumColImgs * 2 + 1].at<uchar>( cv::Point( x, y ) );

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

    yDec = GrayToDec( grayRow );

    if( ( yDec >= m_ProjectorHeight || xDec >= m_ProjectorWidth ) )
    {
        return false;
    }

    projPix.x = xDec;
    projPix.y = yDec;

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
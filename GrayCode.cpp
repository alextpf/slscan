#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
//#include <opencv2/structured_light.hpp>

#include "GrayCode.h"

//======================================
GrayCode::GrayCode()
    : m_Width( 1280 ) // VANKYO Leisure 510
    , m_Height( 768 )
    , m_NumColImgs( 0 )
    , m_NumRowImgs( 0 )
    , m_BlackThresh( 40 )
    , m_WhiteThresh( 5 )
{}

//======================================
void GrayCode::GeneratePattern()
{
	// allocate
	for ( int i = 0; i < m_NumPatternImgs; i++ )
	{
		cv::Mat tmp( m_Height, m_Width, CV_8U );
		m_Pattern.push_back( tmp );
	}

	uchar flag = 0;

	for ( int j = 0; j < m_Width; j++ )  // rows loop
	{
		int rem = 0; // remain
		int num = j;
		int prevRem = j % 2;

		for ( size_t k = 0; k < m_NumColImgs; k++ )  // images loop
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

			for ( int i = 0; i < m_Height; i++ )  // rows loop
			{
				uchar pixel_color = (uchar)flag * 255;

				m_Pattern[2 * m_NumColImgs - 2 * k - 2].at<uchar>( i, j ) = pixel_color;

				if ( pixel_color > 0 )
				{
					pixel_color = (uchar)0;
				}
				else
				{
					pixel_color = (uchar)255;
				}

				m_Pattern[2 * m_NumColImgs - 2 * k - 1].at<uchar>( i, j ) = pixel_color;  // inverse
			} // for i

			prevRem = rem;

		} // for k
	}//for j

	//====================

	for ( int i = 0; i < m_Height; i++ )  // rows loop
	{
		int rem = 0;
		int num = i;
		int prevRem = i % 2;

		for ( int k = 0; k < m_NumRowImgs; k++ )
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

			for ( int j = 0; j < m_Width; j++ )
			{

				uchar pixel_color = (uchar)flag * 255;
				m_Pattern[2 * m_NumRowImgs - 2 * k + 2 * m_NumColImgs - 2].at<uchar>( i, j ) = pixel_color;

				if ( pixel_color > 0 )
				{
					pixel_color = (uchar)0;
				}
				else
				{
					pixel_color = (uchar)255;
				}

				m_Pattern[2 * m_NumRowImgs - 2 * k + 2 * m_NumColImgs - 1].at<uchar>( i, j ) = pixel_color;
			}//for j

			prevRem = rem;
		} // for k
	} // for i

	// generate black and white imgs
	m_WhiteImage = cv::Mat( m_Height, m_Width, CV_8U, cv::Scalar( 255 ) );
	m_BlackImage = cv::Mat( m_Height, m_Width, CV_8U, cv::Scalar( 0 ) );

	m_Pattern.push_back( m_WhiteImage );
	m_Pattern.push_back( m_BlackImage );
}// GeneratePattern

//===========================================================================
void GrayCode::ComputeNumPatternImgs( const int w, const int h )
{
	SetWidth( w );
	SetHeight( h );

	m_NumColImgs = ceil( log( double( m_Width ) ) / log( 2.0 ) );
	m_NumRowImgs = ceil( log( double( m_Height ) ) / log( 2.0 ) );

	m_NumPatternImgs = 2 * m_NumColImgs + 2 * m_NumRowImgs;

}// ComputeNumPatternImgs

//===========================================================================
void GrayCode::GenerateShadowMask(
    const vector<cv::Mat>& blackImg,
    const vector<cv::Mat>& whiteImg )
{
    const int numSrc = 2;

    // allocate space
    for( int i = 0; i < numSrc; i++ )
    {
        m_ShadowMask.push_back( cv::Mat() );
    }
    int camWidth = whiteImg[0].cols;
    int camHeight = whiteImg[0].rows;

    for( int k = 0; k < numSrc; k++ )
    {
        m_ShadowMask[k] = cv::Mat( camHeight, camWidth, CV_8U );

        for( int i = 0; i < camWidth; i++ )
        {
            for( int j = 0; j < camHeight; j++ )
            {
                double white = whiteImg[k].at<uchar>( cv::Point( i, j ) );
                double black = blackImg[k].at<uchar>( cv::Point( i, j ) );

                if( abs( white - black ) > m_BlackThresh )
                {
                    m_ShadowMask[k].at<uchar>( cv::Point( i, j ) ) = (uchar)1;
                }
                else
                {
                    m_ShadowMask[k].at<uchar>( cv::Point( i, j ) ) = (uchar)0;
                }
            }// for j
        }//for i
    }// for k

}//GenerateShadowMask

//===========================================================================
void GrayCode::Decode(
    const vector<vector<cv::Mat>>& captured,
    const vector<cv::Mat>& blackImages,
    const vector<cv::Mat>& whiteImages )
{
    const int numSrc = static_cast<int>( captured.size() );

    GenerateShadowMask( blackImages, whiteImages );

    int camWidth = blackImages[0].cols;
    int camHeight = blackImages[0].rows;

    std::vector<std::vector<std::vector<cv::Point> > > camsPixels;
    camsPixels.resize( numSrc );

    for( int k = 0; k < numSrc; k++ )
    {
        camsPixels[k].resize( camWidth * camHeight );

        for( int i = 0; i < camWidth; i++ )
        {
            for( int j = 0; j < camHeight; j++ )
            {
                //if the pixel is not shadowed, reconstruct
                if( m_ShadowMask[k].at<uchar>( j, i ) )
                {

                    cv::Point projPixel;

                    //for a (x,y) pixel of the camera returns the corresponding projector pixel by calculating the decimal number
                    bool error = GetProjPixel( captured[k], i, j, projPixel );

                    if( error )
                    {
                        continue;
                    }

                    camsPixels[k][projPixel.x * camHeight + projPixel.y].push_back( cv::Point( i, j ) );
                }
            } // for j
        } // for i
    }// for k
}//Decode

//===========================================================================
bool GrayCode::GetProjPixel(
    const vector<cv::Mat>& captured,
    int x,
    int y,
    cv::Point& projPix )
{
    bool error = false;
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
            error = true;
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
            error = true;
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

    if( ( yDec >= m_Height || xDec >= m_Width ) )
    {
        error = true;
    }

    projPix.x = xDec;
    projPix.y = yDec;

    return error;

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
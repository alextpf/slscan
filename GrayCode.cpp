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

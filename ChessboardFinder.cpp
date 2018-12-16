#include "ChessboardFinder.h"
#include <opencv2/opencv.hpp>

ChessboardFinder::ChessboardFinder()
    : m_Width( 0 )
    , m_Height( 0 )
{
}

//===========================================
bool ChessboardFinder::FindBoard(
	cv::Mat & input,
	cv::Mat & output )
{
	// copy to output first
	output = input.clone();

    //int board_n = board_w * board_h;
    cv::Size boardSize = cv::Size( m_Width, m_Height );

    vector<cv::Point2f> corners;
    const bool found = cv::findChessboardCorners( output, boardSize, corners );

	if ( found )
	{
		cv::cornerSubPix( output, corners, cv::Size( 11, 11 ), cv::Size( -1, -1 ),
			cv::TermCriteria( CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1 ) );

		// cache image points

		// draw it
		cv::drawChessboardCorners( output, boardSize, corners, found );
	}

	return found;

}//FindBoard
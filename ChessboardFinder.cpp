#include "ChessboardFinder.h"
#include <opencv2/opencv.hpp>

ChessboardFinder::ChessboardFinder()
    : m_Width( 0 )
    , m_Height( 0 )
{
}

void ChessboardFinder::Process(
	cv::Mat & input,
	cv::Mat & output )
{
    //int board_n = board_w * board_h;
    cv::Size boardSize = cv::Size( m_Width, m_Height );

    vector<cv::Point2f> corners;
    bool found = cv::findChessboardCorners( input, boardSize, corners );

}//Process
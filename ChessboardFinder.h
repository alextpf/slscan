#pragma once
#include "LiveViewProcessor.h"

class ChessboardFinder
{
public:
    ChessboardFinder::ChessboardFinder();

	bool FindBoard( cv::Mat & input, cv::Mat & output );

    void SetWidth( const unsigned int w )
    {
        m_Width = w;
    }

    void SetHeight( const unsigned int h )
    {
        m_Height = h;
    }

private:

};//ChessboardFinder

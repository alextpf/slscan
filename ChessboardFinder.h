#pragma once
#include "LiveViewProcessor.h"

class ChessboardFinder :public FrameProcessor
{
public:
    ChessboardFinder::ChessboardFinder();

	void Process( cv::Mat & input, cv::Mat & output ) override;

    void SetWidth( const unsigned int w )
    {
        m_Width = w;
    }

    void SetHeight( const unsigned int h )
    {
        m_Height = h;
    }

private:
    unsigned int m_Width;
    unsigned int m_Height;
};//ChessboardFinder

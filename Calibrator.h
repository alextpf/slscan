#pragma once
#include "LiveViewProcessor.h"
#include "ChessboardFinder.h"

class Calibrator : public LiveViewProcessor
{
public:
	Calibrator::Calibrator();//ctor

	void Run() override;

    int GetNumCaliImgs() const
    {
        return m_NumCaliImgs;
    }

    void SetWidth( const unsigned int w )
    {
        m_ChessboardFinder.SetWidth( w );
    }

    void SetHeight( const unsigned int h )
    {
        m_ChessboardFinder.SetHeight( h );
    }

private:
	void CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output );

	ChessboardFinder m_ChessboardFinder;
	int m_NumCaliImgs;
};//Calibrator
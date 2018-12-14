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

private:
	void CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output );

	ChessboardFinder m_ChessboardFinder;
	int m_NumCaliImgs;
};//Calibrator
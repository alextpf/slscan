#pragma once
#include "LiveViewProcessor.h"
#include "ChessboardFinder.h"

class Calibrator : public LiveViewProcessor
{
public:
	Calibrator::Calibrator();//ctor

	void Run() override;
private:
	void CaptureOptions( vector<cv::Mat>& frame, vector<cv::Mat>& output );

	ChessboardFinder m_ChessboardFinder;
	int m_NumCaliImgs;
};//Calibrator
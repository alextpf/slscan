#pragma once
#include "LiveViewProcessor.h"

class ChessboardFinder :public FrameProcessor
{
public:
	void Process( cv::Mat & input, cv::Mat & output ) override;
};//ChessboardFinder

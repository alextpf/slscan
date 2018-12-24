#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

class Exporter
{
public:
	static void ExportToObj( const cv::Mat& pts, string path );
/*
	void SetWidth( const int w )
	{
		m_w = w;
	}

	void SetHeight( const int h )
	{
		m_h = h;
	}*/

private:
	/*int		m_w;
	int		m_h;*/
}; // Exporter
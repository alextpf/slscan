#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

class Exporter
{
public:
	static void ExportToObj(
		const vector<cv::Point3d>& pts,
		string path );

	static void ExportToPly(
		const vector<cv::Point3d>& pts,
		const vector<cv::Point3i>& color,
		string path );
private:
}; // Exporter
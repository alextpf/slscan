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

    // don't use it yet
    static void ExportToWrl(
        const vector<cv::Point3d>& pts,
        const vector<cv::Vec2f>& textureCoord,
        string path,
        string texturePath );

    static void SaveXYZAndTexture(
        const vector<cv::Point3d>& pts,
        const vector<cv::Vec2f>& textureCoord,
        string path );

private:
}; // Exporter
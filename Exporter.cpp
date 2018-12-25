#include <iostream>
#include <fstream>
#include "Exporter.h"

void Exporter::ExportToObj(
	const cv::Mat& pts,
	string path )
{
	const int w = pts.size().width;
	const int h = pts.size().height;

	std::ofstream out;
	out.open( path.c_str() );

	std::cout << "Export " << path << "...";

	for ( int r = 0; r < h; r++ )
	{
		for ( int c = 0; c < w; c++ )
		{
			cv::Point3f p = pts.at<float>( r, c );
			out << "v " << p.x << " " << p.y << " " << p.z << "\n";
		}
	}

	out.close();
	std::cout << "done!\n";
}//ExportToObj

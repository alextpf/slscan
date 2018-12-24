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

	for ( int i = 0; i < w; i++ )
	{
		for ( int j = 0; j < h; j++ )
		{
			cv::Point3f p = pts.at<double>( i, j );
			out << "v " << p.x << " " << p.y << " " << p.z << "\n";
		}
	}

	out.close();
	std::cout << "done!\n";
}//ExportToObj

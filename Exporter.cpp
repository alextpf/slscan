#include <iostream>
#include <fstream>
#include "Exporter.h"

void Exporter::ExportToObj(
	const vector<cv::Point3d>& pts,
	string path )
{
	const int siz = static_cast<int>( pts.size() );

	std::ofstream out;
	out.open( path.c_str() );

	std::cout << "Export " << path << "...";

	for ( int i = 0; i < siz; i++ )
	{
		out << "v " << pts[i].x << " " << pts[i].y << " " << pts[i].z << "\n";
	}

	out.close();
	std::cout << "done!\n";
}//ExportToObj

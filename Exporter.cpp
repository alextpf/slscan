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

//==================================
void Exporter::ExportToPly(
	const vector<cv::Point3d>& pts,
	const vector<cv::Point3i>& color,
	string path )
{
	const int siz = static_cast<int>( pts.size() );

	std::ofstream out;
	out.open( path.c_str() );

	std::cout << "Export " << path << "...";

	//ply headers
	out << "ply\n";
	out << "format ascii 1.0\n";

	out << "element vertex " << siz << "\n";
	out << "property float x\n";
	out << "property float y\n";
	out << "property float z\n";

	out << "property uchar red\n";
	out << "property uchar green\n";
	out << "property uchar blue\n";
	out << "end_header\n";

	for ( int i = 0; i < siz; i++ )
	{
		out << pts[i].x << " " << pts[i].y << " " << pts[i].z << "\n";
		out << color[i].x << " " << color[i].y << " " << color[i].z << "\n";
	}

	out.close();
}//ExportToPly
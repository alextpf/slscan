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
    std::cout << "done!\n";
}//ExportToPly

//==================================================
// Don't use!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void Exporter::ExportToWrl(
    const vector<cv::Point3d>& pts,
    const vector<cv::Vec2f>& textureCoord,
    string path,
    string texturePath )
{
    // BUG!!!! Need to generate faces (i.e. meshes)
    const int siz = static_cast<int>( pts.size() );

    std::ofstream out;
    out.open( path.c_str() );

    std::cout << "Export " << path << "...";

    out << "#VRML V2.0 utf8\n\n";
    out << "Shape {\n";
    out << "         appearance\n";
    out << "         Appearance {\n";
    out << "                       texture ImageTexture\n";
    out << "                          {\n";
    out << "                             url \"";
    out << texturePath << "\"\n";
    out << "#                            repeatS FALSE\n"; // some software doesn't read these 2 lines, ex: MeshLab
    out << "#                            repeatT FALSE\n";
    out << "                          }\n";
    out << "                    }\n";


    out << "         geometry\n";
    out << "         IndexedFaceSet {\n";
    out << "                           coord\n";

    // x, y, z
    out << "                           Coordinate {\n";
    out << "                                         point [\n";
    for( int i = 0; i < siz - 1; i++ )
    {
        out << "                                                " << pts[i].x << " " << pts[i].y << " " << pts[i].z << ",\n";
    }
    out << "                                                " << pts[siz-1].x << " " << pts[siz - 1].y << " " << pts[siz - 1].z << "\n";
    out << "                                               ]\n";
    out << "                                      }\n";// end of Coordinate

    // texture s t
    out << "                           texCoord\n";
    out << "                           TextureCoordinate {\n";
    out << "                                                point [\n";
    for( int i = 0; i < siz - 1; i++ )
    {
        out << "                                                       " << textureCoord[i][0] << " " << textureCoord[i][1] << ",\n";
    }
    out << "                                                       " << textureCoord[siz - 1][0] << " " << textureCoord[siz - 1][1] << "\n";
    out << "                                                      ]\n"; // end of point
    out << "                                             }\n"; // end of TextureCoordinate
    out << "                        }\n"; // end of IndexedFaceSet
    out << "      }\n"; // end of Shape
    out << "\n#End of VRML File\n";

    out.close();
    std::cout << "done!\n";
}//ExportToWrl

void Exporter::SaveXYZAndTexture(
    const vector<cv::Point3d>& pts,
    const vector<cv::Vec2f>& textureCoord,
    string path )
{
    const int siz = static_cast<int>( pts.size() );

    std::ofstream out;
    out.open( path.c_str() );

    std::cout << "Export " << path << "...";

    for( int i = 0; i < siz; i++ )
    {
        out << pts[i].x << " " << pts[i].y << " " << pts[i].z << "\n";
        out << textureCoord[i][0] << " " << textureCoord[i][1] << "\n";
    }

    out.close();
    std::cout << "done!\n";
}//SaveXYZAndTexture

//==========================
void Exporter::GeneratePlyFromXYZ(
    const string xyz,
    const string texture,
    const string ply )
{
    std::ifstream xyzFile;
    xyzFile.open( xyz.c_str(), std::ifstream::in );

    cv::Mat tex = cv::imread( texture.c_str() );

    cv::Size siz = tex.size();

    string line;

    vector<cv::Point3d> ptXYZ;
    vector<cv::Point3i> ptRGB;

    while( getline( xyzFile, line ) )
    {
        cv::Point3d XYZ;
        stringstream pts;
        pts.str( line );
        pts >> XYZ.x;
        pts >> XYZ.y;
        pts >> XYZ.z;

        ptXYZ.push_back( XYZ );

        getline( xyzFile, line );
        stringstream st;
        float s, t;

        st.str( line );
        st >> s;
        st >> t;

        int row = static_cast<int>( ( 1.0f - t ) * static_cast<float>( siz.height ) + 0.5f );
        int col = static_cast<int>( s * static_cast<float>( siz.width ) + 0.5f );

        cv::Point3i RGB;
        RGB.x = int( tex.at<cv::Vec3b>( row, col )[2] );// r
        RGB.y = int( tex.at<cv::Vec3b>( row, col )[1] );// g
        RGB.z = int( tex.at<cv::Vec3b>( row, col )[0] );// b

        ptRGB.push_back( RGB );
    }
    xyzFile.close();

    const int numPts = int( ptXYZ.size() );
    //

    std::ofstream out;
    out.open( ply.c_str() );

    //ply headers
    out << "ply\n";
    out << "format ascii 1.0\n";

    out << "element vertex " << numPts << "\n";
    out << "property float x\n";
    out << "property float y\n";
    out << "property float z\n";

    out << "property uchar red\n";
    out << "property uchar green\n";
    out << "property uchar blue\n";
    out << "end_header\n";

    std::cout << "Export " << ply << "...";

    for( int i = 0; i < numPts; i++ )
    {
        out << ptXYZ[i].x << " " << ptXYZ[i].y << " " << ptXYZ[i].z << "\n";
        out << ptRGB[i].x << " " << ptRGB[i].y << " " << ptRGB[i].z << "\n";
    }

    out.close();
    std::cout << "done!\n";

}//GeneratePlyFromXYZ

//=============================
void Exporter::ScaleZ(
	const string inPly,
	const float s,
	const string outPly )
{
	std::ifstream inFile;
	inFile.open( inPly.c_str(), std::ifstream::in );

	string line;

	vector<cv::Point3d> ptXYZ;
	vector<cv::Point3i> ptRGB;

	while ( getline( inFile, line ) )
	{

	}
}//ScaleZ
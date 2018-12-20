#pragma once
#include <vector>
using namespace std;

class GrayCode
{
public:
    GrayCode();

    void GeneratePattern();

	void ComputeNumPatternImgs(const int w, const int h );

	int GetWidth()
	{
		return m_Width;
	}

	int GetHeight()
	{
		return m_Height;
	}

	const vector<cv::Mat>& GetPattern()
	{
		return m_Pattern;
	}
private:

	void SetWidth( const int w )
	{
		m_Width = w;
	}

	void SetHeight( const int h )
	{
		m_Height = h;
	}

    int					m_Width;
    int					m_Height;
	int					m_NumColImgs;
	int					m_NumRowImgs;
	int					m_NumPatternImgs;
	vector<cv::Mat>		m_Pattern;
	cv::Mat				m_BlackImage;
	cv::Mat				m_WhiteImage;
}; // GrayCode

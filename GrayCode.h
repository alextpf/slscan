#pragma once

class GrayCode
{
public:
    GrayCode();

    void GeneratePattern();

    void SetWidth( const int w )
    {
        m_Width = w;
    }

    void SetHeight( const int h )
    {
        m_Height = h;
    }

private:
    int     m_Width;
    int     m_Height;
}; // GrayCode

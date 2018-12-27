//////////////////////////////////////////////////////
// Original Author: Jose Julio
// Modified by Alex Chen
//////////////////////////////////////////////////////

//========================================================================================================================================
// Some util functions...
//========================================================================================================================================
int freeRam () {
  extern int __heap_start, *__brkval;
  int v;
  return (int) &v - (__brkval == 0 ? (int) &__heap_start : (int) __brkval);
}
//========================================================================================================================================
// Arduino abs function sometimes fail!
template <class T>
T myAbs(T param)
{
  if (param < 0)
    return -param;
  else
    return param;
}

template <class T>
Point2D<T> Point2Abs( const Point2D<T>& param)
{
    return Point2D<T>(myAbs(param.m_X), myAbs(param.m_Y));
}
//========================================================================================================================================
// Extract sign of a variable
int sign(int val)
{
  if (val < 0)
    return (-1);
  else
    return (1);
}

//============= data input and outport routines============
#pragma once
bool loadCameraSettings(unsigned long (&camSN)[4]);
bool loadLightSettings(int &Imax1, int &Imin1, int &Lnum1, int &Imax234, int &Imin234, int &Lnum2, int &Lnum3, int &Lnum4);
bool readCamCaliData(char *file,CamPara *pCam);
bool writeLightIntensity(int Imax1, int Imin1, int Lnum1, int Imax234, int Imin234, int Lnum2, int Lnum3, int Lnum4);
bool load3DCloneSettings(int &patchSize,int &Wgua,int &Res,int &tPeriod,int &pDif,float &sgma,float &gama,int &overLapping,int &incBright,int &holeFillingMark,int &filterMark,int &AutoLevelsMark);
bool write3DCloneSettings(int patchSize,int Wgua,int Res,int tPeriod,int pDif,float sgma,float gama,int overLapping,int incBright,int holeFillingMark,int filterMark,int AutoLevelsMark);
void worldTOsensor(float *is,float *js,float *xyzWorld,CamPara *pCam);
void getST(float x,float y,float z,CamPara *pCam,int imgWidth,int imgHeight,float *s,float *t);

bool writeWrlFile(char *filename,Range *pXYZ,int txture);
bool writeWrlFile(char *filename,Range *pXYZ,char *jpgfile,int tWidth,int tHeight,CamPara *pCamT);

int getClnSize(int *col3D,int *row3D,int *col2D,int *row2D,char *filename);
bool readClnFile(Range *pXYZ,CamPara *pCamT,char *filename);
bool cln2wrl(char *clnFile,char *wrlFile);
#include "StdAfx.h"
#include <stdio.h>
#include <conio.h>
#include <malloc.h>
#include <stdlib.h>
#include <direct.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <io.h>

#include "dataStructure.h"


//extern int Imax1,Imin1,Lnum1,Imax234,Imin234,Lnum2,Lnum3,Lnum4;

bool loadCameraSettings(unsigned long (&camSN)[4])
{
	bool ret=true;
	
	FILE *fp;
	register int i;
	char *tmp[4];
	
	if(_access("CameraSN.cfg",0)) return ret=false;
	
	if(NULL==(fp=fopen("CameraSN.cfg","rb"))) return ret=false;
	
	for(i=0;i<4;i++)
		{
			tmp[i]=(char *) malloc(8*sizeof(char));
			if(NULL==tmp[i]) return ret=false;
		}
	for(i=0;i<4;i++)
		{
			fread(tmp[i],8*(sizeof (char)),1,fp);
			camSN[i]=atol(*(tmp+i));
		}
    fclose(fp);
	
	for(i=0;i<4;i++)
		{
			if(NULL!=tmp[i]) {free(tmp[i]);tmp[i]=NULL;}
		}

	return ret;
}

bool loadLightSettings(int &Imax1, int &Imin1, int &Lnum1, int &Imax234, int &Imin234, int &Lnum2, int &Lnum3, int &Lnum4)
{
	bool ret=true;
	
	FILE *fp;
	register int i;
	char *tmp[8];
	
	if(_access("LightSettings.cfg",0)) return ret=false;
	
	if(NULL==(fp=fopen("LightSettings.cfg","rb"))) return ret=false;
	
	for(i=0;i<8;i++)
		{
			tmp[i]=(char *) malloc(6*sizeof(char));
			if(NULL==tmp[i]) return ret=false;
		}
	for(i=0;i<8;i++)
		fread(tmp[i],6*(sizeof (char)),1,fp);
	fclose(fp);
	
	Lnum1=atoi(*(tmp+0));
	Imax1=atoi(*(tmp+1));
	Imin1=atoi(*(tmp+2));
	Lnum2=atoi(*(tmp+3));
	Lnum3=atoi(*(tmp+4));
	Lnum4=atoi(*(tmp+5));
	Imax234=atoi(*(tmp+6));
	Imin234=atoi(*(tmp+7));

	for(i=0;i<8;i++)
		{
			if(NULL!=tmp[i]) {free(tmp[i]);tmp[i]=NULL;}
		}

	return ret;
}

bool readCamCaliData(char *file,CamPara *pCam)
{
	bool ret=true;
	int i;
	char sTmp[255];
	FILE *fp;

	if(NULL==(fp=fopen(file,"r"))) return ret=false;

	fscanf(fp,"%s %s %s\n",sTmp,sTmp,sTmp);
	fscanf(fp,"%s\n\n",sTmp);

	fscanf(fp,"%s %s %s %s %s\n",sTmp,sTmp,sTmp,sTmp,sTmp);
	//float a,b,c;
	for(i=0;i<3;i++)
		fscanf(fp,"%f %f %f\n",&pCam->Rw2c[i][0],&pCam->Rw2c[i][1],&pCam->Rw2c[i][2]);
		//fscanf(fp,"%f %f %f\n",&a,&b,&c);

	fscanf(fp,"%s %s %s %s %s\n",sTmp,sTmp,sTmp,sTmp,sTmp);
	fscanf(fp,"%f %f %f\n",&pCam->Tw2c[0],&pCam->Tw2c[1],&pCam->Tw2c[2]);

	fscanf(fp,"%s %s %s %s %s\n",sTmp,sTmp,sTmp,sTmp,sTmp);
	for(i=0;i<3;i++)
		fscanf(fp,"%f %f %f\n",&pCam->Rc2w[i][0],&pCam->Rc2w[i][1],&pCam->Rc2w[i][2]);

	fscanf(fp,"%s %s %s %s %s\n",sTmp,sTmp,sTmp,sTmp,sTmp);
	fscanf(fp,"%f %f %f\n",&pCam->Tc2w[0],&pCam->Tc2w[1],&pCam->Tc2w[2]);

	fscanf(fp,"%s %s %f %f %f %f %f\n",sTmp,sTmp,&pCam->kdist[0],&pCam->kdist[1],&pCam->kdist[2],&pCam->kdist[3],&pCam->kdist[4]);
	fscanf(fp,"%s %s %f\n",sTmp,sTmp,&pCam->f);
	fscanf(fp,"%s %s %f\n",sTmp,sTmp,&pCam->sx);
	fscanf(fp,"%s %s %f %f\n",sTmp,sTmp,&pCam->xc,&pCam->yc);
	fscanf(fp,"%s %s %f %f\n",sTmp,sTmp,&pCam->dx,&pCam->dy);

	fclose(fp);

	return ret;
}

bool writeLightIntensity(int Imax1, int Imin1, int Lnum1, int Imax234, int Imin234, int Lnum2, int Lnum3, int Lnum4)
{
	
	BOOL ret=true;
	char tmp[8][6];
	FILE *fp;
	if(NULL==(fp=fopen("LightSettings.cfg","wb"))) return ret=false;

	itoa(Lnum1,tmp[0],10);
	itoa(Imax1,tmp[1],10);
	itoa(Imin1,tmp[2],10);
	itoa(Lnum2,tmp[3],10);
	itoa(Lnum3,tmp[4],10);
	itoa(Lnum4,tmp[5],10);
	itoa(Imax234,tmp[6],10);
	itoa(Imin234,tmp[7],10);
	
	for (int i=0;i<8;i++)
	{
		fwrite(tmp[i],6*(sizeof (char)),1,fp);		
	}
	fclose(fp);

		
	return ret;
}

bool load3DCloneSettings(int &patchSize,int &Wgua,int &Res,int &tPeriod,int &pDif,float &sgma,float &gama,int &overLapping,int &incBright,int &holeFillingMark,int &filterMark,int &AutoLevelsMark)
{
	bool ret=true;

	FILE *fp;
	register int i;
	char *tmp[12];

	if(_access("Clone3DSettings.cfg",0)) return ret=false;
	if(NULL==(fp=fopen("Clone3DSettings.cfg","rb"))) return ret=false;

	for(i=0;i<12;i++)
	{
		tmp[i]=(char *) malloc(8*sizeof(char));
		if(NULL==tmp[i]) return ret=false;
	}
	for(i=0;i<12;i++)
		fread(tmp[i],8*(sizeof (char)),1,fp);
	fclose(fp);

	Res=atoi(*(tmp+0));
	if(Res>2) Res=2;
	patchSize=atoi(*(tmp+1));
	Wgua=atoi(*(tmp+2));
	sgma=(float) atof(*(tmp+3));
	gama=(float) atof(*(tmp+4));
	tPeriod=atoi(*(tmp+5));
	pDif=atoi(*(tmp+6));
	overLapping=atoi(*(tmp+7));
	incBright=atoi(*(tmp+8));

	i=0;while(tmp[9][i]) {tmp[9][i]=tolower(tmp[9][i]);i++;}
	i=0;while(tmp[10][i]) {tmp[10][i]=tolower(tmp[10][i]);i++;}
	i=0;while(tmp[11][i]) {tmp[11][i]=tolower(tmp[11][i]);i++;}

	if(!strcmp(tmp[9],"enable")) holeFillingMark=1;
	else holeFillingMark=0;
	if(!strcmp(tmp[10],"enable")) filterMark=1;
	else filterMark=0;
	if(!strcmp(tmp[11],"enable")) AutoLevelsMark=1;
	else AutoLevelsMark=0;

	for(i=0;i<12;i++)
	{
		if(NULL!=tmp[i]) {free(tmp[i]);tmp[i]=NULL;}
	}
	return ret;
}
bool write3DCloneSettings(int patchSize,int Wgua,int Res,int tPeriod,int pDif,float sgma,float gama,int overLapping,int incBright,int holeFillingMark,int filterMark,int AutoLevelsMark)
{
	bool ret=true;
	FILE *fp;
	char tmp[12][8];

	if(NULL==(fp=fopen("Clone3DSettings.cfg","wb"))) return ret=false;

	if (Res>2)
		Res=2;
	
	itoa(Res,tmp[0],10);
	itoa(patchSize,tmp[1],10);
	itoa(Wgua,tmp[2],10);
	sprintf(tmp[3],"%f7.4",sgma); tmp[3][7]='\0';
	sprintf(tmp[4],"%f7.4",gama); tmp[4][7]='\0';
	itoa(tPeriod,tmp[5],10);
	itoa(pDif,tmp[6],10);
	itoa(overLapping,tmp[7],10);
	itoa(incBright,tmp[8],10);

	if (holeFillingMark)	
		strcpy(tmp[9],"enable");
	else
		strcpy(tmp[9],"disable");

	if (filterMark)	
		strcpy(tmp[10],"enable");
	else
		strcpy(tmp[10],"disable");

	if (AutoLevelsMark)	
		strcpy(tmp[11],"enable");
	else
		strcpy(tmp[11],"disable");
	
	for (int i=0;i<12;i++)
	{
		fwrite(tmp[i],8*(sizeof (char)),1,fp);
	}	
	fclose(fp);
	return true;
}


void worldTOsensor(float *is,float *js,float *xyzWorld,CamPara *pCam)
{
	int i,j;
	float xyzSensor[3];
	float r2,r4,r6,xx,xy,yy,kk;

	for(j=0;j<3;j++)
	{
		*(xyzSensor+j)=0.0f;
		for(i=0;i<3;i++) *(xyzSensor+j)+=pCam->Rw2c[j][i]*(*(xyzWorld+i));
		*(xyzSensor+j)+=pCam->Tw2c[j];
	}

	*(xyzSensor+0)*=pCam->f/(*(xyzSensor+2));
	*(xyzSensor+1)*=pCam->f/(*(xyzSensor+2));
	*(xyzSensor+2)=pCam->f;

	r2=*(xyzSensor+0)*(*(xyzSensor+0))+*(xyzSensor+1)*(*(xyzSensor+1));
	r4=r2*r2;r6=r4*r2;
	xx=r2+2.0f*(*(xyzSensor+0))*(*(xyzSensor+0));
	yy=r2+2.0f*(*(xyzSensor+1))*(*(xyzSensor+1));
	xy=2.0f*(*(xyzSensor+0))*(*(xyzSensor+1));

	kk=1.0f+pCam->kdist[0]*r2+pCam->kdist[1]*r4+pCam->kdist[2]*r6+pCam->kdist[3]*xx+pCam->kdist[4]*xy;
	*is=pCam->xc+pCam->sx*(*(xyzSensor+0))*kk/pCam->dx;

	kk=1.0f+pCam->kdist[0]*r2+pCam->kdist[1]*r4+pCam->kdist[2]*r6+pCam->kdist[3]*xy+pCam->kdist[4]*yy;
	*js=pCam->yc+*(xyzSensor+1)*kk/pCam->dy;
}

void getST(float x,float y,float z,CamPara *pCam,int imgWidth,int imgHeight,float *s,float *t)
{
	float i,j;
	float xyzWorld[3];

	xyzWorld[0]=x;xyzWorld[1]=y;xyzWorld[2]=z;
	worldTOsensor(&i,&j,xyzWorld,pCam);

	*s=i/(imgWidth-1);*t=1.0f-j/(imgHeight-1);

	if(*s<0) *s=0;
	else if(*s>1.0f) *s=1.0f;
	if(*t<0) *t=0;
	else if(*t>1.0f) *t=1.0f;
}


int getClnSize(int *col3D,int *row3D,int *col2D,int *row2D,char *filename)
{
	int ret;
	short w,h,imgType;
	FILE *fp; 

	if(NULL==(fp=fopen(filename,"rb"))) return ret=-1;

	fseek(fp,537L,SEEK_SET);
	fread(&w,sizeof(short),1,fp);
	fread(&h,sizeof(short),1,fp);
	*col3D=(int) w; *row3D=(int) h;

	fseek(fp,4L,SEEK_CUR);
	fread(&w,sizeof(short),1,fp);
	fread(&h,sizeof(short),1,fp);
	*col2D=(int) w; *row2D=(int) h;

	fread(&imgType,sizeof(short),1,fp);

	fclose(fp);

	return ret=(int) imgType;
}

bool writeWrlFile(char *filename,Range *pXYZ,int txture)
{
	bool ret=true;
	FILE *fp;
	int i,j,Numvertex,Numfacets;
	float *x,*y,*z,*r,*g,*b;
	int *idx,*idx1,*idx2,*idx3;

	Numvertex=pXYZ->col*pXYZ->row;

	x=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==x) return ret=false;
	y=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==y) return ret=false;
	z=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==z) return ret=false;

	if(txture)
	{
		r=(float*) malloc(Numvertex*sizeof(float));
		if(NULL==r) return ret=false;
		g=(float*) malloc(Numvertex*sizeof(float));
		if(NULL==g) return ret=false;
		b=(float*) malloc(Numvertex*sizeof(float));
		if(NULL==b) return ret=false;
	}

	idx=(int*) malloc(Numvertex*sizeof(int));
	if(NULL==idx) return ret=false;

	Numvertex=0;
	for(j=0;j<pXYZ->row;j++)
		for(i=0;i<pXYZ->col;i++)
		{
			if(pXYZ->Z[j][i]>TRASH+100.0f)
			{
				*(x+Numvertex)=pXYZ->X[j][i];
				*(y+Numvertex)=pXYZ->Y[j][i];
				*(z+Numvertex)=pXYZ->Z[j][i];

				if(txture)
				{
					*(r+Numvertex)=(float) pXYZ->R[j*pXYZ->col+i]/255.0f;
					*(g+Numvertex)=(float) pXYZ->G[j*pXYZ->col+i]/255.0f;
					*(b+Numvertex)=(float) pXYZ->B[j*pXYZ->col+i]/255.0f;
				}

				*(idx+j*pXYZ->col+i)=Numvertex;
				Numvertex++;
			}
		}
		*r=(float) rand()/RAND_MAX;*g=(float) rand()/RAND_MAX;*b=(float) rand()/RAND_MAX;

		Numfacets=2*(pXYZ->row-1)*(pXYZ->col-1);

		idx1=(int*) malloc(Numfacets*sizeof(int));
		if(NULL==idx1) return ret=false;
		idx2=(int*) malloc(Numfacets*sizeof(int));
		if(NULL==idx2) return ret=false;
		idx3=(int*) malloc(Numfacets*sizeof(int));
		if(NULL==idx3) return ret=false;

		Numfacets=0;
		for(j=1;j<pXYZ->row;j++)
			for(i=1;i<pXYZ->col;i++)
			{
				if((pXYZ->Z[j][i-1]>(TRASH+100.0f))&&(pXYZ->Z[j-1][i-1]>(TRASH+100.0f))&&(pXYZ->Z[j-1][i]>(TRASH+100.0f)))
				{
					*(idx1+Numfacets)=*(idx+(j-1)*pXYZ->col+i-1);
					*(idx2+Numfacets)=*(idx+j*pXYZ->col+i-1);
					*(idx3+Numfacets)=*(idx+(j-1)*pXYZ->col+i);
					Numfacets++;
				}

				if((pXYZ->Z[j][i-1]>(TRASH+100.0f))&&(pXYZ->Z[j][i]>(TRASH+100.0f))&&(pXYZ->Z[j-1][i]>(TRASH+100.0f)))
				{
					*(idx1+Numfacets)=*(idx+(j-1)*pXYZ->col+i);
					*(idx2+Numfacets)=*(idx+j*pXYZ->col+i-1);
					*(idx3+Numfacets)=*(idx+j*pXYZ->col+i);
					Numfacets++;
				}
			}

			if(NULL==(fp=fopen(filename,"w"))) return ret=false;

			if(fprintf(fp,"#VRML V2.0 utf8\n")<0) return ret=false;
			if(fprintf(fp,"Shape {\n")<0) return ret=false;

			if(!txture)
			{
				if(fprintf(fp,"         appearance\n")<0) return ret=false;
				if(fprintf(fp,"         Appearance {\n")<0) return ret=false;
				if(fprintf(fp,"                       material\n")<0) return ret=false;
				if(fprintf(fp,"                       Material {\n")<0) return ret=false;
				if(fprintf(fp,"                                   diffuseColor  0.8 0.8 0.8\n")<0) return ret=false;
				if(fprintf(fp,"                                   transparency 0\n")<0) return ret=false;
				if(fprintf(fp,"                                }\n")<0) return ret=false;
				if(fprintf(fp,"                    }\n")<0) return ret=false;
			}

			if(fprintf(fp,"         geometry\n")<0) return ret=false;
			if(fprintf(fp,"         IndexedFaceSet {\n")<0) return ret=false;
			if(fprintf(fp,"                           coord\n")<0) return ret=false;
			if(fprintf(fp,"                           Coordinate {\n")<0) return ret=false;
			if(fprintf(fp,"                                         point [\n")<0) return ret=false;
			for(i=0;i<Numvertex-1;i++)
				if(fprintf(fp,"                                                %g %g %g,\n",*(x+i),*(y+i),*(z+i))<0) return ret=false;
			if(fprintf(fp,"                                                %g %g %g\n",*(x+i),*(y+i),*(z+i))<0) return ret=false;
			if(fprintf(fp,"                                               ]\n")<0) return ret=false;
			if(fprintf(fp,"                                      }\n")<0) return ret=false;

			if(fprintf(fp,"                           coordIndex [\n")<0) return ret=false;
			for(i=0;i<Numfacets-1;i++)
				if(fprintf(fp,"                                       %d %d %d %d,\n",*(idx1+i),*(idx2+i),*(idx3+i),-1)<0) return ret=false;
			if(fprintf(fp,"                                       %d %d %d %d\n",*(idx1+i),*(idx2+i),*(idx3+i),-1)<0) return ret=false;
			if(fprintf(fp,"                                      ]\n")<0) return ret=false;

			if(txture) 
			{
				if(fprintf(fp,"            colorPerVertex  TRUE\n")<0) return ret=false;
			}
			else 
			{
				if(fprintf(fp,"		       colorPerVertex FALSE\n")<0) return ret=false;
			}

			if(txture)
			{
				if(fprintf(fp,"                           color\n")<0) return ret=false;
				if(fprintf(fp,"                           Color {\n")<0) return ret=false;
				if(fprintf(fp,"                                    color [\n")<0) return ret=false;
				for(i=0;i<Numvertex-1;i++)
					if(fprintf(fp,"                                           %g %g %g,\n",*(r+i),*(g+i),*(b+i))<0) return ret=false;
				if(fprintf(fp,"                                           %g %g %g\n",*(r+i),*(g+i),*(b+i))<0) return ret=false;
				if(fprintf(fp,"                                          ]\n")<0) return ret=false;
				if(fprintf(fp,"                                 }\n")<0) return ret=false;
			}
			if(fprintf(fp,"                        }\n")<0) return ret=false;
			if(fprintf(fp,"      }\n")<0) return ret=false;
			if(fprintf(fp,"#End of VRML File\n")<0) return ret=false;

			fclose(fp);

			if(NULL!=x) {free(x);x=NULL;}
			if(NULL!=y) {free(y);y=NULL;}
			if(NULL!=z) {free(z);z=NULL;}

			if(txture)
			{
				if(NULL!=r) {free(r);r=NULL;}
				if(NULL!=g) {free(g);g=NULL;}
				if(NULL!=b) {free(b);b=NULL;}
			}
			if(NULL!=idx) {free(idx);idx=NULL;}
			if(NULL!=idx1) {free(idx1);idx1=NULL;}
			if(NULL!=idx2) {free(idx2);idx2=NULL;}
			if(NULL!=idx3) {free(idx3);idx3=NULL;}

			return ret;
}
// NEWLY added by Ping
bool writeWrlFile(char *filename,Range *pXYZ,char *jpgfile,int tWidth,int tHeight,CamPara *pCamT)
{
	bool ret=true;
	FILE *fp;
	int i,j,Numvertex,Numfacets;
	float *x,*y,*z,*s,*t;
	int *idx,*idx1,*idx2,*idx3;

	Numvertex=pXYZ->col*pXYZ->row;

	x=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==x) return ret=false;
	y=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==y) return ret=false;
	z=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==z) return ret=false;

	s=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==s) return ret=false;
	t=(float*) malloc(Numvertex*sizeof(float));
	if(NULL==t) return ret=false;

	idx=(int*) malloc(Numvertex*sizeof(int));
	if(NULL==idx) return ret=false;

	Numvertex=0;
	for(j=0;j<pXYZ->row;j++)
		for(i=0;i<pXYZ->col;i++)
		{
			if(pXYZ->Z[j][i]>TRASH+100.0f)
			{
				*(x+Numvertex)=pXYZ->X[j][i];
				*(y+Numvertex)=pXYZ->Y[j][i];
				*(z+Numvertex)=pXYZ->Z[j][i];
				getST(*(x+Numvertex),*(y+Numvertex),*(z+Numvertex),pCamT,tWidth,tHeight,s+Numvertex,t+Numvertex);

				*(idx+j*pXYZ->col+i)=Numvertex;
				Numvertex++;
			}
		}

		Numfacets=2*(pXYZ->row-1)*(pXYZ->col-1);

		idx1=(int*) malloc(Numfacets*sizeof(int));
		if(NULL==idx1) return ret=false;
		idx2=(int*) malloc(Numfacets*sizeof(int));
		if(NULL==idx2) return ret=false;
		idx3=(int*) malloc(Numfacets*sizeof(int));
		if(NULL==idx3) return ret=false;

		Numfacets=0;
		for(j=1;j<pXYZ->row;j++)
			for(i=1;i<pXYZ->col;i++)
			{
				if((pXYZ->Z[j][i-1]>(TRASH+100.0f))&&(pXYZ->Z[j-1][i-1]>(TRASH+100.0f))&&(pXYZ->Z[j-1][i]>(TRASH+100.0f)))
				{
					*(idx1+Numfacets)=*(idx+(j-1)*pXYZ->col+i-1);
					*(idx2+Numfacets)=*(idx+j*pXYZ->col+i-1);
					*(idx3+Numfacets)=*(idx+(j-1)*pXYZ->col+i);
					Numfacets++;
				}

				if((pXYZ->Z[j][i-1]>(TRASH+100.0f))&&(pXYZ->Z[j][i]>(TRASH+100.0f))&&(pXYZ->Z[j-1][i]>(TRASH+100.0f)))
				{
					*(idx1+Numfacets)=*(idx+(j-1)*pXYZ->col+i);
					*(idx2+Numfacets)=*(idx+j*pXYZ->col+i-1);
					*(idx3+Numfacets)=*(idx+j*pXYZ->col+i);
					Numfacets++;
				}
			}

			if(NULL==(fp=fopen(filename,"w"))) return ret=false;

			if(fprintf(fp,"#VRML V2.0 utf8\n\n")<0) return ret=false;
			if(fprintf(fp,"Shape {\n")<0) return ret=false;
			if(fprintf(fp,"         appearance\n")<0) return ret=false;
			if(fprintf(fp,"         Appearance {\n")<0) return ret=false;
			if(fprintf(fp,"                       texture ImageTexture\n")<0) return ret=false;
			if(fprintf(fp,"                          {\n")<0) return ret=false;
			if(fprintf(fp,"                             url \"")<0) return ret=false;
			if(fprintf(fp,"%s\"\n",jpgfile)<0) return ret=false;
			if(fprintf(fp,"                             repeatS FALSE\n")<0) return ret=false;
			if(fprintf(fp,"                             repeatT FALSE\n")<0) return ret=false;
			if(fprintf(fp,"                          }\n")<0) return ret=false;
			if(fprintf(fp,"                    }\n")<0) return ret=false;

			if(fprintf(fp,"         geometry\n")<0) return ret=false;
			if(fprintf(fp,"         IndexedFaceSet {\n")<0) return ret=false;
			if(fprintf(fp,"                           coord\n")<0) return ret=false;
			if(fprintf(fp,"                           Coordinate {\n")<0) return ret=false;
			if(fprintf(fp,"                                         point [\n")<0) return ret=false;
			for(i=0;i<Numvertex-1;i++)
				if(fprintf(fp,"                                                %g %g %g,\n",*(x+i),*(y+i),*(z+i))<0) return ret=false;
			if(fprintf(fp,"                                                %g %g %g\n",*(x+i),*(y+i),*(z+i))<0) return ret=false;
			if(fprintf(fp,"                                               ]\n")<0) return ret=false;
			if(fprintf(fp,"                                      }\n")<0) return ret=false;

			if(fprintf(fp,"                           coordIndex [\n")<0) return ret=false;
			for(i=0;i<Numfacets-1;i++)
				if(fprintf(fp,"                                       %d %d %d %d,\n",*(idx1+i),*(idx2+i),*(idx3+i),-1)<0) return ret=false;
			if(fprintf(fp,"                                       %d %d %d %d\n",*(idx1+i),*(idx2+i),*(idx3+i),-1)<0) return ret=false;
			if(fprintf(fp,"                                      ]\n")<0) return ret=false;

			if(fprintf(fp,"                           texCoord\n")<0) return ret=false;
			if(fprintf(fp,"                           TextureCoordinate {\n")<0) return ret=false;
			if(fprintf(fp,"                                                point [\n")<0) return ret=false;
			for(i=0;i<Numvertex-1;i++)
				if(fprintf(fp,"                                                       %g %g,\n",*(s+i),*(t+i))<0) return ret=false;
			if(fprintf(fp,"                                                       %g %g\n",*(s+i),*(t+i))<0) return ret=false;
			if(fprintf(fp,"                                                      ]\n")<0) return ret=false;
			if(fprintf(fp,"                                             }\n")<0) return ret=false;

			if(fprintf(fp,"                        }\n")<0) return ret=false;
			if(fprintf(fp,"      }\n")<0) return ret=false;
			if(fprintf(fp,"\n#End of VRML File\n")<0) return ret=false;

			fclose(fp);

			if(NULL!=x) {free(x);x=NULL;}
			if(NULL!=y) {free(y);y=NULL;}
			if(NULL!=z) {free(z);z=NULL;}

			if(NULL!=s) {free(s);s=NULL;}
			if(NULL!=t) {free(t);t=NULL;}

			if(NULL!=idx) {free(idx);idx=NULL;}
			if(NULL!=idx1) {free(idx1);idx1=NULL;}
			if(NULL!=idx2) {free(idx2);idx2=NULL;}
			if(NULL!=idx3) {free(idx3);idx3=NULL;}

			return ret;
}


bool readClnFile(Range *pXYZ,CamPara *pCamT,char *filename)
{
	bool ret=true;
	int i,j;
	short w,h,imgType,gap;
	FILE *fp; 

	if(NULL==(fp=fopen(filename,"rb"))) return ret=false;

	fseek(fp,397L,SEEK_SET);
	fread(pCamT,sizeof(CamPara),1,fp);
	fread(&w,sizeof(short),1,fp);
	fread(&h,sizeof(short),1,fp);

	pXYZ->col=(int) w;pXYZ->row=(int) h;

	fseek(fp,4L,SEEK_CUR);
	fread(&w,sizeof(short),1,fp);
	fread(&h,sizeof(short),1,fp);
	fread(&imgType,sizeof(short),1,fp);

	fseek(fp,473L,SEEK_CUR);

	for(i=0;i<pXYZ->row;i++)
		fread((void *) pXYZ->X[i],pXYZ->col*sizeof(float),1,fp);
	for(i=0;i<pXYZ->row;i++)
		fread((void *) pXYZ->Y[i],pXYZ->col*sizeof(float),1,fp);
	for(i=0;i<pXYZ->row;i++)
		fread((void *) pXYZ->Z[i],pXYZ->col*sizeof(float),1,fp);

	gap=w/pXYZ->col-1;

	if(imgType==1)
	{
		for(j=0;j<pXYZ->row;j++)
		{
			for(i=0;i<pXYZ->col;i++)
			{
				fread((void *) &pXYZ->R[j*pXYZ->col+i],sizeof(unsigned char),1,fp);
				fseek(fp,gap,SEEK_CUR);
			}
			fseek(fp,gap*w,SEEK_CUR);
		}
	}
	else if(imgType==2)
	{
		for(j=0;j<pXYZ->row;j++)
		{
			for(i=0;i<pXYZ->col;i++)
			{
				fread((void *) &pXYZ->R[j*pXYZ->col+i],sizeof(unsigned char),1,fp);
				fseek(fp,gap,SEEK_CUR);
			}
			fseek(fp,gap*w,SEEK_CUR);
		}

		for(j=0;j<pXYZ->row;j++)
		{
			for(i=0;i<pXYZ->col;i++)
			{
				fread((void *) &pXYZ->G[j*pXYZ->col+i],sizeof(unsigned char),1,fp);
				fseek(fp,gap,SEEK_CUR);
			}
			fseek(fp,gap*w,SEEK_CUR);
		}

		for(j=0;j<pXYZ->row;j++)
		{
			for(i=0;i<pXYZ->col;i++)
			{
				fread((void *) &pXYZ->B[j*pXYZ->col+i],sizeof(unsigned char),1,fp);
				fseek(fp,gap,SEEK_CUR);
			}
			fseek(fp,gap*w,SEEK_CUR);
		}
	}

	fclose(fp);

	return ret;
}

bool cln2wrl(char *clnFile,char *wrlFile)
{
	bool ret=true;
	Range *pXYZ;
	CamPara *pCamT;
	char jpgFile[255];
	int imgType,col3D,row3D,col2D,row2D,n,k;

	imgType=getClnSize(&col3D,&row3D,&col2D,&row2D,clnFile);
	if(imgType<0) return ret=false;

	pCamT=(CamPara *) malloc(sizeof(CamPara));
	if(NULL==pCamT) return ret=false;

	pXYZ=(Range *) malloc(sizeof(Range));
	if(NULL==pXYZ) return ret=false;

	pXYZ->col=col3D;pXYZ->row=row3D;n=col3D*row3D;

	pXYZ->X=(float **) malloc(pXYZ->row*sizeof(float));
	if(NULL==pXYZ->X) return ret=false;
	pXYZ->Y=(float **) malloc(pXYZ->row*sizeof(float));
	if(NULL==pXYZ->Y) return ret=false;
	pXYZ->Z=(float **) malloc(pXYZ->row*sizeof(float));
	if(NULL==pXYZ->Z) return ret=false;

	for(k=0;k<pXYZ->row;k++)
	{
		pXYZ->X[k]=(float *) malloc(pXYZ->col*sizeof(float));
		if(NULL==pXYZ->X[k]) return ret=false;
		pXYZ->Y[k]=(float *) malloc(pXYZ->col*sizeof(float));
		if(NULL==pXYZ->Y[k]) return ret=false;
		pXYZ->Z[k]=(float *) malloc(pXYZ->col*sizeof(float));
		if(NULL==pXYZ->Z[k]) return ret=false;
	}

	pXYZ->R=(unsigned char *) malloc(n*sizeof(unsigned char));
	if(NULL==pXYZ->R) return ret=false;
	pXYZ->G=(unsigned char *) malloc(n*sizeof(unsigned char));
	if(NULL==pXYZ->G) return ret=false;
	pXYZ->B=(unsigned char *) malloc(n*sizeof(unsigned char));
	if(NULL==pXYZ->B) return ret=false;

	if(NULL==readClnFile(pXYZ,pCamT,clnFile)) ret=false;

	if(ret)
	{
		if(imgType==3)
		{
			n=(int) strlen(clnFile);
			for(k=0;k<n;k++)
			{
				jpgFile[k]=clnFile[k];
				if(jpgFile[k]=='.') break;
			}
			jpgFile[k]='\0';
			strcat(jpgFile,".jpg");

			if(NULL==writeWrlFile(wrlFile,pXYZ,jpgFile,col2D,row2D,pCamT)) ret=false;
		}
		else
		{
			if(NULL==writeWrlFile(wrlFile,pXYZ,1)) ret=false;
		}
	}

	if(NULL!=pXYZ)
	{
		if(NULL!=pXYZ->X)
		{
			for(k=0;k<pXYZ->row;k++) 
				if(NULL!=pXYZ->X[k]) {free(pXYZ->X[k]);pXYZ->X[k]=NULL;}
				free(pXYZ->X);pXYZ->X=NULL;
		}
		if(NULL!=pXYZ->Y)
		{
			for(k=0;k<pXYZ->row;k++) 
				if(NULL!=pXYZ->Y[k]) {free(pXYZ->Y[k]);pXYZ->Y[k]=NULL;}
				free(pXYZ->Y);pXYZ->Y=NULL;
		}
		if(NULL!=pXYZ->Z)
		{
			for(k=0;k<pXYZ->row;k++) 
				if(NULL!=pXYZ->Z[k]) {free(pXYZ->Z[k]);pXYZ->Z[k]=NULL;}
				free(pXYZ->Z);pXYZ->Z=NULL;
		}

		if(NULL!=pXYZ->R) {free(pXYZ->R);pXYZ->R=NULL;}
		if(NULL!=pXYZ->G) {free(pXYZ->G);pXYZ->G=NULL;}
		if(NULL!=pXYZ->B) {free(pXYZ->B);pXYZ->B=NULL;}

		free(pXYZ);pXYZ=NULL;
	}

	if(NULL!=pCamT) {free(pCamT);pCamT=NULL;}

	return ret;
}

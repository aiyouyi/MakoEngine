#include "MLS.h"
#include <math.h>

MLS::MLS(void)
{
}


MLS::~MLS(void)
{
}

//释放vector容器中的内存
//vector中存放二维数组,二维数组的行数为nheight
static void ClearVector( vector<float**>& vt , int nheight) 
{
	vector<float**>::iterator iter;  
	for (iter=vt.begin(); iter != vt.end(); iter++)  
	{  
		float** tmp = *iter;
		for (int i=0; i<nheight; i++)
		{
			SAFE_DELETE(tmp[i]);
		}
		SAFE_DELETE(tmp);
	}
	vt.clear();
}

void MLS::MLSD2DpointsDefRigid(float** srcPoints, float** desPoints, float** p, float** q, int nplength, int nPoints)
{
	//为权重矩阵weight分配空间并初始化为0
	//大小为nplength行ngvlength列
	float** weight = new float*[nplength];

	for (int i=0; i<nplength; i++)
	{
		weight[i] = new float[nPoints];
		//初始化为0
		memset(weight[i], 0, nPoints * sizeof(float));
	}
	//计算权重weight，大小：nplength行ngvlength列
	PrecomputeWeights(p, srcPoints, weight, nplength, nPoints);	

	//存储转换矩阵A的容器
	vector<float**> vA;

	//normof_v_Pstar为|v-p*|值
	float* normof_v_Pstar = new float[nPoints];
	//初始化为0
	memset(normof_v_Pstar, 0, nPoints*sizeof(float));

	PrecomputeRigid(p, srcPoints, weight, normof_v_Pstar, vA, nPoints, nplength);
	PointsTransformRigid(vA, normof_v_Pstar, weight, q, desPoints, nplength, nPoints);

	/*for (int j = 0;j<nPoints; j++)
	{
		int x = desPoints[1][j] + desPoints[1][j]/abs(desPoints[1][j])* 0.5f;
		int y = desPoints[0][j] + desPoints[0][j]/abs(desPoints[0][j])* 0.5f;
		int t = j;
	}*/
	ClearVector(vA, 4); 
	for (int i=0;i<nplength;i++)
	{
		SAFE_DELETE_ARRAY(weight[i]);
	}
	SAFE_DELETE_ARRAY(weight);
	SAFE_DELETE_ARRAY(normof_v_Pstar);
}

void MLS::MLSD2DpointsDefAffine(float** srcPoints, float** desPoints, float** p, float** q, int nplength, int nPoints)
{
	//为权重矩阵weight分配空间并初始化为0
	//大小为nplength行ngvlength列
	float** weight = new float*[nplength];

	for (int i=0; i<nplength; i++)
	{
		weight[i] = new float[nPoints];
		//初始化为0
		memset(weight[i], 0, nPoints * sizeof(float));
	}
	//计算权重weight，大小：nplength行ngvlength列
	PrecomputeWeights(p, srcPoints, weight, nplength, nPoints);	

	float** fA = new float*[nplength];
	for (int i=0;i<nplength;i++)
	{
		fA[i] = new float[nPoints];
	}
	PrecomputeAffine(p, srcPoints, weight, fA, nPoints, nplength);
	PointsTransformAffine(fA, weight, q, desPoints, nplength, nPoints);

	/*for (int j = 0;j<nPoints; j++)
	{
	int x = desPoints[1][j] + desPoints[1][j]/abs(desPoints[1][j])* 0.5f;
	int y = desPoints[0][j] + desPoints[0][j]/abs(desPoints[0][j])* 0.5f;
	int t = j;
	}*/
	for (int i=0;i<nplength;i++)
	{
		SAFE_DELETE_ARRAY(fA[i]);
		SAFE_DELETE_ARRAY(weight[i]);
	}
	SAFE_DELETE_ARRAY(fA);
	SAFE_DELETE_ARRAY(weight);
}

/*************************************************************************
** FunctionName：    PrecomputeWeights
** @Param p：        控制点p各点坐标信息
** @Param v：        原图各像素点坐标信息
** @Param w:         权重信息
** @Param nplength:  控制点p的个数
** @Param nvlength:  数组v的列数
** @Param a：        调节变形参数，默认为2.0
** Comment:          利用各像素点坐标和控制点p计算权重
** return：          void
** Creator：         Ted
** Date：		     2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：         1.0.0
*************************************************************************/
void MLS::PrecomputeWeights(float** p, float** v, float** w, int nplength, int nvlength)
{
	for (int i=0; i<nplength; i++)
	{
		for (int j=0; j<nvlength; j++)
		{
			float temp = (p[0][i] - v[0][j]) * (p[0][i] - v[0][j]) + (p[1][i] - v[1][j]) * (p[1][i] - v[1][j]);

			w[i][j] = 1/(temp * temp + 0.00000001f);
		}

	}
}

/*********************************************************************
** FunctionName：      PrecomputeWCentroids
** @Param p：          控制点p，大小为2行nweight列
** @Param weight：     计算得出的权重数组，大小为nweight行nlength列
** @Param pPstar:      计算结果p*或q*
** @Param nlength：    weight的列数
** @Param nheight:     p的列数，weight的行数
** Comment:            计算p*或q*的值
                       公式2.5和2.6
** return：            void
** Creator：           Ted
** Date：		       2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：           1.0.0
*********************************************************************/
void MLS::PrecomputeWCentroids(float** p, float** weight, float** pPstar, int nlength, int nwheight)
{
	//p*中间变量
	float** ptemp = new float*[2];
	for (int i=0; i<2; i++)
	{
		ptemp[i] = new float[nlength];
	}
	//p矩阵与weight矩阵相乘存入ptemp中
	muliplymatrix(p, weight, ptemp, nlength, 2, nwheight);

	for (int j = 0; j<nlength; j++)
	{
		float nsumw = 0;
		for (int i=0; i<nwheight; i++)
		{
			nsumw += weight[i][j];
		}
		pPstar[0][j] = ptemp[0][j] / nsumw;
		pPstar[1][j] = ptemp[1][j] / nsumw;
	}

	SAFE_DELETE_ARRAY(ptemp[0]);
	SAFE_DELETE_ARRAY(ptemp[1]);
	SAFE_DELETE_ARRAY(ptemp);
}

/*********************************************************************
** FunctionName：           PrecomputeAffine
** @Param p：               控制点p各点坐标信息
** @Param v：               原图各像素点坐标信息
** @Param weight:           各点对于控制点p各点的权值
** @Param A:                转换矩阵A信息
** @Param nvlength：        v的列数
** @Param nplength：        控制点p的个数(数组p的列数)
** Comment:                 根据仿射变换对原图坐标信息和控制各点p
                            构造公式的A矩阵信息
** return：                 void
** Creator：				Ted
** Date：					2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：				1.0.0
*********************************************************************/
void MLS::PrecomputeAffine(float** p, float** v, float** weight, float** A, int nvlength, int nplength)
{
	//声明p*
	float** pPstar = new float*[2];
	pPstar[0] = new float[nvlength];
	pPstar[1] = new float[nvlength];

	//利用各控制点pi和权重weight计算p*(pPstar)
	PrecomputeWCentroids(p, weight, pPstar, nvlength, nplength);

	float** M1 = new float*[2];
	M1[0] = new float[nvlength];
	M1[1] = new float[nvlength];

	float** F1 = new float*[2];
	F1[0] = new float[nvlength];
	F1[1] = new float[nvlength];

	for (int j=0; j<nvlength; j++)
	{
		M1[0][j] = v[0][j] - pPstar[0][j];
		M1[1][j] = v[1][j] - pPstar[1][j];
	}

	float* a = new float[nvlength];
	float* b = new float[nvlength];
	float* d = new float[nvlength];
	float* det = new float[nvlength];

	float* Ia = new float[nvlength];
	float* Ib = new float[nvlength];
	float* Id = new float[nvlength];
	//初始化为0
	memset(a, 0, nvlength * sizeof(float));
	memset(b, 0, nvlength * sizeof(float));
	memset(d, 0, nvlength * sizeof(float));
	memset(det, 0, nvlength * sizeof(float));

	for (int i=0; i<nplength; i++)
	{
		for (int j = 0;j<nvlength; j++)
		{
			a[j] += weight[i][j] * (p[0][i] - pPstar[0][j]) * (p[0][i] - pPstar[0][j]);
			b[j] += weight[i][j] * (p[0][i] - pPstar[0][j]) * (p[1][i] - pPstar[1][j]);
			d[j] += weight[i][j] * (p[1][i] - pPstar[1][j]) * (p[1][i] - pPstar[1][j]);

		}
	}

	for (int j=0; j<nvlength; j++)
	{
		det[j] = a[j] * d[j] - b[j] * b[j];
		Ia[j] = d[j] / det[j];
		Ib[j] = (-1) * b[j] / det[j];
		Id[j] = a[j] / det[j];

		F1[0][j] = M1[0][j] * Ia[j] + M1[1][j] * Ib[j];
		F1[1][j] = M1[0][j] * Ib[j] + M1[1][j] * Id[j];
	}

	for (int i=0; i<nplength; i++)
	{
		for (int j=0; j<nvlength; j++)
		{
			A[i][j] = (F1[0][j] * (p[0][i] - pPstar[0][j]) + F1[1][j] * (p[1][i] - pPstar[1][j])) * weight[i][j];
		}
	}

	//释放临时变量所占内存
	SAFE_DELETE_ARRAY(pPstar[0]);
	SAFE_DELETE_ARRAY(pPstar[1]);
	SAFE_DELETE_ARRAY(pPstar);
	SAFE_DELETE_ARRAY(M1[0]);
	SAFE_DELETE_ARRAY(M1[1]);
	SAFE_DELETE_ARRAY(M1);
	SAFE_DELETE_ARRAY(a);
	SAFE_DELETE_ARRAY(b);
	SAFE_DELETE_ARRAY(d);
	SAFE_DELETE_ARRAY(det);
	SAFE_DELETE_ARRAY(Ia);
	SAFE_DELETE_ARRAY(Ib);
	SAFE_DELETE_ARRAY(Id);
	SAFE_DELETE_ARRAY(F1[0]);
	SAFE_DELETE_ARRAY(F1[1]);
	SAFE_DELETE_ARRAY(F1);
}

/*********************************************************************
** FunctionName：           PrecomputeRigid
** @Param p：               控制点p各点坐标信息
** @Param v：               原图各像素点坐标信息
** @Param weight:           各点对于控制点p各点的权值
** @Param normof_v_Pstar：  公式2.23中|v-p*|值
** @Param vA:               公式2.23中各A矩阵信息
** @Param nvlength：        v的列数
** @Param nplength：        控制点p的个数(数组p的列数)
** Comment:                 根据刚性变换对原图坐标信息和控制各点p
                            构造公式的A矩阵信息以及|v-p*|值
** return：                 void
** Creator：				Ted
** Date：					2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：				1.0.0
*********************************************************************/
void MLS::PrecomputeRigid(float** p, float** v, float** weight, float* normof_v_Pstar, vector<float**> &vA, int nvlength, int nplength)
{
	//声明p*
	float** pPstar = new float*[2];
	pPstar[0] = new float[nvlength];
	pPstar[1] = new float[nvlength];

	//利用各控制点pi和权重weight计算p*(pPstar)
	PrecomputeWCentroids(p, weight, pPstar, nvlength, nplength);

	//计算p各点减去中心值p*后的值,即^pi,存入vPhat
	vector<float**> vPhat;
	for (int i=0; i<nplength; i++)
	{
		float** phtm = new float*[2];
		phtm[0] = new float[nvlength];
		phtm[1] = new float[nvlength];

		for (int j = 0;j<nvlength; j++)
		{
			phtm[0][j] = p[0][i] - pPstar[0][j];
			phtm[1][j] = p[1][i] - pPstar[1][j];
		}
		vPhat.push_back(phtm);
		phtm = NULL;
	}

	//计算Ai各个矩阵放入vA容器中，The norm of v-Pstar为公式2.23前部分|v-p*|
	PrecomputeA(pPstar, vPhat, v, weight, vA, normof_v_Pstar, nvlength, nplength);
	
	//释放临时变量所占内存
	SAFE_DELETE_ARRAY(pPstar[0]);
	SAFE_DELETE_ARRAY(pPstar[1]);
	SAFE_DELETE_ARRAY(pPstar);
	ClearVector(vPhat, 2);
}

/*********************************************************************
** FunctionName：           muliplymatrix
** @Param leftMatrix：      矩阵相乘的被成数矩阵
** @Param rightMatrix：     矩阵相乘的乘数矩阵
** @Param desMatrix:        结果矩阵
** @Param nwidth：          结果矩阵的列数，乘数矩阵的列数
** @Param nheight:          结果矩阵的行数，被乘数矩阵的行数
** @Param nmiddle：         被乘数矩阵的列数，乘数矩阵的行数
** Comment:                 矩阵相乘
                            leftMatrix大小为nheight*nmiddle
                            rightMatrix大小为nmiddle*nwidth
** return：					void
** Creator：				Ted
** Date：					2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：				1.0.0
*********************************************************************/
void MLS::muliplymatrix(float** leftMatrix, float** rightMatrix, float** desMatrix, int nwidth, int nheight, int nmiddle)
{
	for (int i=0; i<nheight; i++)
	{
		for (int j=0; j<nwidth; j++)
		{
			desMatrix[i][j] = 0;
			for (int k=0; k<nmiddle; k++)
			{
				desMatrix[i][j] += leftMatrix[i][k] * rightMatrix[k][j];
			}
		}
	}
}

/*********************************************************************
** FunctionName：          PointsTransformAffine
** @Param vA:              容器：存放转换矩阵A
** @Param weight：         权重信息
** @Param q:               控制点q的坐标信息
** @Param fv:              图像像素点坐标信息(映射表)
** @Param nqlength:        q的个数，也是weight的行数
** @Param nlength:         fv的列数
** Comment:                按照刚性变换公式2.23构造出转换函数fr(v)
** return：                void
** Creator：               Ted
** Date：		           2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：               1.0.0
*********************************************************************/
void MLS::PointsTransformAffine(float** fA, float** weight, float** q, float** fv, int nqlength, int nlength)
{
	//声明q*
	float** Qstar = new float*[2];
	Qstar[0] = new float[nlength];
	Qstar[1] = new float[nlength];

	//利用控制点q和weight计算q*
	PrecomputeWCentroids(q, weight, Qstar, nlength, nqlength);

	memcpy(fv[0], Qstar[0], nlength * sizeof(float));
	memcpy(fv[1], Qstar[1], nlength * sizeof(float));

	for (int i=0; i<nqlength; i++)
	{
		for (int j=0; j<nlength; j++)
		{
			float Qhat1 = q[0][i] - Qstar[0][j];
			float Qhat2 = q[1][i] - Qstar[1][j];

			fv[0][j] += Qhat1 * fA[i][j];
			fv[1][j] += Qhat2 * fA[i][j];
		}
	}
	SAFE_DELETE_ARRAY(Qstar[0]);
	SAFE_DELETE_ARRAY(Qstar[1]);
	SAFE_DELETE_ARRAY(Qstar);
}

/*********************************************************************
** FunctionName：      PrecomputeA
** @Param Pstar：      p*值
** @Param vPhat：      ^pi = pi-p*的值，vPhat为一个向量容器
** @Param v:           图像像素点坐标信息
** @Param weight：     权重信息
** @Param vA:          vA为容器，存放利用公式2.15得到的各A矩阵信息
** @Param v_Pstar：    为v各点横纵坐标与p*的差值，为求得|v-p*|做准备
** @Param nlength:     v的列数
** @Param nheight:     p的列数，weight的行数
** Comment:            计算公式2.15中转换矩阵中的各A矩阵信息
                       存入vector容器中
** return：            void
** Creator：           Ted
** Date：		       2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：           1.0.0
*********************************************************************/
void MLS::PrecomputeA(float** Pstar, vector<float**> vPhat, float** v, float** weight, vector<float**> &vA, float* v_Pstar,int nlength, int nheight)
{
	//临时变量矩阵R1\R2
	// R1 = v-p*
	// R2 = -(v-p*)⊥
	float** R1 = new float*[2];
	float** R2 = new float*[2];
	R1[0] = new float[nlength];
	R1[1] = new float[nlength];
	R2[0] = new float[nlength];
	R2[1] = new float[nlength];

	//临时变量矩阵L1\L2
	float** L1 = NULL;

	for (int j=0; j<nlength; j++)
	{
		R1[0][j] = v[0][j] - Pstar[0][j];
		R1[1][j] = v[1][j] - Pstar[1][j];
		R2[0][j] = R1[1][j];
		R2[1][j] = (-1) * R1[0][j];
	}

	for (int i=0; i<nheight; i++)
	{
		//矩阵A包含4个分量a、b、c、d
		//为其分配空间
		float** A = new float*[4];
		A[0] = new float[nlength];
		A[1] = new float[nlength];
		A[2] = new float[nlength];
		A[3] = new float[nlength];
		L1 = vPhat[i];   //^pi
		for (int j=0; j<nlength; j++)
		{
			//计算Ai的各维信息 公式2.15
			A[0][j] = weight[i][j] * (L1[0][j] * R1[0][j] + L1[1][j] * R1[1][j]);      //a
			A[1][j] = weight[i][j] * (L1[0][j] * R2[0][j] + L1[1][j] * R2[1][j]);      //b
			A[2][j] = weight[i][j] * (L1[1][j] * R1[0][j] + (-1)*L1[0][j] * R1[1][j]); //c
			A[3][j] = weight[i][j] * (L1[1][j] * R2[0][j] + (-1)*L1[0][j] * R2[1][j]); //d
			//公式2.23前部分|v-p*|
			v_Pstar[j] = sqrt(pow(R1[0][j] ,2) + pow(R1[1][j], 2));
		}
		vA.push_back(A);
		A = NULL;
	}

	L1 = NULL;
	SAFE_DELETE_ARRAY(R1[0]);
	SAFE_DELETE_ARRAY(R1[1]);
	SAFE_DELETE_ARRAY(R1);
	SAFE_DELETE_ARRAY(R2[0]);
	SAFE_DELETE_ARRAY(R2[1]);
	SAFE_DELETE_ARRAY(R2);
}

/*********************************************************************
** FunctionName：          PointsTransformRigid
** @Param vA:              容器：存放转换矩阵中各A矩阵
** @Param normof_v_Pstar： 公式2.23中|v-p*|值
** @Param weight：         权重信息
** @Param q:               控制点q的坐标信息
** @Param fv:              图像像素点坐标信息
** @Param nqlength:        q的个数，也是weight的行数
** @Param nlength:         fv的列数
** Comment:                按照刚性变换公式2.23构造出转换函数fr(v)
** return：                void
** Creator：               Ted
** Date：		           2015-01-12
** Modifier：	   
** ModifyDate：    
** Version：               1.0.0
*********************************************************************/
void MLS::PointsTransformRigid(vector<float**> vA, float* normof_v_Pstar, float** weight, float** q, float** fv, int nqlength, int nlength)
{
	//声明q*
	float** Qstar = new float*[2];
	Qstar[0] = new float[nlength];
	Qstar[1] = new float[nlength];

	//利用控制点q和weight计算q*
	PrecomputeWCentroids(q, weight, Qstar, nlength, nqlength);

	//映射表中间变量
	float** fvtmp = new float*[2];
	fvtmp[0] = new float[nlength];
	fvtmp[1] = new float[nlength];
	//初始化为0
	memset(fvtmp[0], 0, nlength * sizeof(float));
	memset(fvtmp[1], 0, nlength * sizeof(float));

	for (int i=0; i<nqlength; i++)
	{
		float** A = vA[i];
		for (int j=0; j<nlength; j++)
		{
			float Qcha1 = q[0][i] - Qstar[0][j];
			float Qcha2 = q[1][i] - Qstar[1][j];
			//计算公式2.23中加号前部分分子->fr(v)
			fvtmp[0][j] += Qcha1 * A[0][j] + Qcha2 * A[2][j];
			fvtmp[1][j] += Qcha1 * A[1][j] + Qcha2 * A[3][j];
		}
	}

	for (int j=0;j<nlength;j++)
	{
		//计算公式2.23加号前部分分母|->fr(v)|
		float norm_fact = normof_v_Pstar[j] / sqrt(pow(fvtmp[0][j], 2) + pow(fvtmp[1][j], 2));

		//fvtmp[0][j]*norm_fact为公式2.23加号前部
		//fv为完成公式2.23
		fv[0][j] = fvtmp[0][j] * norm_fact + Qstar[0][j];
		fv[1][j] = fvtmp[1][j] * norm_fact + Qstar[1][j];
	}

	SAFE_DELETE_ARRAY(Qstar[0]);
	SAFE_DELETE_ARRAY(Qstar[1]);
	SAFE_DELETE_ARRAY(Qstar);
	SAFE_DELETE_ARRAY(fvtmp[0]);
	SAFE_DELETE_ARRAY(fvtmp[1]);
	SAFE_DELETE_ARRAY(fvtmp);
}
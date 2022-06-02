#include "InterFacePointDeform.h"
#include "MathUtils.h"
#include "Matrices.h"
#include "MLS.h"
#include <algorithm>

//标准人脸的纹理坐标
static Vector2 g_StandCoord[310] = {
	Vector2(0.281250f,0.368750f),
	Vector2(0.312761f,0.353758f),
	Vector2(0.343048f,0.347200f),
	Vector2(0.372110f,0.349075f),
	Vector2(0.399948f,0.359383f),
	Vector2(0.426563f,0.378125f),
	Vector2(0.309258f,0.377888f),
	Vector2(0.337794f,0.383394f),
	Vector2(0.366856f,0.385269f),
	Vector2(0.396446f,0.383513f),
	Vector2(0.585938f,0.371875f),
	Vector2(0.610805f,0.355051f),
	Vector2(0.636833f,0.345702f),
	Vector2(0.664021f,0.343827f),
	Vector2(0.692368f,0.349426f),
	Vector2(0.721875f,0.362500f),
	Vector2(0.614254f,0.377277f),
	Vector2(0.642006f,0.379040f),
	Vector2(0.669194f,0.377165f),
	Vector2(0.695817f,0.371652f),
	Vector2(0.229687f,0.321875f),
	Vector2(0.269062f,0.303710f),
	Vector2(0.308437f,0.294628f),
	Vector2(0.347813f,0.294628f),
	Vector2(0.387188f,0.303710f),
	Vector2(0.426563f,0.321875f),
	Vector2(0.269062f,0.315750f),
	Vector2(0.308437f,0.312688f),
	Vector2(0.347813f,0.312688f),
	Vector2(0.387188f,0.315750f),
	Vector2(0.576562f,0.318750f),
	Vector2(0.613685f,0.299529f),
	Vector2(0.651465f,0.289293f),
	Vector2(0.689903f,0.288043f),
	Vector2(0.728998f,0.295779f),
	Vector2(0.768750f,0.312500f),
	Vector2(0.614717f,0.313632f),
	Vector2(0.653013f,0.310448f),
	Vector2(0.691451f,0.309198f),
	Vector2(0.730029f,0.309882f),
	Vector2(0.501563f,0.320313f),
	Vector2(0.506250f,0.375000f),
	Vector2(0.478125f,0.415625f),
	Vector2(0.450000f,0.456250f),
	Vector2(0.421875f,0.496875f),
	Vector2(0.531250f,0.414583f),
	Vector2(0.556250f,0.454167f),
	Vector2(0.581250f,0.493750f),
	Vector2(0.501563f,0.478125f),
	Vector2(0.501563f,0.515625f),
	Vector2(0.398438f,0.562500f),
	Vector2(0.424646f,0.557339f),
	Vector2(0.450899f,0.553540f),
	Vector2(0.477198f,0.551105f),
	Vector2(0.503543f,0.550033f),
	Vector2(0.529933f,0.550324f),
	Vector2(0.556368f,0.551978f),
	Vector2(0.582849f,0.554995f),
	Vector2(0.609375f,0.559375f),
	Vector2(0.424874f,0.564175f),
	Vector2(0.451290f,0.565260f),
	Vector2(0.477687f,0.565755f),
	Vector2(0.504064f,0.565659f),
	Vector2(0.530421f,0.564974f),
	Vector2(0.556759f,0.563698f),
	Vector2(0.583077f,0.561831f),
	Vector2(0.424919f,0.565542f),
	Vector2(0.451368f,0.567604f),
	Vector2(0.477784f,0.568684f),
	Vector2(0.504168f,0.568784f),
	Vector2(0.530519f,0.567903f),
	Vector2(0.556837f,0.566041f),
	Vector2(0.583122f,0.563198f),
	Vector2(0.425284f,0.576475f),
	Vector2(0.451993f,0.586345f),
	Vector2(0.478565f,0.592112f),
	Vector2(0.505001f,0.593773f),
	Vector2(0.531300f,0.591330f),
	Vector2(0.557462f,0.584783f),
	Vector2(0.583487f,0.574131f),
	Vector2(0.501563f,0.132500f),
	Vector2(0.380975f,0.167218f),
	Vector2(0.289999f,0.218686f),
	Vector2(0.228632f,0.286905f),
	Vector2(0.196875f,0.371875f),
	Vector2(0.618619f,0.166443f),
	Vector2(0.706857f,0.216612f),
	Vector2(0.766276f,0.283006f),
	Vector2(0.796875f,0.365625f),
	Vector2(0.199755f,0.404512f),
	Vector2(0.206184f,0.436745f),
	Vector2(0.216161f,0.468575f),
	Vector2(0.229687f,0.500000f),
	Vector2(0.250248f,0.543021f),
	Vector2(0.281321f,0.581320f),
	Vector2(0.322905f,0.614896f),
	Vector2(0.375000f,0.643750f),
	Vector2(0.442507f,0.669937f),
	Vector2(0.509540f,0.678145f),
	Vector2(0.576100f,0.668375f),
	Vector2(0.642188f,0.640625f),
	Vector2(0.689519f,0.609916f),
	Vector2(0.726390f,0.575200f),
	Vector2(0.752801f,0.536478f),
	Vector2(0.768750f,0.493750f),
	Vector2(0.779816f,0.462112f),
	Vector2(0.788192f,0.430212f),
	Vector2(0.793878f,0.398050f),
	Vector2(0.359530f,0.223215f),
	Vector2(0.429039f,0.225794f),
	Vector2(0.498526f,0.226423f),
	Vector2(0.567991f,0.225103f),
	Vector2(0.637435f,0.221832f),
	Vector2(0.249804f,0.414860f),
	Vector2(0.299853f,0.425207f),
	Vector2(0.349902f,0.435555f),
	Vector2(0.399951f,0.445902f),
	Vector2(0.260107f,0.451778f),
	Vector2(0.314030f,0.466810f),
	Vector2(0.367952f,0.481843f),
	Vector2(0.410156f,0.529687f),
	Vector2(0.345491f,0.509317f),
	Vector2(0.280826f,0.488946f),
	Vector2(0.359399f,0.568773f),
	Vector2(0.320360f,0.575047f),
	Vector2(0.304824f,0.555897f),
	Vector2(0.307232f,0.524615f),
	Vector2(0.384777f,0.549230f),
	Vector2(0.241015f,0.386981f),
	Vector2(0.286249f,0.398961f),
	Vector2(0.332577f,0.407816f),
	Vector2(0.379999f,0.413545f),
	Vector2(0.428515f,0.416148f),
	Vector2(0.746352f,0.409273f),
	Vector2(0.698827f,0.420497f),
	Vector2(0.651301f,0.431720f),
	Vector2(0.603776f,0.442943f),
	Vector2(0.736456f,0.446097f),
	Vector2(0.684721f,0.461981f),
	Vector2(0.632985f,0.477866f),
	Vector2(0.595313f,0.526563f),
	Vector2(0.656814f,0.505079f),
	Vector2(0.718315f,0.483596f),
	Vector2(0.648380f,0.564650f),
	Vector2(0.687385f,0.569925f),
	Vector2(0.700590f,0.550564f),
	Vector2(0.695298f,0.519678f),
	Vector2(0.621846f,0.545606f),
	Vector2(0.578453f,0.413494f),
	Vector2(0.624483f,0.409576f),
	Vector2(0.669340f,0.402830f),
	Vector2(0.713025f,0.393257f),
	Vector2(0.755536f,0.380855f),
	Vector2(0.441115f,0.531777f),
	Vector2(0.472034f,0.532822f),
	Vector2(0.502913f,0.532823f),
	Vector2(0.533752f,0.531780f),
	Vector2(0.564552f,0.529693f),
	Vector2(0.384022f,0.585295f),
	Vector2(0.408547f,0.598751f),
	Vector2(0.432973f,0.609140f),
	Vector2(0.457301f,0.616463f),
	Vector2(0.481530f,0.620719f),
	Vector2(0.505661f,0.621908f),
	Vector2(0.529694f,0.620032f),
	Vector2(0.553628f,0.615088f),
	Vector2(0.577464f,0.607078f),
	Vector2(0.601201f,0.596002f),
	Vector2(0.624840f,0.581859f),
	Vector2(0.360938f,0.365625f),
	Vector2(0.646875f,0.359375f),
};
//标准人脸上Face++标准点纹理坐标
static Vector2 g_StandFacePP39Coord[39] = {
	Vector2(0.510938f,0.678125f),
	Vector2(0.196875f,0.371875f),
	Vector2(0.201563f,0.415625f),
	Vector2(0.229687f,0.500000f),
	Vector2(0.281250f,0.581250f),
	Vector2(0.375000f,0.643750f),
	Vector2(0.796875f,0.365625f),
	Vector2(0.792188f,0.409375f),
	Vector2(0.768750f,0.493750f),
	Vector2(0.726563f,0.575000f),
	Vector2(0.642188f,0.640625f),
	Vector2(0.346875f,0.384375f),
	Vector2(0.281250f,0.368750f),
	Vector2(0.360938f,0.365625f),
	Vector2(0.426563f,0.378125f),
	Vector2(0.351563f,0.346875f),
	Vector2(0.229687f,0.321875f),
	Vector2(0.314063f,0.312500f),
	Vector2(0.426563f,0.321875f),
	Vector2(0.318750f,0.293750f),
	Vector2(0.398438f,0.562500f),
	Vector2(0.506250f,0.593750f),
	Vector2(0.506250f,0.568750f),
	Vector2(0.609375f,0.559375f),
	Vector2(0.506250f,0.565625f),
	Vector2(0.506250f,0.550000f),
	Vector2(0.501563f,0.515625f),
	Vector2(0.421875f,0.496875f),
	Vector2(0.581250f,0.493750f),
	Vector2(0.501563f,0.478125f),
	Vector2(0.660937f,0.378125f),
	Vector2(0.585938f,0.371875f),
	Vector2(0.646875f,0.359375f),
	Vector2(0.721875f,0.362500f),
	Vector2(0.651563f,0.343750f),
	Vector2(0.576562f,0.318750f),
	Vector2(0.679688f,0.309375f),
	Vector2(0.768750f,0.312500f),
	Vector2(0.679688f,0.287500f),
};

//83个特征点
static Vector2 g_StandFacePP83Coord[83] = {
	Vector2(0.514370f,0.685355f),
	Vector2(0.190883f,0.372560f),
	Vector2(0.190077f,0.418810f),
	Vector2(0.200606f,0.460815f),
	Vector2(0.221204f,0.505592f),
	Vector2(0.248094f,0.550312f),
	Vector2(0.284165f,0.587233f),
	Vector2(0.326152f,0.622178f),
	Vector2(0.381147f,0.651393f),
	Vector2(0.443083f,0.676053f),
	Vector2(0.800600f,0.350300f),
	Vector2(0.804293f,0.404788f),
	Vector2(0.789080f,0.452333f),
	Vector2(0.775440f,0.490508f),
	Vector2(0.754167f,0.528703f),
	Vector2(0.724365f,0.567847f),
	Vector2(0.687930f,0.607990f),
	Vector2(0.640700f,0.640517f),
	Vector2(0.589870f,0.672168f),
	Vector2(0.356670f,0.387853f),
	Vector2(0.357747f,0.370007f),
	Vector2(0.285210f,0.369167f),
	Vector2(0.319428f,0.381360f),
	Vector2(0.393425f,0.384243f),
	Vector2(0.362395f,0.366102f),
	Vector2(0.425160f,0.381135f),
	Vector2(0.359180f,0.351492f),
	Vector2(0.321842f,0.355652f),
	Vector2(0.395932f,0.359445f),
	Vector2(0.232666f,0.319655f),
	Vector2(0.288482f,0.316797f),
	Vector2(0.341590f,0.319797f),
	Vector2(0.388548f,0.324898f),
	Vector2(0.423327f,0.322218f),
	Vector2(0.286120f,0.300163f),
	Vector2(0.342850f,0.301263f),
	Vector2(0.390878f,0.307597f),
	Vector2(0.406710f,0.564517f),
	Vector2(0.502112f,0.603670f),
	Vector2(0.460510f,0.570844f),
	Vector2(0.432103f,0.581888f),
	Vector2(0.466158f,0.596210f),
	Vector2(0.554526f,0.570369f),
	Vector2(0.583115f,0.583102f),
	Vector2(0.543518f,0.597188f),
	Vector2(0.508094f,0.574875f),
	Vector2(0.611497f,0.564572f),
	Vector2(0.508014f,0.572181f),
	Vector2(0.480348f,0.553678f),
	Vector2(0.440906f,0.557361f),
	Vector2(0.465942f,0.566516f),
	Vector2(0.528570f,0.552750f),
	Vector2(0.570295f,0.557153f),
	Vector2(0.558526f,0.564878f),
	Vector2(0.507098f,0.558072f),
	Vector2(0.477785f,0.375438f),
	Vector2(0.450125f,0.461530f),
	Vector2(0.469160f,0.512468f),
	Vector2(0.510062f,0.516538f),
	Vector2(0.548340f,0.374437f),
	Vector2(0.568865f,0.459393f),
	Vector2(0.552557f,0.510288f),
	Vector2(0.430897f,0.499507f),
	Vector2(0.587965f,0.498245f),
	Vector2(0.512115f,0.486315f),
	Vector2(0.665422f,0.381742f),
	Vector2(0.662325f,0.365125f),
	Vector2(0.592612f,0.377060f),
	Vector2(0.628378f,0.379340f),
	Vector2(0.701765f,0.373372f),
	Vector2(0.655835f,0.361360f),
	Vector2(0.722398f,0.363847f),
	Vector2(0.658550f,0.348093f),
	Vector2(0.624085f,0.356412f),
	Vector2(0.695748f,0.351787f),
	Vector2(0.583330f,0.319762f),
	Vector2(0.634343f,0.318540f),
	Vector2(0.680758f,0.311838f),
	Vector2(0.727755f,0.308935f),
	Vector2(0.765930f,0.313257f),
	Vector2(0.629533f,0.301278f),
	Vector2(0.680222f,0.294305f),
	Vector2(0.727530f,0.295140f),
};


//106个特征点
static Vector2 g_MTFDCoordPoint[106] =
{
	Vector2(0.205567f, 0.360134f),
	Vector2(0.205816f, 0.386363f),
	Vector2(0.207748f, 0.411930f),
	Vector2(0.213804f, 0.437865f),
	Vector2(0.220513f, 0.463483f),
	Vector2(0.229716f, 0.488585f),
	Vector2(0.240178f, 0.513779f),
	Vector2(0.253009f, 0.538753f),
	Vector2(0.269092f, 0.563188f),
	Vector2(0.288388f, 0.585274f),
	Vector2(0.313025f, 0.605906f),
	Vector2(0.339622f, 0.624438f),
	Vector2(0.369612f, 0.642025f),
	Vector2(0.399714f, 0.658256f),
	Vector2(0.432859f, 0.671522f),
	Vector2(0.469742f, 0.680008f),
	Vector2(0.509825f, 0.681575f),
	Vector2(0.548659f, 0.679122f),
	Vector2(0.586334f, 0.669324f),
	Vector2(0.618034f, 0.654575f),
	Vector2(0.648317f, 0.637467f),
	Vector2(0.675891f, 0.619043f),
	Vector2(0.702652f, 0.599961f),
	Vector2(0.726740f, 0.578821f),
	Vector2(0.746849f, 0.556306f),
	Vector2(0.761917f, 0.532324f),
	Vector2(0.774653f, 0.506772f),
	Vector2(0.785225f, 0.481918f),
	Vector2(0.795268f, 0.456044f),
	Vector2(0.802474f, 0.430424f),
	Vector2(0.806521f, 0.405015f),
	Vector2(0.808092f, 0.378063f),
	Vector2(0.807083f, 0.351784f),
	Vector2(0.249250f, 0.321288f),//
	Vector2(0.279126f, 0.301846f),
	Vector2(0.327292f, 0.300386f),
	Vector2(0.375754f, 0.305140f),
	Vector2(0.416476f, 0.313703f),//
	Vector2(0.410933f, 0.330404f),//
	Vector2(0.372397f, 0.324664f),
	Vector2(0.326333f, 0.320583f),
	Vector2(0.281100f, 0.320458f),
	Vector2(0.595391f, 0.308476f),//
	Vector2(0.633626f, 0.298467f),
	Vector2(0.680728f, 0.293217f),
	Vector2(0.728157f, 0.296202f),
	Vector2(0.756868f, 0.316380f),//
	Vector2(0.725736f, 0.312124f),
	Vector2(0.681825f, 0.311321f),
	Vector2(0.637765f, 0.316340f),
	Vector2(0.601790f, 0.323512f),//
	Vector2(0.297781f, 0.370940f),
	Vector2(0.325820f, 0.357367f),
	Vector2(0.362727f, 0.353331f),
	Vector2(0.399387f, 0.359972f),
	Vector2(0.423397f, 0.377171f),
	Vector2(0.391701f, 0.384211f),
	Vector2(0.356908f, 0.388169f),
	Vector2(0.323788f, 0.382598f),
	Vector2(0.359332f, 0.368327f),
	Vector2(0.359798f, 0.371049f),
	Vector2(0.595209f, 0.372860f),
	Vector2(0.617236f, 0.356129f),
	Vector2(0.651368f, 0.348899f),
	Vector2(0.686232f, 0.352233f),
	Vector2(0.713048f, 0.365038f),
	Vector2(0.689017f, 0.376237f),
	Vector2(0.658030f, 0.381979f),
	Vector2(0.625147f, 0.378865f),
	Vector2(0.658307f, 0.363576f),
	Vector2(0.654820f, 0.365811f),
	Vector2(0.511891f, 0.365353f),
	Vector2(0.512072f, 0.405338f),
	Vector2(0.511777f, 0.444911f),
	Vector2(0.511491f, 0.483256f),
	Vector2(0.467369f, 0.375432f),
	Vector2(0.454814f, 0.463726f),
	Vector2(0.425527f, 0.493732f),
	Vector2(0.450546f, 0.511845f),
	Vector2(0.468808f, 0.504382f),
	Vector2(0.509600f, 0.516868f),
	Vector2(0.552327f, 0.503951f),
	Vector2(0.570092f, 0.511450f),
	Vector2(0.594289f, 0.492726f),
	Vector2(0.566098f, 0.463518f),
	Vector2(0.555244f, 0.375422f),
	Vector2(0.403064f, 0.564569f),
	Vector2(0.441958f, 0.559334f),
	Vector2(0.479343f, 0.552451f),
	Vector2(0.505449f, 0.556439f),
	Vector2(0.531681f, 0.552149f),
	Vector2(0.570412f, 0.558649f),
	Vector2(0.608720f, 0.564156f),
	Vector2(0.582175f, 0.582934f),
	Vector2(0.549296f, 0.596484f),
	Vector2(0.506532f, 0.600887f),
	Vector2(0.463991f, 0.596475f),
	Vector2(0.431039f, 0.582869f),
	Vector2(0.407459f, 0.564784f),
	Vector2(0.457300f, 0.568312f),
	Vector2(0.505611f, 0.572224f),
	Vector2(0.555087f, 0.567434f),
	Vector2(0.604316f, 0.564252f),
	Vector2(0.555522f, 0.569307f),
	Vector2(0.505708f, 0.574235f),
	Vector2(0.456799f, 0.570121f),
};

InterFacePointDeform::InterFacePointDeform()
{
}


InterFacePointDeform::~InterFacePointDeform()
{
}


void InterFacePointDeform::AdjustFacePoint(Vector2*pFacePoint118)
{
	Vector2 Direct = pFacePoint118[0] - pFacePoint118[32];

	Vector2 Center = pFacePoint118[74] - pFacePoint118[32];

	float FaceDist = Direct.length();

	Vector2 DirectUnit = Direct / FaceDist;

	float ProcjectCenter = Center.dot(DirectUnit);

	if (ProcjectCenter * 2 > FaceDist) // left
	{

		int LeftNose[3] = { 76,77,78 };
		int RightNose[3] = { 84,83,82 };

		for (int i = 0; i < 3; i++)
		{
			float ProjectLi = (pFacePoint118[LeftNose[i]] - pFacePoint118[32]).dot(DirectUnit);

			float NoseDistLi = ProjectLi - ProcjectCenter;


			float ProjectRi = (pFacePoint118[RightNose[i]] - pFacePoint118[32]).dot(DirectUnit);

			float NoseDistRi = (std::max)(0.0f, ProcjectCenter - ProjectRi);

			if (NoseDistLi * 8 < NoseDistRi)
			{
				float MoveDist = NoseDistRi * 0.125 - NoseDistLi;
				pFacePoint118[LeftNose[i]] += MoveDist * DirectUnit;
			}
		}

		float FaceDistL = (pFacePoint118[8] - pFacePoint118[32]).dot(DirectUnit);
		float NoseDistL = (pFacePoint118[77] - pFacePoint118[32]).dot(DirectUnit);

		if (FaceDistL < NoseDistL)
		{
			float MoveDist = 2.0f*(NoseDistL - FaceDistL);

			for (int i = 0; i <= 15; i++)
			{
				pFacePoint118[i] += MoveDist * DirectUnit;
			}
		}

	}
	else
	{
		int LeftNose[3] = { 76,77,78 };
		int RightNose[3] = { 84,83,82 };

		for (int i = 0; i < 3; i++)
		{
			float ProjectLi = (pFacePoint118[LeftNose[i]] - pFacePoint118[32]).dot(DirectUnit);

			float NoseDistLi = (std::max)(0.0f, ProjectLi - ProcjectCenter);

			float ProjectRi = (pFacePoint118[RightNose[i]] - pFacePoint118[32]).dot(DirectUnit);

			float NoseDistRi = ProcjectCenter - ProjectRi;

			if (NoseDistRi * 8 < NoseDistLi)
			{
				float MoveDist = NoseDistLi * 0.125 - NoseDistRi;
				pFacePoint118[RightNose[i]] -= MoveDist * DirectUnit;
			}
		}

		float FaceDistR = (pFacePoint118[24] - pFacePoint118[32]).dot(DirectUnit);
		float NoseDistR = (pFacePoint118[83] - pFacePoint118[32]).dot(DirectUnit);

		if (FaceDistR > NoseDistR)
		{
			float MoveDist = 2.0f*(FaceDistR - NoseDistR);

			for (int i = 17; i <= 32; i++)
			{
				pFacePoint118[i] -= MoveDist * DirectUnit;
			}
		}
	}
}



int InterFacePointDeform::CalEyePoint(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	vector<Vector2> InPoint;
	vector<Vector2> OuPoint;
	Vector2 srcTri[3], dstTri[3];
	Matrix3 warp_mat_inv;

	//左眼
	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[12];
	dstTri[1] = FacePoint[15];
	dstTri[2] = FacePoint[14];
	srcTri[0] = g_StandFacePP39Coord[12];
	srcTri[1] = g_StandFacePP39Coord[15];
	srcTri[2] = g_StandFacePP39Coord[14];

	warp_mat_inv = CMathUtils::getAffineTransform(srcTri, dstTri);

	for (int i = 0; i <= 5; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv);

	for (int i = 0; i <= 5; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[12];
	dstTri[1] = FacePoint[11];
	dstTri[2] = FacePoint[14];
	srcTri[0] = g_StandFacePP39Coord[12];
	srcTri[1] = g_StandFacePP39Coord[11];
	srcTri[2] = g_StandFacePP39Coord[14];

	warp_mat_inv = CMathUtils::getAffineTransform(srcTri, dstTri);
	InPoint.push_back(g_StandCoord[0]);
	for (int i = 6; i <= 9; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}

	InPoint.push_back(g_StandCoord[5]);
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv);

	for (int i = 1; i <= 4; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	//////////////////////////////////////////////////////////////////////////

	//右眼
	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[31];
	dstTri[1] = FacePoint[34];
	dstTri[2] = FacePoint[33];
	srcTri[0] = g_StandFacePP39Coord[31];
	srcTri[1] = g_StandFacePP39Coord[34];
	srcTri[2] = g_StandFacePP39Coord[33];


	warp_mat_inv = CMathUtils::getAffineTransform(srcTri, dstTri);

	for (int i = 10; i <= 15; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv);

	for (int i = 0; i <= 5; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[31];
	dstTri[1] = FacePoint[30];
	dstTri[2] = FacePoint[33];
	srcTri[0] = g_StandFacePP39Coord[31];
	srcTri[1] = g_StandFacePP39Coord[30];
	srcTri[2] = g_StandFacePP39Coord[33];

	warp_mat_inv = CMathUtils::getAffineTransform(srcTri, dstTri);
	InPoint.push_back(g_StandCoord[10]);
	for (int i = 16; i <= 19; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}

	InPoint.push_back(g_StandCoord[15]);
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv);

	for (int i = 1; i <= 4; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}
	return nCount;
}

int InterFacePointDeform::CalEyeBrowPoint(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	vector<Vector2> InPoint;
	vector<Vector2> OuPoint;
	Vector2 srcTri[3], dstTri[3];
	Matrix3 warp_mat_inv1;
	//左眉毛
	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[16];
	dstTri[1] = FacePoint[17];
	dstTri[2] = FacePoint[19];
	srcTri[0] = g_StandFacePP39Coord[16];
	srcTri[1] = g_StandFacePP39Coord[17];
	srcTri[2] = g_StandFacePP39Coord[19];

	warp_mat_inv1 = CMathUtils::getAffineTransform(srcTri, dstTri);
	for (int i = 20; i <= 22; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv1);
	for (int i = 0; i <= 2; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[18];
	dstTri[1] = FacePoint[17];
	dstTri[2] = FacePoint[19];
	srcTri[0] = g_StandFacePP39Coord[18];
	srcTri[1] = g_StandFacePP39Coord[17];
	srcTri[2] = g_StandFacePP39Coord[19];

	Matrix3 warp_mat_inv2 = CMathUtils::getAffineTransform(srcTri, dstTri);
	for (int i = 23; i <= 25; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv2);
	for (int i = 0; i <= 2; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();

	for (int i = 26; i <= 27; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv1);
	for (int i = 0; i <= 1; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}
	InPoint.clear();
	OuPoint.clear();
	for (int i = 28; i <= 29; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv2);
	for (int i = 0; i <= 1; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	//右眉毛
	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[35];
	dstTri[1] = FacePoint[38];
	dstTri[2] = FacePoint[36];
	srcTri[0] = g_StandFacePP39Coord[35];
	srcTri[1] = g_StandFacePP39Coord[38];
	srcTri[2] = g_StandFacePP39Coord[36];

	warp_mat_inv1 = CMathUtils::getAffineTransform(srcTri, dstTri);
	for (int i = 30; i <= 32; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv1);
	for (int i = 0; i <= 2; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	dstTri[0] = FacePoint[38];
	dstTri[1] = FacePoint[36];
	dstTri[2] = FacePoint[37];
	srcTri[0] = g_StandFacePP39Coord[38];
	srcTri[1] = g_StandFacePP39Coord[36];
	srcTri[2] = g_StandFacePP39Coord[37];

	warp_mat_inv2 = CMathUtils::getAffineTransform(srcTri, dstTri);
	for (int i = 33; i <= 35; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv2);
	for (int i = 0; i <= 2; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}
	InPoint.clear();
	OuPoint.clear();
	for (int i = 36; i <= 37; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv1);
	for (int i = 0; i <= 1; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}
	InPoint.clear();
	OuPoint.clear();
	for (int i = 38; i <= 39; i++)
	{
		InPoint.push_back(g_StandCoord[i]);
	}
	CMathUtils::transform(InPoint, InPoint, warp_mat_inv2);
	for (int i = 0; i <= 1; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	return nCount;
}

int InterFacePointDeform::CalHeadPoint(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	vector<Vector2> InPoint;
	vector<Vector2> OuPoint;

	InPoint.clear();
	OuPoint.clear();
	Vector2 Mind = (DstPoint[80] + DstPoint[40]) / 2.0;
	InPoint.push_back(DstPoint[82]);
	InPoint.push_back(Mind);
	InPoint.push_back(DstPoint[86]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 7);
	for (int i = 1; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	return nCount;
}

int InterFacePointDeform::CalLeftCheekPoint(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	vector<Vector2> InPoint;
	vector<Vector2> OuPoint;
	int ans = nCount;

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[89]);
	InPoint.push_back(DstPoint[43]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 6);
	for (int i = 1; i < 5; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[90]);
	InPoint.push_back(DstPoint[44]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);
	for (int i = 1; i < 4; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	Vector2 Mind = (DstPoint[50] + DstPoint[44]) / 2.0;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(Mind);
	InPoint.push_back(DstPoint[91]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 4);
	for (int i = 0; i < 3; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[50]);
	InPoint.push_back(DstPoint[94]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 4);
	for (int i = 1; i < 3; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[123]);
	InPoint.push_back(DstPoint[93]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 3);
	for (int i = 1; i < 2; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	Mind = (DstPoint[120] + DstPoint[123]) / 2.0;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[92]);
	InPoint.push_back(Mind);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 3);
	for (int i = 1; i < 3; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	Mind = (DstPoint[7] + DstPoint[115]) / 2.0;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[84]);
	InPoint.push_back(Mind);
	InPoint.push_back(DstPoint[42]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 7);
	for (int i = 1; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	return nCount;
}

int InterFacePointDeform::CalRightCheekPoint(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	vector<Vector2> InPoint;
	vector<Vector2> OuPoint;
	int ans = nCount;

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[107]);
	InPoint.push_back(DstPoint[46]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 6);
	for (int i = 1; i < 5; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[106]);
	InPoint.push_back(DstPoint[47]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);
	for (int i = 1; i < 4; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	Vector2 Mind = (DstPoint[58] + DstPoint[47]) / 2.0;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(Mind);
	InPoint.push_back(DstPoint[105]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 4);
	for (int i = 0; i < 3; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[58]);
	InPoint.push_back(DstPoint[102]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 4);
	for (int i = 1; i < 3; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[143]);
	InPoint.push_back(DstPoint[103]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 3);
	for (int i = 1; i < 2; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	Mind = (DstPoint[140] + DstPoint[143]) / 2.0;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[104]);
	InPoint.push_back(Mind);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 3);
	for (int i = 1; i < 3; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	Mind = (DstPoint[18] + DstPoint[135]) / 2.0;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(DstPoint[45]);
	InPoint.push_back(Mind);
	InPoint.push_back(DstPoint[88]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 7);
	for (int i = 1; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	return nCount;
}

int InterFacePointDeform::CalMouthOutPoint(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	vector<Vector2> InPoint;
	vector<Vector2> OuPoint;
	int ans = nCount;
	Vector2 Mind;

	InPoint.clear();
	OuPoint.clear();
	Mind = (DstPoint[49] + DstPoint[54]) / 2.0;
	InPoint.push_back(DstPoint[120]);
	InPoint.push_back(Mind);
	InPoint.push_back(DstPoint[140]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 7);
	for (int i = 1; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	Mind = (DstPoint[98] - DstPoint[76]) * 0.3333f + DstPoint[76];
	InPoint.push_back(DstPoint[123]);
	InPoint.push_back(Mind);
	InPoint.push_back(DstPoint[143]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 13);
	for (int i = 1; i < 12; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	return nCount;
}

int InterFacePointDeform::CalEyepupilPoint(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	DstPoint[nCount++] = FacePoint[13];
	DstPoint[nCount++] = FacePoint[32];

	return nCount;
}

int InterFacePointDeform::CalNoisePoint106(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	vector<Vector2> InPoint;
	vector<Vector2> OuPoint;
	//////////////////////////////////////////////////////////////////////////
	DstPoint[nCount++] = (DstPoint[25] + DstPoint[30]) / 2.0;
	DstPoint[nCount++] = (FacePoint[75] + FacePoint[85]) / 2.0;
	DstPoint[nCount++] = (DstPoint[41] + FacePoint[76]) / 2.0;
	DstPoint[nCount++] = FacePoint[76];
	DstPoint[nCount++] = (FacePoint[77] + FacePoint[78]) / 2.0;
	DstPoint[nCount++] = (DstPoint[41] + FacePoint[84]) / 2.0;
	DstPoint[nCount++] = FacePoint[84];
	DstPoint[nCount++] = (FacePoint[83] + FacePoint[82]) / 2.0;
	DstPoint[nCount++] = FacePoint[74];
	DstPoint[nCount++] = FacePoint[80];

	return nCount;
}
 
Vector2 InterFacePointDeform::get(Vector2 pt1, Vector2 pt2, Vector2 M)
{
	float u = 2.f / 3;
	Vector2 pt1_M = u*pt1 + (1 - u)*M;
	Vector2 pt2_M = u*pt2 + (1 - u)*M;
	Vector2 direction = pt2_M - M;
	return  direction + pt1_M;
}

int InterFacePointDeform::CalFacePoint106(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	std::vector<Vector2> InPoint;
	std::vector<Vector2> OuPoint;
	Vector2 srcTri[3], dstTri[3];
	Matrix3 warp_mat_inv;
	//////////////////////////////////////////////////////////////////////////
	//眼睛中间点
	// 
	//Vector2 Mid = (FacePoint[55] + FacePoint[65]) / 2.0;
	Vector2 Mid = (FacePoint[55] + FacePoint[61]) / 2.0;
	//额头最顶点
	Vector2 pt;
	pt.x = ((Mid.x - FacePoint[80].x) * 1.757f + Mid.x);
	pt.y = ((Mid.y - FacePoint[80].y) * 1.757f + Mid.y);

	Vector2 pt1;
	pt1 = get(FacePoint[0], pt, Mid);
	//
	Vector2 pt2;
	pt2 = get(FacePoint[32], pt, Mid);

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(pt);
	InPoint.push_back(pt1);
	InPoint.push_back(FacePoint[0]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);
	for (int i = 0; i < 5; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(pt);
	InPoint.push_back(pt2);
	InPoint.push_back(FacePoint[32]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);
	for (int i = 1; i < 5; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	int ans = nCount;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[0]);
	InPoint.push_back(FacePoint[5]);
	InPoint.push_back(FacePoint[10]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	//识别点太靠上，换成靠近左眼的点
	DstPoint[84] = OuPoint[1];
	for (int i = 2; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//89 90 91 92

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[5]);
	InPoint.push_back(FacePoint[10]);
	InPoint.push_back(FacePoint[16]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	for (int i = 2; i < 8; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//93 94 95 96 97 98

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[16]);
	InPoint.push_back(FacePoint[22]);
	InPoint.push_back(FacePoint[27]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	for (int i = 1; i < 7; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//99 100 101 102 103 104

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[22]);
	InPoint.push_back(FacePoint[27]);
	InPoint.push_back(FacePoint[32]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	for (int i = 3; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//105 106 107
	//识别点太靠上，换成靠近右眼的点
	DstPoint[88] = OuPoint[6];

	return nCount;
}

int InterFacePointDeform::CalMouthPoint83(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
		std::vector<Vector2> InPoint;
		std::vector<Vector2> OuPoint;
		Vector2 Mind;
		//上嘴唇
		DstPoint[nCount++] = FacePoint[86];
		DstPoint[nCount++] = (FacePoint[110] + FacePoint[87]) * 0.5f;

		DstPoint[nCount++] = (FacePoint[87] + FacePoint[111]) * 0.5f;

		DstPoint[nCount++] = FacePoint[88];
		DstPoint[nCount++] = FacePoint[89];
		DstPoint[nCount++] = FacePoint[90];

		DstPoint[nCount++] = (FacePoint[112] + FacePoint[91]) * 0.5f;

		DstPoint[nCount++] = (FacePoint[91] + FacePoint[113]) * 0.5f;
		DstPoint[nCount++] = FacePoint[92];

		//中上嘴唇
		DstPoint[nCount++] = (FacePoint[98] + FacePoint[99]) * 0.5;
		DstPoint[nCount++] = FacePoint[99];
		DstPoint[nCount++] = FacePoint[106];
		DstPoint[nCount++] = FacePoint[100];
		DstPoint[nCount++] = FacePoint[107];
		DstPoint[nCount++] = FacePoint[101];
		DstPoint[nCount++] = (FacePoint[101] + FacePoint[102]) * 0.5;

		//中下嘴唇
		DstPoint[nCount++] = (FacePoint[98] + FacePoint[105]) * 0.5;
		DstPoint[nCount++] = FacePoint[105];
		DstPoint[nCount++] = FacePoint[108];
		DstPoint[nCount++] = FacePoint[104];
		DstPoint[nCount++] = FacePoint[109];
		DstPoint[nCount++] = FacePoint[103];
		DstPoint[nCount++] = (FacePoint[103] + FacePoint[102]) * 0.5;

		//下嘴唇

		Vector2* pIn = NULL;
		Vector2* pOut = NULL;
		int outCnt = 0;
		int calCnt = 7;
		pIn = new Vector2[calCnt];
		pIn[0] = FacePoint[86];
		pIn[1] = FacePoint[97];
		pIn[2] = FacePoint[96];
		pIn[3] = FacePoint[95];
		pIn[4] = FacePoint[94];
		pIn[5] = FacePoint[93];
		pIn[6] = FacePoint[92];
		outCnt = CMathUtils::SmoothLines2((Vector2*)pOut, (Vector2*)pIn, calCnt, 0.5, 6);
		pOut = new Vector2[outCnt];
		outCnt = CMathUtils::SmoothLines2((Vector2*)pOut, (Vector2*)pIn, calCnt, 0.5, 6);
		for (int i = 5; i < 18; i += 5)
		{
		DstPoint[nCount++] = pOut[i];
		}
		DstPoint[nCount++] = pOut[18];
		for (int i = 21; i < outCnt - 1; i += 5)
		{
		DstPoint[nCount++] = pOut[i];
		}
		SAFE_DELETE_ARRAY(pIn);
		SAFE_DELETE_ARRAY(pOut);

		return nCount;

}

int InterFacePointDeform::CalMoreMouthOut(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	std::vector<Vector2> InPoint;
	Vector2 Mind;
	Vector2 LeftPoint;
	Vector2 RightPoint;

	InPoint.clear();
	Mind = (DstPoint[98] - DstPoint[76]) * 0.6666f + DstPoint[76];
	LeftPoint = (DstPoint[96] - DstPoint[74]) * 0.7f + DstPoint[74];
	RightPoint = (DstPoint[100] - DstPoint[78]) * 0.7f + DstPoint[78];
	InPoint.push_back(DstPoint[124]);
	InPoint.push_back(LeftPoint);
	InPoint.push_back(Mind);
	CMathUtils::LagrangePolyfit(InPoint, InPoint, 7);
	for (int i = 1; i < 6; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}
	DstPoint[nCount++] = Mind;
	InPoint.clear();
	InPoint.push_back(Mind);
	InPoint.push_back(RightPoint);
	InPoint.push_back(DstPoint[144]);
	CMathUtils::LagrangePolyfit(InPoint, InPoint, 7);
	for (int i = 1; i < 6; i++)
	{
		DstPoint[nCount++] = InPoint[i];
	}

	return nCount;


}

int InterFacePointDeform::CalPointMSL(Vector2* SrcMovePoint, Vector2* SrcPoint, int PointNum, Vector2* DstMovePoint, Vector2* DstPoint, int MoveNum, int count)
{
	int nplength = MoveNum;
	int nqlength = MoveNum;
	MLS mls;

	//控制点p和q放入二维数组：
	//第一行为纵坐标值
	//第二行为横坐标值
	int row = 2;
	//原图控制点p
	float** tp = new float*[row];
	for (int i = 0; i < row; i++)
	{
		tp[i] = new float[nplength];
	}

	for (int i = 0; i < nplength; i++)
	{
		tp[0][i] = SrcMovePoint[i].y*1500;
		tp[1][i] = SrcMovePoint[i].x*1000;
	}

	//原图要移动后的控制点q
	float** tq = new float*[row];

	for (int i = 0; i < row; i++)
	{
		tq[i] = new float[nqlength];
	}
	for (int i = 0; i < nplength; i++)
	{
		tq[0][i] = DstMovePoint[i].y;
		tq[1][i] = DstMovePoint[i].x;
	}

	int npoints = PointNum;
	float** srcPoints = new float*[2];
	float** desPoints = new float*[2];
	srcPoints[0] = new float[npoints];
	srcPoints[1] = new float[npoints];
	desPoints[0] = new float[npoints];
	desPoints[1] = new float[npoints];
	int j = 0;

	for (int i = 0; i < PointNum; i++)
	{
		srcPoints[0][i] = SrcPoint[i].y*1500;
		srcPoints[1][i] = SrcPoint[i].x*1000;
	}
	mls.MLSD2DpointsDefAffine(srcPoints, desPoints, tp, tq, nplength, npoints);
	for (int i = 0; i < PointNum; i++)
	{
		DstPoint[count].y = desPoints[0][i];
		DstPoint[count++].x = desPoints[1][i];
	}

	SAFE_DELETE_ARRAY(srcPoints[0]);
	SAFE_DELETE_ARRAY(srcPoints[1]);
	SAFE_DELETE_ARRAY(srcPoints);
	SAFE_DELETE_ARRAY(desPoints[0]);
	SAFE_DELETE_ARRAY(desPoints[1]);
	SAFE_DELETE_ARRAY(desPoints);
	for (int i = 0; i < row; i++)
	{
		SAFE_DELETE_ARRAY(tq[i]);
		SAFE_DELETE_ARRAY(tp[i]);
	}
	SAFE_DELETE_ARRAY(tq);
	SAFE_DELETE_ARRAY(tp);

	return count;
}


Vector2 get1(Vector2 pt1, Vector2 pt2, Vector2 M)
{
	float u = 2.f / 3;
	Vector2 pt1_M = u*pt1 + (1 - u)*M;
	Vector2 pt2_M = u*pt2 + (1 - u)*M;
	Vector2 direction = pt2_M - M;
	return  direction + pt1_M;
}

int CalFacePoint106_2(int nCount, Vector2* FacePoint, Vector2* DstPoint)
{
	std::vector<Vector2> InPoint;
	std::vector<Vector2> OuPoint;
	Vector2 srcTri[3], dstTri[3];
	Matrix3 warp_mat_inv;
	//////////////////////////////////////////////////////////////////////////
	//眼睛中间点
	// 
	//Vector2 Mid = (FacePoint[55] + FacePoint[65]) / 2.0;
	Vector2 Mid = (FacePoint[55] + FacePoint[61]) / 2.0;
	//额头最顶点
	Vector2 pt;
	pt.x = ((Mid.x - FacePoint[80].x) * 1.8f + Mid.x);
	pt.y = ((Mid.y - FacePoint[80].y) * 1.8f + Mid.y);

	Vector2 pt1;
	pt1 = get1(FacePoint[0], pt, Mid);
	//
	Vector2 pt2;
	pt2 = get1(FacePoint[32], pt, Mid);

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(pt);
	InPoint.push_back(pt1);
	InPoint.push_back(FacePoint[0]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(pt);
	pt1.x = OuPoint[2].x;
	pt1.y = OuPoint[1].y;
	InPoint.push_back(pt1);
	InPoint.push_back(FacePoint[0]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);

	for (int i = 0; i < 5; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}


	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(pt);
	InPoint.push_back(pt2);
	InPoint.push_back(FacePoint[32]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(pt);
	pt1.x = OuPoint[2].x;
	pt1.y = OuPoint[1].y;
	InPoint.push_back(pt1);
	InPoint.push_back(FacePoint[32]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 5);

	for (int i = 1; i < 5; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}

	DstPoint[80].x = ((Mid.x - FacePoint[80].x) * 1.9f + Mid.x);
	DstPoint[80].y = ((Mid.y - FacePoint[80].y) * 1.9f + Mid.y);


	int ans = nCount;
	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[0]);
	InPoint.push_back(FacePoint[5]);
	InPoint.push_back(FacePoint[10]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	//识别点太靠上，换成靠近左眼的点
	DstPoint[84] = OuPoint[1];
	for (int i = 2; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//89 90 91 92

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[5]);
	InPoint.push_back(FacePoint[10]);
	InPoint.push_back(FacePoint[16]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	for (int i = 2; i < 8; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//93 94 95 96 97 98

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[16]);
	InPoint.push_back(FacePoint[22]);
	InPoint.push_back(FacePoint[27]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	for (int i = 1; i < 7; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//99 100 101 102 103 104

	InPoint.clear();
	OuPoint.clear();
	InPoint.push_back(FacePoint[22]);
	InPoint.push_back(FacePoint[27]);
	InPoint.push_back(FacePoint[32]);
	CMathUtils::LagrangePolyfit(InPoint, OuPoint, 8);
	for (int i = 3; i < 6; i++)
	{
		DstPoint[nCount++] = OuPoint[i];
	}
	//105 106 107
	//识别点太靠上，换成靠近右眼的点
	DstPoint[88] = OuPoint[6];

	return nCount;
}

void InterFacePointDeform::RunFace118To171(Vector2 * pFacePoint118, int FaceIndex, Vector2 * pDstPoint442)
{
	Vector2 pFacePoint118Copy[118];
	memcpy(pFacePoint118Copy, pFacePoint118, 118 * sizeof(Vector2));
	Vector2 pFace83Point[83];
	Vector2 pFace39Point[39];
	//还原83个点
	int p83Index[83] = { 16,1,2,4,5,7,9,11,13,15,31,30,28,27,25,23,21,19,17,
		57,60,51,58,56,59,55,53,52,54,
		33,41,40,39,37,34,35,36,
		86,95,105,97,96,103,93,94,104,92,100,88,87,99,90,91,101,89,
		75,76,79,80,85,84,81,77,83,74,
		67,70,61,68,66,69,65,63,62,64,
		42,49,48,47,46,43,44,45 };
	for (int i = 0; i < 83; i++)
	{
		if (i == 33 || i == 75)
		{
			if (i == 33)
			{
				pFace83Point[i] = (pFacePoint118Copy[37] + pFacePoint118Copy[38]) / 2.0;
				g_StandFacePP83Coord[i] = (g_MTFDCoordPoint[37] + g_MTFDCoordPoint[38]) / 2.0;
			}
			else
			{
				pFace83Point[i] = (pFacePoint118Copy[42] + pFacePoint118Copy[50]) / 2.0;
				g_StandFacePP83Coord[i] = (g_MTFDCoordPoint[42] + g_MTFDCoordPoint[50]) / 2.0;
			}
			continue;
		}
		pFace83Point[i] = pFacePoint118Copy[p83Index[i]];
		g_StandFacePP83Coord[i] = g_MTFDCoordPoint[p83Index[i]];
	}
	//还原39个点
	int pIndex[39] = { 0,1,2,4,6,8,10,11,13,15,17,19,21,24,25,26,29,31,33,35,37,38,45,46,47,54,58,62,63,64,65,67,70,71,72,75,77,79,81 };
	for (int i = 0; i < 39; i++)
	{
		pFace39Point[i] = pFace83Point[pIndex[i]];
		g_StandFacePP39Coord[i] = g_StandFacePP83Coord[pIndex[i]];
	}

	int nCount = 0;
	//眼睛
	nCount = CalEyePoint(nCount, pFace39Point, pDstPoint442);
	//眉毛
	nCount = CalEyeBrowPoint(nCount, pFace39Point, pDstPoint442);
	//鼻子
	nCount = CalNoisePoint106(nCount, pFacePoint118Copy, pDstPoint442);
	//嘴巴
	nCount = CalMouthPoint83(nCount, pFacePoint118Copy, pDstPoint442);
	//脸轮廓
	nCount = CalFacePoint106_2(nCount, pFacePoint118Copy, pDstPoint442);
	//额头
	nCount = CalHeadPoint(nCount, pFace39Point, pDstPoint442);
	//左脸颊
	nCount = CalLeftCheekPoint(nCount, pFace39Point, pDstPoint442);
	//右脸颊
	nCount = CalRightCheekPoint(nCount, pFace39Point, pDstPoint442);
	//嘴外围
	nCount = CalMouthOutPoint(nCount, pFace39Point, pDstPoint442);
	//眼瞳
	nCount = CalEyepupilPoint(nCount, pFace39Point, pDstPoint442);
	// 
	// 	float pTestPoint[] = {
	// 		298.18,549.24,324.30,537.24,355.93,533.73,380.37,539.00,400.83,553.06,417.31,575.91,318.44,571.04,348.18,581.86,370.83,584.33,394.30,581.02,595.31,569.83,611.32,550.24,631.15,535.14,654.81,524.51,687.01,522.18,713.79,534.09,618.89,573.03,641.73,573.26,666.24,570.31,692.01,558.92,229.14,474.20,246.99,438.18,286.39,430.88,328.24,434.44,367.11,437.07,411.41,471.83,266.91,478.30,295.05,471.73,329.40,476.27,370.64,476.04,606.38,464.02,631.97,434.06,681.36,431.42,727.23,421.99,759.94,425.46,779.05,463.93,639.21,464.67,678.72,471.39,714.19,462.16,738.83,463.52,508.90,467.93,506.72,556.45,478.90,624.53,451.07,692.61,436.48,752.48,537.36,623.10,567.99,689.75,582.21,747.76,511.60,724.45,510.54,773.70,406.07,852.50,432.10,844.76,450.57,840.02,480.66,834.88,507.52,840.86,533.74,834.40,562.08,839.67,579.53,843.88,604.38,850.17,433.38,855.23,447.08,856.38,477.41,858.29,507.95,861.25,536.80,858.22,565.06,855.09,578.04,853.75,433.38,855.23,447.08,856.38,477.41,858.29,507.95,861.25,536.80,858.22,565.06,855.09,578.04,853.75,426.50,872.93,454.12,888.87,487.48,896.96,508.87,898.01,529.48,896.74,560.14,887.77,584.94,870.60,500.20,203.69,373.26,240.09,277.19,306.86,211.98,404.00,180.53,589.21,621.12,235.86,712.16,297.69,773.33,389.18,801.35,566.21,188.47,645.18,201.44,699.42,219.45,751.94,242.50,802.72,261.16,844.02,297.39,892.23,340.60,933.05,390.79,966.47,447.97,992.51,512.12,1011.16,570.46,984.36,621.91,951.71,666.47,913.20,704.13,868.85,734.90,818.64,758.77,762.58,767.58,726.92,782.82,674.51,794.08,620.94,350.08,324.10,422.81,333.84,495.39,336.06,567.80,330.78,640.06,317.99,240.99,654.66,293.51,664.15,346.03,673.64,398.55,683.12,260.20,712.69,318.96,725.95,377.72,739.21,421.27,802.49,354.00,785.64,286.72,768.79,369.84,865.74,333.62,878.98,315.50,854.88,307.93,822.63,373.35,842.54,229.02,605.59,278.00,617.77,327.48,625.75,377.46,629.54,427.93,629.13,748.87,634.70,703.65,648.47,658.43,662.23,613.21,675.99,732.67,692.82,682.52,711.13,632.36,729.45,593.29,798.96,651.39,774.95,709.48,750.93,637.63,856.40,670.88,862.62,686.26,837.52,697.01,797.62,635.26,832.65,583.67,624.37,629.06,621.33,673.53,614.00,717.06,602.37,759.67,586.44,450.52,804.09,479.78,805.68,509.03,807.28,537.12,804.51,565.20,801.74,387.70,888.40,407.76,906.66,430.01,920.53,454.46,929.99,481.11,935.06,509.95,935.72,537.25,932.18,562.14,924.77,584.62,913.49,604.69,898.33,622.36,879.30,362.63,552.06,655.11,542.10,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,180.53,589.21,183.83,510.62,198.32,441.31,223.19,380.61,257.99,328.15,302.65,283.85,357.42,247.94,422.94,220.95,500.20,203.69,574.30,218.73,636.83,243.37,688.80,276.78,730.88,318.41,763.38,367.99,786.27,425.57,799.17,491.44,801.35,566.21,429.53,247.84,501.65,247.73,569.24,245.55,402.87,287.81,503.10,291.77,595.95,284.27,340.18,330.70,422.36,333.26,504.55,335.81,579.99,330.01,655.44,324.21,317.46,380.36,411.73,380.10,506.00,379.85,591.79,375.90,677.59,371.95,275.60,436.95,352.88,432.60,430.16,428.24,507.45,423.89,577.15,424.31,646.86,424.73,716.57,425.15,499.06,151.62,359.43,191.66,253.75,265.11,182.02,371.96,147.43,575.69,632.07,187.00,732.22,255.02,799.51,355.65,830.33,550.39,156.16,637.25,170.43,696.92,190.24,754.68,215.59,810.55,236.12,855.97,275.97,909.01,323.50,953.91,378.71,990.68,441.60,1019.32,512.17,1039.83,576.35,1010.35,632.94,974.43,681.95,932.08,723.38,883.29,757.23,828.06,783.49,766.39,793.18,727.16,809.95,669.51,822.33,610.59,497.92,99.54,345.60,143.22,230.31,223.35,152.06,339.92,114.32,562.16,643.02,138.15,752.27,212.34,825.68,322.12,859.30,534.56,123.84,629.33,139.41,694.42,161.02,757.43,188.68,818.38,211.08,867.93,254.55,925.78,306.40,974.77,366.63,1014.88,435.24,1046.12,512.23,1068.50,582.24,1036.34,643.97,997.16,697.44,950.96,742.63,897.73,779.56,837.48,808.21,770.21,818.77,727.41,837.07,664.52,850.58,600.24,496.78,47.46,331.76,94.79,206.87,181.59,122.09,307.87,81.21,548.64,653.97,89.29,772.33,169.66,851.85,288.60,888.28,518.74,91.53,621.40,108.39,691.91,131.81,760.18,161.77,826.20,186.03,879.89,233.13,942.56,289.30,995.63,354.55,1039.08,428.88,1072.93,512.28,1097.17,588.12,1062.33,655.00,1019.89,712.92,969.83,761.89,912.17,801.89,846.90,832.92,774.02,844.37,727.66,864.19,659.53,878.83,589.89,357.73,904.98,384.03,926.87,412.51,944.66,443.17,958.35,476.01,967.94,511.04,973.43,543.69,963.64,573.94,950.38,601.78,933.64,627.22,913.44,650.25,889.77, 
	// 	};
	// 
	// 	memcpy(pDstPoint442, pTestPoint, nCount * sizeof(Vector2));

	//插值嘴巴最外围点
	nCount = CalMoreMouthOut(nCount, pFacePoint118Copy, pDstPoint442);
}


void InterFacePointDeform::RunFace118To442(Vector2 * pFacePoint118, int FaceIndex, Vector2 * pDstPoint442, bool bAdjsut, bool bFaceOutFix)
{
	Vector2 pFacePoint118Copy[118];
	memcpy(pFacePoint118Copy, pFacePoint118, 118 * sizeof(Vector2));
	if (bAdjsut)
	{
		AdjustFacePoint(pFacePoint118Copy);

	}
	Vector2 pFace83Point[83];
	Vector2 pFace39Point[39];
	//还原83个点
	int p83Index[83] = { 16,1,2,4,5,7,9,11,13,15,31,30,28,27,25,23,21,19,17,
		57,60,51,58,56,59,55,53,52,54,
		33,41,40,39,37,34,35,36,
		86,95,105,97,96,103,93,94,104,92,100,88,87,99,90,91,101,89,
		75,76,79,80,85,84,81,77,83,74,
		67,70,61,68,66,69,65,63,62,64,
		42,49,48,47,46,43,44,45 };
	for (int i = 0; i < 83; i++)
	{
		if (i == 33 || i == 75)
		{
			if (i == 33)
			{
				pFace83Point[i] = (pFacePoint118Copy[37] + pFacePoint118Copy[38]) / 2.0;
				g_StandFacePP83Coord[i] = (g_MTFDCoordPoint[37] + g_MTFDCoordPoint[38]) / 2.0;
			}
			else
			{
				pFace83Point[i] = (pFacePoint118Copy[42] + pFacePoint118Copy[50]) / 2.0;
				g_StandFacePP83Coord[i] = (g_MTFDCoordPoint[42] + g_MTFDCoordPoint[50]) / 2.0;
			}
			continue;
		}
		pFace83Point[i] = pFacePoint118Copy[p83Index[i]];
		g_StandFacePP83Coord[i] = g_MTFDCoordPoint[p83Index[i]];
	}
	//还原39个点
	int pIndex[39] = { 0,1,2,4,6,8,10,11,13,15,17,19,21,24,25,26,29,31,33,35,37,38,45,46,47,54,58,62,63,64,65,67,70,71,72,75,77,79,81 };
	for (int i = 0; i < 39; i++)
	{
		pFace39Point[i] = pFace83Point[pIndex[i]];
		g_StandFacePP39Coord[i] = g_StandFacePP83Coord[pIndex[i]];
	}

	int nCount = 0;
	//眼睛
	nCount = CalEyePoint(nCount, pFace39Point, pDstPoint442);
	//眉毛
	nCount = CalEyeBrowPoint(nCount, pFace39Point, pDstPoint442);
	//鼻子
	nCount = CalNoisePoint106(nCount, pFacePoint118Copy, pDstPoint442);
	//嘴巴
	nCount = CalMouthPoint83(nCount, pFacePoint118Copy, pDstPoint442);
	//脸轮廓
	nCount = CalFacePoint106(nCount, pFacePoint118Copy, pDstPoint442);
	//额头
	nCount = CalHeadPoint(nCount, pFace39Point, pDstPoint442);
	//左脸颊
	nCount = CalLeftCheekPoint(nCount, pFace39Point, pDstPoint442);
	//右脸颊
	nCount = CalRightCheekPoint(nCount, pFace39Point, pDstPoint442);
	//嘴外围
	nCount = CalMouthOutPoint(nCount, pFace39Point, pDstPoint442);
	//眼瞳
	nCount = CalEyepupilPoint(nCount, pFace39Point, pDstPoint442);
// 
// 	float pTestPoint[] = {
// 		298.18,549.24,324.30,537.24,355.93,533.73,380.37,539.00,400.83,553.06,417.31,575.91,318.44,571.04,348.18,581.86,370.83,584.33,394.30,581.02,595.31,569.83,611.32,550.24,631.15,535.14,654.81,524.51,687.01,522.18,713.79,534.09,618.89,573.03,641.73,573.26,666.24,570.31,692.01,558.92,229.14,474.20,246.99,438.18,286.39,430.88,328.24,434.44,367.11,437.07,411.41,471.83,266.91,478.30,295.05,471.73,329.40,476.27,370.64,476.04,606.38,464.02,631.97,434.06,681.36,431.42,727.23,421.99,759.94,425.46,779.05,463.93,639.21,464.67,678.72,471.39,714.19,462.16,738.83,463.52,508.90,467.93,506.72,556.45,478.90,624.53,451.07,692.61,436.48,752.48,537.36,623.10,567.99,689.75,582.21,747.76,511.60,724.45,510.54,773.70,406.07,852.50,432.10,844.76,450.57,840.02,480.66,834.88,507.52,840.86,533.74,834.40,562.08,839.67,579.53,843.88,604.38,850.17,433.38,855.23,447.08,856.38,477.41,858.29,507.95,861.25,536.80,858.22,565.06,855.09,578.04,853.75,433.38,855.23,447.08,856.38,477.41,858.29,507.95,861.25,536.80,858.22,565.06,855.09,578.04,853.75,426.50,872.93,454.12,888.87,487.48,896.96,508.87,898.01,529.48,896.74,560.14,887.77,584.94,870.60,500.20,203.69,373.26,240.09,277.19,306.86,211.98,404.00,180.53,589.21,621.12,235.86,712.16,297.69,773.33,389.18,801.35,566.21,188.47,645.18,201.44,699.42,219.45,751.94,242.50,802.72,261.16,844.02,297.39,892.23,340.60,933.05,390.79,966.47,447.97,992.51,512.12,1011.16,570.46,984.36,621.91,951.71,666.47,913.20,704.13,868.85,734.90,818.64,758.77,762.58,767.58,726.92,782.82,674.51,794.08,620.94,350.08,324.10,422.81,333.84,495.39,336.06,567.80,330.78,640.06,317.99,240.99,654.66,293.51,664.15,346.03,673.64,398.55,683.12,260.20,712.69,318.96,725.95,377.72,739.21,421.27,802.49,354.00,785.64,286.72,768.79,369.84,865.74,333.62,878.98,315.50,854.88,307.93,822.63,373.35,842.54,229.02,605.59,278.00,617.77,327.48,625.75,377.46,629.54,427.93,629.13,748.87,634.70,703.65,648.47,658.43,662.23,613.21,675.99,732.67,692.82,682.52,711.13,632.36,729.45,593.29,798.96,651.39,774.95,709.48,750.93,637.63,856.40,670.88,862.62,686.26,837.52,697.01,797.62,635.26,832.65,583.67,624.37,629.06,621.33,673.53,614.00,717.06,602.37,759.67,586.44,450.52,804.09,479.78,805.68,509.03,807.28,537.12,804.51,565.20,801.74,387.70,888.40,407.76,906.66,430.01,920.53,454.46,929.99,481.11,935.06,509.95,935.72,537.25,932.18,562.14,924.77,584.62,913.49,604.69,898.33,622.36,879.30,362.63,552.06,655.11,542.10,0.00,0.00,0.00,0.00,0.00,0.00,0.00,0.00,180.53,589.21,183.83,510.62,198.32,441.31,223.19,380.61,257.99,328.15,302.65,283.85,357.42,247.94,422.94,220.95,500.20,203.69,574.30,218.73,636.83,243.37,688.80,276.78,730.88,318.41,763.38,367.99,786.27,425.57,799.17,491.44,801.35,566.21,429.53,247.84,501.65,247.73,569.24,245.55,402.87,287.81,503.10,291.77,595.95,284.27,340.18,330.70,422.36,333.26,504.55,335.81,579.99,330.01,655.44,324.21,317.46,380.36,411.73,380.10,506.00,379.85,591.79,375.90,677.59,371.95,275.60,436.95,352.88,432.60,430.16,428.24,507.45,423.89,577.15,424.31,646.86,424.73,716.57,425.15,499.06,151.62,359.43,191.66,253.75,265.11,182.02,371.96,147.43,575.69,632.07,187.00,732.22,255.02,799.51,355.65,830.33,550.39,156.16,637.25,170.43,696.92,190.24,754.68,215.59,810.55,236.12,855.97,275.97,909.01,323.50,953.91,378.71,990.68,441.60,1019.32,512.17,1039.83,576.35,1010.35,632.94,974.43,681.95,932.08,723.38,883.29,757.23,828.06,783.49,766.39,793.18,727.16,809.95,669.51,822.33,610.59,497.92,99.54,345.60,143.22,230.31,223.35,152.06,339.92,114.32,562.16,643.02,138.15,752.27,212.34,825.68,322.12,859.30,534.56,123.84,629.33,139.41,694.42,161.02,757.43,188.68,818.38,211.08,867.93,254.55,925.78,306.40,974.77,366.63,1014.88,435.24,1046.12,512.23,1068.50,582.24,1036.34,643.97,997.16,697.44,950.96,742.63,897.73,779.56,837.48,808.21,770.21,818.77,727.41,837.07,664.52,850.58,600.24,496.78,47.46,331.76,94.79,206.87,181.59,122.09,307.87,81.21,548.64,653.97,89.29,772.33,169.66,851.85,288.60,888.28,518.74,91.53,621.40,108.39,691.91,131.81,760.18,161.77,826.20,186.03,879.89,233.13,942.56,289.30,995.63,354.55,1039.08,428.88,1072.93,512.28,1097.17,588.12,1062.33,655.00,1019.89,712.92,969.83,761.89,912.17,801.89,846.90,832.92,774.02,844.37,727.66,864.19,659.53,878.83,589.89,357.73,904.98,384.03,926.87,412.51,944.66,443.17,958.35,476.01,967.94,511.04,973.43,543.69,963.64,573.94,950.38,601.78,933.64,627.22,913.44,650.25,889.77, 
// 	};
// 
// 	memcpy(pDstPoint442, pTestPoint, nCount * sizeof(Vector2));

	//插值嘴巴最外围点
	nCount = CalMoreMouthOut(nCount, pFacePoint118Copy, pDstPoint442);

	nCount = CalFaceOut4(nCount, pFacePoint118Copy, pDstPoint442,bFaceOutFix);
	OptimizingGrid(pDstPoint442, m_controlpoint, m_vertexOneRingvertex, m_vertexOneRingedgeweight);
}


int InterFacePointDeform::CalFaceOut4(int nCount, Vector2 * FacePoint, Vector2 * DstPoint, bool bFaceOutFix)
{
	Vector2 s;
	Vector2 newPt;
	int num = 0;
	Vector2 center((DstPoint[89].x + DstPoint[107].x)*0.5f, (DstPoint[80].y + DstPoint[98].y)*0.5f);

	if (bFaceOutFix)
	{
		center.x = DstPoint[48].x;
		center.y = DstPoint[48].y;
	}

	Vector2 V1, V2;
	V1.x = 612;
	V1.y = -9.2;
	V2 = FacePoint[32] - FacePoint[0];
	float fNormV1 = V1.length();
	float fNormV2 = V2.length();

	float tmp = (V1.x * V2.x + V1.y * V2.y) / (fNormV1*fNormV2);
	tmp = (std::min)(1.0f, tmp);
	tmp = (std::max)(-1.0f, tmp);
	float angle = -std::acos(tmp);

	float fCrossVal = V1[0] * V2[1] - V2[0] * V1[1];
	if (fCrossVal < 0) angle = -angle;

	Matrix2   pRotate, pInvRoatate, pFinalMat;

	float expandRate = 1.0f;
	pRotate[0] = std::cos(angle)*2.75*expandRate;
	pRotate[1] = -std::sin(angle)*2.75*expandRate;
	pRotate[2] = std::sin(angle)*1.5*expandRate;
	pRotate[3] = std::cos(angle)*1.5*expandRate;

	if (bFaceOutFix)
	{
		pRotate[0] = std::cos(angle);
		pRotate[1] = -std::sin(angle);
		pRotate[2] = std::sin(angle);
		pRotate[3] = std::cos(angle);
	}


	pInvRoatate[0] = std::cos(-angle);
	pInvRoatate[1] = -std::sin(-angle);
	pInvRoatate[2] = std::sin(-angle);
	pInvRoatate[3] = std::cos(-angle);

	pFinalMat = pInvRoatate*pRotate;

	int nBegin = nCount;

	DstPoint[nCount++] = FacePoint[0];
	DstPoint[nCount++] = FacePoint[32];
	DstPoint[84] = FacePoint[1];
	DstPoint[88] = FacePoint[31];

	for (int i = 89; i < 108; i++)
	{
		DstPoint[i] = FacePoint[i - 87];
	}

	for (int i = 21; i < 31; i++)
	{
		DstPoint[nCount++] = FacePoint[i];
	}

	for (int i = 80; i < 84; i++)
	{
		DstPoint[nCount++] = (DstPoint[i] + DstPoint[i + 1])*0.5;
	}

	for (int i = 85; i < 88; i++)
	{
		DstPoint[nCount++] = (DstPoint[i] + DstPoint[i + 1])*0.5;
	}
	DstPoint[nCount++] = (DstPoint[80] + DstPoint[85])*0.5;

	int nEnd = nCount;

	float temp = 0.0f;
	for (int j = 0; j <= 4; j++)
	{
		temp += 0.1f;
		for (int i = 80; i <= 107; i++)
		{
			//计算对应的位移
			s = DstPoint[i] - center;
			s = temp*s;
			s = pFinalMat*s;
			//
			newPt = DstPoint[i] + s;
			DstPoint[nCount++] = newPt;
		}

		for (int i = nBegin; i < nEnd; i++)
		{
			//计算对应的位移
			s = DstPoint[i] - center;
			s = temp*s;
			s = pFinalMat*s;
			//
			newPt = DstPoint[i] + s;
			DstPoint[nCount++] = newPt;
		}
	}

	return nCount;
}


void InterFacePointDeform::OptimizingGrid(Vector2 *Face442Point, int *controlpoint,
	std::vector<std::vector<int>> &vertexOneRingvertex,
	std::vector<std::vector<float>> &vertexOneRingedgeweight) {
	int flag = 0;
	while (flag < 3)
	{
		for (int k = 0; k < 318; k++)
		{
			Face442Point[controlpoint[k]].x = 0;
			Face442Point[controlpoint[k]].y = 0;

			float tmpweight = 0;
			float  coord[2] = { 0,0 };
			for (int i = 0; i < vertexOneRingvertex[k].size(); i++)
			{
				coord[0] += vertexOneRingedgeweight[k][i] * Face442Point[vertexOneRingvertex[k][i]].x;
				coord[1] += vertexOneRingedgeweight[k][i] * Face442Point[vertexOneRingvertex[k][i]].y;
				tmpweight += vertexOneRingedgeweight[k][i];
			}

			Face442Point[controlpoint[k]].x = coord[0] / tmpweight;
			Face442Point[controlpoint[k]].y = coord[1] / tmpweight;
		}
		flag++;
	}

}

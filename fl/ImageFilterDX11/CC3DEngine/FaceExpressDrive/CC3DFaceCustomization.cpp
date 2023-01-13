#include "CC3DFaceCustomization.h"

CC3DFaceCustomization::CC3DFaceCustomization()
{
	for (auto iter = blendShapeMap.begin(); iter != blendShapeMap.end(); iter++)
	{
		auto blendshapeFllow = iter->second;
		for (int i = 0; i < blendshapeFllow.size(); i++)
		{
			_blendShapeName.push_back(blendshapeFllow[i]);

		}
	}


	_coeffs_array.resize(_blendShapeName.size());
	for (int i = 0; i < _blendShapeName.size(); ++i)
	{
		blendShapeFllowIndex[_blendShapeName[i]] = i;
		_coeffs_array[i] = 0.0f;
	}
}

CC3DFaceCustomization::~CC3DFaceCustomization()
{

}

void CC3DFaceCustomization::UpdateCoeffByBSName(const std::string& _name, float ratio)
{
	auto it = blendShapeFllowIndex.find(_name);
	if (it != blendShapeFllowIndex.end())
	{
		_coeffs_array[it->second] = ratio;

	}
	else
	{
		return;
	}
	auto iter = blendShapeMap.find(_name);
	if (iter != blendShapeMap.end())
	{
		auto blendshapeFllow = iter->second;
		for (int i = 0; i < blendshapeFllow.size(); i++)
		{
			_coeffs_array[blendShapeFllowIndex[blendshapeFllow[i]]] = ratio;
		}
	}
}

void CC3DFaceCustomization::ResetBlendShape()
{
	for (auto& item : _coeffs_array)
	{
		item = 0.0f;
	}
}
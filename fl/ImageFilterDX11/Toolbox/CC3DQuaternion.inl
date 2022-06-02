#include "CC3DQuaternion.h"

namespace CC3DMath
{

	inline const CC3DQuaternion CC3DQuaternion::operator*(const CC3DQuaternion& q) const
	{
		CC3DQuaternion result(*this);
		result.multiply(q);
		return result;
	}

	inline CC3DQuaternion& CC3DQuaternion::operator*=(const CC3DQuaternion& q)
	{
		multiply(q);
		return *this;
	}

}
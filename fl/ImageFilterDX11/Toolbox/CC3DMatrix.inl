#include "CC3DMatrix.h"

namespace CC3DMath
{
	inline const CC3DMatrix CC3DMatrix::operator+(const CC3DMatrix& m) const
	{
		CC3DMatrix result(*this);
		result.add(m);
		return result;
	}

	inline CC3DMatrix& CC3DMatrix::operator+=(const CC3DMatrix& m)
	{
		add(m);
		return *this;
	}

	inline const CC3DMatrix CC3DMatrix::operator-(const CC3DMatrix& m) const
	{
		CC3DMatrix result(*this);
		result.subtract(m);
		return result;
	}

	inline CC3DMatrix& CC3DMatrix::operator-=(const CC3DMatrix& m)
	{
		subtract(m);
		return *this;
	}

	inline const CC3DMatrix CC3DMatrix::operator-() const
	{
		CC3DMatrix m(*this);
		m.negate();
		return m;
	}

	inline const CC3DMatrix CC3DMatrix::operator*(const CC3DMatrix& m) const
	{
		CC3DMatrix result(*this);
		result.multiply(m);
		return result;
	}

	inline CC3DMatrix& CC3DMatrix::operator*=(const CC3DMatrix& m)
	{
		multiply(m);
		return *this;
	}

	inline Vector3& operator*=(Vector3& v, const CC3DMatrix& m)
	{
		m.transformVector(&v);
		return v;
	}

	inline const Vector3 operator*(const CC3DMatrix& m, const Vector3& v)
	{
		Vector3 x;
		m.transformVector(v, &x);
		return x;
	}

	inline Vector4& operator*=(Vector4& v, const CC3DMatrix& m)
	{
		m.transformVector(&v);
		return v;
	}

	inline const Vector4 operator*(const CC3DMatrix& m, const Vector4& v)
	{
		Vector4 x;
		m.transformVector(v, &x);
		return x;
	}
}
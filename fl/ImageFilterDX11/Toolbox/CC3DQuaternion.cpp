#include "CC3DQuaternion.h"
#include "CC3DMatrix.h"
#include <string>

namespace CC3DMath
{

	CC3DQuaternion::CC3DQuaternion()
		: x(0.0f), y(0.0f), z(0.0f), w(1.0f)
	{
	}

	CC3DQuaternion::CC3DQuaternion(float x, float y, float z, float w)
		: x(x), y(y), z(z), w(w)
	{
	}

	CC3DQuaternion::CC3DQuaternion(float* array)
	{
		set(array);
	}

	CC3DQuaternion::CC3DQuaternion(const CC3DMatrix& m)
	{
		set(m);
	}

	CC3DQuaternion::CC3DQuaternion(const Vector3& axis, float angle)
	{
		set(axis, angle);
	}

	CC3DQuaternion::CC3DQuaternion(const CC3DQuaternion& copy)
	{
		set(copy);
	}

	CC3DQuaternion::~CC3DQuaternion()
	{
	}

	const CC3DQuaternion& CC3DQuaternion::identity()
	{
		static CC3DQuaternion value(0.0f, 0.0f, 0.0f, 1.0f);
		return value;
	}

	const CC3DQuaternion& CC3DQuaternion::zero()
	{
		static CC3DQuaternion value(0.0f, 0.0f, 0.0f, 0.0f);
		return value;
	}

	bool CC3DQuaternion::isIdentity() const
	{
		return x == 0.0f && y == 0.0f && z == 0.0f && w == 1.0f;
	}

	bool CC3DQuaternion::isZero() const
	{
		return x == 0.0f && y == 0.0f && z == 0.0f && w == 0.0f;
	}

	void CC3DQuaternion::createFromEuler(float yaw, float pitch, float roll, CC3DQuaternion* dst)
	{

		pitch *= 0.5f;
		yaw *= 0.5f;
		roll *= 0.5f;

		float sinp = sin(pitch);
		float siny = sin(yaw);
		float sinr = sin(roll);
		float cosp = cos(pitch);
		float cosy = cos(yaw);
		float cosr = cos(roll);

		dst->w = cosp * cosy * cosr + sinp * siny * sinr;
		dst->x = sinp * cosy * cosr - cosp * siny * sinr;
		dst->y = cosp * siny * cosr + sinp * cosy * sinr;
		dst->z = cosp * cosy * sinr - sinp * siny * cosr;
	}

	void CC3DQuaternion::createFromRotationMatrix(const CC3DMatrix& m, CC3DQuaternion* dst)
	{
		m.getRotation(dst);
	}

	void CC3DQuaternion::createFromAxisAngle(const Vector3& axis, float angle, CC3DQuaternion* dst)
	{


		float halfAngle = angle * 0.5f;
		float sinHalfAngle = sinf(halfAngle);

		Vector3 normal(axis);
		normal.normalize();
		dst->x = normal.x * sinHalfAngle;
		dst->y = normal.y * sinHalfAngle;
		dst->z = normal.z * sinHalfAngle;
		dst->w = cosf(halfAngle);
	}

	void CC3DQuaternion::computeEuler(float* yaw, float* pitch, float* roll)
	{


		*pitch = std::atan2(2 * (w*x + y * z), 1 - 2 * (x*x + y * y));
		*yaw = std::asin(2 * (w*y - z * x));
		*roll = atan2(2 * (w*z + x * y), 1 - 2 * (y*y + z * z));
	}

	void CC3DQuaternion::conjugate()
	{
		conjugate(this);
	}

	void CC3DQuaternion::conjugate(CC3DQuaternion* dst) const
	{


		dst->x = -x;
		dst->y = -y;
		dst->z = -z;
		dst->w = w;
	}

	bool CC3DQuaternion::inverse()
	{
		return inverse(this);
	}

	bool CC3DQuaternion::inverse(CC3DQuaternion* dst) const
	{


		float n = x * x + y * y + z * z + w * w;
		if (n == 1.0f)
		{
			dst->x = -x;
			dst->y = -y;
			dst->z = -z;
			dst->w = w;

			return true;
		}

		// Too close to zero.
		if (n < 0.000001f)
			return false;

		n = 1.0f / n;
		dst->x = -x * n;
		dst->y = -y * n;
		dst->z = -z * n;
		dst->w = w * n;

		return true;
	}

	void CC3DQuaternion::multiply(const CC3DQuaternion& q)
	{
		multiply(*this, q, this);
	}

	void CC3DQuaternion::multiply(const CC3DQuaternion& q1, const CC3DQuaternion& q2, CC3DQuaternion* dst)
	{


		float x = q1.w * q2.x + q1.x * q2.w + q1.y * q2.z - q1.z * q2.y;
		float y = q1.w * q2.y - q1.x * q2.z + q1.y * q2.w + q1.z * q2.x;
		float z = q1.w * q2.z + q1.x * q2.y - q1.y * q2.x + q1.z * q2.w;
		float w = q1.w * q2.w - q1.x * q2.x - q1.y * q2.y - q1.z * q2.z;

		dst->x = x;
		dst->y = y;
		dst->z = z;
		dst->w = w;
	}

	void CC3DQuaternion::normalize()
	{
		normalize(this);
	}

	void CC3DQuaternion::normalize(CC3DQuaternion* dst) const
	{


		if (this != dst)
		{
			dst->x = x;
			dst->y = y;
			dst->z = z;
			dst->w = w;
		}

		float n = x * x + y * y + z * z + w * w;

		// Already normalized.
		if (n == 1.0f)
			return;

		n = sqrt(n);
		// Too close to zero.
		if (n < 0.000001f)
			return;

		n = 1.0f / n;
		dst->x *= n;
		dst->y *= n;
		dst->z *= n;
		dst->w *= n;
	}

	void CC3DQuaternion::rotatePoint(const Vector3& point, Vector3* dst) const
	{
		CC3DQuaternion vecQuat;
		CC3DQuaternion resQuat;
		vecQuat.x = point.x;
		vecQuat.y = point.y;
		vecQuat.z = point.z;
		vecQuat.w = 0.0f;

		CC3DQuaternion conQuat;
		this->conjugate(&conQuat);

		resQuat = vecQuat * conQuat;
		resQuat = (*this) * resQuat;

		dst->set(resQuat.x, resQuat.y, resQuat.z);
	}

	void CC3DQuaternion::set(float x, float y, float z, float w)
	{
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}

	void CC3DQuaternion::set(float* array)
	{


		x = array[0];
		y = array[1];
		z = array[2];
		w = array[3];
	}

	void CC3DQuaternion::set(const CC3DMatrix& m)
	{
		CC3DQuaternion::createFromRotationMatrix(m, this);
	}

	void CC3DQuaternion::set(const Vector3& axis, float angle)
	{
		CC3DQuaternion::createFromAxisAngle(axis, angle, this);
	}

	void CC3DQuaternion::set(const CC3DQuaternion& q)
	{
		this->x = q.x;
		this->y = q.y;
		this->z = q.z;
		this->w = q.w;
	}

	void CC3DQuaternion::setIdentity()
	{
		x = 0.0f;
		y = 0.0f;
		z = 0.0f;
		w = 1.0f;
	}

	float CC3DQuaternion::toAxisAngle(Vector3* axis) const
	{
		CC3DQuaternion q(x, y, z, w);
		q.normalize();
		if (axis)
		{
			axis->x = q.x;
			axis->y = q.y;
			axis->z = q.z;
			axis->normalize();
		}
		return (2.0f * acos(q.w));
	}

	CC3DQuaternion CC3DQuaternion::FromToRotation(Vector3 fromDirection, Vector3 toDirection)
	{
		float dotValue = fromDirection.normalize().dot(toDirection.normalize());
		if (abs(dotValue - 1.0f) < MATH_EPSILON)
		{
			return CC3DQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
		}

		Vector3 axis = fromDirection.cross(toDirection);
		float angle = acos(fromDirection.normalize().dot(toDirection.normalize()));
		CC3DQuaternion qtn;
		CC3DQuaternion::createFromAxisAngle(axis.normalize(), angle, &qtn);
		return qtn;
	}

	void CC3DQuaternion::lerp(const CC3DQuaternion& q1, const CC3DQuaternion& q2, float t, CC3DQuaternion* dst)
	{

		if (t == 0.0f)
		{
			memcpy(dst, &q1, sizeof(float) * 4);
			return;
		}
		else if (t == 1.0f)
		{
			memcpy(dst, &q2, sizeof(float) * 4);
			return;
		}

		float t1 = 1.0f - t;

		dst->x = t1 * q1.x + t * q2.x;
		dst->y = t1 * q1.y + t * q2.y;
		dst->z = t1 * q1.z + t * q2.z;
		dst->w = t1 * q1.w + t * q2.w;
	}

	void CC3DQuaternion::slerp(const CC3DQuaternion& q1, const CC3DQuaternion& q2, float t, CC3DQuaternion* dst)
	{

		CC3DQuaternion::slerp(q1.x, q1.y, q1.z, q1.w, q2.x, q2.y, q2.z, q2.w, t, &dst->x, &dst->y, &dst->z, &dst->w);
	}

	void CC3DQuaternion::squad(const CC3DQuaternion& q1, const CC3DQuaternion& q2, const CC3DQuaternion& s1, const CC3DQuaternion& s2, float t, CC3DQuaternion* dst)
	{


		CC3DQuaternion dstQ(0.0f, 0.0f, 0.0f, 1.0f);
		CC3DQuaternion dstS(0.0f, 0.0f, 0.0f, 1.0f);

		CC3DQuaternion::slerpForSquad(q1, q2, t, &dstQ);
		CC3DQuaternion::slerpForSquad(s1, s2, t, &dstS);
		CC3DQuaternion::slerpForSquad(dstQ, dstS, 2.0f * t * (1.0f - t), dst);
	}

	void CC3DQuaternion::slerp(float q1x, float q1y, float q1z, float q1w, float q2x, float q2y, float q2z, float q2w, float t, float* dstx, float* dsty, float* dstz, float* dstw)
	{
		// Fast slerp implementation by kwhatmough:
		// It contains no division operations, no trig, no inverse trig
		// and no sqrt. Not only does this code tolerate small constraint
		// errors in the input quaternions, it actually corrects for them.


		if (t == 0.0f)
		{
			*dstx = q1x;
			*dsty = q1y;
			*dstz = q1z;
			*dstw = q1w;
			return;
		}
		else if (t == 1.0f)
		{
			*dstx = q2x;
			*dsty = q2y;
			*dstz = q2z;
			*dstw = q2w;
			return;
		}

		if (q1x == q2x && q1y == q2y && q1z == q2z && q1w == q2w)
		{
			*dstx = q1x;
			*dsty = q1y;
			*dstz = q1z;
			*dstw = q1w;
			return;
		}

		float halfY, alpha, beta;
		float u, f1, f2a, f2b;
		float ratio1, ratio2;
		float halfSecHalfTheta, versHalfTheta;
		float sqNotU, sqU;

		float cosTheta = q1w * q2w + q1x * q2x + q1y * q2y + q1z * q2z;

		// As usual in all slerp implementations, we fold theta.
		alpha = cosTheta >= 0 ? 1.0f : -1.0f;
		halfY = 1.0f + alpha * cosTheta;

		// Here we bisect the interval, so we need to fold t as well.
		f2b = t - 0.5f;
		u = f2b >= 0 ? f2b : -f2b;
		f2a = u - f2b;
		f2b += u;
		u += u;
		f1 = 1.0f - u;

		// One iteration of Newton to get 1-cos(theta / 2) to good accuracy.
		halfSecHalfTheta = 1.09f - (0.476537f - 0.0903321f * halfY) * halfY;
		halfSecHalfTheta *= 1.5f - halfY * halfSecHalfTheta * halfSecHalfTheta;
		versHalfTheta = 1.0f - halfY * halfSecHalfTheta;

		// Evaluate series expansions of the coefficients.
		sqNotU = f1 * f1;
		ratio2 = 0.0000440917108f * versHalfTheta;
		ratio1 = -0.00158730159f + (sqNotU - 16.0f) * ratio2;
		ratio1 = 0.0333333333f + ratio1 * (sqNotU - 9.0f) * versHalfTheta;
		ratio1 = -0.333333333f + ratio1 * (sqNotU - 4.0f) * versHalfTheta;
		ratio1 = 1.0f + ratio1 * (sqNotU - 1.0f) * versHalfTheta;

		sqU = u * u;
		ratio2 = -0.00158730159f + (sqU - 16.0f) * ratio2;
		ratio2 = 0.0333333333f + ratio2 * (sqU - 9.0f) * versHalfTheta;
		ratio2 = -0.333333333f + ratio2 * (sqU - 4.0f) * versHalfTheta;
		ratio2 = 1.0f + ratio2 * (sqU - 1.0f) * versHalfTheta;

		// Perform the bisection and resolve the folding done earlier.
		f1 *= ratio1 * halfSecHalfTheta;
		f2a *= ratio2;
		f2b *= ratio2;
		alpha *= f1 + f2a;
		beta = f1 + f2b;

		// Apply final coefficients to a and b as usual.
		float w = alpha * q1w + beta * q2w;
		float x = alpha * q1x + beta * q2x;
		float y = alpha * q1y + beta * q2y;
		float z = alpha * q1z + beta * q2z;

		// This final adjustment to the quaternion's length corrects for
		// any small constraint error in the inputs q1 and q2 But as you
		// can see, it comes at the cost of 9 additional multiplication
		// operations. If this error-correcting feature is not required,
		// the following code may be removed.
		f1 = 1.5f - 0.5f * (w * w + x * x + y * y + z * z);
		*dstw = w * f1;
		*dstx = x * f1;
		*dsty = y * f1;
		*dstz = z * f1;
	}

	void CC3DQuaternion::slerpForSquad(const CC3DQuaternion& q1, const CC3DQuaternion& q2, float t, CC3DQuaternion* dst)
	{

		// cos(omega) = q1 * q2;
		// slerp(q1, q2, t) = (q1*sin((1-t)*omega) + q2*sin(t*omega))/sin(omega);
		// q1 = +- q2, slerp(q1,q2,t) = q1.
		// This is a straight-forward implementation of the formula of slerp. It does not do any sign switching.
		float c = q1.x * q2.x + q1.y * q2.y + q1.z * q2.z + q1.w * q2.w;

		if (fabs(c) >= 1.0f)
		{
			dst->x = q1.x;
			dst->y = q1.y;
			dst->z = q1.z;
			dst->w = q1.w;
			return;
		}

		float omega = acos(c);
		float s = sqrt(1.0f - c * c);
		if (fabs(s) <= 0.00001f)
		{
			dst->x = q1.x;
			dst->y = q1.y;
			dst->z = q1.z;
			dst->w = q1.w;
			return;
		}

		float r1 = sin((1 - t) * omega) / s;
		float r2 = sin(t * omega) / s;
		dst->x = (q1.x * r1 + q2.x * r2);
		dst->y = (q1.y * r1 + q2.y * r2);
		dst->z = (q1.z * r1 + q2.z * r2);
		dst->w = (q1.w * r1 + q2.w * r2);
	}

}
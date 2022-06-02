#include "CC3DMatrix.h"
#include "CC3DQuaternion.h"
#include "Algorithm/MathUtils.h"
#include <intrin.h>


namespace CC3DMath
{
	static const float MATRIX_IDENTITY[16] =
	{
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 1.0f
	};

	CC3DMatrix::CC3DMatrix()
	{
		*this = CC3DMatrix::identity();
	}

	CC3DMatrix::CC3DMatrix(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
	{
		set(m11, m12, m13, m14, m21, m22, m23, m24, m31, m32, m33, m34, m41, m42, m43, m44);
	}

	CC3DMatrix::CC3DMatrix(const float* m)
	{
		set(m);
	}

	CC3DMatrix::CC3DMatrix(const CC3DMatrix& copy)
	{
		memcpy(m, copy.m, MATRIX_SIZE);
	}

	CC3DMatrix::~CC3DMatrix()
	{
	}

	const CC3DMatrix& CC3DMatrix::identity()
	{
		static CC3DMatrix m(
			1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
		return m;
	}

	const CC3DMatrix& CC3DMatrix::zero()
	{
		static CC3DMatrix m(
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0,
			0, 0, 0, 0);
		return m;
	}

	void CC3DMatrix::createLookAt(const Vector3& eyePosition, const Vector3& targetPosition, const Vector3& up, CC3DMatrix* dst)
	{
		createLookAt(eyePosition.x, eyePosition.y, eyePosition.z, targetPosition.x, targetPosition.y, targetPosition.z,
			up.x, up.y, up.z, dst);
	}

	void CC3DMatrix::createLookAt(float eyePositionX, float eyePositionY, float eyePositionZ,
		float targetPositionX, float targetPositionY, float targetPositionZ,
		float upX, float upY, float upZ, CC3DMatrix* dst)
	{
		Vector3 eye(eyePositionX, eyePositionY, eyePositionZ);
		Vector3 target(targetPositionX, targetPositionY, targetPositionZ);
		Vector3 up(upX, upY, upZ);
		up.normalize();

		Vector3 zaxis;
		//Vector3::subtract(eye, target, &zaxis);
		zaxis = eye - target;
		zaxis.normalize();

		Vector3 xaxis;
		//Vector3::cross(up, zaxis, &xaxis);
		xaxis = up.cross(zaxis);
		xaxis.normalize();

		Vector3 yaxis;
		//Vector3::cross(zaxis, xaxis, &yaxis);
		yaxis = zaxis.cross(xaxis);
		yaxis.normalize();

		dst->m[0] = xaxis.x;
		dst->m[1] = yaxis.x;
		dst->m[2] = zaxis.x;
		dst->m[3] = 0.0f;

		dst->m[4] = xaxis.y;
		dst->m[5] = yaxis.y;
		dst->m[6] = zaxis.y;
		dst->m[7] = 0.0f;

		dst->m[8] = xaxis.z;
		dst->m[9] = yaxis.z;
		dst->m[10] = zaxis.z;
		dst->m[11] = 0.0f;

		//dst->m[12] = -Vector3::dot(xaxis, eye);
		//dst->m[13] = -Vector3::dot(yaxis, eye);
		//dst->m[14] = -Vector3::dot(zaxis, eye);
		dst->m[12] = -xaxis.dot(eye);
		dst->m[13] = -yaxis.dot(eye);
		dst->m[14] = -zaxis.dot(eye);
		dst->m[15] = 1.0f;
	}

	void CC3DMatrix::createPerspective(float fieldOfView, float aspectRatio,
		float zNearPlane, float zFarPlane, CC3DMatrix* dst)
	{

		//float f_n = 1.0f / (zFarPlane - zNearPlane);
		//float theta = MATH_DEG_TO_RAD(fieldOfView) * 0.5f;
		//if (fabs(fmod(theta, MATH_PIOVER2)) < MATH_EPSILON)
		//{
		//   // GP_ERROR("Invalid field of view value (%d) causes attempted calculation tan(%d), which is undefined.", fieldOfView, theta);
		//    return;
		//}
		//float divisor = tan(theta);

		//float factor = 1.0f / divisor;

		//memset(dst, 0, MATRIX_SIZE);

		//dst->m[0] = (1.0f / aspectRatio) * factor;
		//dst->m[5] = factor;
		//dst->m[10] = (-(zFarPlane + zNearPlane)) * f_n;
		//dst->m[11] = -1.0f;
		//dst->m[14] = -2.0f * zFarPlane * zNearPlane * f_n;
	}

	void CC3DMatrix::createOrthographic(float width, float height, float zNearPlane, float zFarPlane, CC3DMatrix* dst)
	{
		float halfWidth = width / 2.0f;
		float halfHeight = height / 2.0f;
		createOrthographicOffCenter(-halfWidth, halfWidth, -halfHeight, halfHeight, zNearPlane, zFarPlane, dst);
	}

	void CC3DMatrix::createOrthographicOffCenter(float left, float right, float bottom, float top,
		float zNearPlane, float zFarPlane, CC3DMatrix* dst)
	{


		memset(dst, 0, MATRIX_SIZE);
		dst->m[0] = 2 / (right - left);
		dst->m[5] = 2 / (top - bottom);
		dst->m[12] = (left + right) / (left - right);
		dst->m[10] = 1 / (zNearPlane - zFarPlane);
		dst->m[13] = (top + bottom) / (bottom - top);
		dst->m[14] = zNearPlane / (zNearPlane - zFarPlane);
		dst->m[15] = 1;
	}

	void CC3DMatrix::createBillboard(const Vector3& objectPosition, const Vector3& cameraPosition,
		const Vector3& cameraUpVector, CC3DMatrix* dst)
	{
		createBillboardHelper(objectPosition, cameraPosition, cameraUpVector, NULL, dst);
	}

	void CC3DMatrix::createBillboard(const Vector3& objectPosition, const Vector3& cameraPosition,
		const Vector3& cameraUpVector, const Vector3& cameraForwardVector,
		CC3DMatrix* dst)
	{
		createBillboardHelper(objectPosition, cameraPosition, cameraUpVector, &cameraForwardVector, dst);
	}

	void CC3DMatrix::createBillboardHelper(const Vector3& objectPosition, const Vector3& cameraPosition,
		const Vector3& cameraUpVector, const Vector3* cameraForwardVector,
		CC3DMatrix* dst)
	{
		//Vector3 delta(objectPosition, cameraPosition);
		//bool isSufficientDelta = delta.lengthSquared() > MATH_EPSILON;

		//dst->setIdentity();
		//dst->m[3] = objectPosition.x;
		//dst->m[7] = objectPosition.y;
		//dst->m[11] = objectPosition.z;

		//// As per the contracts for the 2 variants of createBillboard, we need
		//// either a safe default or a sufficient distance between object and camera.
		//if (cameraForwardVector || isSufficientDelta)
		//{
		//    Vector3 target = isSufficientDelta ? cameraPosition : (objectPosition - *cameraForwardVector);

		//    // A billboard is the inverse of a lookAt rotation
		//    CC3DMatrix lookAt;
		//    createLookAt(objectPosition, target, cameraUpVector, &lookAt);
		//    dst->m[0] = lookAt.m[0];
		//    dst->m[1] = lookAt.m[4];
		//    dst->m[2] = lookAt.m[8];
		//    dst->m[4] = lookAt.m[1];
		//    dst->m[5] = lookAt.m[5];
		//    dst->m[6] = lookAt.m[9];
		//    dst->m[8] = lookAt.m[2];
		//    dst->m[9] = lookAt.m[6];
		//    dst->m[10] = lookAt.m[10];
		//}
	}

	//void CC3DMatrix::createReflection(const Plane& plane, CC3DMatrix* dst)
	//{
	//    Vector3 normal(plane.getNormal());
	//    float k = -2.0f * plane.getDistance();
	//
	//    dst->setIdentity();
	//
	//    dst->m[0] -= 2.0f * normal.x * normal.x;
	//    dst->m[5] -= 2.0f * normal.y * normal.y;
	//    dst->m[10] -= 2.0f * normal.z * normal.z;
	//    dst->m[1] = dst->m[4] = -2.0f * normal.x * normal.y;
	//    dst->m[2] = dst->m[8] = -2.0f * normal.x * normal.z;
	//    dst->m[6] = dst->m[9] = -2.0f * normal.y * normal.z;
	//    
	//    dst->m[3] = k * normal.x;
	//    dst->m[7] = k * normal.y;
	//    dst->m[11] = k * normal.z;
	//}

	void CC3DMatrix::createScale(const Vector3& scale, CC3DMatrix* dst)
	{


		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		dst->m[0] = scale.x;
		dst->m[5] = scale.y;
		dst->m[10] = scale.z;
	}

	void CC3DMatrix::createScale(float xScale, float yScale, float zScale, CC3DMatrix* dst)
	{


		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		dst->m[0] = xScale;
		dst->m[5] = yScale;
		dst->m[10] = zScale;
	}


	void CC3DMatrix::createRotation(const CC3DQuaternion& q, CC3DMatrix* dst)
	{
		float x2 = q.x + q.x;
		float y2 = q.y + q.y;
		float z2 = q.z + q.z;

		float xx2 = q.x * x2;
		float yy2 = q.y * y2;
		float zz2 = q.z * z2;
		float xy2 = q.x * y2;
		float xz2 = q.x * z2;
		float yz2 = q.y * z2;
		float wx2 = q.w * x2;
		float wy2 = q.w * y2;
		float wz2 = q.w * z2;

		dst->m[0] = 1.0f - yy2 - zz2;
		dst->m[1] = xy2 + wz2;
		dst->m[2] = xz2 - wy2;
		dst->m[3] = 0.0f;

		dst->m[4] = xy2 - wz2;
		dst->m[5] = 1.0f - xx2 - zz2;
		dst->m[6] = yz2 + wx2;
		dst->m[7] = 0.0f;

		dst->m[8] = xz2 + wy2;
		dst->m[9] = yz2 - wx2;
		dst->m[10] = 1.0f - xx2 - yy2;
		dst->m[11] = 0.0f;

		dst->m[12] = 0.0f;
		dst->m[13] = 0.0f;
		dst->m[14] = 0.0f;
		dst->m[15] = 1.0f;
	}

	void CC3DMatrix::createRotation(const Vector3& axis, float angle, CC3DMatrix* dst)
	{


		float x = axis.x;
		float y = axis.y;
		float z = axis.z;

		// Make sure the input axis is normalized.
		float n = x * x + y * y + z * z;
		if (n != 1.0f)
		{
			// Not normalized.
			n = sqrt(n);
			// Prevent divide too close to zero.
			if (n > 0.000001f)
			{
				n = 1.0f / n;
				x *= n;
				y *= n;
				z *= n;
			}
		}

		float c = cos(angle);
		float s = sin(angle);

		float t = 1.0f - c;
		float tx = t * x;
		float ty = t * y;
		float tz = t * z;
		float txy = tx * y;
		float txz = tx * z;
		float tyz = ty * z;
		float sx = s * x;
		float sy = s * y;
		float sz = s * z;

		dst->m[0] = c + tx * x;
		dst->m[1] = txy + sz;
		dst->m[2] = txz - sy;
		dst->m[3] = 0.0f;

		dst->m[4] = txy - sz;
		dst->m[5] = c + ty * y;
		dst->m[6] = tyz + sx;
		dst->m[7] = 0.0f;

		dst->m[8] = txz + sy;
		dst->m[9] = tyz - sx;
		dst->m[10] = c + tz * z;
		dst->m[11] = 0.0f;

		dst->m[12] = 0.0f;
		dst->m[13] = 0.0f;
		dst->m[14] = 0.0f;
		dst->m[15] = 1.0f;
	}

	void CC3DMatrix::createRotationX(float angle, CC3DMatrix* dst)
	{


		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		float c = cos(angle);
		float s = sin(angle);

		dst->m[5] = c;
		dst->m[6] = s;
		dst->m[9] = -s;
		dst->m[10] = c;
	}

	void CC3DMatrix::createRotationY(float angle, CC3DMatrix* dst)
	{

		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		float c = cos(angle);
		float s = sin(angle);

		dst->m[0] = c;
		dst->m[2] = -s;
		dst->m[8] = s;
		dst->m[10] = c;
	}

	void CC3DMatrix::createRotationZ(float angle, CC3DMatrix* dst)
	{


		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		float c = cos(angle);
		float s = sin(angle);

		dst->m[0] = c;
		dst->m[1] = s;
		dst->m[4] = -s;
		dst->m[5] = c;
	}

	void CC3DMatrix::createFromEuler(float yaw, float pitch, float roll, CC3DMatrix* dst)
	{


		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		dst->rotateY(yaw);
		dst->rotateX(pitch);
		dst->rotateZ(roll);
	}

	void CC3DMatrix::createTranslation(const Vector3& translation, CC3DMatrix* dst)
	{


		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		dst->m[12] = translation.x;
		dst->m[13] = translation.y;
		dst->m[14] = translation.z;
	}

	void CC3DMatrix::createTranslation(float xTranslation, float yTranslation, float zTranslation, CC3DMatrix* dst)
	{

		memcpy(dst, MATRIX_IDENTITY, MATRIX_SIZE);

		dst->m[12] = xTranslation;
		dst->m[13] = yTranslation;
		dst->m[14] = zTranslation;
	}

	void CC3DMatrix::add(float scalar)
	{
		add(scalar, this);
	}

	void CC3DMatrix::add(float scalar, CC3DMatrix* dst)
	{
		CMathUtils::addMatrix(m, scalar, dst->m);
	}

	void CC3DMatrix::add(const CC3DMatrix& m)
	{
		add(*this, m, this);
	}

	void CC3DMatrix::add(const CC3DMatrix& m1, const CC3DMatrix& m2, CC3DMatrix* dst)
	{


		CMathUtils::addMatrix(m1.m, m2.m, dst->m);
	}

	bool CC3DMatrix::decompose(Vector3* scale, CC3DQuaternion* rotation, Vector3* translation) const
	{
		if (translation)
		{
			// Extract the translation.
			translation->x = m[12];
			translation->y = m[13];
			translation->z = m[14];
		}

		// Nothing left to do.
		if (scale == NULL && rotation == NULL)
			return true;

		// Extract the scale.
		// This is simply the length of each axis (row/column) in the matrix.
		Vector3 xaxis(m[0], m[1], m[2]);
		float scaleX = xaxis.length();

		Vector3 yaxis(m[4], m[5], m[6]);
		float scaleY = yaxis.length();

		Vector3 zaxis(m[8], m[9], m[10]);
		float scaleZ = zaxis.length();

		// Determine if we have a negative scale (true if determinant is less than zero).
		// In this case, we simply negate a single axis of the scale.
		float det = determinant();
		if (det < 0)
			scaleZ = -scaleZ;

		if (scale)
		{
			scale->x = scaleX;
			scale->y = scaleY;
			scale->z = scaleZ;
		}

		// Nothing left to do.
		if (rotation == NULL)
			return true;

		// Scale too close to zero, can't decompose rotation.
		if (scaleX < MATH_TOLERANCE || scaleY < MATH_TOLERANCE || fabs(scaleZ) < MATH_TOLERANCE)
			return false;

		float rn;

		// Factor the scale out of the matrix axes.
		rn = 1.0f / scaleX;
		xaxis.x *= rn;
		xaxis.y *= rn;
		xaxis.z *= rn;

		rn = 1.0f / scaleY;
		yaxis.x *= rn;
		yaxis.y *= rn;
		yaxis.z *= rn;

		rn = 1.0f / scaleZ;
		zaxis.x *= rn;
		zaxis.y *= rn;
		zaxis.z *= rn;

		// Now calculate the rotation from the resulting matrix (axes).
		float trace = xaxis.x + yaxis.y + zaxis.z + 1.0f;

		if (trace > 1.0f)
		{
			float s = 0.5f / sqrt(trace);
			rotation->w = 0.25f / s;
			rotation->x = (yaxis.z - zaxis.y) * s;
			rotation->y = (zaxis.x - xaxis.z) * s;
			rotation->z = (xaxis.y - yaxis.x) * s;
		}
		else
		{
			// Note: since xaxis, yaxis, and zaxis are normalized, 
			// we will never divide by zero in the code below.
			if (xaxis.x > yaxis.y && xaxis.x > zaxis.z)
			{
				float s = 0.5f / sqrt(1.0f + xaxis.x - yaxis.y - zaxis.z);
				rotation->w = (yaxis.z - zaxis.y) * s;
				rotation->x = 0.25f / s;
				rotation->y = (yaxis.x + xaxis.y) * s;
				rotation->z = (zaxis.x + xaxis.z) * s;
			}
			else if (yaxis.y > zaxis.z)
			{
				float s = 0.5f / sqrt(1.0f + yaxis.y - xaxis.x - zaxis.z);
				rotation->w = (zaxis.x - xaxis.z) * s;
				rotation->x = (yaxis.x + xaxis.y) * s;
				rotation->y = 0.25f / s;
				rotation->z = (zaxis.y + yaxis.z) * s;
			}
			else
			{
				float s = 0.5f / sqrt(1.0f + zaxis.z - xaxis.x - yaxis.y);
				rotation->w = (xaxis.y - yaxis.x) * s;
				rotation->x = (zaxis.x + xaxis.z) * s;
				rotation->y = (zaxis.y + yaxis.z) * s;
				rotation->z = 0.25f / s;
			}
		}

		return true;
	}

	float CC3DMatrix::determinant() const
	{
		float a0 = m[0] * m[5] - m[1] * m[4];
		float a1 = m[0] * m[6] - m[2] * m[4];
		float a2 = m[0] * m[7] - m[3] * m[4];
		float a3 = m[1] * m[6] - m[2] * m[5];
		float a4 = m[1] * m[7] - m[3] * m[5];
		float a5 = m[2] * m[7] - m[3] * m[6];
		float b0 = m[8] * m[13] - m[9] * m[12];
		float b1 = m[8] * m[14] - m[10] * m[12];
		float b2 = m[8] * m[15] - m[11] * m[12];
		float b3 = m[9] * m[14] - m[10] * m[13];
		float b4 = m[9] * m[15] - m[11] * m[13];
		float b5 = m[10] * m[15] - m[11] * m[14];

		// Calculate the determinant.
		return (a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0);
	}

	void CC3DMatrix::getScale(Vector3* scale) const
	{
		decompose(scale, NULL, NULL);
	}

	bool CC3DMatrix::getRotation(CC3DQuaternion* rotation) const
	{
		return decompose(NULL, rotation, NULL);
	}

	void CC3DMatrix::getTranslation(Vector3* translation) const
	{
		decompose(NULL, NULL, translation);
	}

	void CC3DMatrix::getUpVector(Vector3* dst) const
	{


		dst->x = m[4];
		dst->y = m[5];
		dst->z = m[6];
	}

	void CC3DMatrix::getDownVector(Vector3* dst) const
	{


		dst->x = -m[4];
		dst->y = -m[5];
		dst->z = -m[6];
	}

	void CC3DMatrix::getLeftVector(Vector3* dst) const
	{


		dst->x = -m[0];
		dst->y = -m[1];
		dst->z = -m[2];
	}

	void CC3DMatrix::getRightVector(Vector3* dst) const
	{


		dst->x = m[0];
		dst->y = m[1];
		dst->z = m[2];
	}

	void CC3DMatrix::getForwardVector(Vector3* dst) const
	{


		dst->x = -m[8];
		dst->y = -m[9];
		dst->z = -m[10];
	}

	void CC3DMatrix::getBackVector(Vector3* dst) const
	{


		dst->x = m[8];
		dst->y = m[9];
		dst->z = m[10];
	}

	bool CC3DMatrix::invert()
	{
		return invert(this);
	}

	bool CC3DMatrix::invert(CC3DMatrix* dst) const
	{
		float a0 = m[0] * m[5] - m[1] * m[4];
		float a1 = m[0] * m[6] - m[2] * m[4];
		float a2 = m[0] * m[7] - m[3] * m[4];
		float a3 = m[1] * m[6] - m[2] * m[5];
		float a4 = m[1] * m[7] - m[3] * m[5];
		float a5 = m[2] * m[7] - m[3] * m[6];
		float b0 = m[8] * m[13] - m[9] * m[12];
		float b1 = m[8] * m[14] - m[10] * m[12];
		float b2 = m[8] * m[15] - m[11] * m[12];
		float b3 = m[9] * m[14] - m[10] * m[13];
		float b4 = m[9] * m[15] - m[11] * m[13];
		float b5 = m[10] * m[15] - m[11] * m[14];

		// Calculate the determinant.
		float det = a0 * b5 - a1 * b4 + a2 * b3 + a3 * b2 - a4 * b1 + a5 * b0;

		// Close to zero, can't invert.
		if (fabs(det) <= MATH_TOLERANCE)
			return false;

		// Support the case where m == dst.
		CC3DMatrix inverse;
		inverse.m[0] = m[5] * b5 - m[6] * b4 + m[7] * b3;
		inverse.m[1] = -m[1] * b5 + m[2] * b4 - m[3] * b3;
		inverse.m[2] = m[13] * a5 - m[14] * a4 + m[15] * a3;
		inverse.m[3] = -m[9] * a5 + m[10] * a4 - m[11] * a3;

		inverse.m[4] = -m[4] * b5 + m[6] * b2 - m[7] * b1;
		inverse.m[5] = m[0] * b5 - m[2] * b2 + m[3] * b1;
		inverse.m[6] = -m[12] * a5 + m[14] * a2 - m[15] * a1;
		inverse.m[7] = m[8] * a5 - m[10] * a2 + m[11] * a1;

		inverse.m[8] = m[4] * b4 - m[5] * b2 + m[7] * b0;
		inverse.m[9] = -m[0] * b4 + m[1] * b2 - m[3] * b0;
		inverse.m[10] = m[12] * a4 - m[13] * a2 + m[15] * a0;
		inverse.m[11] = -m[8] * a4 + m[9] * a2 - m[11] * a0;

		inverse.m[12] = -m[4] * b3 + m[5] * b1 - m[6] * b0;
		inverse.m[13] = m[0] * b3 - m[1] * b1 + m[2] * b0;
		inverse.m[14] = -m[12] * a3 + m[13] * a1 - m[14] * a0;
		inverse.m[15] = m[8] * a3 - m[9] * a1 + m[10] * a0;

		multiply(inverse, 1.0f / det, dst);

		return true;
	}

	bool CC3DMatrix::isIdentity() const
	{
		return (memcmp(m, MATRIX_IDENTITY, MATRIX_SIZE) == 0);
	}

	void CC3DMatrix::multiply(float scalar)
	{
		multiply(scalar, this);
	}

	void CC3DMatrix::multiply(float scalar, CC3DMatrix* dst) const
	{
		multiply(*this, scalar, dst);
	}

	void CC3DMatrix::multiply(const CC3DMatrix& m, float scalar, CC3DMatrix* dst)
	{


		CMathUtils::multiplyMatrix(m.m, scalar, dst->m);
	}

	void CC3DMatrix::multiply(const CC3DMatrix& m)
	{
		multiply(*this, m, this);
	}

	void CC3DMatrix::multiply(const CC3DMatrix& m1, const CC3DMatrix& m2, CC3DMatrix* dst)
	{


		CMathUtils::multiplyMatrix(m1.m, m2.m, dst->m);
	}

	void CC3DMatrix::negate()
	{
		negate(this);
	}

	void CC3DMatrix::negate(CC3DMatrix* dst) const
	{


		CMathUtils::negateMatrix(m, dst->m);
	}

	void CC3DMatrix::rotate(const CC3DQuaternion& q)
	{
		rotate(q, this);
	}

	void CC3DMatrix::rotate(const CC3DQuaternion& q, CC3DMatrix* dst) const
	{
		CC3DMatrix r;
		createRotation(q, &r);
		multiply(*this, r, dst);
	}

	void CC3DMatrix::rotate(const Vector3& axis, float angle)
	{
		rotate(axis, angle, this);
	}

	void CC3DMatrix::rotate(const Vector3& axis, float angle, CC3DMatrix* dst) const
	{
		CC3DMatrix r;
		createRotation(axis, angle, &r);
		multiply(*this, r, dst);
	}

	void CC3DMatrix::rotateX(float angle)
	{
		rotateX(angle, this);
	}

	void CC3DMatrix::rotateX(float angle, CC3DMatrix* dst) const
	{
		CC3DMatrix r;
		createRotationX(angle, &r);
		multiply(*this, r, dst);
	}

	void CC3DMatrix::rotateY(float angle)
	{
		rotateY(angle, this);
	}

	void CC3DMatrix::rotateY(float angle, CC3DMatrix* dst) const
	{
		CC3DMatrix r;
		createRotationY(angle, &r);
		multiply(*this, r, dst);
	}

	void CC3DMatrix::rotateZ(float angle)
	{
		rotateZ(angle, this);
	}

	void CC3DMatrix::rotateZ(float angle, CC3DMatrix* dst) const
	{
		CC3DMatrix r;
		createRotationZ(angle, &r);
		multiply(*this, r, dst);
	}

	void CC3DMatrix::scale(float value)
	{
		scale(value, this);
	}

	void CC3DMatrix::scale(float value, CC3DMatrix* dst) const
	{
		scale(value, value, value, dst);
	}

	void CC3DMatrix::scale(float xScale, float yScale, float zScale)
	{
		scale(xScale, yScale, zScale, this);
	}

	void CC3DMatrix::scale(float xScale, float yScale, float zScale, CC3DMatrix* dst) const
	{
		CC3DMatrix s;
		createScale(xScale, yScale, zScale, &s);
		multiply(*this, s, dst);
	}

	void CC3DMatrix::scale(const Vector3& s)
	{
		scale(s.x, s.y, s.z, this);
	}

	void CC3DMatrix::scale(const Vector3& s, CC3DMatrix* dst) const
	{
		scale(s.x, s.y, s.z, dst);
	}

	void CC3DMatrix::set(float m11, float m12, float m13, float m14, float m21, float m22, float m23, float m24,
		float m31, float m32, float m33, float m34, float m41, float m42, float m43, float m44)
	{
		m[0] = m11;
		m[1] = m21;
		m[2] = m31;
		m[3] = m41;
		m[4] = m12;
		m[5] = m22;
		m[6] = m32;
		m[7] = m42;
		m[8] = m13;
		m[9] = m23;
		m[10] = m33;
		m[11] = m43;
		m[12] = m14;
		m[13] = m24;
		m[14] = m34;
		m[15] = m44;
	}

	void CC3DMatrix::set(const float* m)
	{

		memcpy(this->m, m, MATRIX_SIZE);
	}

	void CC3DMatrix::set(const CC3DMatrix& m)
	{
		memcpy(this->m, m.m, MATRIX_SIZE);
	}

	void CC3DMatrix::setIdentity()
	{
		memcpy(m, MATRIX_IDENTITY, MATRIX_SIZE);
	}

	void CC3DMatrix::setZero()
	{
		memset(m, 0, MATRIX_SIZE);
	}

	void CC3DMatrix::subtract(const CC3DMatrix& m)
	{
		subtract(*this, m, this);
	}

	void CC3DMatrix::subtract(const CC3DMatrix& m1, const CC3DMatrix& m2, CC3DMatrix* dst)
	{


		CMathUtils::subtractMatrix(m1.m, m2.m, dst->m);
	}

	void CC3DMatrix::transformPoint(Vector3* point) const
	{

		transformVector(point->x, point->y, point->z, 1.0f, point);
	}

	void CC3DMatrix::transformPoint(const Vector3& point, Vector3* dst) const
	{
		transformVector(point.x, point.y, point.z, 1.0f, dst);
	}

	void minvtransform_point_sse(const float *matrix, const float pointX, const float pointY, const float pointZ, float *r)
	{
		__m128 off = _mm_sub_ps(_mm_set_ps(1.0f, pointZ, pointY, pointX), _mm_loadu_ps(&matrix[12]));
		__m128 r1 = _mm_loadu_ps(matrix);
		__m128 r2 = _mm_loadu_ps(&matrix[4]);
		__m128 r3 = _mm_loadu_ps(&matrix[8]);
		__m128 zero = _mm_setzero_ps();
		__m128 lo = _mm_unpacklo_ps(r1, r3);//r0, u0, r1, u1
		__m128 lo2 = _mm_unpacklo_ps(r2, zero);//f0, 0, f1, 0
		off = _mm_add_ps(
			_mm_add_ps(
				_mm_mul_ps(_mm_unpacklo_ps(lo, lo2), _mm_shuffle_ps(off, off, _MM_SHUFFLE(0, 0, 0, 0))),
				_mm_mul_ps(_mm_unpackhi_ps(lo, lo2), _mm_shuffle_ps(off, off, _MM_SHUFFLE(1, 1, 1, 1)))),
			_mm_mul_ps(
				_mm_unpacklo_ps(_mm_unpackhi_ps(r1, r3), _mm_unpackhi_ps(r2, zero)), _mm_shuffle_ps(off, off, _MM_SHUFFLE(2, 2, 2, 2))));
		_mm_storeu_ps(r, off);
	}

	Vector3 CC3DMatrix::inverseTransformPoinit(Vector3 point)
	{
		CC3DMatrix* pinned = this;
		minvtransform_point_sse((float*)pinned, point.x, point.y, point.z, (float*)&point);
		return point;
	}

	void CC3DMatrix::transformVector(Vector3* vector) const
	{

		transformVector(vector->x, vector->y, vector->z, 0.0f, vector);
	}

	void CC3DMatrix::transformVector(const Vector3& vector, Vector3* dst) const
	{
		transformVector(vector.x, vector.y, vector.z, 0.0f, dst);
	}

	void CC3DMatrix::transformVector(float x, float y, float z, float w, Vector3* dst) const
	{
		CMathUtils::transformVector4(m, x, y, z, w, (float*)dst);
	}

	void CC3DMatrix::transformVector(Vector4* vector) const
	{

		transformVector(*vector, vector);
	}

	void CC3DMatrix::transformVector(const Vector4& vector, Vector4* dst) const
	{


		CMathUtils::transformVector4(m, (const float*)&vector, (float*)dst);
	}

	void CC3DMatrix::translate(float x, float y, float z)
	{
		translate(x, y, z, this);
	}

	void CC3DMatrix::translate(float x, float y, float z, CC3DMatrix* dst) const
	{
		CC3DMatrix t;
		createTranslation(x, y, z, &t);
		multiply(*this, t, dst);
	}

	void CC3DMatrix::translate(const Vector3& t)
	{
		translate(t.x, t.y, t.z, this);
	}

	void CC3DMatrix::translate(const Vector3& t, CC3DMatrix* dst) const
	{
		translate(t.x, t.y, t.z, dst);
	}

	void CC3DMatrix::transpose()
	{
		transpose(this);
	}

	void CC3DMatrix::transpose(CC3DMatrix* dst) const
	{

		CMathUtils::transposeMatrix(m, dst->m);
	}

}
#include "CC3DTransform.h"

 #include "BaseDefine/Define.h"


int CC3DTransform::_suspendTransformChanged(0);
std::vector<CC3DTransform*> CC3DTransform::_transformsChanged;

CC3DTransform::CC3DTransform()
	: _matrixDirtyBits(0), _listeners(NULL)
{
	_scale.set(Vector3(1.0f));
}

CC3DTransform::CC3DTransform(const Vector3& scale, const CC3DMath::CC3DQuaternion& rotation, const Vector3& translation)
	: _matrixDirtyBits(0), _listeners(nullptr)
{
	set(scale, rotation, translation);
}

CC3DTransform::CC3DTransform(const Vector3& scale, const CC3DMath::CC3DMatrix& rotation, const Vector3& translation)
	: _matrixDirtyBits(0), _listeners(NULL)
{
	set(scale, rotation, translation);
}

CC3DTransform::CC3DTransform(const CC3DTransform& copy)
	: _matrixDirtyBits(0), _listeners(NULL)
{
	set(copy);
}

CC3DTransform::~CC3DTransform()
{
	SAFE_DELETE(_listeners);
}

void CC3DTransform::suspendTransformChanged()
{
	_suspendTransformChanged++;
}

void CC3DTransform::resumeTransformChanged()
{
	if (_suspendTransformChanged == 0) // We haven't suspended transformChanged() calls, so do nothing.
		return;

	if (_suspendTransformChanged == 1)
	{
		// Call transformChanged() on all transforms in the list
		size_t transformCount = _transformsChanged.size();
		for (size_t i = 0; i < transformCount; i++)
		{
			CC3DTransform* t = _transformsChanged.at(i);
			t->transformChanged();
		}

		// Go through list and reset DIRTY_NOTIFY bit. The list could potentially be larger here if the 
		// transforms we were delaying calls to transformChanged() have any child nodes.
		transformCount = _transformsChanged.size();
		for (size_t i = 0; i < transformCount; i++)
		{
			CC3DTransform* t = _transformsChanged.at(i);
			t->_matrixDirtyBits &= ~DIRTY_NOTIFY;
		}

		// empty list for next frame.
		_transformsChanged.clear();
	}
	_suspendTransformChanged--;
}

bool CC3DTransform::isTransformChangedSuspended()
{
	return (_suspendTransformChanged > 0);
}

const char* CC3DTransform::getTypeName() const
{
	return "CC3DTransform";
}

const CC3DMath::CC3DMatrix& CC3DTransform::getMatrix() const
{
	if (_matrixDirtyBits & (DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE))
	{
		if (!isStatic())
		{
			bool hasScale = !_scale.isOne();
			bool hasRotation = !_rotation.isIdentity();

			// Compose the matrix in TRS order since we use column-major matrices with column vectors and
			// multiply M*v (as opposed to XNA and DirectX that use row-major matrices with row vectors and multiply v*M).
			CC3DMath::CC3DMatrix::createTranslation(_translation, &_matrix);
			if (hasRotation)
			{
				_matrix.rotate(_rotation);
			}
			if (hasScale)
			{
				_matrix.scale(_scale);
			}
		}

		_matrixDirtyBits &= ~(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
	}

	return _matrix;
}

const Vector3& CC3DTransform::getScale() const
{
	return _scale;
}

void CC3DTransform::getScale(Vector3* scale) const
{
	scale->set(_scale);
}

float CC3DTransform::getScaleX() const
{
	return _scale.x;
}

float CC3DTransform::getScaleY() const
{
	return _scale.y;
}

float CC3DTransform::getScaleZ() const
{
	return _scale.z;
}

const CC3DMath::CC3DQuaternion& CC3DTransform::getRotation() const
{
	return _rotation;
}

void CC3DTransform::getRotation(CC3DMath::CC3DQuaternion* rotation) const
{
	rotation->set(_rotation);
}

void CC3DTransform::getRotation(CC3DMath::CC3DMatrix* rotation) const
{
	CC3DMath::CC3DMatrix::createRotation(_rotation, rotation);
}

float CC3DTransform::getRotation(Vector3* axis) const
{
	return _rotation.toAxisAngle(axis);
}

const Vector3& CC3DTransform::getTranslation() const
{
	return _translation;
}

void CC3DTransform::getTranslation(Vector3* translation) const
{
	translation->set(_translation);
}

float CC3DTransform::getTranslationX() const
{
	return _translation.x;
}

float CC3DTransform::getTranslationY() const
{
	return _translation.y;
}

float CC3DTransform::getTranslationZ() const
{
	return _translation.z;
}

Vector3 CC3DTransform::getForwardVector() const
{
	Vector3 v;
	getForwardVector(&v);
	return v;
}

void CC3DTransform::getForwardVector(Vector3* dst) const
{
	getMatrix().getForwardVector(dst);
}

Vector3 CC3DTransform::getBackVector() const
{
	Vector3 v;
	getBackVector(&v);
	return v;
}

void CC3DTransform::getBackVector(Vector3* dst) const
{
	getMatrix().getBackVector(dst);
}

Vector3 CC3DTransform::getUpVector() const
{
	Vector3 v;
	getUpVector(&v);
	return v;
}

void CC3DTransform::getUpVector(Vector3* dst) const
{
	getMatrix().getUpVector(dst);
}

Vector3 CC3DTransform::getDownVector() const
{
	Vector3 v;
	getDownVector(&v);
	return v;
}

void CC3DTransform::getDownVector(Vector3* dst) const
{
	getMatrix().getDownVector(dst);
}

Vector3 CC3DTransform::getLeftVector() const
{
	Vector3 v;
	getLeftVector(&v);
	return v;
}

void CC3DTransform::getLeftVector(Vector3* dst) const
{
	getMatrix().getLeftVector(dst);
}

Vector3 CC3DTransform::getRightVector() const
{
	Vector3 v;
	getRightVector(&v);
	return v;
}

void CC3DTransform::getRightVector(Vector3* dst) const
{
	getMatrix().getRightVector(dst);
}

void CC3DTransform::rotate(float qx, float qy, float qz, float qw)
{
	if (isStatic())
		return;

	CC3DMath::CC3DQuaternion q(qx, qy, qz, qw);
	_rotation.multiply(q);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::rotate(const CC3DMath::CC3DQuaternion& rotation)
{
	if (isStatic())
		return;

	_rotation.multiply(rotation);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::rotate(const Vector3& axis, float angle)
{
	if (isStatic())
		return;

	CC3DMath::CC3DQuaternion rotationQuat;
	CC3DMath::CC3DQuaternion::createFromAxisAngle(axis, angle, &rotationQuat);
	_rotation.multiply(rotationQuat);
	_rotation.normalize();
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::rotate(const CC3DMath::CC3DMatrix& rotation)
{
	if (isStatic())
		return;

	CC3DMath::CC3DQuaternion rotationQuat;
	CC3DMath::CC3DQuaternion::createFromRotationMatrix(rotation, &rotationQuat);
	_rotation.multiply(rotationQuat);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::rotateX(float angle)
{
	if (isStatic())
		return;

	CC3DMath::CC3DQuaternion rotationQuat;
	CC3DMath::CC3DQuaternion::createFromAxisAngle(Vector3(1.0f, 0.0f, 0.0f), angle, &rotationQuat);
	_rotation.multiply(rotationQuat);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::rotateY(float angle)
{
	if (isStatic())
		return;

	CC3DMath::CC3DQuaternion rotationQuat;
	CC3DMath::CC3DQuaternion::createFromAxisAngle(Vector3(0.0f, 1.0f, 0.0f), angle, &rotationQuat);
	_rotation.multiply(rotationQuat);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::rotateZ(float angle)
{
	if (isStatic())
		return;

	CC3DMath::CC3DQuaternion rotationQuat;
	CC3DMath::CC3DQuaternion::createFromAxisAngle(Vector3(0.0f, 0.0f, 1.0f), angle, &rotationQuat);
	_rotation.multiply(rotationQuat);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::scale(float scale)
{
	if (isStatic())
		return;

	_scale = _scale * scale;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::scale(float sx, float sy, float sz)
{
	if (isStatic())
		return;

	_scale.x *= sx;
	_scale.y *= sy;
	_scale.z *= sz;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::scale(const Vector3& scale)
{
	if (isStatic())
		return;

	_scale.x *= scale.x;
	_scale.y *= scale.y;
	_scale.z *= scale.z;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::scaleX(float sx)
{
	if (isStatic())
		return;

	_scale.x *= sx;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::scaleY(float sy)
{
	if (isStatic())
		return;

	_scale.y *= sy;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::scaleZ(float sz)
{
	if (isStatic())
		return;

	_scale.z *= sz;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::set(const Vector3& scale, const CC3DMath::CC3DQuaternion& rotation, const Vector3& translation)
{
	if (isStatic())
		return;

	_scale.set(scale);
	_rotation.set(rotation);
	_translation.set(translation);
	dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
}

void CC3DTransform::set(const Vector3& scale, const CC3DMath::CC3DMatrix& rotation, const Vector3& translation)
{
	if (isStatic())
		return;

	_scale.set(scale);
	CC3DMath::CC3DQuaternion rotationQuat;
	CC3DMath::CC3DQuaternion::createFromRotationMatrix(rotation, &rotationQuat);
	_rotation.set(rotationQuat);
	_translation.set(translation);
	dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
}

void CC3DTransform::set(const Vector3& scale, const Vector3& axis, float angle, const Vector3& translation)
{
	if (isStatic())
		return;

	_scale.set(scale);
	_rotation.set(axis, angle);
	_translation.set(translation);
	dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
}

void CC3DTransform::set(const CC3DTransform& transform)
{
	if (isStatic())
		return;

	_scale.set(transform._scale);
	_rotation.set(transform._rotation);
	_translation.set(transform._translation);
	dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
}

void CC3DTransform::setIdentity()
{
	if (isStatic())
		return;

	_scale.set(1.0f, 1.0f, 1.0f);
	_rotation.setIdentity();
	_translation.set(0.0f, 0.0f, 0.0f);
	dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
}

void CC3DTransform::setScale(float scale)
{
	if (isStatic())
		return;

	_scale.set(scale, scale, scale);
	dirty(DIRTY_SCALE);
}

void CC3DTransform::setScale(float sx, float sy, float sz)
{
	if (isStatic())
		return;

	_scale.set(sx, sy, sz);
	dirty(DIRTY_SCALE);
}

void CC3DTransform::setScale(const Vector3& scale)
{
	_scale.set(scale);
	dirty(DIRTY_SCALE);
}

void CC3DTransform::setScaleX(float sx)
{
	if (isStatic())
		return;

	_scale.x = sx;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::setScaleY(float sy)
{
	if (isStatic())
		return;

	_scale.y = sy;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::setScaleZ(float sz)
{
	if (isStatic())
		return;

	_scale.z = sz;
	dirty(DIRTY_SCALE);
}

void CC3DTransform::setRotation(const CC3DMath::CC3DQuaternion& rotation)
{
	if (isStatic())
		return;

	_rotation.set(rotation);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::setRotation(float qx, float qy, float qz, float qw)
{
	if (isStatic())
		return;

	_rotation.set(qx, qy, qz, qw);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::setRotation(const CC3DMath::CC3DMatrix& rotation)
{
	if (isStatic())
		return;

	CC3DMath::CC3DQuaternion rotationQuat;
	CC3DMath::CC3DQuaternion::createFromRotationMatrix(rotation, &rotationQuat);
	_rotation.set(rotationQuat);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::setRotation(const Vector3& axis, float angle)
{
	if (isStatic())
		return;

	_rotation.set(axis, angle);
	dirty(DIRTY_ROTATION);
}

void CC3DTransform::setTranslation(const Vector3& translation)
{
	if (isStatic())
		return;

	_translation.set(translation);
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::setTranslation(float tx, float ty, float tz)
{
	if (isStatic())
		return;

	_translation.set(tx, ty, tz);
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::setTranslationX(float tx)
{
	if (isStatic())
		return;

	_translation.x = tx;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::setTranslationY(float ty)
{
	if (isStatic())
		return;

	_translation.y = ty;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::setTranslationZ(float tz)
{
	if (isStatic())
		return;

	_translation.z = tz;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::translate(float tx, float ty, float tz)
{
	if (isStatic())
		return;

	_translation.x += tx;
	_translation.y += ty;
	_translation.z += tz;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::translate(const Vector3& translation)
{
	if (isStatic())
		return;

	_translation.x += translation.x;
	_translation.y += translation.y;
	_translation.z += translation.z;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::translateX(float tx)
{
	if (isStatic())
		return;

	_translation.x += tx;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::translateY(float ty)
{
	if (isStatic())
		return;

	_translation.y += ty;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::translateZ(float tz)
{
	if (isStatic())
		return;

	_translation.z += tz;
	dirty(DIRTY_TRANSLATION);
}

void CC3DTransform::translateLeft(float amount)
{
	if (isStatic())
		return;

	// Force the current transform matrix to be updated.
	getMatrix();

	Vector3 left;
	_matrix.getLeftVector(&left);
	left.normalize();
	left = left * amount;

	translate(left);
}

void CC3DTransform::translateUp(float amount)
{
	if (isStatic())
		return;

	// Force the current transform matrix to be updated.
	getMatrix();

	Vector3 up;
	_matrix.getUpVector(&up);
	up.normalize();
	up = up * amount;

	translate(up);
}

void CC3DTransform::translateForward(float amount)
{
	if (isStatic())
		return;

	// Force the current transform matrix to be updated.
	getMatrix();

	Vector3 forward;
	_matrix.getForwardVector(&forward);
	forward.normalize();
	forward = forward * amount;

	translate(forward);
}

void CC3DTransform::translateSmooth(const Vector3& target, float elapsedTime, float responseTime)
{
	if (isStatic())
		return;

	if (elapsedTime > 0)
	{
		_translation += (target - _translation) * (elapsedTime / (elapsedTime + responseTime));
		dirty(DIRTY_TRANSLATION);
	}
}

void CC3DTransform::transformPoint(Vector3* point)
{
	getMatrix();
	_matrix.transformPoint(point);
}

void CC3DTransform::transformPoint(const Vector3& point, Vector3* dst)
{
	getMatrix();
	_matrix.transformPoint(point, dst);
}

void CC3DTransform::transformVector(Vector3* normal)
{
	getMatrix();
	_matrix.transformVector(normal);
}

void CC3DTransform::transformVector(const Vector3& normal, Vector3* dst)
{
	getMatrix();
	_matrix.transformVector(normal, dst);
}

void CC3DTransform::transformVector(float x, float y, float z, float w, Vector3* dst)
{
	getMatrix();
	_matrix.transformVector(x, y, z, w, dst);
}

bool CC3DTransform::isStatic() const
{
	return false;
}

unsigned int CC3DTransform::getAnimationPropertyComponentCount(int propertyId) const
{
	switch (propertyId)
	{
	case ANIMATE_SCALE_UNIT:
	case ANIMATE_SCALE_X:
	case ANIMATE_SCALE_Y:
	case ANIMATE_SCALE_Z:
	case ANIMATE_TRANSLATE_X:
	case ANIMATE_TRANSLATE_Y:
	case ANIMATE_TRANSLATE_Z:
		return 1;
	case ANIMATE_SCALE:
	case ANIMATE_TRANSLATE:
		return 3;
	case ANIMATE_ROTATE:
		return 4;
	case ANIMATE_SCALE_TRANSLATE:
		return 6;
	case ANIMATE_ROTATE_TRANSLATE:
	case ANIMATE_SCALE_ROTATE:
		return 7;
	case ANIMATE_SCALE_ROTATE_TRANSLATE:
		return 10;
	default:
		return -1;
	}
}





void CC3DTransform::dirty(char matrixDirtyBits)
{
	_matrixDirtyBits |= matrixDirtyBits;
	if (isTransformChangedSuspended())
	{
		if (!isDirty(DIRTY_NOTIFY))
		{
			//suspendTransformChange(this);
		}
	}
	else
	{
		transformChanged();
	}
}

bool CC3DTransform::isDirty(char matrixDirtyBits) const
{
	return (_matrixDirtyBits & matrixDirtyBits) == matrixDirtyBits;
}



void CC3DTransform::addListener(CC3DTransform::Listener* listener, long cookie)
{
	if (_listeners == NULL)
		_listeners = new std::list<TransformListener>();

	TransformListener l;
	l.listener = listener;
	l.cookie = cookie;
	_listeners->push_back(l);
}

void CC3DTransform::removeListener(CC3DTransform::Listener* listener)
{

	if (_listeners)
	{
		for (std::list<TransformListener>::iterator itr = _listeners->begin(); itr != _listeners->end(); ++itr)
		{
			if ((*itr).listener == listener)
			{
				_listeners->erase(itr);
				break;
			}
		}
	}
}

void CC3DTransform::transformChanged()
{
	if (_listeners)
	{
		for (std::list<TransformListener>::iterator itr = _listeners->begin(); itr != _listeners->end(); ++itr)
		{
			TransformListener& l = *itr;
			l.listener->transformChanged(this, l.cookie);
		}
	}
}

//void CC3DTransform::cloneInto(CC3DTransform* transform, NodeCloneContext &context) const
//{
//	GP_ASSERT(transform);
//
//	AnimationTarget::cloneInto(transform, context);
//	transform->_scale.set(_scale);
//	transform->_rotation.set(_rotation);
//	transform->_translation.set(_translation);
//	transform->dirty(DIRTY_TRANSLATION | DIRTY_ROTATION | DIRTY_SCALE);
//}


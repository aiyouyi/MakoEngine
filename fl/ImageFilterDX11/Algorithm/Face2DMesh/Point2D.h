#ifndef POINT_2D_H
#define POINT_2D_H
#include<cmath>

/**
 * @brief 定义2D点类型
 */
class Point2D
{
public:
	/**
	 * @brief 默认构造函数
	 */
	Point2D() : x(0), y(0) {};
	/**
	 * @brief 构造函数
	 * @param [in] x 点x轴维度的值
	 * @param [in] y 点y轴维度的值
	 */
	Point2D(float x, float y) : x(x), y(y) {};
	/**
	 * @brief 设置点的值
	 * @param [in] x 点x轴维度的值
	 * @param [in] y 点y轴维度的值
	 * @return
	 */
	void set(float x, float y);
    /**
     * @brief 计算两点构成向量的欧式距离
     * @return  两点构成向量的欧式距离
     */
	inline float length() const;
	/**
	 * @brief 计算两点坐标之间的欧式距离
	 * @param [in] vec 另一个点坐标
	 * @return  两点坐标之间的欧式距离
	 *///
	float distance(const Point2D& vec) const;
	/**
	 * @brief 两点坐标构成的向量归一化
	 * @return  向量归一化坐标
	 */
	Point2D&    normalize();
	/**
	 * @brief 两点坐标内积
	 * @param [in] vec 另一点坐标
	 * @return 两点坐标内积值
	 */
	float dot(const Point2D& vec) const;          // dot product

	// operators
	/**
	 * @brief 重载取反运算符\f$-\f$
	 * @return 取反后点坐标
	 */
	Point2D     operator-() const;
	/**
	 * @brief 重载加法运算法符\f$+\f$
	 * @param [in] rhs 点坐标
	 * @return 两点相加后的值
	 */
	Point2D     operator+(const Point2D& rhs) const;
	/**
	 * @brief 重载减法运算法符\f$-\f$
	 * @param [in] rhs 点坐标
	 * @return 两点相减后的值
	 */
	Point2D     operator-(const Point2D& rhs) const;
	/**
	 * @brief 重载加且赋值运算法符\f$+=\f$
	 * @param [in] rhs 点坐标
	 * @return 两点相加后的值
	 */
	Point2D&    operator+=(const Point2D& rhs);
	/**
	 * @brief 重载减且赋值运算法符\f$-=\f$
	 * @param [in] rhs 点坐标
	 * @return 两点相减后的向量值
	 */
	Point2D&  operator-=(const Point2D& rhs);
	/**
	 * @brief 重载乘法运算法符\f$*\f$
	 * @param [in] scale 标量值
	 * @return 点与标量相乘的向量值
	 */
	Point2D     operator*(const float scale) const;
	/**
	 * @brief 重载乘法运算法符\f$*\f$
	 * @param [in] rhs 点坐标
	 * @return 点与标量相乘的向量值
	 */
	Point2D     operator*(const Point2D& rhs) const;
	/**
	 * @brief 重载乘且赋值运算法符\f$*=\f$
	 * @param [in] scale 标量值
	 * @return 点与标量相乘的向量值
	 */
	Point2D&    operator*=(const float scale);
	/**
    * @brief 重载乘且赋值运算法符\f$*=\f$
    * @param [in] rhs 点坐标
    * @return 点与标量相乘的向量值
    */
	Point2D&    operator*=(const Point2D& rhs);
	/**
	 * @brief 重载除法运算法符\f$/\f$
	 * @param [in] scale 标量值
	 * @return 点与标量相除的向量值
	 */
	Point2D     operator/(const float scale) const;     // inverse scale
	/**
	 * @brief 重载除且赋值运算法符\f$/=\f$
	 * @param [in] scale 标量值
	 * @return 点与标量相除的向量值
	 */
	Point2D& operator/=(const float scale);
	/**
	 * @brief 重载关系运算符\f$==\F$
	 * @param [in] rhs 点坐标
	 * @return 两点坐标相等返回true，否则返回false
	 */
	bool operator==(const Point2D& rhs) const;
	/**
	 * @brief 重载关系运算符\f$!=\f$
	 * @param [in] rhs 点坐标
	 * @return 两点坐标不等返回true，否则返回false
	 */
	bool operator!=(const Point2D& rhs) const;
	/**
	 * @brief 重载关系运算符\f$<\f$
	 * @param rhs [in] 点坐标
	 * @return 对象点坐标小返回true，否则返回false
	 */
	bool operator<(const Point2D& rhs) const;
	/**
	 * 重载乘法运算符\f$*\f$
	 * @param a 标量值
	 * @param vec 点坐标
	 * @return 点与标量相乘的向量值
	 */
	friend Point2D operator*(const float a, const Point2D vec);
public:
	float x;
	float y;
};//class Point2D

////////////////////////////////////////////////
//class Point2D until
inline Point2D Point2D::operator-() const {
	return Point2D(-x, -y);
}

inline Point2D Point2D::operator+(const Point2D& rhs) const {
	return Point2D(x + rhs.x, y + rhs.y);
}

inline Point2D Point2D::operator-(const Point2D& rhs) const {
	return Point2D(x - rhs.x, y - rhs.y);
}

inline Point2D& Point2D::operator+=(const Point2D& rhs) {
	x += rhs.x; y += rhs.y; return *this;
}

inline Point2D& Point2D::operator-=(const Point2D& rhs) {
	x -= rhs.x; y -= rhs.y; return *this;
}

inline Point2D Point2D::operator*(const float a) const {
	return Point2D(x*a, y*a);
}

inline Point2D Point2D::operator*(const Point2D& rhs) const {
	return Point2D(x*rhs.x, y*rhs.y);
}

inline Point2D& Point2D::operator*=(const float a) {
	x *= a; y *= a; return *this;
}

inline Point2D& Point2D::operator*=(const Point2D& rhs) {
	x *= rhs.x; y *= rhs.y; return *this;
}

inline Point2D Point2D::operator/(const float a) const {
	return Point2D(x / a, y / a);
}

inline Point2D& Point2D::operator/=(const float a) {
	x /= a; y /= a; return *this;
}

inline bool Point2D::operator==(const Point2D& rhs) const {
	return (x == rhs.x) && (y == rhs.y);
}

inline bool Point2D::operator!=(const Point2D& rhs) const {
	return (x != rhs.x) || (y != rhs.y);
}

inline bool Point2D::operator<(const Point2D& rhs) const {
	if (x < rhs.x) return true;
	if (x > rhs.x) return false;
	if (y < rhs.y) return true;
	if (y > rhs.y) return false;
	return false;
}


inline void Point2D::set(float x, float y) {
	this->x = x; this->y = y;
}

inline float Point2D::length() const {
	return sqrtf(x*x + y*y);
}

inline float Point2D::distance(const Point2D& vec) const {
	return sqrtf((vec.x - x)*(vec.x - x) + (vec.y - y)*(vec.y - y));
}

inline Point2D& Point2D::normalize()
{

	float xxyy = x*x + y*y;
	float invLength = 1.0f / sqrtf(xxyy);
	x *= invLength;
	y *= invLength;
	return *this;
}

inline float Point2D::dot(const Point2D& rhs) const {
	return (x*rhs.x + y*rhs.y);
}


inline Point2D operator*(const float a, const Point2D vec) {
	return Point2D(a*vec.x, a*vec.y);
}
#endif 


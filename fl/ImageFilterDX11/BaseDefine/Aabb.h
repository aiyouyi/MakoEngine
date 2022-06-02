#ifndef _CC_AABB_H_
#define _CC_AABB_H_

#include <limits>
#include "BaseDefine/Vectors.h"
#include "glm/detail/type_vec3.hpp"
#include <algorithm>

/**
 * An axis aligned box represented by its min and max coordinates
 */

struct CC3DAabb
{
	using float3 = glm::vec3;

	/** min coordinates */
	float3 min = glm::vec3((std::numeric_limits<float>::max)());

	/** max coordinates */
	float3 max = glm::vec3(std::numeric_limits<float>::lowest());

	/**
	 * Computes the center of the box.
	 * @return (max + min)/2
	 */
	float3 center() const noexcept {
		// float3 ctor needed for visual studio
		return (max + min) * float3(0.5f);
	}

	/**
	 * Computes the half-extent of the box.
	 * @return (max - min)/2
	 */
	float3 extent() const noexcept {
		// float3 ctor needed for visual studio
		return (max - min) * float3(0.5f);
	}

	/**
	 * Whether the box is empty, i.e.: it's volume is null or negative.
	 * @return true if min >= max, i.e: the volume of the box is null or negative
	 */
	bool isEmpty() const noexcept {
		//return any(greaterThanEqual(min, max));
		return !(max.x >= min.x && max.y >= min.y && max.z >= min.z);
	}

	struct Corners {
		using value_type = float3;
		value_type const* begin() const { return vertices; }
		value_type const* end() const { return vertices + 8; }
		value_type * begin() { return vertices; }
		value_type * end() { return vertices + 8; }
		value_type const* data() const { return vertices; }
		value_type * data() { return vertices; }
		size_t size() const { return 8; }
		value_type vertices[8];
	};

	/**
	 * Returns the 8 corner vertices of the AABB.
	 */
	Corners getCorners() const
	{
		CC3DAabb::Corners cor;
		cor.vertices[0] = { min.x, min.y, min.z };
		cor.vertices[1] = { max.x, min.y, min.z };
		cor.vertices[2] = { min.x, max.y, min.z };
		cor.vertices[3] = { max.x, max.y, min.z };
		cor.vertices[4] = { min.x, min.y, max.z };
		cor.vertices[5] = { max.x, min.y, max.z };
		cor.vertices[6] = { min.x, max.y, max.z };
		cor.vertices[7] = { max.x, max.y, max.z };

		return cor;

		//return Aabb::Corners{ .vertices = {
		//	{ min.x, min.y, min.z },
		//	{ max.x, min.y, min.z },
		//	{ min.x, max.y, min.z },
		//	{ max.x, max.y, min.z },
		//	{ min.x, min.y, max.z },
		//	{ max.x, min.y, max.z },
		//	{ min.x, max.y, max.z },
		//	{ max.x, max.y, max.z },
		//} };
	}

	/**
	 * Returns whether the box contains a given point.
	 *
	 * @param p the point to test
	 * @return the maximum signed distance to the box. Negative if p is in the box
	 */
	float contains(float3 p) const noexcept
	{
		float d = min.x - p.x;
		d = (std::max)(d, min.y - p.y);
		d = (std::max)(d, min.z - p.z);
		d = (std::max)(d, p.x - max.x);
		d = (std::max)(d, p.y - max.y);
		d = (std::max)(d, p.z - max.z);
		return d;
	}

	/**
	 * Applies an affine transformation to the AABB.
	 *
	 * @param m the 4x4 transformation to apply
	 * @return the transformed box
	 */
	//Aabb transform(const math::mat4f& m) const noexcept;
};

#endif
#pragma once
#include "Vector3f.h"
#include <math.h>

namespace X
{
	/// \brief A ray in 3D space
	/// \todo Check everything in this class
	class CRay
	{
	public:
		
		/// \brief Default constructor
		CRay()
		{
			mvOrigin.set(0, 0, 0);
			mvDirection.set(0, 0, 1);
		}

		/// \brief Constructor
		///
		/// \param vOrigin The origin of the ray
		/// \param vDirection The direction of the ray
		CRay(const CVector3f& vOrigin, const CVector3f& vDirection)
		{
			mvOrigin = vOrigin;
			mvDirection = vDirection;
		}

		/// \brief Get the point on the ray at the given distance from the origin
		///
		/// \param fDistance The distance from the origin
		/// \return The point on the ray at the given distance from the origin
		CVector3f getPointAtDistance(float fDistance) const
		{
			return mvOrigin + (mvDirection * fDistance);
		}

		/// \brief Check if the ray intersects with a sphere
		///
		/// \param sphereCenter The center of the sphere
		/// \param sphereRadius The radius of the sphere
		/// \param intersectionPoint The point of intersection (if any)
		/// \return True if the ray intersects the sphere, false otherwise
		bool intersectsSphere(const CVector3f& sphereCenter, float sphereRadius, CVector3f& intersectionPoint) const
		{
			CVector3f oc = mvOrigin - sphereCenter;
			float a = mvDirection.getDot(mvDirection);
			float b = 2.0f * oc.getDot(mvDirection);
			float c = oc.getDot(oc) - sphereRadius * sphereRadius;
			float discriminant = b * b - 4 * a * c;
			if (discriminant > 0)
			{
				float t = float(-b - sqrt(discriminant)) / (2.0f * a);
				if (t >= 0)
				{
					intersectionPoint = getPointAtDistance(t);
					return true;
				}
			}
			return false;
		}

		CVector3f mvOrigin;		///< Origin of the ray
		CVector3f mvDirection;	///< Direction of the ray
	};
}
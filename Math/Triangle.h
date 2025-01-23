#pragma once
#include "Vector3f.h"
#include "Ray.h"

namespace X
{
    /// \brief A triangle in 3D space
    /// \todo Check everything in this class
    class CTriangle
    {
    public:
        /// \brief Default constructor
        CTriangle()
        {
            mvVertex0.set(0, 0, 0);
            mvVertex1.set(0, 0, 0);
            mvVertex2.set(0, 0, 0);
        }

        /// \brief Constructor
        ///
        /// \param v0 The first vertex of the triangle
        /// \param v1 The second vertex of the triangle
        /// \param v2 The third vertex of the triangle
        CTriangle(const CVector3f& v0, const CVector3f& v1, const CVector3f& v2)
        {
            mvVertex0 = v0;
            mvVertex1 = v1;
            mvVertex2 = v2;
        }

        /// \brief Calculate the area of the triangle
        ///
        /// \return The area of the triangle
        float getArea() const
        {
            CVector3f edge1 = mvVertex1 - mvVertex0;
            CVector3f edge2 = mvVertex2 - mvVertex0;
            return 0.5f * edge1.getCross(edge2).getMagnitude();
        }

        /// \brief Calculate the normal of the triangle
        ///
        /// \return The normal of the triangle
        CVector3f getNormal() const
        {
            CVector3f edge1 = mvVertex1 - mvVertex0;
            CVector3f edge2 = mvVertex2 - mvVertex0;
            return edge1.getCross(edge2).normalise();
        }

        /// \brief Check if the triangle intersects with a ray
        ///
        /// \param ray The ray to check for intersection
        /// \param intersectionPoint The point of intersection (if any)
        /// \return True if the ray intersects the triangle, false otherwise
        bool intersectsRay(const CRay& ray, CVector3f& intersectionPoint) const
        {
            const float EPSILON = 1e-6f;
            CVector3f edge1 = mvVertex1 - mvVertex0;
            CVector3f edge2 = mvVertex2 - mvVertex0;
            CVector3f h = ray.mvDirection.getCross(edge2);
            float a = edge1.getDot(h);
            if (a > -EPSILON && a < EPSILON)
                return false; // This ray is parallel to this triangle.

            float f = 1.0f / a;
            CVector3f s = ray.mvOrigin - mvVertex0;
            float u = f * s.getDot(h);
            if (u < 0.0f || u > 1.0f)
                return false;

            CVector3f q = s.getCross(edge1);
            float v = f * ray.mvDirection.getDot(q);
            if (v < 0.0f || u + v > 1.0f)
                return false;

            // At this stage we can compute t to find out where the intersection point is on the line.
            float t = f * edge2.getDot(q);
            if (t > EPSILON) // ray intersection
            {
                intersectionPoint = ray.getPointAtDistance(t);
                return true;
            }
            else // This means that there is a line intersection but not a ray intersection.
                return false;
        }

        CVector3f mvVertex0; ///< First vertex of the triangle
        CVector3f mvVertex1; ///< Second vertex of the triangle
        CVector3f mvVertex2; ///< Third vertex of the triangle
    };
}
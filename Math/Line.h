#pragma once
#include "Vector3f.h"
#include <math.h>

namespace X
{
    /// \brief A line segment in 3D space
    /// \todo Check everything in this class
    class CLine3D
    {
    public:
        /// \brief Default constructor
        CLine3D()
        {
            mvStart.set(0, 0, 0);
            mvEnd.set(0, 0, 0);
        }

        /// \brief Constructor
        ///
        /// \param vStart The start point of the line segment
        /// \param vEnd The end point of the line segment
        CLine3D(const CVector3f& vStart, const CVector3f& vEnd)
        {
            mvStart = vStart;
            mvEnd = vEnd;
        }

        /// \brief Calculate the length of the line segment
        ///
        /// \return The length of the line segment
        float getLength() const
        {
            return mvStart.getDistance(mvEnd);
        }

        /// \brief Calculate the midpoint of the line segment
        ///
        /// \return The midpoint of the line segment
        CVector3f getMidpoint() const
        {
            return (mvStart + mvEnd) * 0.5f;
        }

        /// \brief Check if the line segment intersects with another line segment
        ///
        /// \param other The other line segment to check for intersection
        /// \param intersectionPoint The point of intersection (if any)
        /// \return True if the line segments intersect, false otherwise
        bool intersectsLine(const CLine3D& other, CVector3f& intersectionPoint) const
        {
            CVector3f p1 = mvStart;
            CVector3f p2 = mvEnd;
            CVector3f p3 = other.mvStart;
            CVector3f p4 = other.mvEnd;

            CVector3f p13 = p1 - p3;
            CVector3f p43 = p4 - p3;

            if (p43.getMagnitude() < 1e-6)
                return false;

            CVector3f p21 = p2 - p1;

            if (p21.getMagnitude() < 1e-6)
                return false;

            float d1343 = p13.getDot(p43);
            float d4321 = p43.getDot(p21);
            float d1321 = p13.getDot(p21);
            float d4343 = p43.getDot(p43);
            float d2121 = p21.getDot(p21);

            float denom = d2121 * d4343 - d4321 * d4321;
            if (fabs(denom) < 1e-6)
                return false;

            float numer = d1343 * d4321 - d1321 * d4343;

            float mua = numer / denom;
            float mub = (d1343 + d4321 * mua) / d4343;

            CVector3f pa = p1 + p21 * mua;
            CVector3f pb = p3 + p43 * mub;

            if (pa.getDistance(pb) < 1e-6)
            {
                intersectionPoint = pa;
                return true;
            }

            return false;
        }

        CVector3f mvStart; ///< Start point of the line segment
        CVector3f mvEnd;   ///< End point of the line segment
    };
}
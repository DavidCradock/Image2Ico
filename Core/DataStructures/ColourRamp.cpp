#include "ColourRamp.h"
#include "../StringUtils.h"
#include "../Utilities.h"
#include "../Exceptions.h"

namespace X
{
	CColourRamp::CColourRamp()
	{
		setupColourRamp();
	}

	unsigned int CColourRamp::getNumberOfPoints(void)
	{
		return (unsigned int)_mlistPoints.size();
	}

	void CColourRamp::addPoint(float fPointPosition, CColourf pointColour)
	{
		clamp(fPointPosition, 0.0f, 1.0f);

		SPoint newPoint;
		newPoint.colour = pointColour;
		newPoint.fPosition = fPointPosition;

		auto it = _mlistPoints.begin();
		while (it != _mlistPoints.end() && it->fPosition < fPointPosition)
		{
			++it;
		}
		_mlistPoints.insert(it, newPoint);
	}

	void CColourRamp::removePoint(unsigned int uiPointIndex)
	{
		ThrowIfTrue(uiPointIndex >= _mlistPoints.size(), "Invalid point index given.");
		auto it = _mlistPoints.begin();
		std::advance(it, uiPointIndex);
		_mlistPoints.erase(it);
	}

	void CColourRamp::removeAllPoints(void)
	{
		_mlistPoints.clear();
	}

	void CColourRamp::modifyPointColour(unsigned int uiPointIndex, CColourf& newColour)
	{
		ThrowIfTrue(uiPointIndex >= _mlistPoints.size(), "Invalid point index given.");
		auto it = _mlistPoints.begin();
		std::advance(it, uiPointIndex);
		it->colour = newColour;
	}

	void CColourRamp::modifyPointPosition(unsigned int uiPointIndex, float fPointNewPosition)
	{
		ThrowIfTrue(uiPointIndex >= _mlistPoints.size(), "Invalid point index given.");
		clamp(fPointNewPosition, 0.0f, 1.0f);

		auto it = _mlistPoints.begin();
		std::advance(it, uiPointIndex);
		it->fPosition = fPointNewPosition;

		// Re-sort the list if necessary
		_mlistPoints.sort([](const SPoint& a, const SPoint& b) { return a.fPosition < b.fPosition; });
	}

	CColourRamp::SPoint* CColourRamp::getPoint(unsigned int uiPointIndex)
	{
		ThrowIfTrue(uiPointIndex >= _mlistPoints.size(), "Invalid point index given.");
		auto it = _mlistPoints.begin();
		std::advance(it, uiPointIndex);
		return &(*it);
	}

	float CColourRamp::getPointPosition(unsigned int uiPointIndex)
	{
		ThrowIfTrue(uiPointIndex >= _mlistPoints.size(), "Invalid point index given.");
		auto it = _mlistPoints.begin();
		std::advance(it, uiPointIndex);
		return it->fPosition;
	}

	CColourf CColourRamp::getPointColour(unsigned int uiPointIndex)
	{
		ThrowIfTrue(uiPointIndex >= _mlistPoints.size(), "Invalid point index given.");
		auto it = _mlistPoints.begin();
		std::advance(it, uiPointIndex);
		return it->colour;
	}

	/// \brief Given a position from 0.0f to 1.0f along the colour ramp, returns the interpolated colour at that position.
	///
	/// \param fRampPosition Position along the ramp range from 0.0f to 1.0f
	/// \return A CColourf object holding the interpolated colour
	CColourf CColourRamp::getRampColour(float fRampPosition)
	{
		clamp(fRampPosition, 0.0f, 1.0f);

		if (_mlistPoints.size() == 0)
		{
			return CColourf(1.0f, 1.0f, 1.0f, 1.0f); // Return white if no points
		}

		// Find the two closest points
		unsigned int uiPointIndexLeft, uiPointIndexRight;
		getAdjacentPointIndicies(fRampPosition, uiPointIndexLeft, uiPointIndexRight);

		auto it = _mlistPoints.begin();
		std::advance(it, uiPointIndexLeft);
		const SPoint& pointLeft = *it;
		std::advance(it, 1);
		const SPoint& pointRight = *it;

		// Linear interpolation between the two points
		float t = (fRampPosition - pointLeft.fPosition) / (pointRight.fPosition - pointLeft.fPosition);
		//return pointLeft.colour * (1.0f - t) + pointRight.colour * t;
		return pointLeft.colour.interpolate(pointRight.colour, t);
	}

	void CColourRamp::getAdjacentPointIndicies(float fRampPosition, unsigned int& uiPointIndexLeft, unsigned int& uiPointIndexRight)
	{
		ThrowIfTrue(_mlistPoints.size() < 2, "Less than two points in the ramp");

		// Find the insertion point
		auto it = _mlistPoints.begin();
		while (it != _mlistPoints.end() && it->fPosition < fRampPosition)
		{
			++it;
		}
		uiPointIndexRight = (unsigned int)std::distance(_mlistPoints.begin(), it);
		uiPointIndexLeft = uiPointIndexRight - 1;
		if (uiPointIndexRight == 0)
		{
			uiPointIndexRight++;
			uiPointIndexLeft = 0;
		}
		else if (uiPointIndexRight >= _mlistPoints.size())
		{
			uiPointIndexRight = (unsigned int)_mlistPoints.size() - 1;
			uiPointIndexLeft = uiPointIndexRight - 1;
		}
		
	}

	void CColourRamp::setupColourRamp(CColourf colourLeftEdge, CColourf colourRightEdge)
	{
		_mlistPoints.clear();
		SPoint pointLeft, pointRight;
		pointLeft.colour = colourLeftEdge;
		pointLeft.fPosition = 0.0f;
		pointRight.colour = colourRightEdge;
		pointRight.fPosition = 1.0f;
		_mlistPoints.push_back(pointLeft);
		_mlistPoints.push_back(pointRight);
	}

	void CColourRamp::setupColourRampFire(void)
	{
		_mlistPoints.clear();
		SPoint point;
		point.fPosition = 0.0f;	point.colour.set(1.0f, 0.0f, 0.0f, 1.0f);	_mlistPoints.push_back(point);
		point.fPosition = 0.5f;	point.colour.set(1.0f, 1.0f, 0.0f, 1.0f);	_mlistPoints.push_back(point);
		point.fPosition = 1.0f; point.colour.set(0.0f, 0.0f, 0.0f, 1.0f);	_mlistPoints.push_back(point);
	}

	void CColourRamp::setupColourRampRGB(void)
	{
		_mlistPoints.clear();
		SPoint point;
		point.fPosition = 0.0f;	point.colour.set(1.0f, 0.0f, 0.0f, 1.0f);	_mlistPoints.push_back(point);
		point.fPosition = 0.5f;	point.colour.set(0.0f, 1.0f, 0.0f, 1.0f);	_mlistPoints.push_back(point);
		point.fPosition = 1.0f; point.colour.set(0.0f, 0.0f, 1.0f, 1.0f);	_mlistPoints.push_back(point);
	}

	void CColourRamp::saveAsSetupCode(const std::string& strFilename)
	{
		std::ofstream outfile(strFilename);
		if (!outfile.is_open())
			return;

		outfile << "CColourRamp ramp;\n";
		outfile << "ramp.removeAllPoints();\n";

		auto it = _mlistPoints.begin();
		while (it != _mlistPoints.end())
		{
			// Write out a line which looks like "ramp.addPoint(0.0f, CColourf(0.0f, 0.0f, 0.0f, 0.0f));\n"
			std::string str = "ramp.addPoint(";
			StringUtils::appendFloat(str, it->fPosition, 2);
			str.append("f, CColourf(");
			StringUtils::appendFloat(str, it->colour.red, 2);
			str.append("f, ");
			StringUtils::appendFloat(str, it->colour.green, 2);
			str.append("f, ");
			StringUtils::appendFloat(str, it->colour.blue, 2);
			str.append("f, ");
			StringUtils::appendFloat(str, it->colour.alpha, 2);
			str.append("f));\n");
			outfile << str;
			++it;
		}
		outfile.close(); // Close the file
	}
}
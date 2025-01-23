#pragma once
#include "Colourf.h"
#include <list>
#include <string>

namespace X
{
	/// \brief An object for holding two or more colours along a linear axis and interpolating between those colours given a position along that axis.
	///
	/// Upon construction, a gradient containing 2 points are set from black to white
	/// Imagine a smooth transition from a deep, ocean blue to a vibrant, sunlit yellow.
	/// The blue starts off dark and mysterious, gradually lightening as it moves towards the yellow.
	/// The yellow begins as a soft, buttery hue and intensifies into a bright, almost blinding gold.
	/// The two colors blend seamlessly, creating a sense of movement and energy.
	/// This is a simple example of a color ramp.
	/// 
	/// This is similar to Blender's colour ramps.
	/// Imagine a line going from left to right, with two colour "points", one located at the left edge position (0.0f) being black and
	/// another colour "point" being located on the right edge position (1.0f) being white.
	/// We can retrieve the interpolated colour between these two points in the middle by calling getRampColour(0.5f) and this would return a mid-grey colour.
	/// More than two points may be added, as many as needed to enable us to create complex colour gradients.
	/// 
	/// \code
	/// CColourRamp ramp;	// Create a colour ramp with 2 points, one at each end of the ramp, black at 0.0f ramp position and white at 1.0f ramp position
	/// ramp.addPoint(0.5f, CColourf(1.0f, 0.0f, 0.0f, 1.0f));	// Add a red colour point in the middle of the ramp
	/// CColourf interpolatedColour = ramp.getRampColour(0.25f);	// Get the ramp's interpolated colour a quarter from the left.
	/// \endcode
	class CColourRamp
	{
	public:
		/// \brief A colour point within the ramp.
		struct SPoint
		{
			CColourf colour;
			float fPosition;
		};

		/// \brief Constructor.
		/// Sets the colour ramp to have two initial colours, one at each end. The one at the start is black and the one at the end is white.
		CColourRamp();

		/// \brief Returns the current number of colour points within the colour ramp
		///
		/// \return The number of colour points within the ramp.
		unsigned int getNumberOfPoints(void);

		/// \brief Adds a new colour point to the ramp
		///
		/// \param fPointPosition The position of the new point within the colour ramp (Is clamped between 0 and 1)
		/// \param pointColour The colour of the new point
		void addPoint(float fPointPosition, CColourf pointColour);

		/// \brief Removes the indexed colour point
		///
		/// \param uiPointIndex The point index number to remove
		/// 
		/// If an invalid point index is given, an exception occurs
		void removePoint(unsigned int uiPointIndex);

		/// \brief Removes all colour points from the ramp
		void removeAllPoints(void);

		/// \brief Modifies the indexed colour point's colour
		///
		/// \param uiPointIndex The index of the point we wish to modify
		/// \param newColour The new colour of the point.
		/// 
		/// If an invalid index is given, an exception occurs
		void modifyPointColour(unsigned int uiPointIndex, CColourf& newColour);

		/// \brief Modifies the indexed colour point's position within the ramp
		///
		/// \param uiPointIndex The index of the point we wish to modify
		/// \param fPointNewPosition The new position of the point.
		/// 
		/// If an invalid index is given, an exception occurs
		void modifyPointPosition(unsigned int uiPointIndex, float fPointNewPosition);

		/// \brief Returns a pointer to the indexed colour point
		/// 
		/// \param uiPointIndex The index of the point we wish to retrieve it's position
		/// \return A pointer to an SPoint, holding the colour point's information
		CColourRamp::SPoint* getPoint(unsigned int uiPointIndex);

		/// \brief Returns the indexed point's current position
		/// 
		/// \param uiPointIndex The index of the point we wish to retrieve it's position
		/// \return A float holding the point's position within the colour ramp
		/// 
		/// If an invalid index is given, an exception occurs
		float getPointPosition(unsigned int uiPointIndex);

		/// \brief Returns the indexed point's colour
		/// 
		/// \param uiPointIndex The index of the point we wish to retrieve it's colour
		/// \return A CColourf holding the point's colour
		/// 
		/// If an invalid index is given, an exception occurs
		CColourf getPointColour(unsigned int uiPointIndex);

		/// \brief Given a position from 0.0f to 1.0f along the colour ramp, returns the interpolated colour at that position.
		///
		/// \param fRampPosition Position along the ramp range from 0.0f to 1.0f
		/// \return A CColourf object holding the interpolated colour
		/// 
		/// If there are no colour points, white is returned.
		/// If there are less than two colour points, an exception occurs
		CColourf getRampColour(float fRampPosition);

		/// \brief Finds the point indicies of the points within the ramp which are closest to the given ramp position
		///
		/// \param fRampPosition The position within the ramp in which we wish to find the closest left and right adjacent colour point indicies.
		/// \param uiPointIndexLeft The index to the point which is closest to the given ramp position, to the left of that position.
		/// \param uiPointIndexRight The index to the point which is closest to the given ramp position, to the right of that position.
		/// 
		/// If less than two colour points exist, an exception occurs.
		void getAdjacentPointIndicies(float fRampPosition, unsigned int &uiPointIndexLeft, unsigned int &uiPointIndexRight);

		/// \brief Sets the colour ramp to have 2 colour points at the left and right edges of the colour ramp
		///
		/// \param colourLeftEdge The left most colour point's colour
		/// \param colourRightEdge The right most colour point's colour
		void setupColourRamp(CColourf colourLeftEdge = CColourf(0.0f, 0.0f, 0.0f, 1.0f), CColourf colourRightEdge = CColourf(1.0f, 1.0f, 1.0f, 1.0f));

		/// \brief Sets the colour ramp to have multiple colour points which represent a fire like gradient
		void setupColourRampFire(void);

		/// \brief Sets the colour ramp to have multiple colour points which represent go through red, to green to blue
		void setupColourRampRGB(void);

		/// \brief Saves the colour ramp's settings as c++ setup code so after editing the colour ramp, we can recreate it in code.
		///
		/// \param strFilename The text file to save the information to
		/// 
		/// We use this method, instead of using files for permanent storage of the data as it's great to setup a colour ramp in code instead of relying upon external files.
		/// 
		/// The setup code will look something like the following...
		/// \code
		/// CColourRamp ramp;
		/// ramp.removeAllPoints();
		/// ramp.addPoint(0.0f, CColourf(0.0f, 0.0f, 0.0f, 0.0f));
		/// // And the above line for each point within the ramp
		/// \endcode
		void saveAsSetupCode(const std::string& strFilename = "ColourRamp.txt");
	private:
		std::list<SPoint> _mlistPoints;	///< A list holding each colour point within the colour ramp.
										/// They are sorted by their position with a point who's position is 0.0 being first in the list.
	};
}
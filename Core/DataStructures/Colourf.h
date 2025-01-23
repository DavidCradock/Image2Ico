#pragma once
#include <string>

namespace X
{
	/// \brief Used to represent a colour using a float for each colour component RGBA, totalling 16 bytes (4 bytes per float * 4 components)
	class CColourf
	{
	public:
		/// \brief Constructor, sets to default values (white with full alpha)
		CColourf();

		/// \brief Constructor, sets to parsed values
		///
		/// \param fRed A value between 0.0f and 1.0f representing the intensity of the red component of this colour.
		/// \param fGreen A value between 0.0f and 1.0f representing the intensity of the green component of this colour.
		/// \param fBlue A value between 0.0f and 1.0f representing the intensity of the blue component of this colour.
		/// \param fAlpha A value between 0.0f and 1.0f representing the intensity of the alpha component of this colour.
		CColourf(float fRed, float fGreen, float fBlue, float fAlpha);

		/// \brief Sets the colour to the given values.
		///
		/// \param fRed A value between 0.0f and 1.0f representing the intensity of the red component of this colour.
		/// \param fGreen A value between 0.0f and 1.0f representing the intensity of the green component of this colour.
		/// \param fBlue A value between 0.0f and 1.0f representing the intensity of the blue component of this colour.
		/// \param fAlpha A value between 0.0f and 1.0f representing the intensity of the alpha component of this colour.
		/// 
		/// Clamps the values in the range of 0.0f and 1.0f
		void set(float fRed, float fGreen, float fBlue, float fAlpha);

		/// \brief Sets this colour to RGB using the given the HSB (Hue, Saturation and Brightness) values.
		///
		/// \param fHueAmount A value between 0.0f and 1.0f representing the hue.
		/// \param fSaturation A value between 0.0f and 1.0f representing the saturation.
		/// \param fBrightness A value between 0.0f and 1.0f representing the brightness.
		/// 
		/// Leaves the alpha value alone
		/// Hue value should be between 0 and 1 (Although it is clamped)
		/// Saturation value should be between 0 and 1 (Although it is clamped) 0 saturation is full colour
		/// Brightness value should be between 0 and 1 (Although it is clamped)
		void setFromHSB(float fHueAmount, float fSaturation = 0.0f, float fBrightness = 1.0f);

		/// \brief From this colour in RGBA format, computes and sets the passed variables to hold the HSB (Hue, saturation and brightness values)
		///
		/// \param fHue A value between 0.0f and 1.0f representing the hue.
		/// \param fSaturation A value between 0.0f and 1.0f representing the saturation.
		/// \param fBrightness A value between 0.0f and 1.0f representing the brightness.
		void getHSB(float& fHue, float& fSaturation, float& fBrightness) const;

		/// \brief Interpolates between this colour and the one given by the given float and returns the new colour.
		///
		/// \param other The other colour to interpolate between this one.
		/// \param fValue The interpolation value between this and the other colour.
		/// \return The resulting colours interpolated between.
		/// 
		/// If the float is 0.0f, the returned colour will be this colour.
		/// If the float is 0.5f, the returned colour will be half way between this colour and the one given.
		/// If the float is 1.0f, the returned colour will be the one given.
		CColourf interpolate(const CColourf& other, float fValue) const;

		/// \brief Overload the == operator to compare colours.
		///
		/// \param col The other colour to compare with this one.
		bool operator == (const CColourf& col)
		{
			if (red == col.red && green == col.green && blue == col.blue && alpha == col.alpha)
				return true;
			return false;
		}

		/// \brief Overload the * operator to multiply two colours together
		///
		/// \param other The other colour which is multiplied with this one.
		/// \return A CColourf object holding the resulting colour.
		CColourf operator*(const CColourf& other) const
		{
			return CColourf(red * other.red, green * other.green, blue * other.blue, alpha * other.alpha);
		}

		/// \brief Overload the * operator to multiply a color by a scalar
		///
		/// \param scalar The scalar value to multiply each color component by.
		/// \return A CColourf object holding the resulting color.
		CColourf operator*(float scalar) const
		{
			return CColourf(
				red * scalar,
				green * scalar,
				blue * scalar,
				alpha * scalar
			);
		}

		/// \brief Overload the + operator to add two colors together
		///
		/// \param other The other color to add to this one.
		/// \return A CColourf object holding the resulting color.
		CColourf operator+(const CColourf& other) const
		{
			return CColourf(
				red + other.red,
				green + other.green,
				blue + other.blue,
				alpha + other.alpha
			);
		}

		/// \brief Overload the - operator to subtract one color from another
		///
		/// \param other The other color to subtract from this one.
		/// \return A CColourf object holding the resulting color.
		CColourf operator-(const CColourf& other) const
		{
			return CColourf(
				red - other.red,
				green - other.green,
				blue - other.blue,
				alpha - other.alpha
			);
		}

		/// \brief Multiply each of this colour's components by the given float value
		///
		/// \param fValue The value to multiply each of this colour's components by
		void multiply(float fValue);

		/// \brief Converts this colour to a hexadecimal string for use in html etc
		///
		/// \return A string holding the hexadecimal representation of this colour
		/// 
		/// If the colour is white, the string "#FFFFFF" is returned.
		std::string colourToHexStringRGB(void) const;

		/// \brief Converts this colour to a hexadecimal string for use in html etc
		///
		/// \return A string holding the hexadecimal representation of this colour
		/// 
		/// If the colour is white, the string "#FFFFFFFF" is returned.
		std::string colourToHexStringRGBA(void) const;

		float red;		///< Red colour between 0.0f and 1.0f
		float green;	///< Green colour between 0.0f and 1.0f
		float blue;		///< Blue colour between 0.0f and 1.0f
		float alpha;	///< Alpha colour between 0.0f and 1.0f
	};
}
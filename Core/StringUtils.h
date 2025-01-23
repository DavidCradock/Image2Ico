#pragma once
#include "../Math/Vector3f.h"
#include <string>
#include <vector>

// Define platform-specific macros if not already defined
#if defined(_WIN32) || defined(_WIN64)
#define PLATFORM_WINDOWS
#elif defined(__linux__)
#define PLATFORM_LINUX
#else
#error "Unknown platform"
#endif

namespace X
{
	/// \brief Contains various functions for messing around with strings
	namespace StringUtils
	{
		/// \brief Converts a string to wide string
		///
		/// \param string The string to convert
		/// \return The converted string
		std::wstring stringToWide(const std::string& string);

		/// \brief Converts a wide string to a string
		///
		/// \param wstring The string to convert
		/// \return The converted string
		std::string wideToString(const std::wstring& wstring);

		/// \brief Returns a vector of strings holding each filename stored within the given directory.
		/// 
		/// \param strDirectory The directory which we wish to find all files within it.
		/// \param bRecursiveDirs If true, then files within sub directories are also added instead of just the ones in the given directory.
		/// \return A vector of strings holding each filename found.
		/// 
		/// If bRecursiveDirs is true, then files within sub directories are also added instead of just the ones in the given directory.
		std::vector<std::string> getFilesInDir(const std::string& strDirectory, bool bRecursiveDirs = false);

		/// \brief Returns a vector of strings holding each filename stored within the given directory which have the given filename extension
		/// 
		/// \param strDirectory The directory which we wish to find all files within it.
		/// \param bRecursiveDirs If true, then files within sub directories are also added instead of just the ones in the given directory.
		/// \param ext The filename extension for the files which match this to be added to the returned results.
		/// \return A vector of strings holding each filename found.
		/// 
		/// If bRecursiveDirs is true, then files within sub directories are also added instead of just the ones in the given directory.
		/// The parsed filename extension may or may not have the "." character at the start.
		std::vector<std::string> getFilesInDir(const std::string& strDirectory, const std::string& ext, bool bRecursiveDirs = false);

		/// \brief Returns a vector of strings holding each directory stored within the given directory
		///
		/// \param strDirectory The directory which we wish to find all directories within it.
		/// \param bRecursiveDirs If true, then dirs within sub directories are also added instead of just the ones in the given directory.
		/// \return A vector of strings holding each directory found.
		/// 
		/// If bRecursiveDirs is true, then directories within sub directories are also added instead of just the ones in the given directory.
		std::vector<std::string> getDirsInDir(const std::string& strDirectory, bool bRecursiveDirs = false);

		/// \brief Converts the given string to lowercase
		///
		/// \param str The string who's characters are to be set to all lowercase.
		void stringToLowercase(std::string& str);

		/// \brief Takes the given filename and filename extension and adds the given filename extension to the end of the given filename and returns the result.
		/// 
		/// \param strFilenameExtension The filename extension to add.
		/// \param strFilename The filename to add the extension to.
		/// \return The filename with the appended filename extension, all lowercase.
		/// 
		/// The returned filename is all lowercase.
		/// The filename extension may or may not have the "." at the beginning. If it doesn't exist, it is added.
		/// If passed strings are of zero length, an exception occurs.
		/// Also, if the strFilename already has an extension, it is removed and replaced with the new one.
		std::string addFilenameExtension(const std::string& strFilenameExtension, const std::string& strFilename);

		/// \brief Returns true if the given string has the given filename extension
		///
		/// \param strFilename The filename to check.
		/// \param strFilenameExtension The filename extension to check for. May or may not contain the "." at the beginning.
		bool hasFilenameExtension(const std::string& strFilename, const std::string& strFilenameExtension);

		/// \brief Given a full filename path including the directoies, returns just the filename
		///
		/// \param strFullPath The full path including the directories.
		/// \return The filename alone.
		/// 
		/// For example, if the strFullPath is "C:/Projects/SomeOtherDir/someFile.txt", "someFile.txt" is returned.
		std::string getFilenameFromFullPath(const std::string& strFullPath);

		/// \brief Append an integer to given string
		///
		/// \param string The string to have the value appended to.
		/// \param iInt The integer to append.
		void appendInt(std::string& string, int iInt);

		/// \brief Converts an integer to a string and returns the result
		///
		/// \param iInteger The integer to convert to a string
		/// \return The integer as a string
		std::string intToString(int iInteger);

		/// \brief Append an integer to given string
		/// \param string The string to have the value appended to.
		/// \param uiInt The unsigned integer to append.
		void appendUInt(std::string& string, unsigned int uiInt);

		/// \brief Converts an unsigned integer to a string and returns the result
		///
		/// \param uiInteger The unsigned integer to convert to a string
		/// \return The unsigned integer as a string
		std::string unsignedIntToString(unsigned int uiInteger);

		/// \brief Append float to given string with the given number of decimals
		///
		/// \param string The string to have the value appended to.
		/// \param fValue The value to append.
		/// \param uiNumDecimalPoints The number of decimal points of the given value to use (can be from 0 to 9)
		/// 
		/// uiNumDecimalPoints can be from 0 to 9, if outside this range, then 2 is used.
		void appendFloat(std::string& string, float fValue, unsigned int uiNumDecimalPoints = 2);

		/// \brief Converts a float to a string and returns the result
		///
		/// \param fFloat The float to convert to a string
		/// \param uiNumDecimalPoints The number of decimal points of the given value to use (can be from 0 to 9)
		/// \return The float as a string
		std::string floatToString(float fFloat, unsigned int uiNumDecimalPoints = 2);

		/// \brief Append double to given string with the given number of decimals
		/// 
		/// \param string The string to have the value appended to.
		/// \param dValue The value to append.
		/// \param uiNumDecimalPoints The number of decimal points of the given value to use (can be from 0 to 9)
		/// 
		/// uiNumDecimalPoints can be from 0 to 9, if outside this range, then 2 is used.
		void appendDouble(std::string& string, double dValue, unsigned int uiNumDecimalPoints = 2);

		/// \brief Converts a double to a string and returns the result
		///
		/// \param dDouble The double to convert to a string
		/// \param uiNumDecimalPoints The number of decimal points of the given value to use (can be from 0 to 9)
		/// \return The double as a string
		std::string doubleToString(double dDouble, unsigned int uiNumDecimalPoints = 2);

		/// \brief Appends a CVector3's components to the string, seperated by the specified seperator text
		/// 
		/// \param string The string which the vector is to be appened to
		/// \param vector The vector which is to be appended
		/// \param uiNumDecimalPoints The number of decimal points to use for each of the vector's components
		/// \param strSeperatorText The text to add to the end of each of the vector's components.
		///
		/// If the vector's x,y and z components are 0.0f, 1.0f and 2.0f and the seperator text is ", " then the string would look like...
		/// 0.0, 1.0, 2.0
		/// uiNumDecimalPoints can be from 0 to 9, if outside this range, then 2 is used.
		void appendCVector3f(std::string& string, const CVector3f& vector, unsigned int uiNumDecimalPoints = 1, const std::string& strSeperatorText = ", ");

		/// \brief Converts a CVector3f to a string and returns the result
		///
		/// \param vector The vector to convert to a string
		/// \param uiNumDecimalPoints The number of decimal points to use for each of the vector's components
		/// \param strSeperatorText The text to add to the end of each of the vector's components.
		/// \return The vector as a string
		std::string vectorToString(const CVector3f& vector, unsigned int uiNumDecimalPoints = 1, const std::string& strSeperatorText = ", ");

		/// \brief Given a string, finds each occurrence of strSplitChars and returns a vector of strings which
		/// contains strings that are either side of the strSplitChars, with the strSplitChars removed.
		/// 
		/// \param string The string to use as input for the operation
		/// \param strSplitChars The split chars to use for the operation
		/// \return A vector of strings holding the results
		/// 
		/// This is usefull for splitting text into multiple lines, using a specified character sequence used to
		/// represent the end of the line of text.
		/// For example, if the string is "Line1\nLine2\nLine3" and the strSpliteChars is "\n"
		/// then the returned vector would have 3 entries, "Line1" "Line2" and "Line3"
		std::vector<std::string> splitString(const std::string& string, const std::string& strSplitChars = "\n");

		/// \brief Given two strings, one containing a full word and the second a partial word, returns true if the partial word is contained within the full word.
		/// 
		/// Used for checking if a partial word is contained within a full word.
		/// An example usage is in a console where the user types parth of a command in and then we can use this method
		/// to check if the partial word is contained within each of the full commands and if so, there is a partial match
		/// 
		/// \param strFullWord The full word to check
		/// \param strPartialWord The partial word to check for
		/// \return True if the partial word is contained within the full word
		bool partialMatch(const std::string& strFullWord, const std::string& strPartialWord);

		/// \brief Returns true if the given string has nothing but digits
		///
		/// \param string The string to check.
		/// \return True if the given string has nothing but digits
		bool representsNumber(const std::string& string);

		/// \brief Writes out a string to an already opened ofstream.
		/// 
		/// \param strString The string to write to the file
		/// \param file An allready opened ofstream to write the string to.
		/// 
		/// The writes out the size of the string and then the string itself.
		/// It's paired with stringRead()
		/// If the file is not open, an exception occurs
		void stringWrite(const std::string& strString, std::ofstream& file);

		/// Reads in a string from an already opened ifstream.
		/// 
		/// \param strString The string to hold the data read in
		/// \param file An allready opened ifstream to read the string from.
		/// 
		/// This reads in the size of the string, resizes it to make room and then loads it in
		/// If the file is not open or there was an error during reading, an exception occurs.
		void stringRead(std::string& strString, std::ifstream& file);

		/// \brief Helper method to write the given string to an already opened ofstream in binary mode.
		///
		/// \param file The already opened ofstream
		/// \param strString The string to write.
		/// 
		/// If the given ofstream is not opened or an error occurred, an exception occurs
		void writeStringBinary(std::ofstream& file, std::string& strString);

		/// \brief Helper method to read a string from an already opened ifstream in binary mode.
		///
		/// \param file The already opened ifstream
		/// \param strString The string to read.
		/// 
		/// If the given ifstream is not opened or an error occurred, an exception occurs
		/// The given string is cleared before being set.
		void readStringBinary(std::ifstream& file, std::string& strString);

		/// \brief Blender animation frame filename helper.
		///
		/// \param strBasename The base name of all the images
		/// \param strExtension The extension of all the images, for example ".png"
		/// \param iFrameNumber The frame number we wish to create the full filename for.
		/// \return The computed filename from all the given parameters.
		/// 
		/// Say we create an animation in Blender with 100 frames and we save them out as "image"
		/// The first 10 images will be named "image0000.png" through to "image0009.png"
		/// The next 90 will be named "image0010.png" through to "image0099.png"
		/// What this function does is given, a basename, an integer and extension, it returns
		/// a string which holds the correct final name.
		/// So for example, passing "image" as the basename, "png" (or ".png") as the extension and
		/// "50" for the int will return the string "image0050.png"
		std::string blenderAnimFilename(const std::string& strBasename = "image", const std::string& strExtension = ".png", int iFrameNumber = 0);

		/// \brief Converts the given string to a float
		///
		/// \param string The string holding a float
		/// \return The float value contained within the string
		float stringToFloat(const std::string& string);

		/// \brief Converts the given string to an int
		///
		/// \param string The string holding the integer
		/// \return The int value contained within the string
		int stringToInt(const std::string& string);

		/// \brief Helper method to read in a .txt file and return it as a string
		///
		/// \param strFilePath The path to the file to read
		/// \return The contents of the file as a string
		std::string loadFileToString(const std::string& strFilePath);

		/// \brief Helper method to save a string to a file
		///
		/// \param str The string to write
		/// \param strFilePath The path to the file to write to
		void saveStringToFile(const std::string& str, const std::string& strFilePath);

		/// \brief Compares two strings and returns an integer value indicating their relationship. This is case sensitive
		/// 0 if they are the same, a negative value if str1 is less than str2, a positive value if str1 is greater than str2.
		///
		/// \param str1 The first string to compare
		/// \param str2 The second string to compare
		/// \return An integer value indicating the relationship between the two strings. 0 if they are the same, a negative value if str1 is less than str2, a positive value if str1 is greater than str2
		int compareCaseSensitive(const std::string& str1, const std::string& str2);

		/// \brief Compares two strings and returns an integer value indicating their relationship. This is case in-sensitive
		/// 0 if they are the same, a negative value if str1 is less than str2, a positive value if str1 is greater than str2.
		///
		/// \param str1 The first string to compare
		/// \param str2 The second string to compare
		/// \return An integer value indicating the relationship between the two strings. 0 if they are the same, a negative value if str1 is less than str2, a positive value if str1 is greater than str2
		int compareCaseInsensitive(const std::string& str1, const std::string& str2);

	}   // namespace StringUtils
}
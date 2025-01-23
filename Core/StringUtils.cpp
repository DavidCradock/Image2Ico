#include "StringUtils.h"
#include "Exceptions.h"
#include "Logging.h"
#include <filesystem>

#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#elif PLATFORM_LINUX
#include <cstdlib> // For mbstowcs on Linux
#endif

namespace X
{
	namespace StringUtils
	{

		std::wstring stringToWide(const std::string& string)
		{
			if (string.empty())
				return std::wstring();
#ifdef PLATFORM_WINDOWS
			int iSize = MultiByteToWideChar(CP_UTF8, 0, &string[0], (int)string.size(), NULL, 0);
			std::wstring wstrOut(iSize, 0);
			MultiByteToWideChar(CP_UTF8, 0, &string[0], (int)string.size(), &wstrOut[0], iSize);
#elif PLATFORM_LINUX
			size_t iSize = mbstowcs(nullptr, string.c_str(), 0);
			if (iSize == (size_t)-1)
			{
				throw std::runtime_error("Error converting string to wide string");
			}
			std::wstring wstrOut(iSize, 0);
			mbstowcs(&wstrOut[0], string.c_str(), iSize);
#endif
			return wstrOut;
		}

		std::string wideToString(const std::wstring& wstring)
		{
			if (wstring.empty())
				return std::string();
#ifdef PLATFORM_WINDOWS
			int iSize = WideCharToMultiByte(CP_UTF8, 0, &wstring[0], (int)wstring.size(), NULL, 0, NULL, NULL);
			std::string strOut(iSize, 0);
			WideCharToMultiByte(CP_UTF8, 0, &wstring[0], (int)wstring.size(), &strOut[0], iSize, NULL, NULL);
#elif PLATFORM_LINUX
			size_t iSize = wcstombs(nullptr, wstring.c_str(), 0);
			if (iSize == (size_t)-1)
			{
				throw std::runtime_error("Error converting wide string to string");
			}
			std::string strOut(iSize, 0);
			wcstombs(&strOut[0], wstring.c_str(), iSize);
#endif
			return strOut;
		}

		std::vector<std::string> getFilesInDir(const std::string& strDirectory, bool bRecursiveDirs)
		{
			std::vector<std::string> filenames;
			if (bRecursiveDirs)
			{
				for (const auto& filename : std::filesystem::recursive_directory_iterator(strDirectory))
				{
					if (filename.is_regular_file())
						filenames.push_back(filename.path().string());
				}
			}
			else
			{
				for (const auto& filename : std::filesystem::directory_iterator(strDirectory))
				{
					if (filename.is_regular_file())
						filenames.push_back(filename.path().string());
				}
			}
			return filenames;
		}

		std::vector<std::string> getFilesInDir(const std::string& strDirectory, const std::string& ext, bool bRecursiveDirs)
		{
			// Make sure the given extension has the "." character
			ThrowIfTrue(0 == ext.length(), "Extension has length of zero.");
			std::string strExt;
			if (ext.c_str()[0] != '.')
				strExt.append(".");
			strExt.append(ext);

			std::vector<std::string> filenames;
			if (bRecursiveDirs)
			{
				for (const auto& filename : std::filesystem::recursive_directory_iterator(strDirectory))
				{
					if (filename.is_regular_file())
					{
						if (filename.path().extension() == strExt)
							filenames.push_back(filename.path().string());
					}
				}
			}
			else
			{
				for (const auto& filename : std::filesystem::directory_iterator(strDirectory))
				{
					if (filename.is_regular_file())
					{
						if (filename.path().extension() == strExt)
							filenames.push_back(filename.path().string());
					}
				}
			}
			return filenames;
		}

		std::vector<std::string> getDirsInDir(const std::string& strDirectory, bool bRecursiveDirs)
		{
			std::vector<std::string> dirnames;
			if (bRecursiveDirs)
			{
				for (const auto& dirname : std::filesystem::recursive_directory_iterator(strDirectory))
				{
					if (dirname.is_directory())
						dirnames.push_back(dirname.path().string());
				}
			}
			else
			{
				for (const auto& dirname : std::filesystem::directory_iterator(strDirectory))
				{
					if (dirname.is_directory())
						dirnames.push_back(dirname.path().string());
				}
			}
			return dirnames;
		}

		void stringToLowercase(std::string& str)
		{
			std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });
		}

		std::string addFilenameExtension(const std::string& strFilenameExtension, const std::string& strFilename)
		{
			// Make sure valid values given.
			ThrowIfTrue(0 == strFilenameExtension.length(), "Given extension name of zero length.");
			ThrowIfTrue(0 == strFilename.length(), "Given file name of zero length.");

			// Append "." to extension if needed
			std::string strExt = strFilenameExtension;
			if (strExt.c_str()[0] != '.')
			{
				std::string::iterator itBegin = strExt.begin();
				strExt.insert(itBegin, '.');
			}
			std::string strFile = strFilename;
			// Find last position of "." in given file name and remove everything after it
			auto const pos = strFile.find_last_of('.');
			if (pos != std::string::npos)	// If "." found
			{
				// Remove "." and all following text
				strFile.erase(pos, strFile.length() - pos);
			}
			// Append extension to filename
			strFile.append(strExt);

			// Make all lowercase
			stringToLowercase(strFile);
			return strFile;
		}

		bool hasFilenameExtension(const std::string& strFilename, const std::string& strFilenameExtension)
		{
			// Ensure the given extension is not empty
			if (strFilenameExtension.empty())
				return false;

			// Append '.' to the extension if necessary
			std::string expectedExtension = strFilenameExtension;
			if (expectedExtension.front() != '.')
			{
				expectedExtension.insert(expectedExtension.begin(), '.');
			}

			// Extract the extension from the filename
			std::filesystem::path filePath(strFilename);
			std::string fileExtension = filePath.extension().string();

			// Convert both extensions to lowercase for case-insensitive comparison
			StringUtils::stringToLowercase(expectedExtension);
			StringUtils::stringToLowercase(fileExtension);

			// Compare the extensions
			return fileExtension == expectedExtension;
		}

		std::string getFilenameFromFullPath(const std::string& strFullPath)
		{
			return std::filesystem::path(strFullPath).filename().string();
		}

		void appendInt(std::string& string, int iInt)
		{
			string += std::to_string(iInt);
		}

		std::string intToString(int iInteger)
		{
			return std::to_string(iInteger);
		}

		void appendUInt(std::string& string, unsigned int uiInt)
		{
			string += std::to_string(uiInt);
		}

		std::string unsignedIntToString(unsigned int uiInteger)
		{
			return std::to_string(uiInteger);
		}

		void appendFloat(std::string& string, float fValue, unsigned int uiNumDecimalPoints)
		{
			switch (uiNumDecimalPoints)
			{
			case 0:	string += std::format("{:.0f}", fValue);	break;
			case 1:	string += std::format("{:.1f}", fValue);	break;
			case 2:	string += std::format("{:.2f}", fValue);	break;
			case 3:	string += std::format("{:.3f}", fValue);	break;
			case 4:	string += std::format("{:.4f}", fValue);	break;
			case 5:	string += std::format("{:.5f}", fValue);	break;
			case 6:	string += std::format("{:.6f}", fValue);	break;
			case 7:	string += std::format("{:.7f}", fValue);	break;
			case 8:	string += std::format("{:.8f}", fValue);	break;
			case 9:	string += std::format("{:.9f}", fValue);	break;
			default:string += std::format("{:.2f}", fValue);
			}
		}

		std::string floatToString(float fFloat, unsigned int uiNumDecimalPoints)
		{
			switch (uiNumDecimalPoints)
			{
			case 0:	return std::format("{:.0f}", fFloat);
			case 1:	return std::format("{:.1f}", fFloat);
			case 2:	return std::format("{:.2f}", fFloat);
			case 3:	return std::format("{:.3f}", fFloat);
			case 4:	return std::format("{:.4f}", fFloat);
			case 5:	return std::format("{:.5f}", fFloat);
			case 6:	return std::format("{:.6f}", fFloat);
			case 7:	return std::format("{:.7f}", fFloat);
			case 8:	return std::format("{:.8f}", fFloat);
			case 9:	return std::format("{:.9f}", fFloat);
			default:return std::format("{:.2f}", fFloat);
			}
		}

		void appendDouble(std::string& string, double dValue, unsigned int uiNumDecimalPoints)
		{
			switch (uiNumDecimalPoints)
			{
			case 0:	string += std::format("{:.0f}", dValue);	break;
			case 1:	string += std::format("{:.1f}", dValue);	break;
			case 2:	string += std::format("{:.2f}", dValue);	break;
			case 3:	string += std::format("{:.3f}", dValue);	break;
			case 4:	string += std::format("{:.4f}", dValue);	break;
			case 5:	string += std::format("{:.5f}", dValue);	break;
			case 6:	string += std::format("{:.6f}", dValue);	break;
			case 7:	string += std::format("{:.7f}", dValue);	break;
			case 8:	string += std::format("{:.8f}", dValue);	break;
			case 9:	string += std::format("{:.9f}", dValue);	break;
			default:string += std::format("{:.2f}", dValue);
			}
		}
		
		std::string doubleToString(double dDouble, unsigned int uiNumDecimalPoints)
		{
			switch (uiNumDecimalPoints)
			{
			case 0:	return std::format("{:.0f}", dDouble);
			case 1:	return std::format("{:.1f}", dDouble);
			case 2:	return std::format("{:.2f}", dDouble);
			case 3:	return std::format("{:.3f}", dDouble);
			case 4:	return std::format("{:.4f}", dDouble);
			case 5:	return std::format("{:.5f}", dDouble);
			case 6:	return std::format("{:.6f}", dDouble);
			case 7:	return std::format("{:.7f}", dDouble);
			case 8:	return std::format("{:.8f}", dDouble);
			case 9:	return std::format("{:.9f}", dDouble);
			default:return std::format("{:.2f}", dDouble);
			}
		}

		void appendCVector3f(std::string& string, const CVector3f& vector, unsigned int uiNumDecimalPoints, const std::string& strSeperatorText)
		{
			std::string strTmp;
			appendFloat(strTmp, vector.x, uiNumDecimalPoints);
			string += strTmp;
			string += strSeperatorText;

			strTmp.clear();
			appendFloat(strTmp, vector.y, uiNumDecimalPoints);
			string += strTmp;
			string += strSeperatorText;

			strTmp.clear();
			appendFloat(strTmp, vector.z, uiNumDecimalPoints);
			string += strTmp;
		}
		
		std::string vectorToString(const CVector3f& vector, unsigned int uiNumDecimalPoints, const std::string& strSeperatorText)
		{
			std::string strOut;
			appendCVector3f(strOut, vector, uiNumDecimalPoints, strSeperatorText);
			return strOut;
		}

		std::vector<std::string> splitString(const std::string& string, const std::string& strSplitChars)
		{
			std::vector<std::string> out;

			// If no strSplitChars found, simply add the entire string and return the result
			size_t pos = string.find(strSplitChars, 0);
			if (std::string::npos == pos)
			{
				out.push_back(string);
				return out;
			}

			// If we get here, strSplitChars has been found in the string
			std::string strLine;
			std::string strAll = string;
			while (std::string::npos != pos)
			{
				// Copy character upto the position of the found strSplitChars into strLine
				strLine.assign(strAll, 0, pos);

				// Add the line to the output
				out.push_back(strLine);

				// Reset strLine
				strLine.clear();

				// Remove all characters including the strSplitChars from strAll
				strAll.erase(0, pos + strSplitChars.length());

				// Find next position of strSplitChars in strAll
				pos = strAll.find(strSplitChars, 0);
			}
			// If strAll still contains characters, add them to the vector
			if (strAll.length())
			{
				out.push_back(strAll);
			}

			return out;
		}

		bool partialMatch(const std::string& strFullWord, const std::string& strPartialWord)
		{
			if (strPartialWord.empty())
				return false;

			size_t pos = strFullWord.find(strPartialWord);

			if (pos == std::string::npos)
				return false;

			if (pos != 0)
				return false;
			
			return true;
		}

		bool representsNumber(const std::string& string)
		{
			return (std::all_of(string.begin(), string.end(), ::isdigit));
		}

		void stringWrite(const std::string& strString, std::ofstream& file)
		{
			ThrowIfFalse(file.is_open(), "The given ofstream is not open.");
			size_t size = strString.size();
			file.write((char*)&size, sizeof(size));
			file.write(strString.c_str(), size);
			ThrowIfFalse(file.good(), "The ofstream is not good.");
		}

		void stringRead(std::string& strString, std::ifstream& file)
		{
			ThrowIfFalse(file.is_open(), "The given ofstream is not open.");
			strString.clear();
			size_t size;
			file.read((char*)&size, sizeof(size));
			strString.resize(size);
			//file.read(&strString[0], size);
			file.read((char*)strString.c_str(), size);
			ThrowIfFalse(file.good(), "The ifstream is not good.");
		}

		void writeStringBinary(std::ofstream& file, std::string& strString)
		{
			ThrowIfFalse(file.is_open(), "The given ofstream is not open.");
			ThrowIfTrue(file.bad(), "The given ofstream is bad.");

			// Write out length of string
			size_t stringLength = strString.size();
			file.write((char*)&stringLength, sizeof(size_t));

			// Write out characters
			file.write(strString.c_str(), stringLength);

			ThrowIfTrue(file.bad(), "The given ofstream is bad.");
		}

		void readStringBinary(std::ifstream& file, std::string& strString)
		{
			ThrowIfFalse(file.is_open(), "The given ifstream is not open.");
			ThrowIfTrue(file.bad(), "The given ifstream is bad.");

			// Read in length of string
			size_t stringLength;
			file.read((char*)&stringLength, sizeof(size_t));

			// Read in characters
			char* charTemp = new char[stringLength];
			ThrowIfFalse(charTemp, "Temporary memory allocation failed.");
			file.read(charTemp, stringLength);
			ThrowIfTrue(file.bad(), "The given ifstream is bad.");

			strString.clear();
			strString.append(charTemp);
			delete[] charTemp;
		}

		std::string blenderAnimFilename(const std::string& strBasename, const std::string& strExtension, int iFrameNumber)
		{
			ThrowIfTrue(iFrameNumber < 0 || iFrameNumber > 9999, "Given invalid iFrameNumber.");
			std::string output;
			output = strBasename;
			if (iFrameNumber < 10)
			{
				output += "000";
				output += std::to_string(iFrameNumber);
			}
			else if (iFrameNumber < 100)
			{
				output += "00";
				output += std::to_string(iFrameNumber);
			}
			else if (iFrameNumber < 1000)
			{
				output += "0";
				output += std::to_string(iFrameNumber);
			}
			else
			{
				output += std::to_string(iFrameNumber);
			}

			// Append "." to extension if needed
			std::string strExt = strExtension;
			if (strExt.c_str()[0] != '.')
			{
				std::string::iterator itBegin = strExt.begin();
				strExt.insert(itBegin, '.');
			}
			output += strExt;
			return output;
		}

		float stringToFloat(const std::string& string)
		{
			float strfloat = std::stof(string);
			return strfloat;
		}

		int stringToInt(const std::string& string)
		{
			int strInt = std::stoi(string);
			return strInt;
		}

		std::string loadFileToString(const std::string& strFilePath)
		{
			std::ifstream fileStream(strFilePath);
			if (!fileStream.is_open())
			{
				Throw("Could not open file: " + strFilePath);
			}

			std::stringstream buffer;
			buffer << fileStream.rdbuf();
			return buffer.str();
		}

		void saveStringToFile(const std::string& str, const std::string& strFilePath)
		{
			std::ofstream fileStream(strFilePath);
			if (!fileStream.is_open())
			{
				Throw("Could not open file: " + strFilePath);
			}

			fileStream << str;
			if (!fileStream.good())
			{
				Throw("Error occurred while writing to file: " + strFilePath);
			}
		}

		int compareCaseSensitive(const std::string& str1, const std::string& str2)
		{
			return str1.compare(str2);
		}

		int compareCaseInsensitive(const std::string& str1, const std::string& str2)
		{
			std::string str1Lower = str1;
			std::string str2Lower = str2;
			stringToLowercase(str1Lower);
			stringToLowercase(str2Lower);
			return str1Lower.compare(str2Lower);
		}
	}
}
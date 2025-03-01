#include "Image.h"

#include "../Core/Exceptions.h"
#include "../Core/StringUtils.h"
#include "../Core/Utilities.h"
#include "../Math/Vector3f.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include <random>
#include "FastNoiseLite.h"
#include <complex>

namespace X
{
#pragma pack(push, 1) // Ensure structures are packed without padding
	/// \brief Structure for .ico saving
	struct ICONDIR {
		uint16_t idReserved;   // Must be 0
		uint16_t idType;       // 1 for icons
		uint16_t idCount;      // Number of images
	};

	/// \brief Structure for .ico saving
	struct ICONDIRENTRY {
		uint8_t  bWidth;        // Width of the image (0 means 256)
		uint8_t  bHeight;       // Height of the image (0 means 256)
		uint8_t  bColorCount;   // Number of colors (0 if 256 or more)
		uint8_t  bReserved;     // Must be 0
		uint16_t wPlanes;       // Color planes
		uint16_t wBitCount;     // Bits per pixel
		uint32_t dwBytesInRes;  // Size of the image data
		uint32_t dwImageOffset; // Offset of the image data from the beginning of the file
	};
#pragma pack(pop) // Reset to default packing

	CImage::CImage()
	{
		_mpData = 0;
		_muiDataSize = 0;
		free();
	}

	CImage::~CImage()
	{
		free();
	}

	CImage& CImage::operator=(const CImage& other)
	{
		// Guard against self assignment
		if (this == &other)
			return *this;

		other.copyTo(*this);
		return *this;
	}


	void CImage::free(void)
	{
		if (_mpData)
		{
			delete[] _mpData;
			_mpData = NULL;
			_muiDataSize = 0;
		}
		_miWidth = _miHeight = _miNumChannels = 0;
	}

	void CImage::createBlank(unsigned int iWidth, unsigned int iHeight, unsigned short iNumChannels)
	{
		free();
		ThrowIfTrue(iWidth < 1, "Given width < 1.");
		ThrowIfTrue(iHeight < 1, "Given height < 1.");
		ThrowIfTrue(iNumChannels < 3, "Given number of channels < 1. (Only 3 or 4 is valid)");
		ThrowIfTrue(iNumChannels > 4, "Given number of channels > 4. (Only 3 or 4 is valid)");

		_miWidth = iWidth;
		_miHeight = iHeight;
		_miNumChannels = iNumChannels;
		_muiDataSize = _miWidth * _miHeight * _miNumChannels;
		_mpData = new unsigned char[_muiDataSize];
		ThrowIfTrue(!_mpData, "Failed to allocate memory.");

		// Zero out the new memory all to zero
		for (unsigned int i = 0; i < _muiDataSize; ++i)
		{
			_mpData[i] = 0;
		}
	}

	bool CImage::load(const std::string& strFilename, bool bFlipForOpenGL)
	{
		free();

		if (StringUtils::hasFilenameExtension(strFilename, "dif"))
			return _loadDIF(strFilename, bFlipForOpenGL);

		// Use stb_image to load...
		stbi_set_flip_vertically_on_load(bFlipForOpenGL);

		// Get number of channels in the image file
		int iDims[2];
		int iNumChannels = 3;
		loadInfo(strFilename, iDims[0], iDims[1], iNumChannels);
		stbi_uc* pixels = 0;
		if (4 == iNumChannels)
			pixels = stbi_load(strFilename.c_str(), &_miWidth, &_miHeight, &_miNumChannels, STBI_rgb_alpha);
		else if (3 == iNumChannels)
			pixels = stbi_load(strFilename.c_str(), &_miWidth, &_miHeight, &_miNumChannels, STBI_rgb);
		else if (1 == iNumChannels)
			pixels = stbi_load(strFilename.c_str(), &_miWidth, &_miHeight, &_miNumChannels, 1);

		if (!pixels)
			return false;

		// If number of channels is 1, then we convert that 1 channel to 3 and duplicate the R to G and B
		if (1 == iNumChannels)
		{
			_miNumChannels = 3;
		}

		// Compute size and allocate
		_muiDataSize = _miWidth * _miHeight * _miNumChannels;
		_mpData = new unsigned char[_muiDataSize];

		if (1 != iNumChannels)
			memcpy(_mpData, pixels, static_cast<size_t>(_muiDataSize));
		else // We need to copy the R to G and B
		{
			unsigned int iPixelIndex = 0;
			for (unsigned int i = 0; i < _muiDataSize; i += 3)
			{
				_mpData[i] = pixels[iPixelIndex];
				_mpData[i + 1] = pixels[iPixelIndex];
				_mpData[i + 2] = pixels[iPixelIndex];
				iPixelIndex++;
			}
		}
		stbi_image_free(pixels);
		return true;
	}

	bool CImage::loadInfo(const std::string& strFilename, int& iWidth, int& iHeight, int& iNumChannels)
	{
		if (StringUtils::hasFilenameExtension(strFilename, "dif"))
			return _loadInfoDIF(strFilename, iWidth, iHeight, iNumChannels);

		// Use stb_image to load...
		// 
		// To query the width, height and component count of an image without having to
		// decode the full file, you can use the stbi_info family of functions:
		//
		//   int ix,iy,n,ok;
		//   ok = stbi_info(filename, &ix, &iy, &n);
		//   // returns ok=1 and sets ix, iy, n if image is a supported format,
		//   // 0 otherwise.
		return (bool)stbi_info(strFilename.c_str(), &iWidth, &iHeight, &iNumChannels);
	}

	void CImage::saveAsBMP(const std::string& strFilename, bool bFlipOnSave) const
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");
		stbi_flip_vertically_on_write(bFlipOnSave); // flag is non-zero to flip data vertically
		ThrowIfTrue(!stbi_write_bmp(strFilename.c_str(), _miWidth, _miHeight, _miNumChannels, _mpData), "Image failed to be written.");
	}

	void CImage::saveAsJPG(const std::string& strFilename, bool bFlipOnSave, int iQuality) const
	{
		ThrowIfTrue(!_mpData, "CImage::saveAsJPG() failed. Image not yet created.");
		stbi_flip_vertically_on_write(bFlipOnSave); // flag is non-zero to flip data vertically
		ThrowIfTrue(!stbi_write_jpg(strFilename.c_str(), _miWidth, _miHeight, _miNumChannels, _mpData, iQuality), "Image failed to be written.");
	}

	void CImage::saveAsPNG(const std::string& strFilename, bool bFlipOnSave) const
	{
		ThrowIfTrue(!_mpData, "CImage::saveAsPNG() failed. Image not yet created.");
		stbi_flip_vertically_on_write(bFlipOnSave); // flag is non-zero to flip data vertically
		ThrowIfTrue(!stbi_write_png(strFilename.c_str(), _miWidth, _miHeight, _miNumChannels, _mpData, _miWidth * _miNumChannels), "Image failed to be written.");
	}

	void CImage::saveAsTGA(const std::string& strFilename, bool bFlipOnSave) const
	{
		ThrowIfTrue(!_mpData, "CImage::saveAsTGA() failed. Image not yet created.");
		stbi_flip_vertically_on_write(bFlipOnSave); // flag is non-zero to flip data vertically
		ThrowIfTrue(!stbi_write_tga(strFilename.c_str(), _miWidth, _miHeight, _miNumChannels, _mpData), "Image failed to be written.");
	}

	void CImage::fill(unsigned char ucRed, unsigned char ucGreen, unsigned char ucBlue, unsigned char ucAlpha)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		unsigned int i = 0;

		// 3 Colour channels
		if (3 == _miNumChannels)
		{
			while (i < _muiDataSize)
			{
				_mpData[i] = ucRed;
				_mpData[i + 1] = ucGreen;
				_mpData[i + 2] = ucBlue;
				i += _miNumChannels;
			}
		}

		// 4 colour channels
		if (4 == _miNumChannels)
		{
			while (i < _muiDataSize)
			{
				_mpData[i] = ucRed;
				_mpData[i + 1] = ucGreen;
				_mpData[i + 2] = ucBlue;
				_mpData[i + 3] = ucAlpha;
				i += _miNumChannels;
			}
		}
	}

	void CImage::fillCellularNoise(float fFrequency, unsigned int uiOctaves, CColourRamp colourRamp)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		// Create and configure FastNoise object
		FastNoiseLite noise;
		noise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
		noise.SetFrequency(fFrequency);
		noise.SetFractalOctaves(uiOctaves);
		noise.SetFractalType(FastNoiseLite::FractalType_Ridged);
		// Gather noise data
		int index = 0;
		float fNoise;
		CColourf colour;
		if (3 == _miNumChannels)
		{
			for (int y = 0; y < _miHeight; y++)
			{
				for (int x = 0; x < _miWidth; x++)
				{
					// Get noise value for pixel and convert from -1 to 1 to 0 to 1
					fNoise = noise.GetNoise((float)x, (float)y);
					fNoise += 1.0f;
					fNoise *= 0.5f;
					colour = colourRamp.getRampColour(fNoise);

					_mpData[index++] = unsigned char(colour.red * 255.0f);
					_mpData[index++] = unsigned char(colour.green * 255.0f);
					_mpData[index++] = unsigned char(colour.blue * 255.0f);
				}
			}
		}
		else if (4 == _miNumChannels)
		{
			for (int y = 0; y < _miHeight; y++)
			{
				for (int x = 0; x < _miWidth; x++)
				{
					// Get noise value for pixel and convert from -1 to 1 to 0 to 1
					fNoise = noise.GetNoise((float)x, (float)y);
					fNoise += 1.0f;
					fNoise *= 0.5f;
					colour = colourRamp.getRampColour(fNoise);

					_mpData[index++] = unsigned char(colour.red * 255.0f);
					_mpData[index++] = unsigned char(colour.green * 255.0f);
					_mpData[index++] = unsigned char(colour.blue * 255.0f);
					_mpData[index++] = unsigned char(colour.alpha * 255.0f);
				}
			}
		}
	}

	void CImage::fillPerlinNoise(float fFrequency, unsigned int uiOctaves, CColourRamp colourRamp)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");
		
		// Create and configure FastNoise object
		FastNoiseLite noise;
		noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
		noise.SetFrequency(fFrequency);
		noise.SetFractalOctaves(uiOctaves);
		noise.SetFractalType(FastNoiseLite::FractalType_FBm);
		// Gather noise data
		int index = 0;
		float fNoise;
		CColourf colour;
		if (3 == _miNumChannels)
		{
			for (int y = 0; y < _miHeight; y++)
			{
				for (int x = 0; x < _miWidth; x++)
				{
					// Get noise value for pixel and convert from -1 to 1 to 0 to 1
					fNoise = noise.GetNoise((float)x, (float)y);
					fNoise += 1.0f;
					fNoise *= 0.5f;
					colour = colourRamp.getRampColour(fNoise);

					_mpData[index++] = unsigned char(colour.red * 255.0f);
					_mpData[index++] = unsigned char(colour.green * 255.0f);
					_mpData[index++] = unsigned char(colour.blue * 255.0f);
				}
			}
		}
		else if (4 == _miNumChannels)
		{
			for (int y = 0; y < _miHeight; y++)
			{
				for (int x = 0; x < _miWidth; x++)
				{
					// Get noise value for pixel and convert from -1 to 1 to 0 to 1
					fNoise = noise.GetNoise((float)x, (float)y);
					fNoise += 1.0f;
					fNoise *= 0.5f;
					colour = colourRamp.getRampColour(fNoise);

					_mpData[index++] = unsigned char(colour.red * 255.0f);
					_mpData[index++] = unsigned char(colour.green * 255.0f);
					_mpData[index++] = unsigned char(colour.blue * 255.0f);
					_mpData[index++] = unsigned char(colour.alpha * 255.0f);
				}
			}
		}
	}

	void CImage::fillRandomNoise(CColourRamp colourRamp)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");
		unsigned int i = 0;

		// Seed the random number generator with a non-deterministic value
		std::random_device rd;
		std::mt19937 gen(rd());
		// Generate a random integer between 0 and 255
		//std::uniform_int_distribution<> distrib(0, 255);
		std::uniform_real_distribution<> distrib(0.0, 1.0);
		CColourf colour;
		double dPosition;
		if (4 == _miNumChannels)
		{
			while (i < _muiDataSize)
			{
				dPosition = distrib(gen);
				colour = colourRamp.getRampColour(float(dPosition));
				_mpData[i] = unsigned char(colour.red * 255.0f);
				_mpData[i + 1] = unsigned char(colour.green * 255.0f);
				_mpData[i + 2] = unsigned char(colour.blue * 255.0f);
				_mpData[i + 3] = unsigned char(colour.alpha * 255.0f);
				i += _miNumChannels;
			}
		}
		else if (3 == _miNumChannels)
		{
			while (i < _muiDataSize)
			{
				dPosition = distrib(gen);
				colour = colourRamp.getRampColour(float(dPosition));
				_mpData[i] = unsigned char(colour.red * 255.0f);
				_mpData[i + 1] = unsigned char(colour.green * 255.0f);
				_mpData[i + 2] = unsigned char(colour.blue * 255.0f);
				i += _miNumChannels;
			}
		}
	}

	void CImage::fillMandelbrot(CColourRamp colourRamp, double minX, double maxX, double minY, double maxY,unsigned int uiMaxIterations)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");
		ThrowIfTrue(uiMaxIterations == 0, "uiMaxIterations must be at least one.");

		// Define the complex plane boundaries

		// Calculate pixel width and height
		const double dx = (maxX - minX) / _miWidth;
		const double dy = (maxY - minY) / _miHeight;

		// Iterate over each pixel
		unsigned int iIndex;
		if (_miNumChannels == 3)
		{
			for (int y = 0; y < _miHeight; ++y)
			{
				for (int x = 0; x < _miWidth; ++x)
				{
					// Calculate the complex number for the current pixel
					std::complex<double> c(minX + x * dx, minY + y * dy);
					std::complex<double> z(0.0, 0.0);

					unsigned int iterations = 0;
					while (std::abs(z) < 2.0 && iterations < uiMaxIterations) {
						z = z * z + c;
						++iterations;
					}

					// Assign a color based on the number of iterations
					iIndex = x + (y * _miWidth);
					iIndex *= _miNumChannels;
					CColourf colour = colourRamp.getRampColour(float(iterations) / float(uiMaxIterations));

					_mpData[iIndex] = unsigned char(colour.red * 255.0f);
					_mpData[iIndex + 1] = unsigned char(colour.green * 255.0f);
					_mpData[iIndex + 2] = unsigned char(colour.blue * 255.0f);
				}
			}
		}
		else if (_miNumChannels == 4)
		{
			for (int y = 0; y < _miHeight; ++y)
			{
				for (int x = 0; x < _miWidth; ++x)
				{
					// Calculate the complex number for the current pixel
					std::complex<double> c(minX + x * dx, minY + y * dy);
					std::complex<double> z(0.0, 0.0);

					unsigned int iterations = 0;
					while (std::abs(z) < 2.0 && iterations < uiMaxIterations) {
						z = z * z + c;
						++iterations;
					}

					// Assign a color based on the number of iterations
					iIndex = x + (y * _miWidth);
					iIndex *= _miNumChannels;
					CColourf colour = colourRamp.getRampColour(float(iterations) / float(uiMaxIterations));

					_mpData[iIndex] = unsigned char(colour.red * 255.0f);
					_mpData[iIndex + 1] = unsigned char(colour.green * 255.0f);
					_mpData[iIndex + 2] = unsigned char(colour.blue * 255.0f);
					_mpData[iIndex + 3] = unsigned char(colour.alpha * 255.0f);
				}
			}
		}
	}

	void CImage::fillMandelbrotMT(CColourRamp colourRamp, double minX, double maxX, double minY, double maxY, unsigned int uiMaxIterations)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");
		ThrowIfTrue(uiMaxIterations == 0, "uiMaxIterations must be at least one.");

		unsigned int num_threads = std::thread::hardware_concurrency();
		unsigned int iThread = 0;
		unsigned int iStep = _miHeight / num_threads;
		unsigned int uiYFirst = 0;
		unsigned int uiYLast = 0;
		std::vector<std::thread> threads;
		for (unsigned int i = 0; i < num_threads; i++)
		{
			uiYLast += iStep;
			threads.push_back(std::thread());
			threads[i] = std::thread(&CImage::_fillMandelbrotMT_threadMain, this, uiYFirst, uiYLast, colourRamp, minX, maxX, minY, maxY, uiMaxIterations);
			uiYFirst += iStep;
		}
		// Take up any slack
		if (uiYLast < (unsigned int)_miHeight)
		{
			uiYLast = _miHeight;
			threads.push_back(std::thread());
			threads[threads.size()-1] = std::thread(&CImage::_fillMandelbrotMT_threadMain, this, uiYFirst, uiYLast, colourRamp, minX, maxX, minY, maxY, uiMaxIterations);
			num_threads++;
		}

		for (unsigned int i = 0; i < num_threads; i++)
		{
			threads[i].join();
		}

	}

	void CImage::_fillMandelbrotMT_threadMain(unsigned int uiYFirst, unsigned int uiYLast, CColourRamp colourRamp, double minX, double maxX, double minY, double maxY, unsigned int uiMaxIterations)
	{
		// Define the complex plane boundaries

		// Calculate pixel width and height
		const double dx = (maxX - minX) / _miWidth;
		const double dy = (maxY - minY) / _miHeight;

		// Iterate over each pixel
		unsigned int iIndex;
		if (_miNumChannels == 3)
		{
			for (unsigned int y = uiYFirst; y < uiYLast; ++y)
			{
				for (int x = 0; x < _miWidth; ++x)
				{
					// Calculate the complex number for the current pixel
					std::complex<double> c(minX + x * dx, minY + y * dy);
					std::complex<double> z(0.0, 0.0);

					unsigned int iterations = 0;
					while (std::abs(z) < 2.0 && iterations < uiMaxIterations) {
						z = z * z + c;
						++iterations;
					}

					// Assign a color based on the number of iterations
					iIndex = x + (y * _miWidth);
					iIndex *= _miNumChannels;
					CColourf colour = colourRamp.getRampColour(float(iterations) / float(uiMaxIterations));

					_mpData[iIndex] = unsigned char(colour.red * 255.0f);
					_mpData[iIndex + 1] = unsigned char(colour.green * 255.0f);
					_mpData[iIndex + 2] = unsigned char(colour.blue * 255.0f);
				}
			}
		}
		else if (_miNumChannels == 4)
		{
			for (unsigned int y = uiYFirst; y < uiYLast; ++y)
			{
				for (unsigned int x = 0; x < (unsigned int)_miWidth; ++x)
				{
					// Calculate the complex number for the current pixel
					std::complex<double> c(minX + x * dx, minY + y * dy);
					std::complex<double> z(0.0, 0.0);

					unsigned int iterations = 0;
					while (std::abs(z) < 2.0 && iterations < uiMaxIterations) {
						z = z * z + c;
						++iterations;
					}

					// Assign a color based on the number of iterations
					iIndex = x + (y * _miWidth);
					iIndex *= _miNumChannels;
					CColourf colour = colourRamp.getRampColour(float(iterations) / float(uiMaxIterations));

					_mpData[iIndex] = unsigned char(colour.red * 255.0f);
					_mpData[iIndex + 1] = unsigned char(colour.green * 255.0f);
					_mpData[iIndex + 2] = unsigned char(colour.blue * 255.0f);
					_mpData[iIndex + 3] = unsigned char(colour.alpha * 255.0f);
				}
			}
		}
	}

	unsigned char* CImage::getData(void) const
	{
		return _mpData;
	}

	unsigned int CImage::getDataSize(void) const
	{
		return _muiDataSize;
	}

	unsigned int CImage::getWidth(void) const
	{
		return _miWidth;
	}

	unsigned int CImage::getHeight(void) const
	{
		return _miHeight;
	}

	CDimension2D CImage::getDimensions(void) const
	{
		CDimension2D dims(_miWidth, _miHeight);
		return dims;
	}

	CVector2f CImage::getDimensionsAsVector2f(void) const
	{
		CVector2f dims((float)_miWidth, (float)_miHeight);
		return dims;
	}

	unsigned int CImage::getNumChannels(void) const
	{
		return _miNumChannels;
	}

	bool CImage::getDimsArePowerOfTwo(void) const
	{
		int iX = 1;
		int iY = 1;
		while (iX < _miWidth)
			iX *= 2;
		while (iY < _miHeight)
			iY *= 2;
		if (iX != _miWidth || iY != _miHeight)
			return false;
		return true;
	}

	void CImage::swapRedAndBlue(void)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		unsigned int i = 0;
		int i2;
		unsigned char chTemp;
		while (i < _muiDataSize)
		{
			i2 = i + 2;
			chTemp = _mpData[i];
			_mpData[i] = _mpData[i2];
			_mpData[i2] = chTemp;
			i += _miNumChannels;
		}
	}

	void CImage::ditherBayerMatrix(void)
	{
		ThrowIfFalse(_mpData, "Image data is not available.");

		// Bayer matrix for 4x4 dithering
		const int bayerMatrix[4][4] = {
			{  0,  8,  2, 10 },
			{ 12,  4, 14,  6 },
			{  3, 11,  1,  9 },
			{ 15,  7, 13,  5 }
		};

		const int matrixSize = 4;
		const int matrixMaxValue = 16;

		for (int y = 0; y < _miHeight; ++y)
		{
			for (int x = 0; x < _miWidth; ++x)
			{
				unsigned char r, g, b, a;
				getPixel(x, y, r, g, b, a);

				// Apply Bayer matrix thresholding
				int threshold = bayerMatrix[y % matrixSize][x % matrixSize];
				r = (r > threshold * 255 / matrixMaxValue) ? 255 : 0;
				g = (g > threshold * 255 / matrixMaxValue) ? 255 : 0;
				b = (b > threshold * 255 / matrixMaxValue) ? 255 : 0;

				setPixel(x, y, r, g, b, a);
			}
		}
	}

	void CImage::ditherFloydSteinberg(void)
	{
		ThrowIfFalse(_mpData, "Image data is not available.");

		for (int y = 0; y < _miHeight; ++y)
		{
			for (int x = 0; x < _miWidth; ++x)
			{
				unsigned char r, g, b, a;
				getPixel(x, y, r, g, b, a);

				// Find the closest color (0 or 255)
				unsigned char newR = (r > 127) ? 255 : 0;
				unsigned char newG = (g > 127) ? 255 : 0;
				unsigned char newB = (b > 127) ? 255 : 0;

				// Calculate the error
				int errR = r - newR;
				int errG = g - newG;
				int errB = b - newB;

				// Set the new pixel value
				setPixel(x, y, newR, newG, newB, a);

				// Distribute the error to neighboring pixels
				if (x + 1 < _miWidth)
				{
					_ditherFloydSteinbergAddError(x + 1, y, errR, errG, errB, 7.0 / 16.0);
				}
				if (x - 1 >= 0 && y + 1 < _miHeight)
				{
					_ditherFloydSteinbergAddError(x - 1, y + 1, errR, errG, errB, 3.0 / 16.0);
				}
				if (y + 1 < _miHeight)
				{
					_ditherFloydSteinbergAddError(x, y + 1, errR, errG, errB, 5.0 / 16.0);
				}
				if (x + 1 < _miWidth && y + 1 < _miHeight)
				{
					_ditherFloydSteinbergAddError(x + 1, y + 1, errR, errG, errB, 1.0 / 16.0);
				}
			}
		}
	}

	void CImage::_ditherFloydSteinbergAddError(int x, int y, int errR, int errG, int errB, double factor)
	{
		unsigned char r, g, b, a;
		getPixel(x, y, r, g, b, a);

		int newR = r + static_cast<int>(errR * factor);
		int newG = g + static_cast<int>(errG * factor);
		int newB = b + static_cast<int>(errB * factor);

		// Clamp the values to the valid range [0, 255]
		newR = std::clamp(newR, 0, 255);
		newG = std::clamp(newG, 0, 255);
		newB = std::clamp(newB, 0, 255);

		setPixel(x, y, static_cast<unsigned char>(newR), static_cast<unsigned char>(newG), static_cast<unsigned char>(newB), a);
	}

	void CImage::flipVertically(void)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		// Size of a row
		unsigned int iRowSize = _miWidth * _miNumChannels;

		// Allocate new flipped image
		unsigned char* pNewImageStartAddress = new unsigned char[_muiDataSize];
		unsigned char* pNewImage = pNewImageStartAddress;
		ThrowIfTrue(0 == pNewImage, "Failed to allocate memory.");

		// Get pointer to current image
		unsigned char* pOldImage = _mpData;
		// Increment old image pointer to point to last row
		pOldImage += iRowSize * (_miHeight - 1);

		// Copy each row into new image
		unsigned int iRowSizeBytes = iRowSize * sizeof(unsigned char);
		for (int iRow = 0; iRow < _miHeight; ++iRow)
		{
			memcpy(pNewImage, pOldImage, iRowSizeBytes);
			// Adjust pointers
			pNewImage += iRowSizeBytes;
			pOldImage -= iRowSizeBytes;
		}
		// Now pNewImage contains flipped image data
		delete[] _mpData;	// Delete old image data
		_mpData = pNewImageStartAddress;	// Make image data point to the new data
	}

	void CImage::invert(bool bInvertColour, bool bInvertAlpha)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		unsigned int i = 0;
		int iIndex;
		if (bInvertColour)
		{
			while (i < _muiDataSize)
			{
				iIndex = i;
				_mpData[iIndex] = 255 - _mpData[iIndex]; ++iIndex;
				_mpData[iIndex] = 255 - _mpData[iIndex]; ++iIndex;
				_mpData[iIndex] = 255 - _mpData[iIndex];
				i += _miNumChannels;
			}
		}

		if (_miNumChannels == 4 && bInvertAlpha)
		{
			i = 3;
			while (i < _muiDataSize)
			{
				_mpData[i] = 255 - _mpData[i];
				i += _miNumChannels;
			}
		}
	}

	void CImage::greyscaleSimple(void)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		unsigned int i = 0;
		float f1Over3 = 1.0f / 3.0f;
		float fTmp;
		unsigned char cTmp;
		while (i < _muiDataSize)
		{
			fTmp = float(_mpData[i]);
			fTmp += float(_mpData[i + 1]);
			fTmp += float(_mpData[i + 2]);
			fTmp *= f1Over3;
			cTmp = (unsigned char)fTmp;
			_mpData[i] = cTmp;
			_mpData[i + 1] = cTmp;
			_mpData[i + 2] = cTmp;
			i += _miNumChannels;
		}
	}


	void CImage::greyscale(float fRedSensitivity, float fGreenSensitivity, float fBlueSensitivity)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		CVector3f vCol(fRedSensitivity, fGreenSensitivity, fBlueSensitivity);

		unsigned int i = 0;
		float fTmp;
		unsigned char cTmp;
		while (i < _muiDataSize)
		{
			fTmp = 0.0f;
			fTmp = float(_mpData[i]) * vCol.x;
			fTmp += float(_mpData[i + 1]) * vCol.y;
			fTmp += float(_mpData[i + 2]) * vCol.z;
			cTmp = (unsigned char)fTmp;
			_mpData[i] = cTmp;
			_mpData[i + 1] = cTmp;
			_mpData[i + 2] = cTmp;
			i += _miNumChannels;
		}
	}

	void CImage::adjustBrightness(int iAmount)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		unsigned int i = 0;
		int iCol;
		while (i < _muiDataSize)
		{
			iCol = (int)_mpData[i] + iAmount;
			clamp(iCol, 0, 255);
			_mpData[i] = unsigned char(iCol);

			iCol = (int)_mpData[i + 1] + iAmount;
			clamp(iCol, 0, 255);
			_mpData[i + 1] = unsigned char(iCol);

			iCol = (int)_mpData[i + 2] + iAmount;
			clamp(iCol, 0, 255);
			_mpData[i + 2] = unsigned char(iCol);
			i += _miNumChannels;
		}
	}

	void CImage::adjustContrast(int iAmount)
	{
		ThrowIfTrue(!_mpData, "Image not yet created.");

		clamp(iAmount, -100, 100);
		double dPixel;
		double d1Over255 = 1.0 / 255.0;
		double dContrast = (100.0 + double(iAmount)) * 0.01; // 0 and 2
		dContrast *= dContrast;	// 0 and 4
		unsigned int i = 0;
		int iIndex;
		while (i < _muiDataSize)
		{
			iIndex = i;
			dPixel = double(_mpData[iIndex]) * d1Over255;
			dPixel -= 0.5;
			dPixel *= dContrast;
			dPixel += 0.5;
			dPixel *= 255;
			clamp(dPixel, 0.0, 255.0);
			_mpData[iIndex] = unsigned char(dPixel);
			++iIndex;

			dPixel = double(_mpData[iIndex]) * d1Over255;
			dPixel -= 0.5;
			dPixel *= dContrast;
			dPixel += 0.5;
			dPixel *= 255;
			clamp(dPixel, 0.0, 255.0);
			_mpData[iIndex] = unsigned char(dPixel);
			++iIndex;

			dPixel = double(_mpData[iIndex]) * d1Over255;
			dPixel -= 0.5;
			dPixel *= dContrast;
			dPixel += 0.5;
			dPixel *= 255;
			clamp(dPixel, 0.0, 255.0);
			_mpData[iIndex] = unsigned char(dPixel);

			i += _miNumChannels;
		}
	}

	void CImage::copyTo(CImage& destImage) const
	{
		ThrowIfTrue(!_mpData, "Source image not yet created.");

		// If destination image is the same as this one, do nothing
		if (destImage._mpData == this->_mpData)
			return;

		destImage.free();
		destImage.createBlank(_miWidth, _miHeight, _miNumChannels);
		memcpy(destImage._mpData, _mpData, sizeof(unsigned char) * _muiDataSize);
	}

	void CImage::copyRectTo(CImage& destImage, int iSrcPosX, int iSrcPosY, int iSrcWidth, int iSrcHeight, int iDestPosX, int iDestPosY) const
	{
		// Check that both images have data
		ThrowIfTrue(!_mpData, "Source image not yet created.");
		ThrowIfTrue(!destImage._mpData, "Destination image not yet created.");

		// Compute source rect
		int iSrcLeft = iSrcPosX;
		int iSrcBot = iSrcPosY;
		int iSrcRight = iSrcLeft + iSrcWidth;
		int iSrcTop = iSrcBot + iSrcHeight;
		// Compute destination rect
		int iDstLeft = iDestPosX;
		int iDstBot = iDestPosY;
		int iDstRight = iDstLeft + iSrcWidth;
		int iDstTop = iDstBot + iSrcHeight;

		// The above may be invalid due to different sizes, invalid positions, dims etc.
		// Invalid starting positions
		if (iSrcLeft >= _miWidth)
			return;
		if (iSrcBot >= _miHeight)
			return;
		if (iDstLeft >= destImage._miWidth)
			return;
		if (iDstBot >= destImage._miHeight)
			return;
		// Clamp right and top to edges of their respective images
		clamp(iSrcRight, iSrcLeft, _miWidth);
		clamp(iSrcTop, iSrcBot, _miHeight);
		clamp(iDstRight, iDstLeft, destImage._miWidth);
		clamp(iDstTop, iDstBot, destImage._miHeight);
		// Compute rect dims for both images
		unsigned int iSrcRectWidth = iSrcRight - iSrcLeft;
		unsigned int iSrcRectHeight = iSrcTop - iSrcBot;
		unsigned int iDstRectWidth = iDstRight - iDstLeft;
		unsigned int iDstRectHeight = iDstTop - iDstBot;
		// Compute smallest rect
		unsigned int iMinWidth = iSrcRectWidth;
		if (iMinWidth > iDstRectWidth)
			iMinWidth = iDstRectWidth;
		unsigned int iMinHeight = iSrcRectHeight;
		if (iMinHeight > iDstRectHeight)
			iMinHeight = iDstRectHeight;
		// If minimum = zero, then do nothing
		if (iMinWidth == 0)
			return;
		if (iMinHeight == 0)
			return;

		unsigned char colTmp[4];
		unsigned int isx, isy;
		unsigned int idx, idy;
		for (unsigned int ix = 0; ix < iMinWidth; ++ix)
		{
			for (unsigned int iy = 0; iy < iMinHeight; ++iy)
			{
				isx = iSrcLeft + ix;
				isy = iSrcBot + iy;
				idx = iDstLeft + ix;
				idy = iDstBot + iy;
				getPixel(isx, isy, colTmp[0], colTmp[1], colTmp[2], colTmp[3]);
				destImage.setPixel(idx, idy, colTmp[0], colTmp[1], colTmp[2], colTmp[3]);
			}
		}
	}

	void CImage::copyToAddBorder(CImage& outputImage) const
	{
		ThrowIfTrue(!_mpData, "Image data doesn't exist.");

		// Compute new larger dimensions and create the larger image
		int newWidth = _miWidth + 2;
		int newHeight = _miHeight + 2;
		outputImage.createBlank(newWidth, newHeight, _miNumChannels);

		// Copy this image to the centre of the larger image
		copyRectTo(outputImage, 0, 0, _miWidth, _miHeight, 1, 1);

		// Now copy the edges of this image to the destination image
		unsigned char r, g, b, a;
		int heightOfOutputImageMinusOne = newHeight - 1;
		// Top and bottom edges
		for (int iX = 0; iX < _miWidth; iX++)
		{
			// Top pixel row
			getPixel(iX, 0, r, g, b, a);
			outputImage.setPixel(iX + 1, 0, r, g, b, a);

			// Bottom pixel row
			getPixel(iX, _miHeight - 1, r, g, b, a);
			outputImage.setPixel(iX + 1, heightOfOutputImageMinusOne, r, g, b, a);
		}
		int widthOfOutputImageMinusOne = newWidth - 1;
		// Left and right edges
		for (int iY = 0; iY < _miHeight; iY++)
		{
			// Left pixel column
			getPixel(0, iY, r, g, b, a);
			outputImage.setPixel(0, iY + 1, r, g, b, a);

			// Right pixel column
			getPixel(_miWidth - 1, iY, r, g, b, a);
			outputImage.setPixel(widthOfOutputImageMinusOne, iY + 1, r, g, b, a);
		}
	}

	void CImage::rotateClockwise(void)
	{
		CImage oldImage;
		copyTo(oldImage);

		unsigned char col[4];
		int idstX;
		int idstY;

		// Non squared?
		if (_miWidth != _miHeight)
		{
			createBlank(_miHeight, _miWidth, _miNumChannels);
		}

		for (int isrcX = 0; isrcX < oldImage._miWidth; ++isrcX)
		{
			idstY = _miHeight - isrcX - 1;
			for (int isrcY = 0; isrcY < oldImage._miHeight; ++isrcY)
			{
				idstX = isrcY;
				oldImage.getPixel(isrcX, isrcY, col[0], col[1], col[2], col[3]);
				setPixel(idstX, idstY, col[0], col[1], col[2], col[3]);
			}
		}
	}

	void CImage::edgeDetect(CImage& outputImage, unsigned char r, unsigned char g, unsigned char b)
	{
		ThrowIfTrue(!_mpData, "Image data doesn't exist.");
		ThrowIfTrue(_miNumChannels < 3, "Some image data exists, but doesn't have enough colour channels.");

		outputImage.createBlank(_miWidth, _miHeight, 4);
		int iX = 0;
		int iY = 0;
		while (iX < (int)_miWidth)
		{
			while (iY < (int)_miHeight)
			{
				if (_isPixelEdge(iX, iY, r, g, b))
					outputImage.setPixel(iX, iY, 255, 255, 255, 255);
				else
					outputImage.setPixel(iX, iY, 0, 0, 0, 0);
				++iY;
			}
			++iX;
			iY = 0;
		}
	}

	void CImage::removeAlphaChannel(void)
	{
		ThrowIfTrue(!_mpData, "Image data doesn't exist.");
		ThrowIfTrue(_miNumChannels != 4, "Some image data exists, but the alpha data doesn't exist (Image doesn't hold 4 channels)");

		// Copy this image to a new tmp image
		CImage old;
		copyTo(old);

		// Recreate this one, but with 3 channels
		createBlank(old.getWidth(), old.getHeight(), 3);

		// Copy RGB from old to this...
		unsigned int iIndex = 0;
		int iIndexOld = 0;
		while (iIndex < _muiDataSize)
		{
			_mpData[iIndex] = old._mpData[iIndexOld];		// Red
			_mpData[iIndex + 1] = old._mpData[iIndexOld + 1];	// Green
			_mpData[iIndex + 2] = old._mpData[iIndexOld + 2];	// Blue
			iIndex += 3;
			iIndexOld += 4;
		}
	}

	void CImage::addAlphaChannel(unsigned char ucAlpha)
	{
		ThrowIfTrue(!_mpData, "Image data doesn't exist.");

		// Simply overwrite alpha channel values with the one passed in
		if (4 == _miNumChannels)
		{
			unsigned int iIndex = 0;
			while (iIndex < _muiDataSize)
			{
				_mpData[iIndex + 3] = ucAlpha;
				iIndex += 4;
			}
			return;
		}
		else if (3 == _miNumChannels)
		{
			// Copy this image to a new tmp image
			CImage old;
			copyTo(old);
			// Recreate this one, but with 4 channels
			createBlank(old.getWidth(), old.getHeight(), 4);
			// Copy RGB from old to this...
			unsigned int iIndex = 0;
			int iIndexOld = 0;
			while (iIndex < _muiDataSize)
			{
				_mpData[iIndex] = old._mpData[iIndexOld];			// Red
				_mpData[iIndex + 1] = old._mpData[iIndexOld + 1];	// Green
				_mpData[iIndex + 2] = old._mpData[iIndexOld + 2];	// Blue
				_mpData[iIndex + 3] = ucAlpha;	// Alpha
				iIndex += 4;
				iIndexOld += 3;
			}
			return;
		}
		else
		{
			Throw("Image doesn't have 3 or 4 channels.");
		}
	}

	void CImage::copyAlphaChannelToRGB(void)
	{
		ThrowIfTrue(!_mpData, "Image data doesn't exist.");
		ThrowIfTrue(_miNumChannels != 4, "Some image data exists, but the alpha data doesn't exist (Image doesn't hold 4 channels)");

		unsigned int iIndex = 0;
		while (iIndex < _muiDataSize)
		{
			_mpData[iIndex] = _mpData[iIndex + 3];	// Red
			_mpData[iIndex + 1] = _mpData[iIndex + 3];	// Green
			_mpData[iIndex + 2] = _mpData[iIndex + 3];	// Blue
			iIndex += 4;
		}
	}

	void CImage::normalmap(CImage& outputImage, float fScale) const
	{
		ThrowIfTrue(!_mpData, "Image data doesn't exist.");

		clamp(fScale, 0.0f, 1.0f);

		// Copy this image into a new one so this is left unaffected.
		// This uses the copyToAddBorder() method which adds a border and copies the edge pixels to the new pixels in the border.
		// This makes it so we don't have to mess around with edge cases.
		CImage imageGreyscale;
		copyToAddBorder(imageGreyscale);

		// Greyscale the image
		imageGreyscale.greyscaleSimple();

		// Create output image with the same size as this one
		outputImage.createBlank(_miWidth, _miHeight, 3);

		// Now loop through greyscale image, computing each normal and storing in the output image.
		unsigned char r[3], g[3], b[3], a;
		float fX, fY, fZ;
		float fLength;
		for (int y = 0; y < _miHeight; y++)
		{
			for (int ix = 0; ix < _miWidth; ix++)
			{
				// we add +1 to imageGreyscale pixel positions as it has a border

				// Get height values of centre and surrounding pixels
				imageGreyscale.getPixel(ix + 1, y + 1, r[0], g[0], b[0], a);	// Current pixel
				imageGreyscale.getPixel(ix, y + 1, r[1], g[1], b[1], a);		// Left pixel
				imageGreyscale.getPixel(ix + 1, y + 2, r[2], g[2], b[2], a);	// Above pixel

				fX = float(r[1] - r[0]) / 255.0f;	// Convert to -1.0f to 1.0f
				fY = float(r[2] - r[0]) / 255.0f;	// ....
				fZ = fScale;

				// Compute length of vector and normalize
				fLength = sqrt((fX * fX) + (fY * fY) + (fZ * fZ));
				if (areFloatsEqual(fLength, 0.0f))	// If length is nearly zero, just set as up vector
				{
					fX = 0.0f;
					fY = 0.0f;
					fZ = fScale;
				}
				else
				{
					fX = fX / fLength;
					fY = fY / fLength;
					fZ = fZ / fLength;
				}

				// Convert from -1, +1 to 0, 255
				fX += 1.0f;	fX *= 127.0f;
				fY += 1.0f;	fY *= 127.0f;
				fZ += 1.0f;	fZ *= 127.0f;
				r[0] = unsigned char(fX);
				g[0] = unsigned char(fY);
				b[0] = unsigned char(fZ);
				outputImage.setPixel(ix, y, r[0], g[0], b[0], a);
			}
		}
	}

	void CImage::createColourWheel(unsigned int iWidthAndHeightOfImage, unsigned char ucBrightness)
	{
		ThrowIfTrue(iWidthAndHeightOfImage < 1, "Parsed iWidthAndHeightOfImage must be at least 1");
		createBlank(iWidthAndHeightOfImage, iWidthAndHeightOfImage, 4);

		float fBrightness = float(ucBrightness) / 255.0f;
		CVector2f vCentrePixelPosition;
		vCentrePixelPosition.x = float(iWidthAndHeightOfImage) * 0.5f;
		vCentrePixelPosition.y = vCentrePixelPosition.x;

		CVector2f vCurrentPixelPosition;
		CVector2f vCurrentPixelOffsetFromCentre;
		CColourf colour;
		float fCircleRadius = float(iWidthAndHeightOfImage) * 0.5f;
		float fDistanceFromCentre;
		float fSaturation;	// 0.0f = white, 1.0f = full colour
		float fAngleDegrees;
		float fOneOver360 = 1.0f / 360.0f;
		unsigned int iPixelIndex = 0;
		for (unsigned int iPosX = 0; iPosX < (unsigned int)_miWidth; iPosX++)
		{
			vCurrentPixelPosition.x = (float)iPosX;
			for (unsigned int iPosY = 0; iPosY < (unsigned int)_miHeight; iPosY++)
			{
				vCurrentPixelPosition.y = (float)iPosY;
				vCurrentPixelOffsetFromCentre = vCurrentPixelPosition - vCentrePixelPosition;
				fDistanceFromCentre = vCurrentPixelOffsetFromCentre.getMagnitude();
				fSaturation = fCircleRadius - fDistanceFromCentre;
				fSaturation /= fCircleRadius;	// 0 at edge of circle, 1 at centre. Can be < 0 which is outside circle
				fAngleDegrees = vCurrentPixelOffsetFromCentre.getAngleDegrees360();
				fAngleDegrees *= fOneOver360;	// 0 when pixel is north, 0.25 when east etc.
				if (fSaturation < 0.0f)
					colour.set(0.0f, 0.0f, 0.0f, 0.0f);
				else
				{
					colour.setFromHSB(fAngleDegrees, fSaturation, fBrightness);
					colour.alpha = 1.0f;
				}
				_mpData[iPixelIndex] = unsigned char(colour.red * 255);
				_mpData[iPixelIndex + 1] = unsigned char(colour.green * 255);
				_mpData[iPixelIndex + 2] = unsigned char(colour.blue * 255);
				_mpData[iPixelIndex + 3] = unsigned char(colour.alpha * 255);
				iPixelIndex += 4;
			}
		}
	}

	CColourf CImage::getColourWheelColour(unsigned int iPositionX, unsigned int iPositionY, unsigned int iWidthAndHeightOfImage, unsigned char ucBrightness)
	{
		ThrowIfTrue(iWidthAndHeightOfImage < 1, "Parsed iWidthAndHeightOfImage must be at least 1");

		CColourf colour;
		CVector2f vCurrentPixelPosition((float)iPositionX, float(iPositionY));
		CVector2f vCentrePixelPosition;
		vCentrePixelPosition.x = float(iWidthAndHeightOfImage) * 0.5f;
		vCentrePixelPosition.y = vCentrePixelPosition.x;
		CVector2f vCurrentPixelOffsetFromCentre = vCurrentPixelPosition - vCentrePixelPosition;
		float fDistanceFromCentre = vCurrentPixelOffsetFromCentre.getMagnitude();
		float fCircleRadius = float(iWidthAndHeightOfImage) * 0.5f;
		float fSaturation = fCircleRadius - fDistanceFromCentre;
		fSaturation /= fCircleRadius;	// 0 at edge of circle, 1 at centre. Can be < 0 which is outside circle
		float fAngleDegrees = vCurrentPixelOffsetFromCentre.getAngleDegrees360();
		fAngleDegrees /= 360.0f;	// 0 when pixel is north, 0.25 when east etc.
		if (fSaturation < 0.0f)
			colour.set(0.0f, 0.0f, 0.0f, 0.0f);
		else
		{
			colour.setFromHSB(fAngleDegrees, fSaturation, float(ucBrightness) / 255.0f);
			colour.alpha = 1.0f;
		}
		return colour;
	}

	void CImage::createGradient(unsigned int iWidth, unsigned int iHeight, unsigned int iNumChannels, const CColourf& colour0, const CColourf& colour1)
	{
		ThrowIfTrue(iWidth < 1 || iHeight < 1, "Invalid dimensions given.");
		ThrowIfTrue(iNumChannels < 3 || iNumChannels > 4, "Number of channels must be either 3 or 4.");
		createBlank(iWidth, iHeight, iNumChannels);
		bool bHorizontal = true;
		if (iHeight > iWidth)
			bHorizontal = false;

		CColourf colour;
		unsigned int iPixelIndex = 0;
		if (bHorizontal)
		{
			for (unsigned int iPosX = 0; iPosX < iWidth; iPosX++)
			{
				colour = colour0.interpolate(colour1, float(iPosX) / float(iWidth));
				for (unsigned int iPosY = 0; iPosY < iHeight; iPosY++)
				{
					iPixelIndex = iPosX + (iPosY * _miWidth);
					iPixelIndex *= iNumChannels;
					_mpData[iPixelIndex] = unsigned char(colour.red * 255);
					_mpData[iPixelIndex + 1] = unsigned char(colour.green * 255);
					_mpData[iPixelIndex + 2] = unsigned char(colour.blue * 255);
					if (iNumChannels == 4)
						_mpData[iPixelIndex + 3] = unsigned char(colour.alpha * 255);
				}
			}
		}
		else
		{
			for (unsigned int iPosY = 0; iPosY < iHeight; iPosY++)
			{
				colour = colour0.interpolate(colour1, float(iPosY) / float(iHeight));
				for (unsigned int iPosX = 0; iPosX < iWidth; iPosX++)
				{
					iPixelIndex = iPosX + (iPosY * _miWidth);
					iPixelIndex *= iNumChannels;
					_mpData[iPixelIndex] = unsigned char(colour.red * 255);
					_mpData[iPixelIndex + 1] = unsigned char(colour.green * 255);
					_mpData[iPixelIndex + 2] = unsigned char(colour.blue * 255);
					if (iNumChannels == 4)
						_mpData[iPixelIndex + 3] = unsigned char(colour.alpha * 255);
				}
			}
		}
	}

	void CImage::createCircle(unsigned int iWidthAndHeightOfImage, const CColourf& colourInner, const CColourf& colourOuter)
	{
		ThrowIfTrue(iWidthAndHeightOfImage < 1, "Parsed iWidthAndHeightOfImage must be at least 1");
		createBlank(iWidthAndHeightOfImage, iWidthAndHeightOfImage, 4);

		CVector2f vCentrePixelPosition;
		vCentrePixelPosition.x = float(iWidthAndHeightOfImage) * 0.5f;
		vCentrePixelPosition.y = vCentrePixelPosition.x;

		CVector2f vCurrentPixelPosition;
		CVector2f vCurrentPixelOffsetFromCentre;
		CColourf colour;
		float fCircleRadius = float(iWidthAndHeightOfImage) * 0.5f;
		float fDistanceFromCentre;
		float fOneOver360 = 1.0f / 360.0f;
		unsigned int iPixelIndex = 0;
		for (unsigned int iPosX = 0; iPosX < (unsigned int)_miWidth; iPosX++)
		{
			vCurrentPixelPosition.x = (float)iPosX;
			for (unsigned int iPosY = 0; iPosY < (unsigned int)_miHeight; iPosY++)
			{
				vCurrentPixelPosition.y = (float)iPosY;
				vCurrentPixelOffsetFromCentre = vCurrentPixelPosition - vCentrePixelPosition;
				fDistanceFromCentre = fCircleRadius - vCurrentPixelOffsetFromCentre.getMagnitude();
				fDistanceFromCentre /= fCircleRadius;	// 0 at edge of circle, 1 at centre. Can be < 0 which is outside circle
				if (fDistanceFromCentre < 0.0f)
					colour.set(0.0f, 0.0f, 0.0f, 0.0f);
				else
					colour = colourOuter.interpolate(colourInner, fDistanceFromCentre);
				_mpData[iPixelIndex] = unsigned char(colour.red * 255);
				_mpData[iPixelIndex + 1] = unsigned char(colour.green * 255);
				_mpData[iPixelIndex + 2] = unsigned char(colour.blue * 255);
				_mpData[iPixelIndex + 3] = unsigned char(colour.alpha * 255);
				iPixelIndex += 4;
			}
		}
	}

	void CImage::helper_ExtractImagesFromSpriteSheet(const std::string& strSpritesheetImageFilename, const std::string& strOutputfilenameBase, CDimension2D dimensionsOfEachIndividualImage)
	{
		CImage input, output;
		
		if (!input.load(strSpritesheetImageFilename))
			Throw("Unable to load sprite sheet image.");

		output.createBlank(dimensionsOfEachIndividualImage.width, dimensionsOfEachIndividualImage.height, input.getNumChannels());
		std::string strOutputFilename;

		int iIndex = 0;
		// Move along the spritesheet from left to right, then down one and repeat from left to right until end reached
		for (int iy=0; iy < (int)input.getHeight(); iy += dimensionsOfEachIndividualImage.height)
		{
			for (int ix = 0; ix < (int)input.getWidth(); ix += dimensionsOfEachIndividualImage.width)
			{
				// Copy individual image from sprite sheet into output image
				input.copyRectTo(output, ix, iy, dimensionsOfEachIndividualImage.width, dimensionsOfEachIndividualImage.height, 0, 0);

				// Compute name of output image filename
				strOutputFilename = StringUtils::blenderAnimFilename(strOutputfilenameBase, ".png", iIndex);
				iIndex++;

				// Save out the file
				output.saveAsPNG(strOutputFilename, false);
			}
		}
	}

	bool CImage::_loadDIF(const std::string& strFilename, bool bFlipForOpenGL)
	{
		// Open the file in binary mode
		std::ifstream file(strFilename, std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}

		// Read the magic number "DIF\0"
		unsigned char cMagic[4];
		file.read(reinterpret_cast<char*>(cMagic), 4);
		if (cMagic[0] != 'D' || cMagic[1] != 'I' || cMagic[2] != 'F' || cMagic[3] != 0)
		{
			return false;
		}

		// Read the width, height, number of channels, and data size
		size_t width, height, dataSize;
		unsigned char numChannels;
		file.read(reinterpret_cast<char*>(&width), sizeof(width));
		file.read(reinterpret_cast<char*>(&height), sizeof(height));
		file.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));
		file.read(reinterpret_cast<char*>(&dataSize), sizeof(dataSize));

		// Validate the read values
		if (width == 0 || height == 0 || numChannels < 1 || numChannels > 4 || dataSize != width * height * numChannels)
		{
			return false;
		}

		// Allocate memory for the image data
		_miWidth = static_cast<int>(width);
		_miHeight = static_cast<int>(height);
		_miNumChannels = static_cast<int>(numChannels);
		_muiDataSize = static_cast<unsigned int>(dataSize);
		_mpData = new unsigned char[_muiDataSize];
		if (!_mpData)
		{
			return false;
		}

		// Read the image data
		file.read(reinterpret_cast<char*>(_mpData), _muiDataSize);

		// Read the ending magic number "DIF\0"
		file.read(reinterpret_cast<char*>(cMagic), 4);
		if (cMagic[0] != 'D' || cMagic[1] != 'I' || cMagic[2] != 'F' || cMagic[3] != 0)
		{
			delete[] _mpData;
			_mpData = nullptr;
			return false;
		}

		// Close the file
		file.close();

		// Flip the image vertically if required
		if (bFlipForOpenGL)
		{
			flipVertically();
		}

		return true;
	}

	bool CImage::_loadInfoDIF(const std::string& strFilename, int& iWidth, int& iHeight, int& iNumChannels)
	{
		// Open the file in binary mode
		std::ifstream file(strFilename, std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}

		// Read the magic number "DIF\0"
		unsigned char cMagic[4];
		file.read(reinterpret_cast<char*>(cMagic), 4);
		if (cMagic[0] != 'D' || cMagic[1] != 'I' || cMagic[2] != 'F' || cMagic[3] != 0)
		{
			return false;
		}

		// Read the width, height, and number of channels
		size_t width, height;
		unsigned char numChannels;
		file.read(reinterpret_cast<char*>(&width), sizeof(width));
		file.read(reinterpret_cast<char*>(&height), sizeof(height));
		file.read(reinterpret_cast<char*>(&numChannels), sizeof(numChannels));

		// Validate the read values
		if (width == 0 || height == 0 || numChannels < 1 || numChannels > 4)
		{
			return false;
		}

		// Set the output parameters
		iWidth = static_cast<int>(width);
		iHeight = static_cast<int>(height);
		iNumChannels = static_cast<int>(numChannels);

		// Close the file
		file.close();

		return true;
	}

	/// \brief Save image to DIF file to disk
		///
		/// \param strFilename The filename to save the image data to
		/// \param bFlipOnSave If true, will flip the data vertically upon saving (Not the data in memory, just what's stored in the file)
		/// 
		/// Throws exception if image contains no data or saving fails.
	void CImage::saveAsDIF(const std::string& strFilename, bool bFlipOnSave)
	{
		ThrowIfTrue(!_mpData, "CImage::saveAsDIF() failed. Image not yet created.");

		// Flip the image vertically if required
		if (bFlipOnSave)
		{
			flipVertically();
		}

		// Open the file in binary mode
		std::ofstream file(strFilename, std::ios::binary);
		ThrowIfFalse(file.is_open(), "Failed to open file: " + strFilename);

		// Write the magic number "DIF\0"
		unsigned char cMagic[4] = { 'D', 'I', 'F', 0 };
		file.write(reinterpret_cast<const char*>(cMagic), 4);

		// Write the width, height, number of channels, and data size
		size_t width = static_cast<size_t>(_miWidth);
		size_t height = static_cast<size_t>(_miHeight);
		unsigned char numChannels = static_cast<unsigned char>(_miNumChannels);
		size_t dataSize = static_cast<size_t>(_muiDataSize);

		file.write(reinterpret_cast<const char*>(&width), sizeof(width));
		file.write(reinterpret_cast<const char*>(&height), sizeof(height));
		file.write(reinterpret_cast<const char*>(&numChannels), sizeof(numChannels));
		file.write(reinterpret_cast<const char*>(&dataSize), sizeof(dataSize));

		// Write the image data
		file.write(reinterpret_cast<const char*>(_mpData), _muiDataSize);

		// Write the ending magic number "DIF\0"
		file.write(reinterpret_cast<const char*>(cMagic), 4);

		// Close the file
		file.close();

		// Flip the image back if it was flipped for saving
		if (bFlipOnSave)
		{
			flipVertically();
		}
	}

	bool CImage::resize(unsigned int iNewWidth, unsigned int iNewHeight)
	{
		if (!_mpData)	// Image not yet created
			return false;

		// If the new dimensions are the same as the current ones, do nothing
		if (iNewWidth == _miWidth && iNewHeight == _miHeight)
			return true;
		
		// If the new dimensions are invalid
		if (iNewWidth < 1 || iNewHeight < 1)
			return false;

		// Set pixel layout for the resize function
		stbir_pixel_layout pixel_layout;
		if (3 == _miNumChannels)
			pixel_layout = stbir_pixel_layout::STBIR_RGB;
		else if (4 == _miNumChannels)
			pixel_layout = STBIR_RGBA;
		else
			return false;

		// Create a new image with the new dimensions
		// This will hold the resized image data
		CImage newImage;
		newImage.createBlank(iNewWidth, iNewHeight, _miNumChannels);

		// Resize the image
		unsigned char* result = stbir_resize_uint8_srgb(
			_mpData,			// Pointer to the image data
			_miWidth,			// Source image width
			_miHeight,			// Source image height
			0,					// Input stride	in bytes
			newImage._mpData,	// Pointer to the new image data
			(int)iNewWidth,		// Destination image width
			(int)iNewHeight,	// Destination image height
			0,					// Output stride in bytes
			pixel_layout);		// Number of channels

		if (0 == result)
			return false;

		// Copy the new image to this one
		newImage.copyTo(*this);
		return true;
	}

	bool CImage::saveAsICO(const std::string& strFilename) const
	{
		if (!_mpData)
			return false;

		// Create a source image from this one, ensuring it has an alpha channel
		CImage imageSourceWithAlpha;
		this->copyTo(imageSourceWithAlpha);
		if (3 == imageSourceWithAlpha.getNumChannels())
		{
			imageSourceWithAlpha.addAlphaChannel(255);
		}

		// Desired icon sizes
		std::vector<int> iconSizes = { 16, 32, 48, 64, 128, 256 };

		// Will hold the image data as BMP or PNG for each size image
		std::vector<std::vector<uint8_t>> vecIcoDataForImages;

		// For each icon size
		for (int size : iconSizes)
		{
			// Copy the source image to a new image which will hold the resized image
			CImage imageResized;
			imageSourceWithAlpha.copyTo(imageResized);

			// Resize the image
			if (!imageResized.resize(size, size))
				return false;

			// Create ICO image data
			std::vector<uint8_t> icoData;
			icoData = _icoCreatePNGData(imageResized.getData(), size, size);

			// Add the ICO image data to vecIcoDataForImages
			vecIcoDataForImages.push_back(std::move(icoData));

		}

		// Change filename to have the .ico extension
		std::string strOutputFilename = StringUtils::addFilenameExtension(".ico", strFilename);
		
		// vecIcoDataForImages now holds the ICO image data for each of the image sizes.
		std::ofstream ofs(strOutputFilename, std::ios::binary);
		if (!ofs)
			return false;

		ICONDIR iconDir = {};
		iconDir.idReserved = 0;
		iconDir.idType = 1; // Icon resource
		iconDir.idCount = static_cast<uint16_t>(vecIcoDataForImages.size());

		// Calculate the image offset
		uint32_t imageOffset = uint32_t(sizeof(ICONDIR) + sizeof(ICONDIRENTRY) * vecIcoDataForImages.size());

		// Write ICONDIR
		ofs.write(reinterpret_cast<const char*>(&iconDir), sizeof(ICONDIR));

		// Write ICONDIRENTRY for each image
		for (size_t i = 0; i < vecIcoDataForImages.size(); ++i)
		{
			ICONDIRENTRY entry = {};
			int size = iconSizes[i];
			entry.bWidth = (size == 256) ? 0 : static_cast<uint8_t>(size);
			entry.bHeight = (size == 256) ? 0 : static_cast<uint8_t>(size);
			entry.bColorCount = 0; // 256 or more colors
			entry.bReserved = 0;
			entry.wPlanes = 1;
			entry.wBitCount = 32;
			entry.dwBytesInRes = static_cast<uint32_t>(vecIcoDataForImages[i].size());
			entry.dwImageOffset = imageOffset;

			ofs.write(reinterpret_cast<const char*>(&entry), sizeof(ICONDIRENTRY));
			imageOffset += entry.dwBytesInRes;
		}

		// Write Image Data
		for (const auto& img : vecIcoDataForImages) {
			ofs.write(reinterpret_cast<const char*>(img.data()), img.size());
		}

		ofs.close();
		return true;
	}

	std::vector<uint8_t> CImage::_icoCreateBMPData(const uint8_t* pixels, int width, int height) const
	{
		// Define BITMAPINFOHEADER structure
#pragma pack(push, 1)
		struct BITMAPINFOHEADER {
			uint32_t biSize;
			int32_t  biWidth;
			int32_t  biHeight;
			uint16_t biPlanes;
			uint16_t biBitCount;
			uint32_t biCompression;
			uint32_t biSizeImage;
			int32_t  biXPelsPerMeter;
			int32_t  biYPelsPerMeter;
			uint32_t biClrUsed;
			uint32_t biClrImportant;
		};
#pragma pack(pop)

		BITMAPINFOHEADER bih = {};
		bih.biSize = sizeof(BITMAPINFOHEADER);
		bih.biWidth = width;
		bih.biHeight = height * 2; // Image and mask
		bih.biPlanes = 1;
		bih.biBitCount = 32;
		bih.biCompression = 0; // BI_RGB

		// Initialize BMP data vector
		std::vector<uint8_t> bmpData(sizeof(BITMAPINFOHEADER));
		memcpy(bmpData.data(), &bih, sizeof(BITMAPINFOHEADER));

		// Append pixel data (bottom-up DIB)
		for (int y = height - 1; y >= 0; --y) {
			const uint8_t* row = pixels + y * width * 4; // Assuming RGBA format
			bmpData.insert(bmpData.end(), row, row + width * 4);
		}

		// Append an empty AND mask (opaque)
		size_t maskSize = ((width + 31) / 32) * 4 * height;
		bmpData.resize(bmpData.size() + maskSize, 0xFF);

		return bmpData;
	}

	std::vector<uint8_t> CImage::_icoCreatePNGData(const uint8_t* pixels, int width, int height) const
	{
		std::vector<uint8_t> pngData;

		// Callback function to collect PNG data into pngData vector
		auto WriteCallback = [](void* context, void* data, int size) {
			std::vector<uint8_t>* pngData = static_cast<std::vector<uint8_t>*>(context);
			pngData->insert(pngData->end(), (uint8_t*)data, (uint8_t*)data + size);
			};

		// Write PNG data using stb_image_write
		stbi_write_png_to_func(WriteCallback, &pngData, width, height, 4, pixels, width * 4);

		return pngData;
	}
}

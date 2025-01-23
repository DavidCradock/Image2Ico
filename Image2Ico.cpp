// Image2Ico.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
#include "Globals.h"
#include "Core/Exceptions.h"
#include "Core/Utilities.h"
#include "Core/StringUtils.h"
#include "Image/Image.h"

using namespace X;
#include <iostream>

void displayAcceptedImageFormats(void)
{
    std::cout << "The following image formats are supported...\n";
    std::cout << "JPEG baseline & progressive (12 bpc/arithmetic not supported, same as stock IJG lib)\n";
    std::cout << "PNG 1 / 2 / 4 / 8 / 16 - bit - per - channel\n";
    std::cout << "TGA(not sure what subset, if a subset)\n";
    std::cout << "BMP non - 1bpp, non - RLE\n";
    std::cout << "PSD(composited view only, no extra channels, 8 / 16 bit - per - channel)\n";
    std::cout << "GIF(*comp always reports as 4 - channel)\n";
    std::cout << "HDR(radiance rgbE format)\n";
    std::cout << "PIC(Softimage PIC)\n";
    std::cout << "PNM(PPM and PGM binary only)\n";
}

void writeAutorunFile(const std::string& strIcoFilename)
{
	std::cout << "Writing Autorun.inf file using \"" + strIcoFilename + "\" as the .ico file name.\n";

    std::ofstream outFile("Autorun.inf");
    if (outFile)
    {
        outFile << "[autorun]\n";
        outFile << "icon=.\\";
        outFile << strIcoFilename;
		outFile << "\n";
        outFile.close();
        std::cout << "Autorun.inf file created successfully.\n";
        std::cout << "To set a custom icon for a drive, copy the Autorun.inf file and the .ico file to the root of the drive.\n";
    }
    else
    {
        std::cerr << "Failed to create Autorun.inf file.\n";
    }
}

/// \brief Main entry point of application
///
/// \param argc The number of arguments passed to the program
/// \param argv The arguments passed to the program
int main(int argc, char* argv[])
{
	pGlobals = new CGlobals;

    if (argc < 2)
    {
        std::cout << "No arguments passed to the Image2Ico.\nPlease specify the image file name to convert to an icon file.\n";
        std::cout << "Usage: Image2Ico <image file name>\n";
        std::cout << "Example: Image2Ico myimage.png\n";
        std::cout << "Type: Image2Ico help for more information.\n";
        return 0;
    }
    if (argc == 2)
    {
        std::string strParam = argv[1];
        StringUtils::stringToLowercase(strParam);
        if ("help" == strParam)
        {
			std::cout << "Help for Image2Ico\n";
			std::cout << "Image2Ico is a command line utility to convert an image file to an icon file.\n";
			std::cout << "Usage: Image2Ico <image file name>\n";
			std::cout << "Example: Image2Ico myimage.png\n";
			std::cout << "The above will attempt to read in the myimage.png file, create the neccessary image sizes and save it as an icon file.\n";
            std::cout << "\n";
            displayAcceptedImageFormats();
            std::cout << "\n";
            std::cout << "This also creates and saves a text file \"Autorun.inf\" with the name of the converted .ico file.\n";
			std::cout << "This \"Autorun.inf\" file can be copied, along with the output .ico file to a USB stick, or hard drive, to create a custom icon for the drive.\n";
			std::cout << "Any issues, please contact the developer.\n";
            std::cout << "Developer's e-mail address is djpcradock@gmail.com\n";
        }
        else
        {
            // strParam should be the file name of the image to convert if we get here

            CImage image;
            if (!image.load(argv[1]))
            {
				std::cout << "Unable to load image file: " << strParam << "\n";
                std::cout << "\n";
				displayAcceptedImageFormats();
				return 0;
            }

            if (image.getWidth() != 256 || image.getHeight() != 256)
            {
                std::cout << "Input image should ideally have dimensions of 256x256.\n";
				std::cout << "The input image's current dimensions are: " << image.getWidth() << "x" << image.getHeight() << "\n";
				std::cout << "The image will be resized to 256x256.\n";
				std::cout << "For optimal results, please use an image with dimensions of 256x256.\n";
            }
            
			strParam = StringUtils::addFilenameExtension(".ico", strParam);
            if (!image.saveAsICO(strParam))
				std::cout << "Image file could not be saved as an icon file.\n";
            else
				std::cout << "Image file saved as an icon file: " << strParam << "\n";

            writeAutorunFile(strParam);
            return 0;
        }
    }
    else
    {
		std::cout << "Too many arguments passed. Please specify only the image file name to convert to an icon file.\n";
		std::cout << "Usage: Image2Ico <image file name>\n";
		std::cout << "Example: Image2Ico myimage.png\n";
		std::cout << "Type: Image2Ico help for more information.\n";
    }

    return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

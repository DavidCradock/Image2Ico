This was written as an easy to use console application which takes an image file such as .bmp/.png etc, loads it in, creates scaled images of that image and then saves those into a single .ico file which can be used on Microsoft's Windows operating system as image data for icons.

As well as creating the .ico file, it also outputs a text file named Autorun.inf.
After using this tool to create the above mentioned files, copying those two files to the root directory of a drive in Windows' file explorer, will set that drive to show the .ico as the drive's image.

Simply download the release executable of Image2Ico.exe and place it in the root drive's directory which you wish to set that drive's image to.
Copy an image to the same folder that you wish to use for that drive's image. It should preferably have dimensions of 256x256, but it will be re-scaled for you anyway.
Then open a console window by pressing the Windows key and typing "cmd" and pressing enter.
Type in the name of the drive, for example "C:"
Change to the root directory with "cd /"
Run Image2Ico with "Image2Ico"
Run Image2Ico with "help" to show you some help "Image2Ico help"
Finally, type "Image2Ico " for example: "Image2Ico myImage.png" and if all goes well, the .ico file will be generated as well as the autorun.inf file.
Now your drive will have that image shown in Windows' explorer :)

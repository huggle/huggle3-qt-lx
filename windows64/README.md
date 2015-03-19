Windows
=========

This is used to build packages for windows, how-to:

* Download http://nsis.sourceforge.net/Download
* Install

Now it's a little bit tricky, you will need to create this structure in this folder:

* Make a folder called "release"
* Build huggle both without python engine and call it just huggle.exe, then with python support and call it py_hug.exe
* Copy huggle.ico from Resources folder to both root and other folder
* Now, keep running NSIS compiler on the nsi file and look for the error it will require lot of files

For each .dll file which is missing you need to browse the folder where Qt is installed and copy it there to location where it looks for it.

# XDGSearch
XDGSearch is a XAPIAN based file indexer and search tool

XDGSearch can be thought as your file retrieve tool, it indexes the standard output of programs, so-called helpers, that they retrieve "plain text" from files of many format.
Therefore the job of an helper program is to send plain text to the standard output in order to have "XDGSearch" processing it and the result stored into a XAPIAN database.
There is a plethora of programs that performs these kind of job for many files format such as PDF, JPEG, AVI, OGG, MP3, etc.
XDGSearch relies on:
- /bin/cat for *.txt, *.cpp, *.h file extensions 
- pstotext for *.pdf file extension provided by _pstotext_ package
- odt2txt for *.odt, *.ods file extensions provided by _odt2txt_ package
- iinfo for *.jpg, *.png file extensions provided by _openimageio-tools_ package
- mediainfo for *.mp3, *.ogg, *.mpv, *.avi, *.webm file extensions provided by _mediainfo_ package

These packages are not required in order to have XDGSearch running, however they are suggested. Of course it is possible to add or modify an helper through a proper dialog window thus the end-user can choose which program does the conversion or even write a new one that will fit his needs.

XDGSearch stores data retrieved by helpers in XAPIAN databases called pool. A "pool" encompasses:
- the XDG key
- the directory which the XDG key refers
- localized name of the pool
- which set of helpers will collect data
- the XAPIAN stem for the localized idioms (eventually)
- the XAPIAN stopwords file for the localized idioms (eventually)

The XAPIAN databases building process is threaded and once it ends the database is compacted.
XDGSearch requires to configure 7 pools plus one optional. The user will be asked to provide 7 directory path during the wizard setup configuration process, this is mandatory because XDGSearch was written to search information stored in the file-system hierarchy provided in the home directory by the _xdg-user-dirs_ Debian GNU Linux package thus to have installed this package is **highly recommended**, for Debian based distribution run the command:
```
~# apt-get install xdg-user-dirs
```
to install, then in your home directory run the command:
```
~$ xdg-user-dirs-update
```
doing so XDGSearch now has the necessary to auto-configure itself through the wizard process.

## Getting XDGSearch

XDGSearch was originally developed on Kubuntu 16.04.1 LTS with Qt Creator 3.5.1, now it's developed under Debian 9.x Stretch using Qt Creator 4.2.0 based on:
- Qt 5.7.1
- g++ (Debian 6.3.0-18+deb9u1) 6.3.0 20170516
- libxapian-dev 1.4.3-2+deb9u1

First install the development environment provided by the following packages:
```
~# apt-get install qt5-qmake qtbase5-dev-tools libxapian-dev qtbase5-dev:amd64 qt5-default
```
then get and build XDGSearch:
```
~$ git clone https://github.com/frank67/XDGSearch
~$ cd XDGSearch
~$ /usr/lib/x86_64-linux-gnu/qt5/bin/qmake
~$ make -j5
~$ ./xdgsearch
```

Dependencies :
- libqt5core5a
- libqt5widgets5
- libqt5svg5
- libxapian30:amd64

If you wish to give XDGSearch a try please follow these steps:
- first open a terminal window (e.g. Konsole)
- copy a pdf formatted file into the _Desktop_ directory
```
~$ cp /path/of/pdf/file.pdf $(xdg-user-dir DESKTOP)
```
- run XDGSearch and type some word to search into the "searched terms" bar and hit return
- accept the dialog asking you for build the database
- enjoy the results :-)

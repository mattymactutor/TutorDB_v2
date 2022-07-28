# TutorDB_v2

## Compile the GUI
-Install qt files on RPI

For rpi 4
```
sudo apt-get install qtbase5-dev qtchooser qt5-qmake qtbase5-dev-tools
```
if your application uses QSerialPort you need to run
```
sudo apt-get install libqt5serialport5
sudo apt-get install libqt5serialport5-dev
```

for rpi3 (untested),if qt-default is not found use the rpi4 code above
```
sudo apt-get install qt5-default
```

-Clone the repository on RPI4 which will create a TutorDB_v2 folder and put all of the GIT files in it
```
git clone https://github.com/mattymactutor/TutorDB_v2
```

-use qmake to generate the makefile. 
In this case, the .pro file should be called DarianB_Vitamin.pro but replace that with the name of your .pro file.
```
qmake -o Makefile TutorDB_v2.pro
```

-now use make which will call the Makefile you just created
```
make
```
-Run the file
```
./TutorDB_v2
```


-After updates are made to the code use this to rebuild
```
git pull
make
```

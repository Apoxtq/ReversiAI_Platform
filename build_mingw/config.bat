@echo off
cd /d D:\Project\Reversi\ReversiAI_Platform\build_mingw
cmake -G "MinGW Makefiles" -DCMAKE_PREFIX_PATH=D:/Dev/SDKs/Qt/6.10.1/mingw_64 -DQt6_ROOT=D:/Dev/SDKs/Qt/6.10.1/mingw_64 -DBUILD_QT_GUI=ON ..
pause


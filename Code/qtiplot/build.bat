@echo off
svn update
python release_date.py
cd qtiplot 
IF "%VCINSTALLDIR%"=="" CALL "%VS80COMNTOOLS%"vsvars32
rem qmake
nmake
if errorlevel 1 goto err
:err
echo nmake failed

@echo off
pushd "%CD%" 
cd %~dp0
erase *.obj
erase brailleblasterlib\*.dll
erase liblouisutdml.dll
popd
@echo on

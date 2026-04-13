call "%~dp0..\version.inc.bat"

"%programfiles(x86)%\jkmsigen\jkmsigen.exe" --upgrade-code=cfb86682-eb9c-4773-983f-6afb572d5b0a --name "FishyRecorder Lite" --version "%KUEMMELRECORDER_VERSION%" --output-msi "%~dp0out\FishyRecorder-%KUEMMELRECORDER_VERSION%.msi" "%~dp0bindir" --shortcut "KuemmelRecorder-Lite.exe" --icon "%~dp0..\img\fishy.ico" --x64 --installdir "FishyRecorder Lite" --with-ui "en-us"

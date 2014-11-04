@rem -------------------------------------------------------------------------
@rem Batch file for compiling hoof
@rem -------------------------------------------------------------------------

@echo Smgn build started
@set SMGNDIR=%NCIHOME%\suif\suif2b\tools\smgn

@echo smgndir set %SMGNDIR%

@echo command parameters smgn.exe -I%SMGNDIR% -Dgenerating=%1 %SMGNDIR%\suif.grm %1 %SMGNDIR%\suif.mac

@%NCIHOME%\bin\smgn.exe -I%NCIHOME%\suif\suif2b\basesuif -I%SMGNDIR% -Dgenerating=%1 %SMGNDIR%\suif.grm %1 %SMGNDIR%\suif.mac

@echo Done.


if "%WATCOM%" == "" goto error

mkdir WC32
wmake -f makefile.w32 WC32\DOOM32WC.EXE
del *.err
goto end

:error
@echo Set the environment variables before running this script!

:end

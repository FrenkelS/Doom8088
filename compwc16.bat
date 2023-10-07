if "%WATCOM%" == "" goto error

mkdir WC16
wmake -f makefile.w16 WC16\DOOM16WC.EXE
del *.err
goto end

:error
@echo Set the environment variables before running this script!

:end

@ECHO OFF
TITLE Update Server.
ECHO Getting the latest Git from rAthena Master
Echo Please wait ...
git pull https://github.com/rathena/rathena.git
Echo ---------------------------------------------------
timeout /t 1
cls

ECHO ===================================================
ECHO Update complete!
ECHO ===================================================
ECHO.
ECHO Latest commit information:
ECHO ---------------------------------------------------
git log -1 --pretty=format:"Commit Hash: %%H%%nAuthor: %%an%%nDate: %%ad%%nMessage: %%s" --date=local
ECHO.
ECHO.
ECHO ---------------------------------------------------
ECHO Last 10 commits:
ECHO ---------------------------------------------------
git log -11 --oneline
ECHO.
ECHO ===================================================
ECHO Press any key to exit...
pause >nul
exit
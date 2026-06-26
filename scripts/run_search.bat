@echo off
setlocal

pushd "%~dp0.." || (
  echo Project folder not found: "%~dp0.."
  pause
  exit /b 1
)

if not exist "cmake-build-debug\search_app.exe" (
  echo cmake-build-debug\search_app.exe not found. Build the project first.
  popd
  pause
  exit /b 1
)

cmake-build-debug\search_app.exe
popd

echo.
echo Search completed. Check answers.json in project root folder.



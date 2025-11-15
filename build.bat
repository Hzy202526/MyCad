@echo off
REM MyCad构建脚本

echo 正在创建构建目录...
if not exist build mkdir build
cd build

echo 正在配置CMake...
cmake .. -G "Visual Studio 17 2022" -A x64

if %ERRORLEVEL% NEQ 0 (
    echo CMake配置失败！
    pause
    exit /b 1
)

echo 正在编译项目...
cmake --build . --config Release

if %ERRORLEVEL% NEQ 0 (
    echo 编译失败！
    pause
    exit /b 1
)

echo 构建完成！
cd ..
pause


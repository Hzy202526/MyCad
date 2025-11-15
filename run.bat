@echo off
setlocal EnableExtensions EnableDelayedExpansion

REM ===== MyCad 运行脚本 =====
REM 此脚本自动配置运行环境并启动程序
REM 如需修改路径，请编辑下面的配置部分

echo.
echo ========================================
echo    MyCad 运行环境配置
echo ========================================
echo.

REM ===== 默认路径配置 (请根据您的安装路径修改) =====
set "QT_DIR=D:\Qt5.15.2\5.15.2\msvc2019_64"
set "OCC_DIR=F:\OpenCasCAD\opencascade-7.9.2\occt-vc14-64"
set "OCC_BASE_DIR=F:\OpenCasCAD\opencascade-7.9.2"

REM ===== 环境变量覆盖支持 =====
REM 可以通过设置环境变量来覆盖默认路径
if defined QT_DIR_OVERRIDE (
    set "QT_DIR=%QT_DIR_OVERRIDE%"
    echo [Info] 使用环境变量覆盖的Qt路径: %QT_DIR%
)
if defined OCC_DIR_OVERRIDE (
    set "OCC_DIR=%OCC_DIR_OVERRIDE%"
    echo [Info] 使用环境变量覆盖的OCC路径: %OCC_DIR%
)
if defined OCC_BASE_DIR_OVERRIDE (
    set "OCC_BASE_DIR=%OCC_BASE_DIR_OVERRIDE%"
    echo [Info] 使用环境变量覆盖的OCC基础路径: %OCC_BASE_DIR%
)

REM ===== Compute bin directories =====
set "QT_BIN=%QT_DIR%\bin"
set "OCC_BIN=%OCC_DIR%\win64\vc14\bin"

REM ===== Third-party library paths =====
set "FREETYPE_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\freetype-2.13.3-x64\bin"
set "FFMPEG_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\ffmpeg-3.3.4-64\bin"
set "TBB_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\tbb-2021.13.0-x64\bin"
set "VTK_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\vtk-9.4.1-x64\bin"
set "TCL_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\tcltk-8.6.15-x64\bin"
set "TK_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\tk\bin"
set "FREEIMAGE_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\freeimage-3.18.0-x64\bin"
set "GL2PS_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\openvr-1.14.15-64\bin\win64"
set "TESSELATION_BIN=%OCC_BASE_DIR%\3rdparty-vc14-64\jemalloc-vc14-64\bin"

REM ===== 验证关键目录 =====
echo [Info] 检查Qt目录: %QT_BIN%
if not exist "%QT_BIN%" (
	echo [Error] Qt bin目录未找到: "%QT_BIN%"
	echo [Error] 请检查Qt安装路径是否正确
	echo [Error] 或设置环境变量 QT_DIR_OVERRIDE 来指定正确的路径
	goto :error_exit
)

echo [Info] 检查OpenCASCADE目录: %OCC_BIN%
if not exist "%OCC_BIN%" (
	echo [Error] OpenCASCADE bin目录未找到: "%OCC_BIN%"
	echo [Error] 请检查OpenCASCADE安装路径是否正确
	echo [Error] 或设置环境变量 OCC_DIR_OVERRIDE 来指定正确的路径
	goto :error_exit
)

REM ===== 构建PATH环境变量 =====
set "PATH=%QT_BIN%;%OCC_BIN%"

echo [Info] 使用Qt bin目录: %QT_BIN%
echo [Info] 使用OCC bin目录: %OCC_BIN%
echo.

REM ===== 添加第三方库到PATH (如果存在) =====
echo [Info] 检查并添加第三方库...

set "ADDED_LIBS=0"

if exist "%FREETYPE_BIN%" (
	set "PATH=%PATH%;%FREETYPE_BIN%"
	echo [Info] 添加FreeType: %FREETYPE_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%TBB_BIN%" (
	set "PATH=%PATH%;%TBB_BIN%"
	echo [Info] 添加TBB: %TBB_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%FFMPEG_BIN%" (
	set "PATH=%PATH%;%FFMPEG_BIN%"
	echo [Info] 添加FFmpeg: %FFMPEG_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%VTK_BIN%" (
	set "PATH=%PATH%;%VTK_BIN%"
	echo [Info] 添加VTK: %VTK_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%TCL_BIN%" (
	set "PATH=%PATH%;%TCL_BIN%"
	echo [Info] 添加TCL: %TCL_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%TK_BIN%" (
	set "PATH=%PATH%;%TK_BIN%"
	echo [Info] 添加TK: %TK_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%FREEIMAGE_BIN%" (
	set "PATH=%PATH%;%FREEIMAGE_BIN%"
	echo [Info] 添加FreeImage: %FREEIMAGE_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%GL2PS_BIN%" (
	set "PATH=%PATH%;%GL2PS_BIN%"
	echo [Info] 添加GL2PS: %GL2PS_BIN%
	set /a ADDED_LIBS+=1
)
if exist "%TESSELATION_BIN%" (
	set "PATH=%PATH%;%TESSELATION_BIN%"
	echo [Info] 添加Tessellation: %TESSELATION_BIN%
	set /a ADDED_LIBS+=1
)

echo [Info] 总共添加了 %ADDED_LIBS% 个第三方库
echo.

REM ===== 查找可执行文件 =====
echo [Info] 查找可执行文件...

set "APP=build\Release\MyCad.exe"
if not exist "%APP%" (
    echo [Info] Release版本未找到，尝试Debug版本...
    set "APP=build\Debug\MyCad.exe"
)

if not exist "%APP%" (
    echo [Error] 未找到可执行文件！
    echo [Error] 请先编译项目：
    echo [Error]   1. 运行 build.bat 进行编译
    echo [Error]   2. 或使用 Visual Studio 打开 build\MyCad.sln
    echo [Error] 期望位置: build\Release\MyCad.exe 或 build\Debug\MyCad.exe
    goto :error_exit
)

echo [Info] 找到可执行文件: %APP%
echo.

REM ===== 启动程序 =====
echo ========================================
echo    正在启动 MyCad...
echo ========================================
echo.

"%APP%"

REM ===== 程序结束处理 =====
if %ERRORLEVEL% neq 0 (
    echo.
    echo [Warning] 程序异常退出，错误代码: %ERRORLEVEL%
    echo [Info] 请检查控制台输出以获取更多信息
) else (
    echo.
    echo [Info] 程序正常退出
)

goto :end

REM ===== 错误退出 =====
:error_exit
echo.
echo ========================================
echo    配置失败，请检查上述错误信息
echo ========================================
echo.
echo 常见解决方案：
echo 1. 检查Qt和OpenCASCADE是否正确安装
echo 2. 修改run.bat中的路径配置
echo 3. 使用环境变量覆盖路径设置
echo 4. 参考 PATH_CONFIG_GUIDE.md 获取详细说明
echo.
pause
exit /b 1

REM ===== 正常结束 =====
:end
endlocal

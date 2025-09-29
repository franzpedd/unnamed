@echo off
REM Batch script to compile GLSL shaders to SPIR-V using glslc

REM Set the path to glslc (if not in PATH)
REM set GLSL_COMPILER="C:\VulkanSDK\<version>\Bin\glslc.exe"

REM Output directory for SPIR-V files
set OUTPUT_DIR=compiled

REM Create the output directory if it doesn't exist
if not exist "%OUTPUT_DIR%" (
    mkdir "%OUTPUT_DIR%"
)

REM List of shader base names (without extensions)
set SHADER_BASES=^
 grid^
 quad^
 quad_picking

REM Loop through each shader base name and compile both .vert and .frag
for %%b in (%SHADER_BASES%) do (
    echo Compiling %%b.vert...
    glslc -o "%OUTPUT_DIR%\%%b.vert.spv" "%%b.vert"
    if errorlevel 1 (
        echo Failed to compile %%b.vert
        exit /b 1
    )

    echo Compiling %%b.frag...
    glslc -o "%OUTPUT_DIR%\%%b.frag.spv" "%%b.frag"
    if errorlevel 1 (
        echo Failed to compile %%b.frag
        exit /b 1
    )
)

echo All shaders compiled successfully.
pause
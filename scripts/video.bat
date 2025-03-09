@echo off
setlocal

:: Set parameters
set "input=../tmp/video%%d.png" :: Input frame pattern (video0.png, video1.png, ...)
set "output=../render.mp4"      :: Output video file
set "fps=30"                    :: Frames per second
set "crf=18"                    :: Quality (lower is better, 0-51, 18 is visually lossless)

:: Check if ffmpeg is installed
where ffmpeg >nul 2>nul
if %errorlevel% neq 0 (
    echo Error: ffmpeg not found. Please install it and add to PATH.
    exit /b 1
)

:: Encode video
ffmpeg -r %fps% -i %input% -c:v libx264 -crf %crf% -pix_fmt yuv420p %output%

echo Video encoding complete: %output%
pause
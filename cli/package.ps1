$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Push-Location $dir
docker run --rm -v ${dir}\..:c:\build lolisamurai/msvc
Pop-Location

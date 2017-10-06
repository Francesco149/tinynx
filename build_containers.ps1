$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition
Push-Location "$dir"
$d = "docker/msvc"
Copy-Item $d/Dockerfile .
docker build -t "lolisamurai:msvc" .
Remove-Item Dockerfile
Pop-Location

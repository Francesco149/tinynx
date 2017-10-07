# you must allow script execution by running
# 'Set-ExecutionPolicy RemoteSigned' in an admin powershell
# this requires vcvars to be already set (see vcvarsall17.ps1)
# 7zip is also required (choco install 7zip and add it to path)

Get-Location
Get-ChildItem

$dir = Split-Path -Parent $MyInvocation.MyCommand.Definition

Push-Location "$dir"\cli
git pull origin master

function Compile-And-Package
{
    Write-Host ""
    Write-Host "########################"
    Write-Host "> Compiling and stripping"
    cmd /c "build.bat"

    Write-Host ""
    Write-Host "########################"
    Write-Host "> Packaging"
    $folder = "nx-" + $(.\nx.exe -version) + "-windows-"
    $clout = &cl 2>&1
    cl
    "$clout" -match "Microsoft \(R\) C/C\+\+ Optimizing Compiler Version [0-9.]+ for ([a-zA-Z0-9\-_]+)" | Out-Null
    $folder = $folder + $Matches[1]
    mkdir $folder
    Copy-Item nx.exe $folder
    git archive HEAD --prefix=src\ -o $folder\src.zip
    Set-Location $folder
    &7z x src.zip
    Set-Location ..

    if (Test-Path "$folder.zip") {
        Remove-Item "$folder.zip"
    }

    &7z a "$folder.zip" $folder\nx.exe $folder\src

    Write-Host ""
    Write-Host "########################"
    Write-Host "> Result:"
    &7z l "$folder.zip"
    Remove-Item $folder -Force -Recurse
}

. .\vcvarsall17.ps1 x64
Compile-And-Package
. .\vcvarsall17.ps1 x86
Compile-And-Package

Pop-Location

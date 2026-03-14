param(
    [Parameter(Mandatory = $true)]
    [string]$FileName,
    [switch]$UseUPX
)


$coolerfilename = [System.IO.Path]::GetFileNameWithoutExtension($FileName) + ".dll"


$flags = @(
    "-shared",                
    "-s", "-Os",              
    "-static-libstdc++",      
    "-static-libgcc",         
    "-lwinhttp"             
)


Write-Host "Compiling $FileName -> $coolerfilename ..."
g++ $FileName -o $coolerfilename $flags

if ($LASTEXITCODE -ne 0) {
    Write-Error "Compilation failed!"
    exit $LASTEXITCODE
}

Write-Host "Compilation successful: $coolerfilename"


if ($UseUPX) {
    if (-not (Test-Path "C:\upx\upx.exe")) {
        Write-Warning "UPX not found at C:\upx\upx.exe"
    }
    else {
        Write-Host "Compressing DLL with UPX..."
        & "C:\upx\upx.exe" --best --lzma --compress-resources=0 --brute $coolerfilename

        if ($LASTEXITCODE -eq 0) {
            Write-Host "UPX compression done."
        }
        else {
            Write-Warning "UPX failed. DLL left uncompressed."
        }
    }
}
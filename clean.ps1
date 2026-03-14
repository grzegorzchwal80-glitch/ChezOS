param(
    [switch]$Ask
)

tasklist | findstr outDebug.exe
tasklist | findstr ChezOS.exe
taskkill /f /im outDebug.exe
taskkill /f /im ChezOS.exe

g++ "Source\Main\C++\ChezOSKERNEL64.cpp" "Source\Main\C++\ChezOSBOOT64.cpp" -fpermissive -std=c++20 -Os -s -fexceptions -fno-rtti -Wunused-variable -IInclude -Wall -Wextra -Wpedantic -lcrypt32 -lwinmm -lgdi32 -luser32 -lgdiplus -lwinhttp -mwindows -static-libstdc++ -static-libgcc -o ChezOS.exe #mingw g++


if ($?) {
    Write-Host "Build succeeded."

    & "C:\upx\upx.exe" --best --lzma --compress-resources=0 ChezOS.exe

    Get-ChildItem Cert:\CurrentUser\My\ | Where-Object {$_.Subject -eq "CN=ChezOSDev"} | Remove-Item
    $cert = New-SelfSignedCertificate -Type CodeSigning -Subject "CN=ChezOSDev" -CertStoreLocation Cert:\CurrentUser\My
    Set-AuthenticodeSignature -FilePath ChezOS.exe -Certificate $cert
    # i do have family safety due that im under 13, so still i have to do this so windows trusts this file 5x more. same for you!

    if ($Ask) {
        Start-Process -FilePath "ChezOS.exe"
    }
    else {
        $reply = Read-Host "Press Y to run, N to not run"
        if ($reply -eq "Y") {
            Start-Process -FilePath "ChezOS.exe"
        }
        else {
            Write-Host "Not running."
        }
    }
}
else {
    Write-Host "Build failed."
}

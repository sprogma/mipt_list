pushd $PSScriptRoot
@("std.cpp", "soa_list.c", "aos_list.c") | % {
    if ($_-match"\.cpp$")
    {
        g++ test.cpp $_ -o a.exe -DTEST_CPP_REALIZATION -Ofast -flto
    }
    else
    {
        gcc test.cpp $_ -o a.exe -DTEST_CPP_REALIZATION -Ofast -flto -lstdc++
    }
    Write-Host "Running $_" -ForegroundColor green
    ./a.exe
}

popd

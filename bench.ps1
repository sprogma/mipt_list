pushd $PSScriptRoot
$l = @(ls *.c,*.cpp | ?{(gc $_ -Raw) -match "\*\s*list_create\s*\(\s*int(32_t)?\s+capacity\s*\)"} | % N*e)
Write-Host "Testing" $l
$res = [ordered]@{}

$l | %{
    $id = $l.IndexOf($_) + 1
    Write-Host "Builing $_" -ForegroundColor yellow
    if ($_-match"\.cpp$")
    {
        g++ test.cpp $_ -o "a$id.exe" -DTEST_CPP_REALIZATION -DNDEBUG -Ofast -flto
    }
    else
    {
        gcc test.cpp $_ -o "a$id.exe" -Ofast -DNDEBUG -flto -lstdc++
    } 
}

while (1)
{
    $l | % {
        $id = $l.IndexOf($_) + 1
        Write-Host "Running $_" -ForegroundColor green
        $dat = -split(& "./a$id.exe")
        if ($res[$_] -eq $null)
        {
            $res[$_] = [pscustomobject]@{name=$_;data=$dat}
        }
        else
        {
            for ($i = 0; $i -lt $dat.Length; $i++)
            {
                $res[$_].data[$i] = [Math]::min($res[$_].data[$i], $dat[$i])
            }
        }
        .\draw.ps1 @($res.Values)
        see .\result.png
    }
    
    $res.Values | oh
}

popd

pushd $PSScriptRoot
$l = @(ls *.c,*.cpp | ?{(gc $_ -Raw) -match "\*\s*list_create\s*\(\s*int(32_t)?\s+capacity\s*\)"} | % N*e)
Write-Host "Testing" $l
$res = [ordered]@{}

$l | %{
    $id = $l.IndexOf($_) + 1
    Write-Progress -Activity "building" -Status "$_" -PercentComplete ([int](100*($id-1)/$l.Count))
    if ($_-match"\.cpp$")
    {
        g++ test.cpp $_ -o "a$id.exe" -DTEST_CPP_REALIZATION -DNDEBUG -Ofast -march=native -flto
    }
    else
    {
        gcc test.cpp $_ -o "a$id.exe" -Ofast -DNDEBUG -flto -lstdc++ -march=native
    }
    Write-Host "built $_" -ForegroundColor green
}
Write-Progress -Activity "building" -Status "finished" -PercentComplete 100 -Completed

while (1)
{
    $l | % {
        $id = $l.IndexOf($_) + 1
        Write-Progress -Activity "running" -Status "$_" -PercentComplete ([int](100*($id-1)/$l.Count))
        $dat = -split(& "./a$id.exe" 2>$null)
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
    Write-Progress -Activity "running" -Status "finished" -PercentComplete 100 -Completed
    Import-Csv "result.dat" -Delimiter ';' | fl | oh
    Import-Csv "result.dat" -Delimiter ';' | ft | oh
}

popd

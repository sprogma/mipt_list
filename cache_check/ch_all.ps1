gci ch*.c | % {
    Write-Host "Running $($_.Name)" -ForegroundColor green
    $t = 2
    if ($_.Name -match "barrier")
    {
        $t = 4
    }
    if ($_.Name -match "hard")
    {    
        .\check.ps1 $_ -Div (512*$t)
    }
    elseif ($_.Name -match "medium")
    {    
        .\check.ps1 $_ -Div (256*$t)
    }
    else
    {
        .\check.ps1 $_ -Div (128*$t)
    }
}

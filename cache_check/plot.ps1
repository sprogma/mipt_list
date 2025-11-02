param(
    [string]$imageFile, 
    [string]$resultFile
)

pushd $PSScriptRoot

$text = Get-Content $resultFile


$rows = $text |? {$_.Trim()} |% {
    $_ -match "^((No)\s*prefetch(-hard)?|prefetch(-hard)?\s+x(\d+)\s+([\w+]+)?).*UserTime:\s*([\d.]+)" >$null
    $no, $h1, $h2, $mode, $count, $time = $Matches[2, 3, 4, 6, 5, 7]
    $mode += $h1 + $h2
    $mode = "$($no ?? "prefetch")-$mode"
    [pscustomobject]@{mode=$mode;count=$count;time=$time}
}

$rows += $rows[0].PsObject.Copy()
$rows[0].count = 0
$rows[-1].count = ($rows | % count | measure -Max).Maximum

# $rows += $rows[1].PsObject.Copy()
# $rows[1].count = 0
# $rows[-1].count = ($rows | % count | measure -Max).Maximum

$modes = $rows | s -exp mode -u

$files = @{}
$rows | group mode |% {
    $f = $_.Name -replace "[^a-z0-9]",'_'
    $f = "./modes/$f.dat"
    $files[$_.Name] = $f
    $_.Group | % { "{0} {1}" -f $_.count,$_.time } >$f
}

$plot = @"
set terminal pngcairo size 1600,900 background rgb '#222222'
set output "$imageFile"
set datafile separator whitespace
set key outside
set grid
set key textcolor rgb "white"
set xlabel 'prefetch range' tc rgb 'white'
set ylabel 'elements per us' tc rgb 'white'
set border lc rgb 'white'

"@

$plot += "plot " + (($modes|% {$id=0}{
    $id++; "  '$($files[$_])' using 1:2 with linespoints lt $id pt $id lw 2 title '$_' "
})-join",\`n")

$plot >plot.gp
gnuplot -persist plot.gp

rm plot.gp
rm modes/*

see $imageFile

popd

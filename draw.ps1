param(
  [Parameter(Mandatory=$true)] [PSObject[]] $Table
)

pushd $PSScriptRoot


"Function; Deijkstra; BFS; Insert 0; Insert 100; Read; Optimization; Read after Opt" >result.dat
$table | %{"$($_.name-replace"_","-"); $($_.data -join ";")"} >>result.dat

$lines = gc "result.dat" | % { ,($_ -split ";") }
(0..($lines[0].Count-1)|%{$c=$_;($lines|% {$_[$c]})-join ";"})-join "`n" >result.dat

& {gnuplot plot.gp; Write-Output "Generated result.png"} || Write-Error "Failed to run gnuplot."

popd

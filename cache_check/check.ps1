param(
    [string]$inputFile,
    # less SIZE_DIV - more acuracy, less speed
    [long]$div=128
)
pushd $PSScriptRoot

$s = gc $inputFile -Raw

$script:res = ""

# num of threads at build stage
$j = 128

function Run($obj)
{
    $s = $obj.message.PadRight(50)
    $script:res += $s
    Write-Host $s -NoNewLine
    $s = & $obj.exe
    $script:res += $s + "`n"
    Write-Host $s 
}


$script:fid = 0
function Check($message, $code)
{
    $script:fid++
    $code >"tmp/$($script:fid).c"
    Start-ThreadJob -ScriptBlock {
        param($message, $id, $DIV)
        gcc "tmp/$id.c" -o "bin/$id.exe" -Ofast -mprfchw -march=native "-DSIZE_DIV=$DIV"
        [System.Console]::writeLine("Built [$message] bin/$id.exe")
        [pscustomobject]@{exe="bin/$id.exe";message=$message}
    } -ArgumentList ($message, $script:fid, $div) -ThrottleLimit $j
}

# check no prefetch
$size = 2147483648n
$i = 1
$jobs = @()

$jobs += Check "No prefetch" ($s-replace"PREFETCH_POSITION")
$jobs += Check "No prefetch-direct" ($s-replace"PREFETCH_POSITION","pos = i")
while ($i -le 128)
{
    $jobs += Check "prefetch x$i"       ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), _MM_HINT_T0)")
    $jobs += Check "prefetch x$i E"     ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), _MM_HINT_ET0)")
    $jobs += Check "prefetch x$i NTA"   ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), _MM_HINT_NTA)")
    # not found _MM_HINT_ENTA
    # $jobs += Check "prefetch x$i E+NTA" ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), _MM_HINT_ENTA)")
    $jobs += Check "prefetch x$i W"     ($s-replace"PREFETCH_POSITION","__builtin_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), 1, 0)")
    if ($i -lt 16)
    {
        $i += 1
    }
    elseif ($i -lt 64)
    {
        $i += 2
    }
    else
    {
        $i += 4
    }
}

$jobs | Wait-Job | Out-Null

$jobs | Receive-Job | % {Run $_}

$imageFile = "$(Split-Path $inputFile -LeafBase).png"
$resultFile = "$(Split-Path $inputFile -LeafBase).txt"

$script:res >$resultFile

.\plot.ps1 $imageFile $resultFile

popd

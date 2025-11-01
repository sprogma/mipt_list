pushd $PSScriptRoot

$s = gc check.c -Raw

function Check($message, $code)
{
    $code >tmp.c
    gcc tmp.c -o a.exe -Ofast -march=native
    $s = $message.PadRight(50)
    Write-Host $s -NoNewLine
    ./a.exe
}

# check no prefetch
$size = 2147483648n
Check "No prefetching" ($s-replace"PREFETCH_POSITION")
for ($i = 1; $i -le 128; $i *= 2)
{
    Write-Host
    Check "prefetch x$i"       ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), _MM_HINT_T0)")
    Check "prefetch x$i E"     ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), _MM_HINT_ET0)")
    Check "prefetch x$i NTA"   ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), _MM_HINT_NTA)")
    Check "prefetch x$i E+NTA" ($s-replace"PREFETCH_POSITION","_mm_prefetch(array + ((pos + $((998244353n * $i) % $size)) & SIZE_MASK), 4)")
}

popd

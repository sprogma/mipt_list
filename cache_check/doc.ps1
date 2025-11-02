pushd $PSScriptRoot

&{
<# functions to simplify text generation #>
function Title { [CmdletBinding()]param([Parameter(ValueFromPipeline=$true)]$Text) process{"<h1>$text</h1>"} }
function Color { [CmdletBinding()]param($Color, [Parameter(ValueFromPipeline=$true)]$Text) process{"<div style=""color:$color"">$_</div>"} }
function Code { [CmdletBinding()]param([Parameter(ValueFromPipeline=$true)]$Text) process{"<pre><code>$text</code></pre>"} }
function Summary { [CmdletBinding()]param($Summary, [Parameter(ValueFromPipeline=$true)]$Text, [switch]$Open) process{"<details $($Open ? "open" : '')><summary>$Summary</summary>$text</details>"} }
@"
<!doctype html>
<html>
<head>
    <meta charset="utf-8" />
        <style>
            body {
                background-color: #131513;
                color: #0e8;
            }
            h1 {
                text-align: center;
            }
        </style>
    <title>Cache line check</title>
</head>
<body>
$(
    gci "ch*.c" | % N*e | % {
        $file = $_
        
        $file | Title | Color "#0ff"

        $code = gc $file -Raw
        
        <# show source code #>
        $code -split "(\n.*QueryPerformanceCounter(?:.|\n)*QueryPerformanceCounter.*\n)" | 
            & { 
                $a = @($input);
                @(($a[0] | Color "#ccc"), ($a[1]), ($a[2] | Color "#ccc"))-join""
            } | Code | Summary "View Code"

        <# show example of disassembly #>
        & {
            gcc -g $_ -DPREFETCH_POSITION -DSIZE_DIV=1 -Ofast -march=native -c -o "obj/$file.o" >$null
            $res = objdump -d "obj/$_.o" --line-numbers -Mintel 
            
            $lineStart, $lineEnd = $code | sls QueryPerformanceCounter -AllMatches | % Matches | % {$_.Index} | % {($code[0..$_] -eq "`n").Count + 1}

            $lines = $res | 
                % {$prev=""}{
                if ($_-like"*$file*"){$prev=$_}
                [pscustomobject]@{prev=$prev;line=$_}
            } | ? {
                if ($_.prev -ne $_.line -and $_.prev -match ":(\d+)$")
                {
                    $_.prev = $Matches[1]
                    $true
                }
            } | group prev | ? {
                [int]$_.Name -gt $lineStart -and [int]$_.Name -lt $lineEnd
            } | % Group | % line

            <# extract that lines, in right order #>
            ($res | ? { $_ -in $lines })-join"`n" | Code | Summary "View Assembly" -Open
        }
        
        <# images #>
        $imageFile = "$(Split-Path $_ -LeafBase).png"
        $resultFile = "$(Split-Path $_ -LeafBase).txt"
        if (Test-Path $imageFile)
        {
            @"
                <img style="max-width:100%;max-height:100%;" src="./$imageFile" alt="benchmark image for $_" />
"@
        }
        else
        {
            @"
                <p style="color:red; font-size: 2em">No image found for $_ : $imageFile file not exists</p>
"@
        }
    }
)
</body>
</html>
"@} >result.html

ii result.html

popd

$shadersLines = cloc ../shaders --quiet --json | Out-String | ConvertFrom-Json | Select-Object -ExpandProperty SUM | Select-Object -ExpandProperty code
$srcLines = cloc ../src --quiet --json | Out-String | ConvertFrom-Json | Select-Object -ExpandProperty SUM | Select-Object -ExpandProperty code
$thirdPartyLines = cloc ../shaders/third_party --quiet --json | Out-String | ConvertFrom-Json | Select-Object -ExpandProperty SUM | Select-Object -ExpandProperty code

$shadersLines = [int]$shadersLines
$thirdPartyLines = [int]$thirdPartyLines

$ownCodeLines = $shadersLines - $thirdPartyLines

Write-Output "GLSL: $ownCodeLines"
Write-Output "C++:  $srcLines"
Write-Output "SUM:  $($ownCodeLines + $srcLines)"

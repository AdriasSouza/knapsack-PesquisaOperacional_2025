# Compila e executa knapSA_incompleto.cpp em lote e gera um CSV com os resultados.
# Este script compila uma vez e executa o binário para cada problemInstances/**/test.in,
# parseando a saída textual:
# - Extrai 'Greedy: <valor>' e 'S.A: <valor>'
# - Usa o caminho da instância como coluna 'instancia'

$ErrorActionPreference = 'Stop'

# Caminhos
$root = $PSScriptRoot
$src  = Join-Path $root 'knapSA_incompleto.cpp'
$exe  = Join-Path $root 'knapSA_incompleto.exe'
$csv  = Join-Path $root 'resultados_incompleto.csv'
$instancesRoot = Join-Path $root 'problemInstances'

if (!(Test-Path $src)) {
    Write-Error "Arquivo não encontrado: $src"
    exit 1
}

Write-Host "Compilando $src ..."
& g++ $src -o $exe -O3 -std=c++11
if ($LASTEXITCODE -ne 0) {
    Write-Error 'Falha na compilação.'
    exit 1
}


# CSV header
'instancia,lucro_guloso,lucro_sa' | Set-Content -Encoding UTF8 $csv

# Itera por todas as instâncias
$instances = Get-ChildItem -Path $instancesRoot -Recurse -File -Filter 'test.in' | Sort-Object FullName
if ($instances.Count -eq 0) {
    Write-Warning "Nenhuma instância encontrada em $instancesRoot"
}

foreach ($inst in $instances) {
    Write-Host "Executando: $($inst.FullName)"
    $out = & $exe $inst.FullName 2>&1
    $exit = $LASTEXITCODE
    if ($exit -ne 0) {
        Write-Warning "Falha na execução: $($inst.FullName)"
        continue
    }
    $greedyMatch = [regex]::Match($out, 'Greedy:\s*(-?\d+)')
    $saMatch     = [regex]::Match($out, 'S\.A:\s*(-?\d+)')
    if (-not $greedyMatch.Success -or -not $saMatch.Success) {
        Write-Warning "Não foi possível extrair lucros para: $($inst.FullName)"
        continue
    }
    $greedy = $greedyMatch.Groups[1].Value
    $sa     = $saMatch.Groups[1].Value
    "${($inst.FullName)},$greedy,$sa" | Add-Content -Encoding UTF8 $csv
}

Write-Host "Concluído. Resultados em $csv"

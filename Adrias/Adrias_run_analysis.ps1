# Compila e executa a análise para todas as instâncias.
# Uso: pode ser executado de qualquer pasta; caminhos são relativos a este script.
# Requisitos: PowerShell 5+ e g++ no PATH.

$ErrorActionPreference = 'Stop'

Write-Host 'Compilando Adrias_knapSA.cpp...'
# Pastas principais
$scriptDir = $PSScriptRoot
$rootDir   = Split-Path -Parent $scriptDir
$src       = Join-Path $scriptDir 'Adrias_knapSA.cpp'
$exe       = Join-Path $rootDir 'knapSA.exe'
$results   = Join-Path $rootDir 'resultados.csv'
$instancesRoot = Join-Path $rootDir 'problemInstances'

# Compila com otimização e C++11; executável no diretório raiz do repositório
& g++ $src -o $exe -O3 -std=c++11
if ($LASTEXITCODE -ne 0) {
    Write-Error 'Falha na compilação.'
    exit 1
}

# Cria/limpa o arquivo de resultados com cabeçalho CSV
"instancia,lucro_guloso,lucro_sa,tempo_guloso_ms,tempo_sa_ms,tempo_total_ms" | Set-Content -Encoding ASCII $results

# Encontra todas as instâncias test.in dentro de problemInstances
$instances = Get-ChildItem -Path $instancesRoot -Recurse -File -Filter 'test.in' | Sort-Object FullName

if ($instances.Count -eq 0) {
    Write-Warning 'Nenhuma instância encontrada em problemInstances.'
}

# Execução paralela controlada
$maxParallel = [math]::Max(1, [int]$env:NUMBER_OF_PROCESSORS)
Write-Host "Executando com até $maxParallel tarefas em paralelo."

$jobs = @()

function Flush-CompletedJobs {
    param([string]$resultsPath)
    $completed = $jobs | Where-Object { $_.State -ne 'Running' }
    foreach ($j in $completed) {
        try {
            $out = Receive-Job $j -Keep
            if ($out) { $out | Add-Content -Encoding ASCII $resultsPath }
        } catch {
            Write-Warning "Falha ao receber resultado do job Id=$($j.Id): $($_.Exception.Message)"
        } finally {
            Remove-Job $j -Force | Out-Null
            $script:jobs = $jobs | Where-Object { $_.Id -ne $j.Id }
        }
    }
}

foreach ($inst in $instances) {
    while ( ($jobs | Where-Object { $_.State -eq 'Running' }).Count -ge $maxParallel ) {
        Start-Sleep -Milliseconds 150
        Flush-CompletedJobs -resultsPath $results
    }

    Write-Host "Enfileirando: $($inst.FullName)"
    $job = Start-Job -ScriptBlock {
        param($exePath, $filePath)
        & $exePath $filePath
    } -ArgumentList $exe, $inst.FullName
    $jobs += $job
}

while ($jobs.Count -gt 0) {
    Start-Sleep -Milliseconds 200
    Flush-CompletedJobs -resultsPath $results
}

Write-Host "Análise concluída. Resultados salvos em $results"

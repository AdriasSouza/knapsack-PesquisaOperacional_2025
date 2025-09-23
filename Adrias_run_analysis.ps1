# PowerShell script to compile and run knapsack analysis across all instances
# Usage: run in the repository root (same folder as knapSA_incompleto.cpp)
# Requirements: PowerShell 5+, g++ available in PATH (e.g., MinGW or similar)

$ErrorActionPreference = 'Stop'

Write-Host 'Compilando knapSA_incompleto.cpp...'
# Compila com otimizações e C++11 conforme solicitado
& g++ knapSA_incompleto.cpp -o knapSA -O3 -std=c++11
if ($LASTEXITCODE -ne 0) {
    Write-Error 'Falha na compilação.'
    exit 1
}

# Cria/limpa o arquivo de resultados com cabeçalho CSV
"instancia,lucro_guloso,lucro_sa,tempo_guloso_ms,tempo_sa_ms,tempo_total_ms" | Set-Content -Encoding ASCII resultados.csv

# Encontra todas as instâncias test.in dentro de problemInstances
$instances = Get-ChildItem -Path 'problemInstances' -Recurse -File -Filter 'test.in' | Sort-Object FullName

if ($instances.Count -eq 0) {
    Write-Warning 'Nenhuma instância encontrada em problemInstances.'
}

foreach ($inst in $instances) {
    Write-Host "Processando: $($inst.FullName)"
    # Executa o binário passando o caminho do arquivo; a saída já é CSV
    $line = & .\knapSA.exe $inst.FullName
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "Execução falhou para: $($inst.FullName)"
        continue
    }
    $line | Add-Content -Encoding ASCII resultados.csv
}

Write-Host 'Análise concluída. Resultados salvos em resultados.csv'

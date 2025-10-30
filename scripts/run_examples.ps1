Param(
  [string]$BuildDir = "$(Split-Path -Parent $PSCommandPath)\..\build"
)

$BuildDir = Resolve-Path $BuildDir

function Run-Exe($path) {
  if (Test-Path $path) {
    Write-Host "Running $(Split-Path $path -Leaf)"
    & $path
    Write-Host ""
  } else {
    Write-Warning "Missing executable: $path"
  }
}

Run-Exe (Join-Path $BuildDir 'example.exe')
Run-Exe (Join-Path $BuildDir 'example_lru.exe')
Run-Exe (Join-Path $BuildDir 'example_lfu.exe')
Run-Exe (Join-Path $BuildDir 'example_arc.exe')
Run-Exe (Join-Path $BuildDir 'example_concurrency.exe')
Run-Exe (Join-Path $BuildDir 'example_mixed.exe')


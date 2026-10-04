<#
.SYNOPSIS
Runs the uaenative.library probe in 32-bit (winuae.exe) and 64-bit
(winuae64.exe) WinUAE, using the boot setup of the reference tests.
Needs build/spikes from the build commands in spikes/uaenative/README.md.
#>
param(
	[string]$Settings = (Join-Path $PSScriptRoot '..\..\tests\settings.local.psd1'),
	[int]$TimeoutSec = 120
)

$ErrorActionPreference = 'Stop'
$root = Resolve-Path (Join-Path $PSScriptRoot '..\..')
$cfg = Import-PowerShellDataFile $Settings
$work = Join-Path $root 'build\spikes'
$template = Get-Content (Join-Path $root 'tests\winuae\test.uae.in') -Raw

foreach ($exe in 'winuae.exe', 'winuae64.exe') {
	$source = Join-Path $cfg.WinUAEDir $exe
	if (-not (Test-Path $source)) { Write-Warning "$source not found, skipped"; continue }

	$run = Join-Path $work ([IO.Path]::GetFileNameWithoutExtension($exe))
	$uae = Join-Path $run 'winuae'
	$qttest = Join-Path $run 'qttest'
	if (Test-Path $qttest) { Remove-Item $qttest -Recurse -Force }
	foreach ($dir in $uae, (Join-Path $qttest 'C'), (Join-Path $qttest 'S'), (Join-Path $qttest 'Libs'), (Join-Path $qttest 'capture')) {
		New-Item -ItemType Directory -Force $dir | Out-Null
	}
	Copy-Item $source $uae -Force
	if (-not (Test-Path (Join-Path $uae 'winuae.ini'))) { New-Item -ItemType File (Join-Path $uae 'winuae.ini') | Out-Null }
	Copy-Item (Join-Path $work 'qtprobe-windows-*.dll') $uae -Force
	Copy-Item (Join-Path $work 'probe') (Join-Path $qttest 'C')
	Copy-Item (Join-Path $cfg.WinUAEDir 'Amiga Programs\UAEquit') (Join-Path $qttest 'C')
	$script = "FailAt 21`nQTTEST:C/probe >QTTEST:capture/amiga.log`nQTTEST:C/UAEquit`n"
	[IO.File]::WriteAllText((Join-Path $qttest 'S\run-tests'), $script, [Text.Encoding]::ASCII)

	$config = Join-Path $run 'probe.uae'
	$template.Replace('@KICKSTART@', $cfg.Kickstart).Replace('@HARDFILE@', $cfg.HardFile).
		Replace('@BOOTDIR@', (Join-Path $root 'tests\amiga\boot')).Replace('@TESTDIR@', $qttest).Replace('@JITCACHE@', '0') |
		Set-Content -Path $config -Encoding ASCII

	Write-Host "== $exe"
	$process = Start-Process (Join-Path $uae $exe) -ArgumentList '-f', "`"$config`"" -WorkingDirectory $uae -PassThru
	if (-not $process.WaitForExit($TimeoutSec * 1000)) {
		Stop-Process $process -Force
		Write-Warning "$exe did not quit within $TimeoutSec s"
	}
	$log = Join-Path $qttest 'capture\amiga.log'
	if (Test-Path $log) { Get-Content $log | ForEach-Object { Write-Host "   $_" } }
	else { Write-Warning "no output from the probe" }
}

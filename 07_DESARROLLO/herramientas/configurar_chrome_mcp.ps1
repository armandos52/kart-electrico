$ErrorActionPreference = 'Stop'
$taskConfigPath = 'C:\Users\nandi\.codex\config.toml'
$taskNodePath = 'C:\Program Files\nodejs\node.exe'
$taskNpxPath = 'C:\Program Files\nodejs\node_modules\npm\bin\npx-cli.js'
$taskChromePath = 'C:\Program Files\Google\Chrome\Application\chrome.exe'
foreach ($taskRequiredPath in @($taskConfigPath, $taskNodePath, $taskNpxPath, $taskChromePath)) {
    if (-not (Test-Path -LiteralPath $taskRequiredPath)) { throw "No existe: $taskRequiredPath" }
}
$taskConfigText = [IO.File]::ReadAllText($taskConfigPath)
if ($taskConfigText -match '(?m)^\[mcp_servers\.(chrome-devtools|chrome_devtools)\]') {
    throw 'Ya existe Chrome DevTools MCP; revisar su configuración antes de modificarla.'
}
$taskBackupPath = $taskConfigPath + '.antes-chrome-' + (Get-Date -Format 'yyyyMMdd-HHmmss') + '.bak'
Copy-Item -LiteralPath $taskConfigPath -Destination $taskBackupPath
$taskMcpBlock = @'

[mcp_servers.chrome-devtools]
command = 'C:\Program Files\nodejs\node.exe'
args = ['C:\Program Files\nodejs\node_modules\npm\bin\npx-cli.js', '-y', 'chrome-devtools-mcp@latest', '--executable-path=C:\Program Files\Google\Chrome\Application\chrome.exe', '--isolated', '--no-usage-statistics', '--no-performance-crux']
startup_timeout_sec = 60
tool_timeout_sec = 120
enabled = true
'@
[IO.File]::WriteAllText($taskConfigPath, $taskConfigText.TrimEnd() + "`r`n" + $taskMcpBlock + "`r`n", [Text.UTF8Encoding]::new($false))
Write-Output "Configuración añadida: $taskConfigPath"
Write-Output "Copia previa: $taskBackupPath"
& 'C:\Users\nandi\AppData\Local\Programs\OpenAI\Codex\bin\codex.exe' mcp get chrome-devtools --json
if ($LASTEXITCODE -ne 0) { throw 'Codex no ha podido validar la configuración añadida.' }

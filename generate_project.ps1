param (
	[string]$toolset = "vs2022",
	[string]$target_abi = $null,
	[string]$device
)

# Let the environment override a toolset, if there is an environment variable set.
if ($env:toolset)
{
	$toolset = $env:toolset
}

$parameters = "--file=.\Builder\Project_Builder.lua"

if ($device)
{
	$parameters = "$parameters --target-device=$device"
}
if ($target_abi)
{
	$parameters = "$parameters --target-abi=$target_abi"
}

# Append the toolset.
$parameters = "$parameters $toolset"
$executable = ".\Builder\utils\premake5.exe"

$process = Start-Process $executable $parameters -Wait -NoNewWindow -PassThru
if ($process.ExitCode -ne 0) { throw "Project generation error " + $process.ExitCode }

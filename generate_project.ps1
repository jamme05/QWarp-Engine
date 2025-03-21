param (
	[string]$toolset = "vs2022",
	[string]$target_abi = $null,
	[string]$device,
	[switch]$unity_build = $false
)

# Let the environment override a toolset, if there is an environment variable set.
if ($env:toolset)
{
	$toolset = $env:toolset
}

$parameters = ""

if ($device)
{
	$parameters = "$parameters --target-device=$device"
}
if ($library_share_cache)
{
	$parameters = "$parameters --library-share-cache"
}
if ($unity_build)
{
	$processor_info = Get-WmiObject -Class Win32_Processor -Property "NumberOfLogicalProcessors"
	$num_logical_processors = ($processor_info."NumberOfLogicalProcessors" -split '\n')[0]
	$parameters = "$parameters --compilationunit=$num_logical_processors"
}
if ($target_abi)
{
	$parameters = "$parameters --target-abi=$target_abi"
}

# Append the toolset.
$parameters = "$parameters $toolset"
$executable = "utils\premake5"

$process = Start-Process $executable $parameters -Wait -NoNewWindow -PassThru
if ($process.ExitCode -ne 0) { throw "Project generation error " + $process.ExitCode }

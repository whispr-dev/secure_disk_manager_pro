New-Item -Path $PROFILE -Type File -Force

oh-my-posh init pwsh --config C:\Users\<YourUser>\.oh-my-posh\settings.json | Invoke-Expression

Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned

Start-Transcript -Path "C:\Users\<YourUser>\powershell_transcript_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt" -Append


# Initialize oh-my-posh
oh-my-posh init pwsh --config C:\Users\<YourUser>\.oh-my-posh\settings.json | Invoke-Expression

# Dynamic default directory
$currentDrive = (Get-Location).Drive.Name
if ($currentDrive -eq "C") {
    Set-Location C:\
} elseif ($currentDrive -eq "D") {
    Set-Location D:\code\repos\GitHub_Desktop\
}

# Start transcript for logging with timestamps
Start-Transcript -Path "C:\Users\<YourUser>\powershell_transcript_$(Get-Date -Format 'yyyyMMdd_HHmmss').txt" -Append



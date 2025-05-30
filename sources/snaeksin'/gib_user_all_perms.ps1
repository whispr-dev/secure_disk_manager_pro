`icacls "C:\Users\<name>\Documents\PowerShell" /grant "<user>:F"`


also optional alt to set bypass powers:
`Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy Bypass`

or RemoteSigned if you smort enough to settle for less:
`Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy RemoteSigned`


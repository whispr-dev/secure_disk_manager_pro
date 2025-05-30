# Option 4: Change Execution Policy (if Group Policy Allows)
----------------------------------------------------------

If Group Policy isn’t locking you down, set the execution policy to Bypass or `Unrestricted` at a scope you control (e.g., `CurrentUser`).

## Steps:


### Check Policy Scopes:

`powershell`
`Get-ExecutionPolicy -List`

Look for scopes like `CurrentUser` or `LocalMachine` that you can modify.

### Set Policy for `CurrentUser`:
Try setting the policy at the `CurrentUser` scope (no admin rights needed):


`powershell`
`Set-ExecutionPolicy -Scope CurrentUser -ExecutionPolicy Bypass -Force`

### Run the Script:
Execute your command as before.


## Why This Works:

Setting Bypass at CurrentUser overrides RemoteSigned for your user account, allowing unsigned scripts.

`-Force` suppresses prompts.

- Note: If Group Policy enforces RemoteSigned at MachinePolicy or UserPolicy, this won’t work, and you’ll need Option 1 or 2.

------------------------------------------------------------------------------
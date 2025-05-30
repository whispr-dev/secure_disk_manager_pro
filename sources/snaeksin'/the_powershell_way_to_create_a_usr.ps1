the powershell way to create a new user:

`net user <devuser> <password> /add`
`net localgroup Users <devuser> /add`

beware, devuser and password are your hardcore root win user usr/pwd for a long time if you don't got powers - choose wisely
[in theory this is only goo dfor std users - it wont on its own build you an admin ever - that requires some v specific powers, settings and choices of terminal etc.]


youn can however tempry elevate to admin even from within std usr by:
`Start-Process powershell -Verb runAs`
[just run as admin via cli tbh]

need proof?
`whoami /groups | findstr "Mandatory"`

we _can_ however, with the right admin powers then proceed to:
`runas /user:<devuser> powershell`
yup! we can assume the powers of other users, including perma admins, gods etc. and ofc once oyu got thier powers oyu then have pwoer to gib yourself what you please!!!


and a neat lil tricks - you want someone's full profile setup at oyur fingertips? then:
`$sourceProfile = "C:\Users\<adminusr>\Documents\PowerShell\Microsoft.PowerShell_profile.ps1"`
`$destProfile = "C:\Users\<devuser>\Documents\PowerShell\Microsoft.PowerShell_profile.ps1"`
`New-Item -Path (Split-Path $destProfile -Parent) -ItemType Directory -Force`
`Copy-Item -Path $sourceProfile -Destination $destProfile -Force`

will copy entire usr profile absolute identikit across to you _no matter come what may_!!!

[oh, or ofc:
`Copy-Item -Path "C:\Users\<Admin>\.config\powershell\profile.ps1" -Destination "C:\Users\<devuser>\.config\powershell\profile.ps1" -Force`
if they are using rofile as custom choce in alt path with a .config]


hehehe - this is slightly dumber, but - istheir oh-my-posh profile rather fetching and you fancy it?
`Copy-Item -Path "C:\Users\<admin>\.oh-my-posh\settings.json" -Destination "C:\Users\<devuser>\.oh-my-posh\settings.json" -Force`
hahahaha - yurp - now you got it wholesal to edit into an even better one for your very own self!!!



don't forget ofc you nee oh-my-posh installe:
`winget install JanDeDobbeleer.OhMyPosh`



you wanted the _real_ meat? the true evils? ...ok, ok - here we go:
`net localgroup Administrators <adnmin> /delete`
oh noes! admin isn't admin no more!!!

[you're NOT deleting _them_ - only their admin profile - so, they should now see:
`whoami /groups | findstr "Mandatory"`  Should show Medium Mandatory Level (standard user).` D: ...oh deary me!


btw, personal bugbear: whilst you're admin, please, please, please - open group policy editor form winsearch, and
`[Computer Configuration] > [Administrative Templates] > [System] > [Group Policy]` - upon navigating here check the settings to:
“Group Policy slow link detection” - if set below 1000 or is disable/offtheen your own system could be throttling your internet qutie SIGNIFICANTLY!!!
[default is 500 if disabled, off or no config. so if too low under approx. 1000-2000 or is defaulting to it stoopit crazy 500 default, guess what -you get coppercable broadband speeds!! yup, willy windows never accounted for the world going fibre 15yrs after they set this!]

if you do make changes, and then decide they're actually not crazy excessive n making it wroserer, then don't forget:
in PS, do `gpupdate /force` to 'fix' them in place or they'll just revert all over again. doh!




it's nice when you're admin to set things _your_ way ofc :D

PS C:\Users\user> `rustup default stable-gnu` or `rustup default stable-msvc` will gib gnu toolchained updated rust por msvc toolchained updtaed stable rust as per preferece.

[output:]
`info: using existing install for 'stable-x86_64-pc-windows-gnu'`
`info: default toolchain set to 'stable-x86_64-pc-windows-gnu'`

`  stable-x86_64-pc-windows-gnu unchanged - rustc 1.86.0 (05f9846f8 2025-03-31)`

[to check it:]
PS C:\Users\phine> `rustup show`

[output:]
`Default host: x86_64-pc-windows-gnu`
`rustup home:  C:\Users\phine\.rustup`

`installed toolchains`
`--------------------`
`stable-x86_64-pc-windows-gnu (active, default)`

`active toolchain`
`----------------`
`name: stable-x86_64-pc-windows-gnu`
`active because: it's the default toolchain`
`installed targets:`
`  x86_64-pc-windows-gnu`

and ofc you can do similarly for your preferred if it be C++ or Python or AL or Ruby or w/e... eh, make yourself at home and set your stuff up stable/fully tooled/latest ver. etc. - after all, you deserve it, right?



btw, you like the whole "custom powershell profile" thing but ain't too bothered bout the whole destroyink your bosses life thing from earlier?
then simply:
`$customProfile = "C:\Users\devuser\.config\powershell\profile.ps1"`
`New-Item -Path (Split-Path $customProfile -Parent) -ItemType Directory -Force`
`Copy-Item -Path $PROFILE -Destination $customProfile -Force`

then do:
`C:\Users\devuser\.config\powershell\profile.ps1`
to update `$PROFILE`


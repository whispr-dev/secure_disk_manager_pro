
How to Crack a WiFi Network’s WPA/WPA2 Password with Reaver
Mar 26th, 2012 by carrumba

Just to complete the list of WLAN hacking posts I decided to add this article as well.
The original article (written by Adam Pash) can be found at lifehacker.com

How to Crack a Wi-Fi Network’s WPA Password with Reaver
A new, free, open-source tool called Reaver exploits a security hole in wireless routers and can crack most routers’ current passwords with relative ease. Here’s how to crack a WPA or WPA2 password, step by step, with Reaver—and how to protect your network against Reaver attacks.

What You’ll Need
You don’t have to be a networking wizard to use Reaver, the command-line tool that does the heavy lifting, and if you’ve got a blank DVD, a computer with compatible Wi-Fi, and a few hours on your hands, you’ve got basically all you’ll need. There are a number of ways you could set up Reaver, but here are the specific requirements for this guide:

The BackTrack 5 Live DVD.
BackTrack is a bootable Linux distribution that’s filled to the brim with network testing tools, and while it’s not strictly required to use Reaver, it’s the easiest approach for most users. Download the Live DVD from BackTrack’s download page and burn it to a DVD. You can alternately download a virtual machine image if you’re using VMware, but if you don’t know what VMware is, just stick with the Live DVD. As of this writing, that means you should select BackTrack 5 R1 from the Release drop-down, select Gnome, 32- or 64-bit depending on your CPU (if you don’t know which you have, 32 is a safe bet), ISO for image, and then download the ISO.

A computer with Wi-Fi and a DVD drive.
BackTrack will work with the wireless card on most laptops, so chances are your laptop will work fine. However, BackTrack doesn’t have a full compatibility list, so no guarantees. You’ll also need a DVD drive, since that’s how you’ll boot into BackTrack. I used a six-year-old MacBook Pro.

A nearby WPA-secured Wi-Fi network.
Technically, it will need to be a network using WPA security with the WPS feature enabled. I’ll explain in more detail in the “How Reaver Works” section how WPS creates the security hole that makes WPA cracking possible.

A little patience.
This is a 4-step process, and while it’s not terribly difficult to crack a WPA password with Reaver, it’s a brute-force attack, which means your computer will be testing a number of different combinations of cracks on your router before it finds the right one. When I tested it, Reaver took roughly 2.5 hours to successfully crack my password. The Reaver home page suggests it can take anywhere from 4-10 hours. Your mileage may vary.

Let’s Get Crackin’
At this point you should have BackTrack burned to a DVD, and you should have your laptop handy.

Step 1: Boot into BackTrack
To boot into BackTrack, just put the DVD in your drive and boot your machine from the disc. (Google around if you don’t know anything about live CDs/DVDs and need help with this part.) During the boot process, BackTrack will prompt you to to choose the boot mode. Select “BackTrack Text – Default Boot Text Mode” and press Enter.

Eventually BackTrack will boot to a command line prompt. When you’ve reached the prompt, type startx and press Enter. BackTrack will boot into its graphical interface.

Step 2: Install Reaver
Reaver has been added to the bleeding edge version of BackTrack, but it’s not yet incorporated with the live DVD, so as of this writing, you need to install Reaver before proceeding. (Eventually, Reaver will simply be incorporated with BackTrack by default.) To install Reaver, you’ll first need to connect to a Wi-Fi network that you have the password to.

Click Applications > Internet > Wicd Network Manager
Select your network and click Connect, enter your password if necessary, click OK, and then click Connect a second time.
Now that you’re online, let’s install Reaver. Click the Terminal button in the menu bar (or click Applications > Accessories > Terminal). At the prompt, type:

  
root@root:~# apt-get update
And then, after the update completes:

  
root@root:~# apt-get install reaver
If all went well, Reaver should now be installed. It may seem a little lame that you need to connect to a network to do this, but it will remain installed until you reboot your computer. At this point, go ahead and disconnect from the network by opening Wicd Network Manager again and clicking Disconnect. (You may not strictly need to do this. I did just because it felt like I was somehow cheating if I were already connected to a network.)

Step 3: Gather Your Device Information, Prep Your Crackin’
In order to use Reaver, you need to get your wireless card’s interface name, the BSSID of the router you’re attempting to crack (the BSSID is a unique series of letters and numbers that identifies a router), and you need to make sure your wireless card is in monitor mode. So let’s do all that.

Find your wireless card:

  
root@root:~# iwconfig
lo        no wireless extensions.

eth0      no wireless extensions.

wlan0     IEEE 802.11abg  ESSID:"default"
          Mode:Managed  Frequency:2.437 GHz  Access Point: Not-Associated
          Tx-Power=20 dBm
          Retry  long limit:7   RTS thr:off   Fragment thr:off
          Encryption key:off
          Power Management:off
Put your wireless card into monitor mode: Assuming your wireless card’s interface name is wlan0, execute the following command to put your wireless card into monitor mode:

  
root@root:~# airmon-ng start wlan0
This command will output the name of monitor mode interface, which you’ll also want to make note of. Most likely, it’ll be mon0. Make note of that.

Find the BSSID of the router you want to crack: Lastly, you need to get the unique identifier of the router you’re attempting to crack so that you can point Reaver in the right direction. To do this, execute the following command:

  
root@root:~# airodump-ng wlan0
(Note: If airodump-ng wlan0 doesn’t work for you, you may want to try the monitor interface instead—e.g., airodump-ng mon0.)

You’ll see a list of the wireless networks in range—it’ll look something like the screenshot below:

  
[ CH  1 ] [Elapsed: 4s ] [ 2012-03-20 13:23] [ WPA handshake: 00:14:6C:7E:40:80
                                                                                                            
 BSSID              PWR RXQ  Beacons    #Data, #/s  CH  MB   ENC  CIPHER AUTH ESSID
                                                                                                            
 00:09:6B:1C:AA:1D   11  16       10        0    0  11  54.  OPN              NETGEAR                         
 00:14:9C:7A:41:81   34 100       57       14    1   9  11e  WEP  WEP         bigbear 
 00:14:DC:7E:40:80   32 100      752       73    2   9  54   WPA  TKIP   PSK  teddy                             
...                                                                                   
When you see the network you want, press Ctrl+C to stop the list from refreshing, then copy that network’s BSSID (it’s the series of letters, numbers, and colons on the far left). The network should have WPA or WPA2 listed under the ENC column.
Now, with the BSSID and monitor interface name in hand, you’ve got everything you need to start up Reaver.

Step 4: Crack a Network’s WPA Password with Reaver
Now execute the following command in the Terminal, replacing bssid and moninterface with the BSSID and monitor interface and you copied down above:

  
root@root:~# reaver -i moninterface -b bssid -vv
For example, if your monitor interface was mon0 like mine, and your BSSID was 8D:AE:9D:65:1F:B2 (a BSSID I just made up), your command would look like:

  
root@root:~# reaver -i mon0 -b 8D:AE:9D:65:1F:B2 -vv
Press Enter, sit back, and let Reaver work its disturbing magic. Reaver will now try a series of PINs on the router in a brute force attack, one after another. This will take a while. In my successful test, Reaver took 2 hours and 30 minutes to crack the network and deliver me with the correct password. As mentioned above, the Reaver documentation says it can take between 4 and 10 hours, so it could take more or less time than I experienced, depending.

A few important factors to consider:
Reaver worked exactly as advertised in my test, but it won’t necessarily work on all routers (see more below). Also, the router you’re cracking needs to have a relatively strong signal, so if you’re hardly in range of a router, you’ll likely experience problems, and Reaver may not work. Throughout the process, Reaver would sometimes experience a timeout, sometimes get locked in a loop trying the same PIN repeatedly, and so on. I just let it keep on running, and kept it close to the router, and eventually it worked its way through.

Also of note, you can also pause your progress at any time by pressing Ctrl+C while Reaver is running. This will quit the process, but Reaver will save any progress so that next time you run the command, you can pick up where you left off-as long as you don’t shut down your computer (which, if you’re running off a live DVD, will reset everything).

How Reaver Works
Now that you’ve seen how to use Reaver, let’s take a quick overview of how Reaver works. The tool takes advantage of a vulnerability in something called Wi-Fi Protected Setup, or WPS. It’s a feature that exists on many routers, intended to provide an easy setup process, and it’s tied to a PIN that’s hard-coded into the device. Reaver exploits a flaw in these PINs; the result is that, with enough time, it can reveal your WPA or WPA2 password.

How to Protect Yourself Against Reaver Attacks
Since the vulnerability lies in the implementation of WPS, your network should be safe if you can simply turn off WPS (or, even better, if your router doesn’t support it in the first place). Unfortunately, as Gallagher points out as Ars, even with WPS manually turned off through his router’s settings, Reaver was still able to crack his password.

So that’s kind of a bummer. You may still want to try disabling WPS on your router if you can, and test it against Reaver to see if it helps.

You could also set up MAC address filtering on your router (which only allows specifically whitelisted devices to connect to your network), but a sufficiently savvy hacker could detect the MAC address of a whitelisted device and use MAC address spoofing to imitate that computer.

Double bummer. So what will work?

I have the open-source router firmware DD-WRT installed on my router and I was unable to use Reaver to crack its password. As it turns out, DD-WRT does not support WPS, so there’s yet another reason to love the free router-booster. If that’s got you interested in DD-WRT, check their supported devices list to see if your router’s supported. It’s a good security upgrade, and DD-WRT can also do cool things like monitor your internet usage, set up a network hard drive, act as a whole-house ad blocker, boost the range of your Wi-Fi network, and more. It essentially turns your $60 router into a $600 router.


#####


Wireless Security: A Partial Glossary of Wireless Security Terms

Just about a month ago, in early November, the news came out that the first cracks were appearing in WPA, or Wi-Fi Protected Access, a very popular wireless security standard. The compromise that was accomplished by some researchers was not a real killer, but the affected version of WPA (and the associated encryption process, TKIP, or Temporal Key Integrity Protocol), was always meant as a stopgap standard.

For some time now there have been better standards implemented in shipping wireless products, and there have been many articles published with good advice on improving your wireless security. For example, this one from eWEEK Labs’ Andrew Garcia discusses the attack itself and how you can protect yourself from it. This one from Dan Croft discusses bigger issues of wireless security architecture and policy.

After reading the wireless security news, Steven M. Bellovin, professor of Computer Science at Columbia University, decided to tighten up the security on his own home wireless network. Bellovin’s house is not an enterprise, so much of the advice and solutions available for dealing with this problem aren’t really applicable. Bellovin didn’t really know how to proceed, and if a professor of Computer Science at an Ivy League school can’t make immediate sense of it all, how is everyone else supposed to?

I decided to examine the wireless terms that Bellovin encountered in the various products he has to see what they all meant:

* WEP (Wired Equivalent Privacy)—The old, original, now discredited wireless security standard. Easily cracked.
* WEP 40/128-bit key, WEP 128-bit Passphrase—See WEP. The user key for WEP is generally either 40- or 128-bit, and generally has to be supplied as a hexadecimal string.
* WPA, WPA1—Wi-Fi Protected Access. The initial version of WPA, sometimes called WPA1, is essentially a brand name for TKIP. TKIP was chosen as an interim standard because it could be implemented on WEP hardware with just a firmware upgrade.
* WPA2—The trade name for an implementation of the 802.11i standard, including AES and CCMP.
* TKIP—Temporal Key Integrity Protocol. The replacement encryption system for WEP. Several features were added to make keys more secure than they were under WEP.
* AES—Advanced Encryption Standard. This is now the preferred encryption method, replacing the old TKIP. AES is implemented in WPA2/802.11i.
* Dynamic WEP (802.1x)—When the WEP key/passphrase is entered by a key management service. WEP as such did not support dynamic keys until the advent of TKIP and CCMP.
* EAP—Extensible Authentication Protocol. A standard authentication framework. EAP supplies common functions and a negotiation mechanism, but not a specific authentication method. Currently there are about 40 different methods implemented for EAP. See WPA Enterprise.
* 802.1x, IEEE8021X—The IEEE family of standards for authentication on networks. In this context, the term is hopelessly ambiguous.
* LEAP, 802.1x EAP (Cisco LEAP)—(Lightweight Extensible Authentication Protocol) A proprietary method of wireless LAN authentication developed by Cisco Systems. Supports dynamic WEP, RADIUS and frequent reauthentication.
* WPA-PSK, WPA-Preshared Key—Use of a shared key, meaning one manually set and manually managed. Does not scale with a large network either for manageability or security, but needs no external key management system.
* RADIUS—Remote Authentication Dial In User Service. A very old protocol for centralizing authentication and authorization management. The RADIUS server acts as a remote service for these functions.
* WPA Enterprise, WPA2 Enterprise—A trade name for a set of EAP types. Products certified as WPA Enterprise or WPA2 Enterprise will interoperate. The included types are:
o EAP-TLS
o EAP-TTLS/MSCHAPv2
o PEAPv0/EAP-MSCHAPv2
o PEAPv1/EAP-GTC
o EAP-SIM
* WPA-Personal, WPA2-Personal—See Pre-Shared Key.
* WPA2-Mixed—Support for both WPA1 and WPA2 on the same access point.
* authentication algorithms: OPEN, SHARED and LEAP—OPEN in this context meant no authentication; the network was open to all. SHARED refers to preshared key. for LEAP see LEAP.

And I’ll add a few more:

* 802.11i—An IEEE standard specifying security mechanisms for 802.11 networks. 802.11i uses AES and includes improvements in key management, user authentication through 802.1X and data integrity of headers.
* CCMP—Counter Mode with Cipher Block Chaining Message Authentication Code Protocol. An encryption protocol that uses AES.

Wireless Security: A Partial Glossary of Wireless Security Terms – Wireless Security: The Main Point
There, all clear now? Me neither.

The real point of Bellovin’s blog, which is clearly underscored by the definitions above, is that wireless products throw a vat of alphabet soup at users and it’s no surprise if they make bad decisions in configuration. It’s so easy to find a completely wide-open wireless network; is this because people just don’t care or because securing them is too hard? Some of both, I suppose.

The terms WPA Personal and WPA Enterprise are attempts to move beyond this problem, at least at the point of purchase. Home users would just look for WPA Personal-compliant products, and enterprises would look for WPA Enterprise, and be assured of a fairly high level of interoperability. But it’s no guarantee of plug-and-play secure networks.

There’s no easy way out of this problem. Unfortunately, vendors have a strong incentive to retain support for old standards, as they are widely implemented, and this means that configuration screens will be loaded up with lots of the alphabet soup above. Education is not usually a great solution for a security problem, but that’s all we’re left with in wireless security configuration. Wish us luck.


#####

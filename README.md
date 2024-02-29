# foo_drpc
Foobar2000 music status for Discord Rich Presence!

# Notice
I'm currently no longer actively developing this, but will still merge PRs and reference releases at the [release page](https://github.com/ultrasn0w/foo_drpc/releases).

You mabe also want to check out [this wonderful alternative](https://github.com/TheQwertiest/foo_discord_rich) that was made much more recently.

# How to use
1. Grab [release](https://github.com/ultrasn0w/foo_drpc/releases), drop included **foo_drpc** directory in \%userdir%\AppData\Roaming\foobar2000\user-components\ (if you have not moved your AppData somewhere else) or place included .dll Files in \foobar2000\components\.
2. ~~Add foobar2000 to discords detected games (Settings -> Games -> Add it).~~

![compact view](/foo_drpc1.PNG?raw=true)
![big view 1](/foo_drpc2.PNG?raw=true)
![big view 2](/foo_drpc3.PNG?raw=true)

# How to compile
0. Compiled with VS 2017.
1. Grab foobar2000 SDK and create \foo_drpc\ directory in \foobar2000\.
2. Drop contents from repository in the prevoiusly created \foo_drpc\.
3. Grab release from https://github.com/discordapp/discord-rpc and place \lib\ with contained discord-rpc.lib in \foo_drpc\.
4. Do the same with \include\discord-rpc.h but this time directly into \foo_drpc\.
5. Get/Create a Discord Application ID which resembles your App at Discords end and fill in APPLICATION_ID in \foo_drpc\secret.h.
6. Upload 1 large asset for your App with the key "logo", 3 small ones with keys "play", "stop" and "pause". Use the files in optional_images if you like.

# License and Warranty
Check [LICENSE](../master/LICENSE).

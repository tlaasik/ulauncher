# Simple launcher for Unity Windows builds

Features:
  * Runs 32 and 64 bit builds from the same data folder
  * Detects game crash and asks user to send output_log to devs

It appears that Unity 64 and 32 bit builds can use the same data folder, but there are two mono related DLLs that can't be the same. The launcher ensures that those two dll-s are what they need to be, then finds either .x64 or .x86 file and executes it. Then it waits until game terminates and if there is "Crash!!!" in log then it shows a message about it.

[How the game directory looks like](ulauncher_forum_pic.png)

Setup:
  * Make win64 build of your game, this will be your working folder
  * Rename data folder to "Data" and exe to <dontcare>.x64
  * If using mono 2.0 rename "Data\Mono\mono.dll"->"Data\Mono\mono-x64.dll" and "Data\Mono\MonoPosixHelper.dll"->"Data\Mono\MonoPosixHelper-x64.dll"
  * If using mono 4.6 rename Rename "Data\MonoBleedingEdge\EmbedRuntime\mono-2.0-bdwgc.dll"->"Data\MonoBleedingEdge\EmbedRuntime\mono-2.0-bdwgc-x86.dll" and "Data\MonoBleedingEdge\EmbedRuntime\MonoPosixHelper.dll"->"Data\MonoBleedingEdge\EmbedRuntime\MonoPosixHelper-x86.dll"
  * Make win32 build of your game
  * Copy win32 exe into working folder and rename it to <dontcare>.x86
  * Copy 2 DLLs from win32 build data folder into working folder just like before, but with "-x86" prefix
  * Download launcher32.exe and launcher64.exe or build them with VS2015 (good for changing icon if not more)
  * Put launcher exe files into game folder, rename if you want to

[Download precompiled launcher exe files here](http://shuugames.com/download/ulauncher_0_1.zip)

If you don't want to deal with Visual Studio C++ part, but just want to change icon and error message texts in launcher then use lightweight resource modification tools. For example this one [Resource Hacker](http://www.angusj.com/resourcehacker)

This should run on both 32 and 64 bit versions of Windows 7, 8 and 10. It's verified to work on 64bit win10 and 64bit win8.1.

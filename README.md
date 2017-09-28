# Simple launcher for Unity Windows builds

Features:
  * Runs 32 and 64 bit builds from the same data folder
  * Detects game crash and asks user to send output_log to devs

Simple setup:
  * Build 64 bit windows version of your game
  * Rename game data folder to "Data" and delete exe file
  * Download and unpack "Launcher for Unity x.x.x.x.zip" matching your unity version into your project folder, overwrite files

If you don't trust my code blindly (why should you) you should build the launcher yourself with VS2015 and integrate it with your project. It appears that 64 and 32 bit builds can use same data folder, but there two mono related dll files that can't. The launcher ensures that those two dll-s are what they need to be, then finds either .x64 or .x86 file and executes it. Then it waits until game terminates and if there is "Crash!!!" in log then it shows a message about it.

More involved setup:
  * Make win64 build of your game, this will be your working folder
  * Rename data folder to "Data" and exe to <gamename>.x64
  * Rename "Data\Mono\mono.dll"->"Data\Mono\mono-x64.dll" and "Data\Mono\MonoPosixHelper.dll"->"Data\Mono\MonoPosixHelper-x64.dll"
  * Make win32 build of your game
  * Copy win32 exe into working folder and rename it to <gamename>.x86
  * Copy 2 dlls into working folder "Data\Mono\..." and rename them like before, but with "-x86" prefix
  * Download Launcher32.exe or Launcher64.exe or build them with VS2015 (menu Build/Batch build, edit source before if needed)
  * Put launcher exe files into game folder, rename if you want to

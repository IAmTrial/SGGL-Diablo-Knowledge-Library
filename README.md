# SlashGaming Game Loader - Knowledge Library
An extension library for SlashGaming Game Loader (SGGL) that displays game information and enables library injection for Windows 9X systems.

## How to Use
The DLL must be loaded by SGGL in order via the -k parameter like so:
SGGL.exe -k "SGGLDK.dll" (other options)

## How the Program Operates
This is highly technical and wordy so read if you are actually interested!

The program first uses the file strings to determine what game and version is run. While this works for most games, some versions share file string information. To get around this, several bytes are read in from a known location in one of the game's files. Afterwards, the values are compared against a table, where the game version is finally determined.

Injection is a lot more complicated than the base Game loader. Windows 9X systems do not have access to some of the functions commonly used to inject libraries. Therefore, the program must use a hacky form of inter-process communication via ReadProcessMemory and WriteProcessMemory to facilitate injection. Another issue is that there are several Windows functions that need to be called. The functions cannot be directly called due to the nature of how the call op works in x86.

The game processes are initially suspended, so code patches are applied a few bytes past the entry point of the game process. This is required for the cleanup patch that will be applied at the entry point. Next, the game process is resumed by the SGGL. SGGL then spinlocks, waiting for the "free space" stack pointer value to be set. The game process executes the entry hijack code, where the game process will call the payload function. The return address will be stored on the stack. In the payload function, the return address is modified to point to the location of the entry hijack patch's starting point. This will make it so when the patches are undone and the function returns, the execution continues like normal. The game process then sets the "free space" value located at a designated address to become the stack pointer. This is utilized by SGGL, for inter-process communication. The game process continues to execute initialization code until it enters a spinlock. This spinlock can only end when SGGL is able to determine the stack pointer. The SGGL is then able to freely initialize the stack data with pointers to the required Windows functions (e.g. SuspendThread, VirtualAlloc). Afterwards, the spinlock ends and the game process continues execution. An initial buffer is allocated using VirtualAlloc for a library path to be written to, and the size of the buffer is stored in the stack. Afterwards, the game process suspends itself using SuspendThread. After the first suspension, the cleanup patch is applied.

Using a combination of SuspendThread and ResumeThread, SGGL waits for the game process to suspend and checks whether the VirtualAlloc buffer is of sufficient size for the library path to be injected. If not, the game process's resize flag is set and the game process is resumed. SGGL then wait for the game process to suspend again. Each time the resize flag is set, the size is doubled and the buffer is reallocated. This repeats until the buffer is of sufficient size. From there, the path of the injected library is copied to the allocated buffer and the game process is resumed. SGGL waits for the game process to suspend. The game process then loads the library using LoadLibraryA, with the allocated buffer address being the parameter. Once the library is loaded, the game process suspends again. This is all repeated until there are no more libraries to inject.

Once there are no more libraries left to inject, the game process begins the process of cleanup. The cleanup function is located at the entry point, because the code has already been executed when the patch was not applied. This means that when the game process returns, this code can still be modified without any issue. Before the cleanup function is called, SGGL is waiting for the game process to suspend. The game process first frees the allocated buffer, then jumps to the cleanup function. The game process then suspends itself as its first action. SGGL then removes the entry hijack patch and the payload function patch so that normal game code can be executed. SGGL resumes the game process and then spinlocks, waiting for the game stack data to be modified. The game process then returns to where it left off, executing normal code. Once it SGGL has determined that the stack has been modified, it then removes the cleanup patch and finishes library injection for that process.

## Multiplayer Use
This program will likely work in most multiplayer games, but using the injection functionality will likely get you banned by whatever game's anticheat is being used. Use this program at your own risk.

## License
SlashGaming Game Loader - Diablo Knowledge Library is licensed under the Affero General Public License, version 3 or higher, with exceptions. If you would like to apply for a licensing exception, please contact Mir Drualga via the SlashDiablo Discord.

## Thanks
- [/r/SlashGaming](https://www.reddit.com/r/slashdiablo/): Getting me interested in working on Diablo II.
- [PhrozenKeep](https://d2mods.info): Providing tons of resources for Diablo II modders.

## Legal Notice
Diablo, Diablo II, and Diablo II: Lord of Destruction are registered trademarks of Blizzard Entertainment. This project is not affiliated with Blizzard Entertainment in any way.

Diablo: Hellfire is a registered trademark of Sierra Entertainment. This project is not affiliated with Sierra Entertainment in any way.

# Important INFO
This is the complete source code for Quake 2, version 3.19, buildable with
visual C++ 6.0.  The linux version should be buildable, but we haven't
tested it for the release.

The code is all licensed under the terms of the GPL (gnu public license).  
You should read the entire license, but the gist of it is that you can do 
anything you want with the code, including sell your new version.  The catch 
is that if you distribute new binary versions, you are required to make the 
entire source code available for free to everyone.

The primary intent of this release is for entertainment and educational 
purposes, but the GPL does allow commercial exploitation if you obey the 
full license.  If you want to do something commercial and you just can't bear 
to have your source changes released, we could still negotiate a separate 
license agreement (for $$$), but I would encourage you to just live with the 
GPL.

All of the Q2 data files remain copyrighted and licensed under the 
original terms, so you cannot redistribute data from the original game, but if 
you do a true total conversion, you can create a standalone game based on 
this code.

Thanks to Robert Duffy for doing the grunt work of building this release.

John Carmack
Id Software

# MELEE AND MAGIC IN QUAKE 2
Mod includes an implementation of simulation of melee weapons and spells to combat enemies.
Certain enemies have weaknesses and resistances to spell according to their element.

# How to INSTALL
Assuming you already own Quake 2 on Steam and that you have a GitHub account

1) Create a new repository and clone this one
2) Pull the contents to your machine
3) Look in Program Files and search for the folder Steam. Look for where Quake 2 is installed.
   Steam->steamapps->common->Quake 2
4) Create a folder (choose a name)
5) Enter the pulled repo and follow game->release->gamex86.dll
6) Copy gamex86.dll into your folder from step 4
7) Create a shortcut of quake2.exe. Enter the properties of the shortcut
   and modify Target and add (after the exe") +set game FolderNameFromStep4
8) Run the game using the shortcut. And then quit.
9) Go to your folder, find the config file and edit it with NotePad
10) Find the respective keybinds and change:

bind c "fire"

bind v "void"

bind b "aero"

bind n "blaze"

bind MOUSE2 "tp"
    
# How to PLAY and TEST
Go around and kill a few enemies. They should drop equipment. However, they do not have resistances.
I have included the console command spawn that spawns mosnters in front of where the player is looking. 
The working versions are:

1) spawn soldier
2) spawn berserk
3) spawn gunner
4) spawn infantry
5) spawn flyer

These monsters include the changes for resistances. Spawn a few and test their resistance of weakness to a spell.
A message should display everytime a monster is spawned telling you the HP and Element of the monster.
Attack the monster with a spell and check the resistances and weaknesses on the messages displayed
The weapons and spells are very overpowered, mostly to showcase a change.
You might want to use the following command for playing and testing

1) god (makes you invincible)
2) give all (give you everything included all weapons)

In game, press F1 to display the help guide.

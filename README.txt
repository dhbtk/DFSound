                   ____  _____ ____                         _ 
                  |  _ \|  ___/ ___|   ___  _   _ _ __   __| |
                  | | | | |_  \___ \  / _ \| | | | '_ \ / _` |
                  | |_| |  _|  ___)| | (_) | |_| | | | | (_| |
                  |____/|_|   |____/  \___/ \__,_|_| |_|\__,_|                  

                  By NieXS                      Version 1.0.0

+-----------------\
| What is DFSound? \
+-------------------\

DFSound is a game log parser for Dwarf Fortress. It parses the log for events,
currently supporting playing music and sound effects. It uses an XML file for
event input and a GTK+ GUI for user interaction. DFSound is tested under Linux,
but the code is not Linux-dependent and should compile under OS X and Windows.

DFSound comes with a default sound pack. Creating your own packs is easy: you
only need to know basic XML syntax and basic Regexes. DFSound can also use
SoundSense's sound packs.

+---------------------------------\
| Installing/Compiling under Linux \
+-----------------------------------\

DFSound depends on GTK+ 2.0, SDL, SDL-Mixer and libxml2. On Debian-derived
distributions, you can install them like this:

	# apt-get install libgtk-2.0-dev libsdl1.2-dev libsdl-mixer1.2-dev libxml2-dev

You will also need the basic development tools, if you don't have them installed:

	# apt-get install build-essential

DFSound uses waf for compiling, and thus requires python to be installed during
compilation. Using waf is simple:

	$ ./waf configure --prefix=/home/you
	$ ./waf build
	$ ./waf install

The executable will then be installed in your bin/ folder. You can also uninstall
DFSound easily running ./waf uninstall.

+--------------\
| Using DFSound \
+----------------\

1. Launch the dfsound file.
2. Pick your gamelog.txt file by clicking on the button under "Log file:".
2. Pick your events file by clicking on the button under "Events:".
3. Click the "Active" button.
4. Wait for events.
5. Have fun!

+---------------------\
| Creating sound packs \
+-----------------------\

DFSound uses XML files for its events file. Here's a sample file:

	<?xml version="1.0"?>
	<events>
		<event pattern="Loading Fortress" default="default">
			<music>song_game.ogg</music>
		</event>
		<event pattern="has given birth to a (boy|girl)">
			<sfx>child_birth.ogg</sfx>
		</event>
		<event pattern="^A vile force of darkness">
			<music>siege.mp3</music>
		</event>
	</events>

The pattern is a Perl-compatible regular expression. These are simple when used
for simple things, like this one. You can find plenty of regex information on
the internet, for example, in the website <http://www.regular-expressions.info>.

The "default" attribute is used to make an event the default one, for things
like background music.

"music" and "sfx" indicate which songs/sound effects to play. If more than one
of them are provided, one is picked at random. Music and SFX must be placed in
file_path/music and file_path/sfx, respectively. The supported formats are all
that SDL_mixer can play, such as MP3, Ogg Vorbis, WAV files, etc.

+-------------\
| Known issues \
+---------------\

* Some random segfaults on corner use cases.
* Leaks memory after switching event files, so don't switch files like a madman!

+-------------\
| Wanted stuff \
+---------------\

* An OS X build.
* Linux builds, both 32 and 64 bit, for the lazy.

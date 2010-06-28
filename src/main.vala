errordomain DFSoundError {
	MISSING_LOG,
	MALFORMED_PATTERN,
	MISSING_PATTERN
}
class DFSound : Object {
	private string[] sound_effects = {};
	private List<Event> events = new List<Event>();
	private Event current_stateful_event;
	private DataInputStream gamelog;
	private SDLMixer.Music music;
	
	private class Event {
		public string pattern;
		public string[] music;
		public string[] sfx;
		public bool stateful;
		public Event(string? pattern,string?[] music,string?[] sfx,bool stateful) {
			this.pattern  = pattern;
			this.music    = music;
			this.sfx      = sfx;
			this.stateful = stateful;
		}
	}
	
	public DFSound(string[] args) {
		print("DFSound 0.1\n");
		print("Copyright (c) 2010 NieXS - All rights reserved.\n");
		print("See the file LICENSE for licensing information.\n");
		print("\n");
		print("Starting up...\n");
		print("Starting up SDL...\n");
		SDL.init(SDL.InitFlag.AUDIO);
		SDLMixer.open(44010,SDL.AudioFormat.S16,2,512);
		SDLMixer.Channel.allocate(6);
		print("Loading events...\n");
		Xml.Parser.init();
		Xml.Doc* doc = Xml.Parser.parse_file("events.xml");
		Xml.Node* root = doc->get_root_element();
		for(Xml.Node* node = root->children;node != null; node = node->next) {
			if(node->type != Xml.ElementType.ELEMENT_NODE) continue;
			string pattern = node->get_prop("pattern");
			if(pattern != null) {
				try {
					new Regex(pattern);
				} catch(RegexError e) {
					throw new DFSoundError.MALFORMED_PATTERN(pattern);
				}
			} else {
				throw new DFSoundError.MISSING_PATTERN("Missing pattern");
			}
			string[] music = {};
			string[] sfx = {};
			bool stateful = false;
			for(Xml.Node* child = node->children;child != null; child = child->next) {
				if(node->type != Xml.ElementType.ELEMENT_NODE) continue;
				if(child->name == "music") {
					music += child->get_content();
					print(child->get_content()+"\n");
				} else if(child->name == "sfx") {
					sfx += child->get_content();
					print(child->get_content()+"\n");
				} else if(child->name == "stateful") {
					stateful = true;
				}
			}
			events.append(new Event(pattern,music,sfx,stateful));
		}
		print("Opening game log...\n");
		File gamelog_file = File.new_for_path(args[1]);
		if(!gamelog_file.query_exists(null)) {
			print("---ERROR---\n");
			print("Could not open game log for some reason. Check the path.\n");
			throw new DFSoundError.MISSING_LOG("missing log");
		}
		gamelog = new DataInputStream(gamelog_file.read(null));
		// Seeking... ugly hack, but DataInputStream lacks a seeking function
		while(gamelog.read_line(null,null) != null);
		print("Ready.\n");
	}
	
	public void main_loop() {
		while(true) {
			string line = gamelog.read_line(null,null);
			if(line != null) {
				line = convert(line,(ssize_t)line.size(),"UTF-8","ISO-8859-1");
				assert(line.validate());
				print(line+"\n");
				foreach(Event event in events) {
					if(Regex.match_simple(event.pattern,line)) {
						print("Found event\n");
						if(event.stateful) {
							current_stateful_event = event;
						}
						if(event.music.length != 0) {
							print("Playing music\n");
							string music = event.music[Random.int_range(0,event.music.length)];
							this.music = new SDLMixer.Music("music/"+music);
							this.music.play(0);
						}
						if(event.sfx.length != 0) {
							print("Playing sfx\n");
							string sfx_fname = event.sfx[Random.int_range(0,event.sfx.length)];
							SDLMixer.Chunk sfx = new SDLMixer.Chunk.WAV(new SDL.RWops.from_file("sfx/"+sfx_fname,"r"));
							SDLMixer.play(-1,sfx,0);
						}
						break;
					}
				}
			} else {
				Posix.usleep(100);
			}
			if(SDLMixer.Music.is_playing() != 1 && current_stateful_event != null) {
				string music = current_stateful_event.music[Random.int_range(0,current_stateful_event.music.length)];
				this.music = new SDLMixer.Music("music/"+music);
				this.music.play(1);
			}
			print(SDL.get_error());
		}
	}
}
int main(string[] args) {
	Random.set_seed((uint32)time_t());
	DFSound dfs = new DFSound(args);
	dfs.main_loop();
	return 0;
}
require 'yaml'
file = YAML::load(open(ARGV[0]))
string = "<events>\n"
for event in file
	string << "\t<event pattern=\"#{event['pattern']}\">\n"
	if event['music']
		if event['music'].kind_of?(String)
			string << "\t\t<music>#{event['music']}</music>\n"
		else
			for i in event['music']
				string << "\t\t<music>#{i}</music>\n"
			end
		end
	end
	if event['sfx']
		if event['sfx'].kind_of?(String)
			string << "\t\t<sfx>#{event['music']}</sfx>\n"
		else
			for i in event['sfx']
				string << "\t\t<sfx>#{i}</sfx>\n"
			end
		end
	end
	if event['stateful']
		string << "\t\t<stateful/>\n"
	end
	string << "\t</event>\n"
end
string << "</events>\n"
print string
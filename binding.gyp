{
	"targets": [{
		"target_name": "ansifilter",
		"sources": ["ansifilter.cc"],
		"include_dirs" : [
			"<!(node -e \"require('nan')\")"
		]
	}]
}

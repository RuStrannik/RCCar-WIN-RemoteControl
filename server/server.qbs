import qbs

Project {
	minimumQbsVersion: "1.7.1"
	name: "WinsocTCP_Server"

	CppApplication {
		consoleApplication: true
		files: [
			"server.cpp",
		]

		Group {     // Properties for the produced executable
			fileTagsFilter: product.type
			qbs.install: true
		}
	}
}

import qbs

Project {
	minimumQbsVersion: "1.7.1"
	name: "WinsocTCP_Client"

	CppApplication {
		consoleApplication: true
		files: [
			"client.cpp",
		]

		cpp.staticLibraries: [
			"User32.lib",
			"Ws2_32.lib",
			"Mswsock.lib",
			"Advapi32.lib",
		]

		Group {     // Properties for the produced executable
			fileTagsFilter: product.type
			qbs.install: true
		}
	}
}

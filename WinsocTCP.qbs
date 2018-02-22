import qbs

Project {
	name: "HD FPV RC Car over internet";

	references: [
		"server/server.qbs",
		"client/client.qbs",
	]
}

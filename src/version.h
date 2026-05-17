#ifndef _VERSION_H
#define _VERSION_H

static const char* CODE_NAMES[] = {
	"Perlica",    // v1.0 ⚡
	// For future use
	"Chen",       // v1.1 ⚔️
	"DaPan",      // v1.2 🥟
	"Endmin",     // v1.3 👨‍💻
};

#define VERSION  "1.0"
#define CODENAME CODE_NAMES[0]
#define BUILD    "17.05.2026"

static void BuildVersionString(char* buffer, size_t bufferSize) {
	SDL_snprintf(buffer, bufferSize, "Version %s \"%s\" (build %s)", VERSION, CODENAME, BUILD);
}

#endif
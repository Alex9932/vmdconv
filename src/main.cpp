/*
 * VMD animation convertor
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

static const char* CODE_NAMES[] = {
	"Perlica",    // v1.0 ⚡
	// For future use xD 
	"Chen",       // v1.1 ⚔️
	"DaPan",      // v1.2 🥟
	"Endmin",     // v1.3 👨‍💻
};

#define VERSION  "1.0"
#define CODENAME CODE_NAMES[0]
#define BUILD    "11.05.2026"

static uint32_t errors = 0;

static const char* ReadString(const char* path) {
	FILE* file = fopen(path, "r");
	if (!file) {
		return NULL;
	}

	fseek(file, 0, SEEK_END);
	size_t length = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* buffer = (char*)malloc(length + 1);
	if (!buffer) {
		fclose(file);
		return NULL;
	}

	fread(buffer, 1, length, file);
	buffer[length] = '\0';

	fclose(file);
	return buffer;
}

static void WriteString(const char* path, const char* str) {
	FILE* f = fopen(path, "w");
	if (f) {
		fprintf(f, "%s", str);
		fclose(f);
	}
}

static void PrintBanner() {
	const char* quotes[] = {
		"Factory operations nominal.",
		"Do not touch the Originium...",
		"Cooking something special...",
		"Endmin is watching you.",
		"Over here Endmin!",
		"Have you seen my wrench?"
	};

	srand(time(NULL));

	printf("\033[1;36m");
	printf("**\n");
	printf("** %s\n", quotes[rand() % 6]);
	printf("**\n");
	printf("** VMDCONV (Version %s \"%s\", build %s) by Alex9932\n", VERSION, CODENAME, BUILD);
	printf("**  - Made with love for Talos - II pioneers.\n");
	printf("**\n");
	printf("\033[0m");
}

static void printhelp() {
	printf("\n** ~ ~ ~ VMD Animation convertor ~ ~ ~\n\n");
	printf("** Usage:\n");
	printf("**   TODO: make help screen!\n");
}



int main(int argc, char** argv) {

	PrintBanner();

	if (argc < 2) {
		printhelp();
		return 0;
	}

	if (strcmp(argv[1], "make") == 0 && argc > 2) {
		printf("** Executing \"make\" command\n");
		//MakeConfig(argv[2]);
	} else if (strcmp(argv[1], "fetch") == 0) {
		printf("** Executing \"fetch\" command\n");
		//FetchList();
	} else if (strcmp(argv[1], "squad") == 0) {
		printf("** Executing \"squad\" command\n");
		printf("\033[1;36m");
		printf("**\n** MCU Development Squad:\n");
		printf("** 1: Alex9932\n");
		for (int i = 0; i < 4; i++) {
			printf("** %d: %s\n", i + 2, CODE_NAMES[i]);
		}
		printf("**\n");
		printf("\033[0m");

	} else {
		printhelp();
	}


	printf("** Done with %d errors\n", errors);

	return 0;
}
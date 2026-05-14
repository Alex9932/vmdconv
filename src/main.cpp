/*
 * VMD animation convertor
 * 
 * -mdl <pmd/pmx file> For animation simulation
 * -vmd <vmd file> Animation file
 * -o <output file> gltf output file
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include <allocator.h>
#include <filesystem.h>
#include <simulation.h>
#include <gltfanim.h>

#include "version.h"

#define TEST_BUILD 1

static uint32_t errors = 0;

static const char* MDL_FILE = NULL;
static const char* VMD_FILE = NULL;
static const char* OUTPUT_FILE = NULL;

// Utility functions for reading/writing files - not used yet, but can be useful for future features
// TODO: Move this to a separate file
#if 0
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
#endif

static void PrintBanner() {
	const char* quotes[] = {
		"Factory operations nominal.",
		"Do not touch the Originium...",
		"Cooking something special...",
		"Endmin is watching you.",
		"Over here Endmin!",
		"Have you seen my wrench?"
	};

	char versionString[128];
	BuildVersionString(versionString, sizeof(versionString));

	srand(time(NULL));

	printf("\033[1;36m");
	printf("**\n");
	printf("** %s\n", quotes[rand() % 6]);
	printf("**\n");
	//printf("** VMDCONV (Version %s \"%s\", build %s) by Alex9932\n", VERSION, CODENAME, BUILD);
	printf("** VMDCONV %s by Alex9932\n", versionString);
	printf("**  - Made with love for Talos - II pioneers.\n");
	printf("**\n");
	printf("\033[0m");
}

static void printhelp() {
	printf("\n** ~ ~ ~ VMD Animation convertor ~ ~ ~\n\n");
	printf("** Arguments:\n");
	printf("**   -mdl <pmd/pmx file>   For animation simulation\n");
	printf("**   -vmd <vmd file>       Animation file\n");
	printf("**   -o <output file>      gltf output file\n");
	printf("**   -h                    Show this help message\n");
	printf("** Usage example:\n");
	printf("**   vmdconv -mdl model.pmd -vmd animation.vmd -o output.glb\n");
	printf("**\n");
}

struct AnimationData {

};

#if TEST_BUILD
int _main(int argc, char** argv);
int main(int argc, char** argv) {

	// Test parameters - you can change these to test with different files

	int test_argc = 7;
	//const char* test_argv[] = { argv[0], "-mdl", "vmd/Model/Rin_Kagamine.pmd", "-vmd", "vmd/wavefile_v2.vmd", "-o", "anim.glb" };
	const char* test_argv[] = { argv[0], "-mdl", "..\\build\\vmd\\Model\\Miku_Hatsune.pmd", "-vmd", "..\\build\\vmd\\wavefile_v2.vmd", "-o", "anim.glb" };

	_main(test_argc, (char**)test_argv);
}
int _main(int argc, char** argv) {
#else
int main(int argc, char** argv) {
#endif
	PrintBanner();

	if (argc < 2) {
		printhelp();
		return 0;
	}

	for (size_t i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-o") == 0 && i + 1 < argc) {
			OUTPUT_FILE = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-mdl") == 0 && i + 1 < argc) {
			MDL_FILE = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-vmd") == 0 && i + 1 < argc) {
			VMD_FILE = argv[i + 1];
			i++;
		}
		else if (strcmp(argv[i], "-squad") == 0) {
			printf("** Executing \"squad\" command\n");
			printf("\033[1;36m");
			printf("**\n** VMDCONV Development Squad:\n");
			printf("** 1: Alex9932\n");
			for (int i = 0; i < 4; i++) {
				printf("** %d: %s\n", i + 2, CODE_NAMES[i]);
			}
			printf("**\n");
			printf("\033[0m");
		}
	}

	printf("** Setup engine env\n");
	Engine::STDAllocator* alloc = new Engine::STDAllocator("Default allocator");
	Engine::SetDefaultAllocator(alloc);
	Engine::Filesystem_Initialize(NULL);


	Simulation sim;
	SimulationSetupInfo setupInfo = {};

	// Invalid parameters
	if (MDL_FILE == NULL || VMD_FILE == NULL || OUTPUT_FILE == NULL) {
		printf("!! Invalid parameters! Use -h for help\n");
		errors++;
		//printhelp();
		goto exit;
	}

	// Setup simulation
	setupInfo.model = MDL_FILE;
	setupInfo.animation = VMD_FILE;
	if (!sim.Setup(&setupInfo)) {
		printf("** Simulation setup failed\n");
		errors++;
		goto exit;
	}

	printf("** Simulating...\n");
	Uint32 frame = 0;
	while (!sim.Step(1.0 / 60.0)) {
		// Simulation step
		printf("\r** Frame: %d", frame);
		fflush(stdout);
		frame++;
	}
	printf("\n");

	GLTFAnimExportInfo gltfinfo = {};
	gltfinfo.file = OUTPUT_FILE;
	gltfinfo.mdl  = sim.model;
	gltfinfo.boneAnimations = (BoneAnimation*)malloc(sizeof(BoneAnimation) * sim.model->GetBoneCount());

	ExportGLTF(&gltfinfo);

	sim.Free();

exit:
	printf("** Done with %d errors\n", errors);

	Engine::Filesystem_Destroy();
	delete alloc;
	return 0;
}
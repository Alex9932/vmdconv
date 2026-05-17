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

#define TEST_BUILD 0

static uint32_t errors = 0;

static const char* MDL_FILE    = NULL;
static const char* VMD_FILE    = NULL;
static const char* OUTPUT_FILE = NULL;
static Bool        VISUALIZE   = false;

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
	//printf("**   -v                    Visualize animation (not supported on headless systems)\n");
	printf("**   -h                    Show this help message\n");
	printf("** Usage example:\n");
	printf("**   vmdconv -mdl model.pmd -vmd animation.vmd -o output.glb\n");
	printf("**\n");
}

static void normalizeQuat(quat* q) {
	Float32 len = sqrtf(q->x * q->x + q->y * q->y + q->z * q->z + q->w * q->w);
	if (len > 0.0f) {
		q->x /= len;
		q->y /= len;
		q->z /= len;
		q->w /= len;
	}
}

#if TEST_BUILD
int _main(int argc, char** argv);
int main(int argc, char** argv) {

	// Test parameters - you can change these to test with different files

	int test_argc = 7;
	//const char* test_argv[] = { argv[0], "-mdl", "vmd/Model/Rin_Kagamine.pmd", "-vmd", "vmd/wavefile_v2.vmd", "-o", "anim.glb" };
	const char* test_argv[] = { argv[0], "-mdl", "..\\build\\vmd\\Model\\Miku_Hatsune.pmd", "-vmd", "..\\build\\vmd\\player\\sneaking.vmd", "-o", "anim.glb" };

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
#if 0
		else if (strcmp(argv[i], "-v") == 0) {
			VISUALIZE = true;
		}
#endif
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

	if (VISUALIZE) {
		printf("@@ Used -v flag\n");
		printf("** Setup rendering context\n");

	}


	Simulation sim;
	SimulationSetupInfo setupInfo = {};

	// Invalid parameters
	if (MDL_FILE == NULL || VMD_FILE == NULL || OUTPUT_FILE == NULL) {
		printf("!! Invalid parameters! Use -h for help\n");
		errors++;
		//printhelp();
		goto exit;
	}

	String* names = NULL;

	// Setup simulation
	setupInfo.model = MDL_FILE;
	setupInfo.animation = VMD_FILE;
	setupInfo.names = &names;
	if (!sim.Setup(&setupInfo)) {
		printf("** Simulation setup failed\n");
		errors++;
		goto exit;
	}

	GLTFAnimExportInfo gltfinfo = {};
	gltfinfo.file = OUTPUT_FILE;
	gltfinfo.mdl = sim.model;
	gltfinfo.boneAnimations = (BoneAnimation*)rg_malloc(sizeof(BoneAnimation) * sim.model->GetBoneCount());
	gltfinfo.boneNames = names;

	// VMD Animation in 30 fps, we sample it in 60 fps
	Uint32 frames = (sim.animation->GetLastFrame() / sim.animation->GetFramerate() * 60.0f);
	for (size_t i = 0; i < sim.model->GetBoneCount(); i++) {
		gltfinfo.boneAnimations[i].name = sim.model->GetBone(i)->name;
		gltfinfo.boneAnimations[i].keyframeCount = frames;
		gltfinfo.boneAnimations[i].timestamps = (Float32*)rg_malloc(sizeof(Float32) * frames);
		gltfinfo.boneAnimations[i].translations = (vec3*)rg_malloc(sizeof(vec3) * frames);
		gltfinfo.boneAnimations[i].rotations = (quat*)rg_malloc(sizeof(quat) * frames);
	}
	gltfinfo.offsets = (mat4*)rg_malloc(sizeof(mat4) * sim.model->GetBoneCount());

	for (size_t i = 0; i < sim.model->GetBoneCount(); i++) {
		gltfinfo.offsets[i] = sim.model->GetBones()[i].offset;
	}

	printf("** Simulating...\n");
	Uint32 frame = 0;
	// Simulation steps
	Bool runSim = true;
	while (runSim) {
		runSim = !sim.Step(1.0 / 60.0);
		// Save bone animation data
		for (size_t i = 0; i < sim.model->GetBoneCount(); i++) {
			gltfinfo.boneAnimations[i].timestamps[frame] = (Float32)frame * (1.0f / 60.0f);
			gltfinfo.boneAnimations[i].translations[frame] = sim.model->GetBone(i)->position;
			gltfinfo.boneAnimations[i].translations[frame].z = -gltfinfo.boneAnimations[i].translations[frame].z;
			gltfinfo.boneAnimations[i].rotations[frame] = sim.model->GetBone(i)->rotation;
			gltfinfo.boneAnimations[i].rotations[frame].z = -gltfinfo.boneAnimations[i].rotations[frame].z;
			gltfinfo.boneAnimations[i].rotations[frame].w = -gltfinfo.boneAnimations[i].rotations[frame].w;
			normalizeQuat(&gltfinfo.boneAnimations[i].rotations[frame]);
		}

		printf("\r** Frame: %d", frame);
		fflush(stdout);
		frame++;
		if (frame > frames) {
			printf("\n@@ Invalid operation at: %d", frame);
			errors++;
			break;
		}
	}
	printf("\n");

	ExportGLTF(&gltfinfo);

	for (size_t i = 0; i < sim.model->GetBoneCount(); i++) {
		rg_free(gltfinfo.boneAnimations[i].timestamps);
		rg_free(gltfinfo.boneAnimations[i].translations);
		rg_free(gltfinfo.boneAnimations[i].rotations);
	}
	rg_free(gltfinfo.offsets);
	rg_free(gltfinfo.boneAnimations);
	rg_free(*setupInfo.names);

	sim.Free();

exit:
	printf("** Done with %d errors\n", errors);

	Engine::Filesystem_Destroy();
	delete alloc;
	return 0;
}
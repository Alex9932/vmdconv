#include "simulation.h"

#include <mmdimporter.h>
#include <rgstring.h>
#include <allocator.h>
#include <filesystem.h>

#include <kinematicsmodel.h>

static PMDImporter pmdimport;
static PMXImporter pmximport;
static VMDImporter vmdimport;

Bool Simulation::Setup(const SimulationSetupInfo* info) {
	printf("** Setting up simulation...\n");

	stringAlloc = RG_NEW(Engine::LinearAllocator)("String allocator", 1024 * 1024); // 1MB for strings

	char _path[1024];
	Engine::FS_ReplaceSeparators(_path, info->model);

	char path[256];
	char file[256];
	Engine::FS_SeparatePathFile(path, 256, file, 256, _path);

	R3DRiggedModelInfo rmdl = {};
	ModelExtraData extra = {};
	ImportModelInfo mdlinfo = {};
	mdlinfo.path = path;
	mdlinfo.file = file;
	mdlinfo.info.as_rigged = &rmdl;
	mdlinfo.extra = &extra;

	Engine::RiggedModelImporter* importer = NULL;
	BoneNameGetter* boneNameGetter = NULL;

	if (Engine::rg_strenw(info->model, ".pmd")) {
		// Set PMD inporter
		printf("** Using PMD importer\n");
		importer = &pmdimport;
		boneNameGetter = &pmdimport;
	} else if (Engine::rg_strenw(info->model, ".pmx")) {
		// Set PMX inporter
		printf("** Using PMX importer\n");
		importer = &pmximport;
		boneNameGetter = &pmximport;
	} else {
		// Unsupported model format
		printf("!! Unknown model format: %s\n", file);
		return false;
	}

	printf("** Loading model data for: %s\n", file);
	importer->ImportRiggedModel(&mdlinfo);
	printf("** Create kinematics model\n");
	this->model = importer->ImportKinematicsModel(&mdlinfo);

	// Copy bone names to string buffer
	String* names = (String*)rg_malloc(sizeof(String) * this->model->GetBoneCount());
	*info->names = names;
	
	for (size_t i = 0; i < this->model->GetBoneCount(); i++) {
		//String boneName = this->model->GetBone(i)->name;
		String boneName = "";
		if (boneNameGetter->GetENBoneName(&mdlinfo, i)) {
			boneName = boneNameGetter->GetENBoneName(&mdlinfo, i);
		} else {
			boneName = boneNameGetter->GetBoneName(&mdlinfo, i);
		}
		size_t nameLength = SDL_strlen(boneName) + 1;
		names[i] = (String)stringAlloc->Allocate(nameLength);
		SDL_snprintf((char*)names[i], nameLength, "%s", boneName);
	}

	// We need KinematicsModel only, so we can free the rest of the data
	printf("** Free resources\n");
	FreeModelInfo freeinfo = {};
	freeinfo.info.as_rigged = &rmdl;
	freeinfo.extra = mdlinfo.extra;
	freeinfo.userdata = mdlinfo.userdata;
	importer->FreeRiggedModelData(&freeinfo);


	Engine::FS_ReplaceSeparators(_path, info->animation);
	printf("** Loading animation: %s\n", _path);
	this->animation = vmdimport.ImportAnimation(_path, this->model, false); // Do not map bone names
	this->animator = new Engine::Animator(this->model);

	this->animator->PlayAnimation(this->animation);

	printf("** Setup complete!\n");
	return true;
}

void Simulation::Free() {
	printf("** Free simulation\n");
	stringAlloc->Deallocate();
	using namespace Engine;
	RG_DELETE(Animation, animation);
	RG_DELETE(LinearAllocator, stringAlloc);
	delete this->animator;
	delete this->model;
}

Bool Simulation::Step(Float64 dt) {
	this->animator->Update(dt);
	this->model->RebuildSkeleton();
	this->model->SolveCCDIK();
	this->model->RecalculateTransform();

	return this->animator->IsAnimationEnded();
}
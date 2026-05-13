#ifndef _IMPORTER_H
#define _IMPORTER_H

#include "rgtypes.h"
#include "rendertypes.h"

typedef struct NameField {
	char name[128];
	// For future use
} NameField;

typedef struct ModelExtraData {
	NameField* mesh_names;
	NameField* mat_names;
	NameField* bone_names;
	NameField* custom0;
	NameField* custom1;
	NameField* custom2;
	NameField* custom3;
	NameField* custom4;
} ModelExtraData;

typedef struct ImportModelInfo {
	String path;
	String file;
	union {
		R3DStaticModelInfo* as_static;
		R3DRiggedModelInfo* as_rigged;
	} info;
	ModelExtraData* extra;
	Bool skipFirstMat; // Need for skipping unused default material in some models
	const void* userdata;
} ImportModelInfo;

typedef struct FreeModelInfo {
	union {
		R3DStaticModelInfo* as_static;
		R3DRiggedModelInfo* as_rigged;
	} info;
	ModelExtraData* extra;
	const void* userdata;
} FreeModelInfo;


namespace Engine {
	class KinematicsModel;

	class ModelImporter {
		public:
			ModelImporter() {}
			~ModelImporter() {}

			virtual void ImportModel(ImportModelInfo* info) {}
			virtual void FreeModelData(FreeModelInfo* data) {}
	};

	class RiggedModelImporter {
		public:
			RiggedModelImporter() {}
			~RiggedModelImporter() {}

			virtual void ImportRiggedModel(ImportModelInfo* info) {}
			virtual void FreeRiggedModelData(FreeModelInfo* data) {}
			virtual KinematicsModel* ImportKinematicsModel(ImportModelInfo* info) { return NULL; }
	};
}

#endif
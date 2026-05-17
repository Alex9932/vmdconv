#ifndef _MMDIMPORTER_H
#define _MMDIMPORTER_H

#include <importer.h>
//#include <animation.h>

namespace Engine {
	class KinematicsModel;
	class Animation;
}

class BoneNameGetter {
	public:
		virtual String GetBoneName(ImportModelInfo* info, Uint32 id) = 0;
		virtual String GetENBoneName(ImportModelInfo* info, Uint32 id) = 0;
};	

class PMDImporter : public virtual BoneNameGetter, public virtual Engine::ModelImporter, public virtual Engine::RiggedModelImporter {
	public:
		RG_INLINE PMDImporter()  {}
		RG_INLINE ~PMDImporter() {}

		RG_DECLSPEC void ImportModel(ImportModelInfo* info) override;
		RG_DECLSPEC void FreeModelData(FreeModelInfo* data) override;

		RG_DECLSPEC void ImportRiggedModel(ImportModelInfo* info) override;
		RG_DECLSPEC void FreeRiggedModelData(FreeModelInfo* data) override;

		RG_DECLSPEC String GetBoneName(ImportModelInfo* info, Uint32 id) override;
		RG_DECLSPEC String GetENBoneName(ImportModelInfo* info, Uint32 id) override;

		RG_DECLSPEC Engine::KinematicsModel* ImportKinematicsModel(ImportModelInfo* path) override;
};

class PMXImporter : public virtual BoneNameGetter, public virtual Engine::ModelImporter, public virtual Engine::RiggedModelImporter {
	public:
		RG_INLINE PMXImporter() {}
		RG_INLINE ~PMXImporter() {}

		RG_DECLSPEC void ImportModel(ImportModelInfo* info) override;
		RG_DECLSPEC void FreeModelData(FreeModelInfo* data) override;

		RG_DECLSPEC void ImportRiggedModel(ImportModelInfo* info) override;
		RG_DECLSPEC void FreeRiggedModelData(FreeModelInfo* data) override;

		RG_DECLSPEC String GetBoneName(ImportModelInfo* info, Uint32 id) override;
		RG_DECLSPEC String GetENBoneName(ImportModelInfo* info, Uint32 id) override;

		RG_DECLSPEC Engine::KinematicsModel* ImportKinematicsModel(ImportModelInfo* path) override;
};

class VMDImporter {
	public:
		RG_INLINE VMDImporter()  {}
		RG_INLINE ~VMDImporter() {}

		// TODO: Import animation from vmd file
		RG_DECLSPEC Engine::Animation* ImportAnimation(String path, Engine::KinematicsModel* model, Bool mapBoneNames = false);
};

#endif
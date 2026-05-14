#ifndef _GLTFANIM_H
#define _GLTFANIM_H

#include <rgtypes.h>
#include <kinematicsmodel.h>

typedef struct BoneAnimation {
	String name;
	Uint32 keyframeCount;
	Uint32 _offset;
	Float32* timestamps;
	vec3* translations;
	vec4* rotations;
} BoneAnimation;

typedef struct GLTFAnimExportInfo {
	String file;
	Engine::KinematicsModel* mdl;
	BoneAnimation* boneAnimations;
} GLTFAnimExportInfo;

void ExportGLTF(GLTFAnimExportInfo* info);

#endif
#include "gltfanim.h"
#include "version.h"

#include <filesystem.h>

#include <cJSON.h>

#define GLTF_TYPE_SINT8  5120
#define GLTF_TYPE_UINT8  5121
#define GLTF_TYPE_SINT16 5122
#define GLTF_TYPE_UINT16 5123
#define GLTF_TYPE_UINT32 5125
#define GLTF_TYPE_FLOAT  5126

struct GLTFHeader {
	Uint32 magic;
	Uint32 version;
	Uint32 length;
};

struct GLTFChunk {
	Uint32 chunkLength;
	Uint32 chunkType;
};

static void WriteString(const char* path, const char* str) {
	FILE* f = fopen(path, "w");
	if (f) {
		fprintf(f, "%s", str);
		fclose(f);
	}
}

static void MakeNodeHierarchy(cJSON* nodes, Engine::KinematicsModel* mdl, String* names) {
	// Create nodes for each bone
	for (Sint32 i = 0; i < mdl->GetBoneCount(); i++) {
		Bone* bone = mdl->GetBone(i);
		cJSON* node = cJSON_CreateObject();

		Float32 scale[3] = { 1.0f, 1.0f, 1.0f };

		cJSON* p = cJSON_CreateFloatArray(bone->offset_pos.array, 3); // Position
		cJSON* r = cJSON_CreateFloatArray(bone->offset_rot.v4.array, 4); // Rotation
		cJSON* s = cJSON_CreateFloatArray(scale, 3); // Scale

		cJSON_AddStringToObject(node, "name", names[i]);// bone->name);
		cJSON_AddItemToObject(node, "translation", p);
		cJSON_AddItemToObject(node, "rotation", r);
		cJSON_AddItemToObject(node, "scale", s);
		//if (bone->parent != -1) {
		//	cJSON_AddNumberToObject(node, "parent", bone->parent);
		//}
		cJSON_AddItemToArray(nodes, node);
	}

	// Add children information
	for (Sint32 i = 0; i < mdl->GetBoneCount(); i++) {
		Bone* bone = mdl->GetBone(i);
		if (bone->parent != -1) {
			cJSON* parentNode = cJSON_GetArrayItem(nodes, bone->parent);
			cJSON* children = cJSON_GetObjectItem(parentNode, "children");
			if (!children) {
				children = cJSON_CreateArray();
				cJSON_AddItemToObject(parentNode, "children", children);
			}
			cJSON_AddItemToArray(children, cJSON_CreateNumber(i));
		}
	}
}

static void MakeAnimation(cJSON* anims, GLTFAnimExportInfo* info) {
	cJSON* animation = cJSON_CreateObject();

	cJSON* samplers = cJSON_CreateArray();
	for (size_t i = 0; i < info->mdl->GetBoneCount(); i++) {
		cJSON* p_sampler = cJSON_CreateObject();
		cJSON_AddNumberToObject(p_sampler, "input", i * 3);
		cJSON_AddNumberToObject(p_sampler, "output", i * 3 + 1);
		cJSON_AddStringToObject(p_sampler, "interpolation", "LINEAR");
		cJSON_AddItemToArray(samplers, p_sampler);

		cJSON* r_sampler = cJSON_CreateObject();
		cJSON_AddNumberToObject(r_sampler, "input", i * 3);
		cJSON_AddNumberToObject(r_sampler, "output", i * 3 + 2);
		cJSON_AddStringToObject(r_sampler, "interpolation", "LINEAR");
		cJSON_AddItemToArray(samplers, r_sampler);
	}
	cJSON_AddItemToObject(animation, "samplers", samplers);

	cJSON* channels = cJSON_CreateArray();
	for (size_t i = 0; i < info->mdl->GetBoneCount(); i++) {
		cJSON* p_channel = cJSON_CreateObject();
		cJSON* p_target = cJSON_CreateObject();
		cJSON_AddNumberToObject(p_target, "node", i);
		cJSON_AddStringToObject(p_target, "path", "translation");
		cJSON_AddNumberToObject(p_channel, "sampler", i * 2);
		cJSON_AddItemToObject(p_channel, "target", p_target);
		cJSON_AddItemToArray(channels, p_channel);

		cJSON* r_channel = cJSON_CreateObject();
		cJSON* r_target = cJSON_CreateObject();
		cJSON_AddNumberToObject(r_target, "node", i);
		cJSON_AddStringToObject(r_target, "path", "rotation");
		cJSON_AddNumberToObject(r_channel, "sampler", i * 2 + 1);
		cJSON_AddItemToObject(r_channel, "target", r_target);
		cJSON_AddItemToArray(channels, r_channel);
	}
	cJSON_AddItemToObject(animation, "channels", channels);

	cJSON_AddItemToArray(anims, animation);
}

static void MakeAccessors(cJSON* accessors, GLTFAnimExportInfo* info) {
	size_t offset = 0;

	for (size_t i = 0; i < info->mdl->GetBoneCount(); i++) {

		size_t keyframeCount = info->boneAnimations[i].keyframeCount;
		size_t tsSize = sizeof(Float32) * keyframeCount;
		size_t pSize = sizeof(vec3) * keyframeCount;
		size_t rSize = sizeof(vec4) * keyframeCount;

		cJSON* accessor_ts = cJSON_CreateObject();
		cJSON_AddNumberToObject(accessor_ts, "bufferView", 0);
		cJSON_AddNumberToObject(accessor_ts, "componentType", GLTF_TYPE_FLOAT);
		cJSON_AddNumberToObject(accessor_ts, "byteOffset", offset);
		cJSON_AddStringToObject(accessor_ts, "type", "SCALAR");
		cJSON_AddNumberToObject(accessor_ts, "count", keyframeCount);
		Float32 min_val = 0.0f;
		Float32 max_val = info->boneAnimations[i].timestamps[keyframeCount - 1];
		cJSON_AddItemToObject(accessor_ts, "min", cJSON_CreateFloatArray(&min_val, 1));
		cJSON_AddItemToObject(accessor_ts, "max", cJSON_CreateFloatArray(&max_val, 1));
		cJSON_AddItemToArray(accessors, accessor_ts);
		offset += tsSize;

		cJSON* accessor_p = cJSON_CreateObject();
		cJSON_AddNumberToObject(accessor_p, "bufferView", 0);
		cJSON_AddNumberToObject(accessor_p, "componentType", GLTF_TYPE_FLOAT);
		cJSON_AddNumberToObject(accessor_p, "byteOffset", offset);
		cJSON_AddStringToObject(accessor_p, "type", "VEC3");
		cJSON_AddNumberToObject(accessor_p, "count", keyframeCount);
		cJSON_AddItemToArray(accessors, accessor_p);
		offset += pSize;

		cJSON* accessor_r = cJSON_CreateObject();
		cJSON_AddNumberToObject(accessor_r, "bufferView", 0);
		cJSON_AddNumberToObject(accessor_r, "componentType", GLTF_TYPE_FLOAT);
		cJSON_AddNumberToObject(accessor_r, "byteOffset", offset);
		cJSON_AddStringToObject(accessor_r, "type", "VEC4");
		cJSON_AddNumberToObject(accessor_r, "count", keyframeCount);
		cJSON_AddItemToArray(accessors, accessor_r);
		offset += rSize;
	}

	cJSON* accessor_matrices = cJSON_CreateObject();
	cJSON_AddNumberToObject(accessor_matrices, "bufferView", 0);
	cJSON_AddNumberToObject(accessor_matrices, "componentType", GLTF_TYPE_FLOAT);
	cJSON_AddNumberToObject(accessor_matrices, "byteOffset", offset);
	cJSON_AddStringToObject(accessor_matrices, "type", "MAT4");
	cJSON_AddNumberToObject(accessor_matrices, "count", info->mdl->GetBoneCount());
	cJSON_AddItemToArray(accessors, accessor_matrices);
	offset += info->mdl->GetBoneCount() * sizeof(mat4);
}

static void MakeSkins(cJSON* skins, GLTFAnimExportInfo* info) {
	Uint32 accessorID = info->mdl->GetBoneCount() * 3; // Each bone has 3 accessors (timestamps, translations, rotations)
	cJSON* skin = cJSON_CreateObject();
	cJSON_AddNumberToObject(skin, "inverseBindMatrices", accessorID);
	cJSON* joints = cJSON_CreateArray();
	for (size_t i = 0; i < info->mdl->GetBoneCount(); i++) {
		cJSON_AddItemToArray(joints, cJSON_CreateNumber(i));
	}
	cJSON_AddItemToObject(skin, "joints", joints);
	cJSON_AddItemToArray(skins, skin);
}

static cJSON* BuildSchema(GLTFAnimExportInfo* info, size_t binlen) {
	char versionString[128];
	char generator[128];
	BuildVersionString(versionString, sizeof(versionString));
	SDL_snprintf(generator, 128, "VMDCONV %s", versionString);

	cJSON* root = cJSON_CreateObject();

	cJSON* asset = cJSON_CreateObject();
	cJSON_AddStringToObject(asset, "version", "2.0");
	cJSON_AddStringToObject(asset, "generator", generator);
	cJSON_AddItemToObject(root, "asset", asset);

	cJSON* nodes = cJSON_CreateArray();
	MakeNodeHierarchy(nodes, info->mdl, info->boneNames);
	cJSON_AddItemToObject(root, "nodes", nodes);

	cJSON* skins = cJSON_CreateArray();
	MakeSkins(skins, info);
	cJSON_AddItemToObject(root, "skins", skins);

	cJSON* animations = cJSON_CreateArray();
	MakeAnimation(animations, info);
	cJSON_AddItemToObject(root, "animations", animations);

	cJSON* accessors = cJSON_CreateArray();
	MakeAccessors(accessors, info);
	cJSON_AddItemToObject(root, "accessors", accessors);

	cJSON* bufferViews = cJSON_CreateArray();
	cJSON* bufferView = cJSON_CreateObject();
	cJSON_AddNumberToObject(bufferView, "buffer", 0);
	cJSON_AddNumberToObject(bufferView, "byteOffset", 0);
	cJSON_AddNumberToObject(bufferView, "byteLength", binlen);
	cJSON_AddItemToArray(bufferViews, bufferView);
	cJSON_AddItemToObject(root, "bufferViews", bufferViews);

	cJSON* buffers = cJSON_CreateArray();
	cJSON* buffer = cJSON_CreateObject();
	cJSON_AddNumberToObject(buffer, "byteLength", binlen);
	cJSON_AddItemToArray(buffers, buffer);
	cJSON_AddItemToObject(root, "buffers", buffers);

	//WriteString("output.json", cJSON_Print(root));

	return root;

}

void ExportGLTF(GLTFAnimExportInfo* info) {
	GLTFHeader header;
	header.magic   = 0x46546C67; // "glTF"
	header.version = 2;
	header.length  = 0; // Will be updated later after we calculate the actual length of the file

	// Calculate binary chunk length
	//size_t frames = (animation->GetLastFrame() / sim.animation->GetFramerate() * 60.0f);
	size_t len = 0;
	for (Uint32 i = 0; i < info->mdl->GetBoneCount(); i++) {
		BoneAnimation* anim = &info->boneAnimations[i];
		len += sizeof(Float32) * anim->keyframeCount; // timestamps
		len += sizeof(vec3) * anim->keyframeCount;    // translations
		len += sizeof(vec4) * anim->keyframeCount;    // rotations
	}
	len += sizeof(mat4) * info->mdl->GetBoneCount();  // bone offset matrices

	Engine::FSWriter writer(info->file);

	// Write header first, we will update the length later
	writer.Write(&header, sizeof(GLTFHeader));

	// Build and write the JSON schema
	cJSON* json = BuildSchema(info, len);
	String jsonStr = cJSON_Print(json);
	size_t jsonlen = SDL_strlen(jsonStr);
	size_t padding = (4 - (jsonlen % 4)) % 4;

	GLTFChunk jsonChunk = {};
	jsonChunk.chunkLength = jsonlen + padding;
	jsonChunk.chunkType   = 0x4E4F534A;
	writer.Write(&jsonChunk, sizeof(GLTFChunk));

	writer.Write(jsonStr, jsonlen);

	// Append ' ' padding to align to 4 bytes
	for (size_t i = 0; i < padding; i++) {
		writer.Write(" ", 1);
	}

	// Write binary animation data
	GLTFChunk binChunk = {};
	binChunk.chunkLength = len;
	binChunk.chunkType   = 0x004E4942;
	writer.Write(&binChunk, sizeof(GLTFChunk));

	// Write binary data for each bone animation
	for (Uint32 i = 0; i < info->mdl->GetBoneCount(); i++) {
		Bone* bone = info->mdl->GetBone(i);
		BoneAnimation* anim = &info->boneAnimations[i];

		// Write keyframe data
		writer.Write(anim->timestamps, sizeof(Float32) * anim->keyframeCount);
		writer.Write(anim->translations, sizeof(vec3) * anim->keyframeCount);
		writer.Write(anim->rotations, sizeof(vec4) * anim->keyframeCount);
	}

	// Write bone offset matrices
	writer.Write(info->offsets, sizeof(mat4) * info->mdl->GetBoneCount());

	// Update header with actual length
	//header.length = sizeof(GLTFHeader) + sizeof(GLTFChunk)*2 + jsonlen + padding + len;
	header.length = writer.GetOffset();
	writer.Seek(RG_FS_SEEK_SET, 0);
	writer.Write(&header, sizeof(GLTFHeader));

	// Clean up
	cJSON_Delete(json);
}
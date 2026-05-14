#include "gltfanim.h"
#include "version.h"

#include <filesystem.h>

#include <cJSON.h>

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

static void MakeNodeHierarchy(cJSON* nodes, Engine::KinematicsModel* mdl) {
	for (Uint32 i = 0; i < mdl->GetBoneCount(); i++) {
		Bone* bone = mdl->GetBone(i);
		cJSON* node = cJSON_CreateObject();

		Float32 scale[3] = { 1.0f, 1.0f, 1.0f };

		cJSON* p = cJSON_CreateFloatArray(bone->offset_pos.array, 3); // Position
		cJSON* r = cJSON_CreateFloatArray(bone->offset_rot.v4.array, 4); // Rotation
		cJSON* s = cJSON_CreateFloatArray(scale, 3); // Scale

		cJSON_AddStringToObject(node, "name", bone->name);
		cJSON_AddNumberToObject(node, "index", i); // Custom property TODO: Remove this later
		cJSON_AddItemToObject(node, "translation", p);
		cJSON_AddItemToObject(node, "rotation", r);
		cJSON_AddItemToObject(node, "scale", s);
		if (bone->parent != -1) {
			cJSON_AddNumberToObject(node, "parent", bone->parent);
		}
		cJSON_AddItemToArray(nodes, node);
	}
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
	MakeNodeHierarchy(nodes, info->mdl);
	cJSON_AddItemToObject(root, "nodes", nodes);

	cJSON* animations = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "animations", animations);

	cJSON* accessors = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "accessors", accessors);

	cJSON* bufferViews = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "bufferViews", bufferViews);

	cJSON* buffers = cJSON_CreateArray();
	cJSON* buffer = cJSON_CreateObject();
	cJSON_AddNumberToObject(buffer, "byteLength", binlen);
	cJSON_AddItemToArray(buffers, buffer);
	cJSON_AddItemToObject(root, "buffers", buffers);

	WriteString("output.json", cJSON_Print(root));

	return root;

}

void ExportGLTF(GLTFAnimExportInfo* info) {
	GLTFHeader header;
	header.magic   = 0x46546C67; // "glTF"
	header.version = 2;
	header.length  = 0; // Will be updated later after we calculate the actual length of the file

	// Calculate binary chunk length
	size_t len = 0;
	for (Uint32 i = 0; i < info->mdl->GetBoneCount(); i++) {
		BoneAnimation* anim = &info->boneAnimations[i];
		len += sizeof(Float32) * anim->keyframeCount; // timestamps
		len += sizeof(vec3) * anim->keyframeCount;    // translations
		len += sizeof(vec4) * anim->keyframeCount;    // rotations
	}

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

	// Update header with actual length
	header.length = writer.GetOffset();
	writer.Seek(RG_FS_SEEK_SET, 0);
	writer.Write(&header, sizeof(GLTFHeader));

	// Clean up
	cJSON_Delete(json);
}
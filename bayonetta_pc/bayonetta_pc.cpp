//this is kind of a poor example for plugins, since the format's not totally known and the code is WIP.
//but it does showcase some interesting usages.

#include "stdafx.h"
#undef max
#undef min
#include "half.hpp"
using half_float::half;
#include <map>
#include <stack>

const char *g_pPluginName = "bayonetta_pc";
const char *g_pPluginDesc = "Bayonetta PC model handler, by Dick, Kerilk.";

FILE *bayo_log;
#ifdef _DEBUG
#define DBGLOG(fmt, ...) fprintf(bayo_log, fmt, __VA_ARGS__)
#define OPENLOG() (bayo_log = fopen("bayo.log", "w"))
#define CLOSELOG() fclose(bayo_log)
#define DBGFLUSH() fflush(bayo_log)
#else
#define DBGLOG(fmt, ...) do { if (0) fprintf(bayo_log, fmt, __VA_ARGS__); } while (0)
#define OPENLOG() do { if (0) bayo_log = fopen("bayo.log", "w+"); } while (0)
#define CLOSELOG() do { if (0) fclose(bayo_log); } while (0)
#define DBGFLUSH() do { if (0) fflush(bayo_log); } while (0)
#endif

#define DBG_ANIM_DATA 0
#if DBG_ANIM_DATA
#define DBGALOG(fmt, ...) DBGLOG(fmt, __VA_ARGS__)
#else
#define DBGALOG(fmt, ...) do { if (0) DBGLOG(fmt, __VA_ARGS__); } while (0)
#endif

typedef enum game_e {
	BAYONETTA,
	BAYONETTA2,
	VANQUISH,
	NIER_AUTOMATA
} game_t;
typedef struct bayoV4F_s
{
	float x;
	float y;
	float z;
	float w;
} bayoV4F_t;
template <bool big>
struct bayoV4F : public bayoV4F_s {
	bayoV4F(bayoV4F_t * ptr) : bayoV4F_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(x);
			LITTLE_BIG_SWAP(y);
			LITTLE_BIG_SWAP(z);
			LITTLE_BIG_SWAP(w);
		}
	}
};
typedef struct bayoDat_s
{
	BYTE			id[4];
	int				numRes;
	int				ofsRes;
	int				ofsType;
	int				ofsNames;
	int				ofsSizes;
} bayoDat_t;

template <bool big>
struct bayoDat : public bayoDat_s {
	bayoDat( bayoDat_t * ptr ): bayoDat_s(*ptr){
		if (big) {
			LITTLE_BIG_SWAP(numRes);
			LITTLE_BIG_SWAP(ofsRes);
			LITTLE_BIG_SWAP(ofsType);
			LITTLE_BIG_SWAP(ofsNames);
			LITTLE_BIG_SWAP(ofsSizes);
		}
	}
};
typedef struct bayoDatFile_s
{
	char				*name;
	BYTE				*data;
	int					dataSize;
} bayoDatFile_t;
typedef struct bayoWTBHdr_s
{
	BYTE				id[4];
	int					unknown;
	int					numTex;
	int					ofsTexOfs;
	int					ofsTexSizes;
	int					ofsTexFlags;
	int					texIdxOffset;
	int					texInfoOffset;
} bayoWTBHdr_t;
template <bool big>
struct bayoWTBHdr : public bayoWTBHdr_s {
	bayoWTBHdr( bayoWTBHdr_t *ptr ): bayoWTBHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(*((int *)id));
			LITTLE_BIG_SWAP(unknown);
			LITTLE_BIG_SWAP(numTex);
			LITTLE_BIG_SWAP(ofsTexOfs);
			LITTLE_BIG_SWAP(ofsTexSizes);
			LITTLE_BIG_SWAP(ofsTexFlags);
			LITTLE_BIG_SWAP(texIdxOffset);
			LITTLE_BIG_SWAP(texInfoOffset);
		}
	}
};
typedef struct bayoWTAHdr_s
{
	BYTE				id[4];
	int					unknown;
	int					numTex;
	int					ofsTexOfs;
	int					ofsTexSizes;
	int					ofsTexFlags;
	int					texIdxOffset;
	int					texInfoOffset;
	int					ofsMipmapOfs;
} bayoWTAHdr_t;
template <bool big>
struct bayoWTAHdr : public bayoWTAHdr_s {
	bayoWTAHdr( bayoWTAHdr_t *ptr ): bayoWTAHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(*((int *)id));
			LITTLE_BIG_SWAP(unknown);
			LITTLE_BIG_SWAP(numTex);
			LITTLE_BIG_SWAP(ofsTexOfs);
			LITTLE_BIG_SWAP(ofsTexSizes);
			LITTLE_BIG_SWAP(ofsTexFlags);
			LITTLE_BIG_SWAP(texIdxOffset);
			LITTLE_BIG_SWAP(texInfoOffset);
			LITTLE_BIG_SWAP(ofsMipmapOfs);
		}
	}
};
// Texture is now DDS
typedef struct ddsPixelFormat_s
{
	int					size;
	int					flags;
	int					id;
	int					rgbBitCount;
	int					rBitMask;
	int					gBitMask;
	int					bBitMask;
	int					aBitMask;
} ddsPixelFormat_t;
typedef struct ddsTexHdr_s
{
	BYTE				id[4];
	int					hSize;
	int					flags;
	int					height;
	int					width;
	int					pitchOrLinearSize;
	int					depth;
	int					mipMapCount;
	int					reserved1[11];
	ddsPixelFormat_t	pixelFormat;
	int					caps;
	int					caps2;
	int					caps3;
	int					caps4;
	int					reserved2;
} ddsTexHdr_t;
typedef struct wtbTexHdr_s
{
	int					unknownA;
	int					unknownB;
	int					unknownC;
	int					unknownD;
	int					unknownE;
	short				unknownF;
	short				unknownG;
	short				unknownH;
	short				unknownI;
	short				unknownJ;
	short				unknownK;
	int					texFmt;
	WORD				heightBits;
	WORD				widthBits;
	int					unknownO;
	int					unknownP;
	int					unknownQ;
} wtbTexHdr_t;
typedef struct GX2Hdr_s {
	DWORD dimension;
	DWORD width;
	DWORD height;
	DWORD depth;
	DWORD numMipmaps;
	DWORD format;
	DWORD AAMode;
	DWORD usage;
	DWORD dataLength;
	DWORD dataPointer;
	DWORD mipmapsDataLength;
	DWORD mimapsPointer;
	DWORD tileMode;
	DWORD swizzleValue;
	DWORD alignment;
	DWORD pitch;
	DWORD mimapOffsets[13];
	DWORD firstMipmap;
	DWORD numMipmaps2;
	DWORD firstSlice;
	DWORD numSlices;
	BYTE component[4];
	DWORD textureRegisters[5];
} GX2Hdr_t;
template <bool big>
struct GX2Hdr : public GX2Hdr_s {
	GX2Hdr( GX2Hdr_t *ptr ): GX2Hdr_s(*ptr) {
		LITTLE_BIG_SWAP(dimension);
		LITTLE_BIG_SWAP(width);
		LITTLE_BIG_SWAP(height);
		LITTLE_BIG_SWAP(depth);
		LITTLE_BIG_SWAP(numMipmaps);
		LITTLE_BIG_SWAP(format);
		LITTLE_BIG_SWAP(AAMode);
		LITTLE_BIG_SWAP(usage);
		LITTLE_BIG_SWAP(dataLength);
		LITTLE_BIG_SWAP(dataPointer);
		LITTLE_BIG_SWAP(mipmapsDataLength);
		LITTLE_BIG_SWAP(mimapsPointer);
		LITTLE_BIG_SWAP(tileMode);
		LITTLE_BIG_SWAP(swizzleValue);
		LITTLE_BIG_SWAP(alignment);
		LITTLE_BIG_SWAP(pitch);
		for (int i = 0; i < 13; i++) LITTLE_BIG_SWAP(mimapOffsets);
		LITTLE_BIG_SWAP(firstMipmap);
		LITTLE_BIG_SWAP(numMipmaps2);
		LITTLE_BIG_SWAP(firstSlice);
		LITTLE_BIG_SWAP(numSlices);
		for (int i = 0; i < 5; i++) LITTLE_BIG_SWAP(textureRegisters[i]);
	}
};
typedef struct bayoSCRHdr_s
{
	BYTE				id[4];
	int					numModels;
	unsigned int		ofsTextures;
	BYTE				unknown[4];
} bayoSCRHdr_t;
template <bool big>
struct bayoSCRHdr : public bayoSCRHdr_s {
	bayoSCRHdr(bayoSCRHdr_t * ptr) : bayoSCRHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(numModels);
			LITTLE_BIG_SWAP(ofsTextures);
		}
	}
};
typedef struct bayo2SCRHdr_s
{
	BYTE				id[4];
	unsigned short		unknownA;
	unsigned short		numModels;
	unsigned int		ofsOffsetsModels;
} bayo2SCRHdr_t;
template <bool big>
struct bayo2SCRHdr : public bayo2SCRHdr_s {
	bayo2SCRHdr(bayo2SCRHdr_t * ptr) : bayo2SCRHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(numModels);
			LITTLE_BIG_SWAP(ofsOffsetsModels);
		}
	}
};
typedef struct bayoSCRModelDscr_s
{
	BYTE				name[16];
	unsigned int		offset;
	float				transform[9];
	short				unknownA[42];
}bayoSCRModelDscr_t;
template <bool big>
struct bayoSCRModelDscr : public bayoSCRModelDscr_s {
	bayoSCRModelDscr(bayoSCRModelDscr_t * ptr) : bayoSCRModelDscr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(offset);
			for (int i = 0; i < 9; i++) {
				LITTLE_BIG_SWAP(transform[i]);
			}
			for (int i = 0; i < 42; i++) {
				LITTLE_BIG_SWAP(unknownA[i]);
			}
		}
	}
};
typedef struct bayo2SCRModelDscr_s
{
	unsigned int		offset;
	BYTE				name[64];
	float				transform[9];
	short				unknownA[18]; //Speculations
}bayo2SCRModelDscr_t;
template <bool big>
struct bayo2SCRModelDscr : public bayo2SCRModelDscr_s {
	bayo2SCRModelDscr(bayo2SCRModelDscr_t * ptr) : bayo2SCRModelDscr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(offset);
			for (int i = 0; i < 9; i++) {
				LITTLE_BIG_SWAP(transform[i]);
			}
			for (int i = 0; i < 18; i++) {
				LITTLE_BIG_SWAP(unknownA[i]);
			}
		}
	}
};
typedef struct bayoVector_s {
	float x;
	float y;
	float z;
	void swap(void) {
		LITTLE_BIG_SWAP(x);
		LITTLE_BIG_SWAP(y);
		LITTLE_BIG_SWAP(z);
	}
} bayoVector_t;
typedef struct nierWMBHdr_s
{
	BYTE				id[4];				// 0
	unsigned int		version;			// 4
	int					unknownA;			// 8
	short				unknownB;			// C Seems related to vertex index size
	short				unknownC;			// E
	float				boundingBox[6];		//10
	unsigned int		ofsBones;			//28
	int					numBones;			//2C
	unsigned int		ofsBoneIndexTT;		//30
	int					sizeBoneIndexTT;	//34
	unsigned int		ofsVertexGroups;	//38
	int					numVertexGroups;	//3C
	unsigned int		ofsBatches;			//40
	int					numBatches;			//44
	unsigned int		ofsLods;			//48
	int					numLods;			//4C
	unsigned int		ofsUnknownD;		//50
	int					numUnknownE;		//54
	unsigned int		ofsBoneMap;			//58
	int					sizeBoneMap;		//5C
	unsigned int		ofsBoneSets;		//60
	int					numBoneSets;		//64
	unsigned int		ofsMaterials;		//68
	int					numMaterials;		//6C
	unsigned int		ofsMeshes;			//70
	int					numMeshes;			//74
	unsigned int		ofsMeshMaterial;	//78
	int					numMeshMaterial;	//7C
	unsigned int		ofsUnknownF;		//80
	int					numUnknownG;		//84
} nierWMBHdr_t;
template <bool big>
struct nierWMBHdr : public nierWMBHdr_s {
	nierWMBHdr(nierWMBHdr_t * ptr) : nierWMBHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(*((int *)id));
			LITTLE_BIG_SWAP(version);
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(unknownB);
			LITTLE_BIG_SWAP(unknownC);
			for (int i = 0; i < 6; i++) {
				LITTLE_BIG_SWAP(boundingBox[i]);
			}
			LITTLE_BIG_SWAP(ofsBones);
			LITTLE_BIG_SWAP(numBones);
			LITTLE_BIG_SWAP(ofsBoneIndexTT);
			LITTLE_BIG_SWAP(sizeBoneIndexTT);
			LITTLE_BIG_SWAP(ofsVertexGroups);
			LITTLE_BIG_SWAP(numVertexGroups);
			LITTLE_BIG_SWAP(ofsBatches);
			LITTLE_BIG_SWAP(numBatches);
			LITTLE_BIG_SWAP(ofsLods);
			LITTLE_BIG_SWAP(numLods);
			LITTLE_BIG_SWAP(ofsUnknownD);
			LITTLE_BIG_SWAP(numUnknownE);
			LITTLE_BIG_SWAP(ofsBoneMap);
			LITTLE_BIG_SWAP(sizeBoneMap);
			LITTLE_BIG_SWAP(ofsBoneSets);
			LITTLE_BIG_SWAP(numBoneSets);
			LITTLE_BIG_SWAP(ofsMaterials);
			LITTLE_BIG_SWAP(numMaterials);
			LITTLE_BIG_SWAP(ofsMeshes);
			LITTLE_BIG_SWAP(numMeshes);
			LITTLE_BIG_SWAP(ofsMeshMaterial);
			LITTLE_BIG_SWAP(numMeshMaterial);
			LITTLE_BIG_SWAP(ofsUnknownF);
			LITTLE_BIG_SWAP(numUnknownG);
		}
	}
};
typedef struct nierMaterial_s
{

	short			version[4];
	unsigned int	ofsName;
	unsigned int	ofsShaderName;
	unsigned int	ofsTechniqueName;
	unsigned int	unknownA;
	unsigned int	ofsTextures;
	unsigned int	numTextures;
	unsigned int	ofsParameterGroups;
	unsigned int	numParametersGroup;
	unsigned int	ofsVariables;
	unsigned int	numVariables;
} nierMaterial_t;
template <bool big>
struct nierMaterial : public nierMaterial_s {
	nierMaterial(nierMaterial_t * ptr) : nierMaterial_s(*ptr) {
		if (big) {
			for (int i = 0; i < 4; i++) {
				LITTLE_BIG_SWAP(version[i]);
			}
			LITTLE_BIG_SWAP(ofsName);
			LITTLE_BIG_SWAP(ofsShaderName);
			LITTLE_BIG_SWAP(ofsTechniqueName);
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(ofsTextures);
			LITTLE_BIG_SWAP(numTextures);
			LITTLE_BIG_SWAP(ofsParameterGroups);
			LITTLE_BIG_SWAP(numParametersGroup);
			LITTLE_BIG_SWAP(ofsVariables);
			LITTLE_BIG_SWAP(numVariables);
		}
	}
};
typedef struct nierTexture_s
{
	unsigned int	ofsName;
	int				id;
} nierTexture_t;
template <bool big>
struct nierTexture : public nierTexture_s {
	nierTexture(nierTexture_t * ptr) : nierTexture_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(ofsName);
			LITTLE_BIG_SWAP(id);
		}
	}
};
typedef struct nierBone_s
{
	short			id;
	short			parentIndex;
	bayoVector_t	localPosition;
	bayoVector_t	localRotation;
	bayoVector_t	localScale;
	bayoVector_t	position;
	bayoVector_t	rotation;
	bayoVector_t	scale;
	bayoVector_t	tPosition;
} nierBone_t;
template <bool big>
struct nierBone : public nierBone_s {
	nierBone(nierBone_t * ptr) : nierBone_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(id);
			LITTLE_BIG_SWAP(parentIndex);
			localPosition.swap();
			localRotation.swap();
			localScale.swap();
			position.swap();
			rotation.swap();
			scale.swap();
			tPosition.swap();
		}
	}
};
typedef struct nierVertexGroup_s {
	unsigned int	ofsVerts;
	unsigned int	ofsVertsExData;
	unsigned int	unknownA;
	unsigned int	unknownB;
	unsigned int	sizeVert;
	unsigned int	sizeVertExData;
	unsigned int	unknownD;
	unsigned int	unknownE;
	unsigned int	numVerts;
	unsigned int	vertExDataFlag;
	unsigned int	ofsIndexBuffer;
	unsigned int	numIndexes;
} nierVertexGroup_t;
template <bool big>
struct nierVertexGroup : public nierVertexGroup_s {
	nierVertexGroup(nierVertexGroup_t * ptr) : nierVertexGroup_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(ofsVerts);
			LITTLE_BIG_SWAP(ofsVertsExData);
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(unknownB);
			LITTLE_BIG_SWAP(sizeVert);
			LITTLE_BIG_SWAP(sizeVertExData);
			LITTLE_BIG_SWAP(unknownD);
			LITTLE_BIG_SWAP(unknownE);
			LITTLE_BIG_SWAP(numVerts);
			LITTLE_BIG_SWAP(vertExDataFlag);
			LITTLE_BIG_SWAP(ofsIndexBuffer);
			LITTLE_BIG_SWAP(numIndexes);
		}
	}
};
typedef struct nierMesh_s {
	unsigned int	ofsName;
	float			boundingBox[6];
	unsigned int	ofsMaterials;
	unsigned int	numMaterials;
	unsigned int	ofsBones;
	unsigned int	numBones;
} nierMesh_t;
template <bool big>
struct nierMesh : public nierMesh_s {
	nierMesh(nierMesh_t * ptr) : nierMesh_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(ofsName);
			for (int i = 0; i < 6; i++) {
				LITTLE_BIG_SWAP(boundingBox[i]);
			}
			LITTLE_BIG_SWAP(ofsMaterials);
			LITTLE_BIG_SWAP(numMaterials);
			LITTLE_BIG_SWAP(ofsBones);
			LITTLE_BIG_SWAP(numBones);
		}
	}
};
typedef struct nierLod_s
{
	unsigned int	ofsName;
	int				lodLevel;
	unsigned int	batchStart;
	unsigned int	ofsBatchInfos;
	unsigned int	numBatchInfos;
} nierLod_t;
template <bool big>
struct nierLod : public nierLod_s {
	nierLod(nierLod_t * ptr) : nierLod_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(ofsName);
			LITTLE_BIG_SWAP(lodLevel);
			LITTLE_BIG_SWAP(batchStart);
			LITTLE_BIG_SWAP(ofsBatchInfos);
			LITTLE_BIG_SWAP(numBatchInfos);
		}
	}
};
typedef struct nierBatchInfo_s
{
	unsigned int	vertexGroupIndex;
	unsigned int	meshIndex;
	unsigned int	materialIndex;
	int				unknownA;
	unsigned int	meshMaterialPairIndex;
	int				unknownB;
} nierBatchInfo_t;
template <bool big>
struct nierBatchInfo : nierBatchInfo_s {
	nierBatchInfo(nierBatchInfo_t * ptr) : nierBatchInfo_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(vertexGroupIndex);
			LITTLE_BIG_SWAP(meshIndex);
			LITTLE_BIG_SWAP(materialIndex);
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(meshMaterialPairIndex);
			LITTLE_BIG_SWAP(unknownB);
		}
	}
};
typedef struct nierBatch_s {
	unsigned int	vertexGroupIndex;
	int				boneSetIndex;
	unsigned int	vertexStart;
	unsigned int	indexStart;
	unsigned int	numVerts;
	unsigned int	numIndices;
	unsigned int	numPrimitives;
} nierBatch_t;
template <bool big>
struct nierBatch : public nierBatch_s {
	nierBatch(nierBatch_t * ptr) : nierBatch_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(vertexGroupIndex);
			LITTLE_BIG_SWAP(boneSetIndex);
			LITTLE_BIG_SWAP(vertexStart);
			LITTLE_BIG_SWAP(indexStart);
			LITTLE_BIG_SWAP(numVerts);
			LITTLE_BIG_SWAP(numIndices);
			LITTLE_BIG_SWAP(numPrimitives);
		}
	}
};
typedef struct nierBoneSet_s {
	unsigned int	ofsBoneSet;
	unsigned int	numBoneIndices;
} nierBoneSet_t;
template <bool big>
struct nierBoneSet : public nierBoneSet_s {
	nierBoneSet(nierBoneSet_t * ptr) : nierBoneSet_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(ofsBoneSet);
			LITTLE_BIG_SWAP(numBoneIndices);
		}
	}
};

#define BAYO_VERTEX_FMT_PRESENT		0x40000000
#define BAYO_VERTEX_FMT_UNKNOWNA	0x20000000
#define BAYO_VERTEX_FMT_UNKNOWNB	0x10000000
#define BAYO_VERTEX_FMT_COMPRESSED	0x08000000
#define BAYO_VERTEX_FMT_POSITION2	0x00000020
#define BAYO_VERTEX_FMT_BONEINFOS	0x00000010
#define BAYO_VERTEX_FMT_NORMAL		0x00000008
#define BAYO_VERTEX_FMT_MAPPING		0x00000004
#define BAYO_VERTEX_FMT_COLOR		0x00000002
#define BAYO_VERTEX_FMT_TANGENTS	0x00000001

typedef struct bayoWMBHdr_s
{
	BYTE				id[4];				// 0
	int					unknownA;			// 4
	int					vertexFormat;			// 8
	int					numVerts;			// C
	BYTE				numMapping;			//10
	BYTE				unknownD;			//11
	WORD				unknownE;			//12
	int					ofsPositions;		//14
	int					ofsVerts;			//18
	int					ofsVertExData;		//1C
	int					unknownG[4];		//20
	int					numBones;			//30
	int					ofsBoneHie;			//34
	int					ofsBoneDataA;		//38
	int					ofsBoneDataB;		//3C
	int					ofsBoneHieB;		//40
	int					numMaterials;		//44
	int					ofsMaterialsOfs;	//48
	int					ofsMaterials;		//4C
	int					numMeshes;			//50
	int					ofsMeshOfs;			//54
	int					ofsMeshes;			//58
	int					unknownK;			//5C
	int					unknownL;			//60
	int					ofsUnknownJ;		//64
	int					ofsBoneSymmetries;	//68
	int					ofsBoneFlags;		//6C
	int					exMatInfo[4];		//70
} bayoWMBHdr_t;
template <bool big>
struct bayoWMBHdr : public bayoWMBHdr_s {
	bayoWMBHdr( bayoWMBHdr_t * ptr ): bayoWMBHdr_s(*ptr){
		if (big) {
			LITTLE_BIG_SWAP(*((int *)id));
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(vertexFormat);
			LITTLE_BIG_SWAP(numVerts);
			LITTLE_BIG_SWAP(unknownE);
			LITTLE_BIG_SWAP(ofsPositions);
			LITTLE_BIG_SWAP(ofsVerts);
			LITTLE_BIG_SWAP(ofsVertExData);
			for(int i=0; i<4; i++) LITTLE_BIG_SWAP(unknownG[i]);
			LITTLE_BIG_SWAP(numBones);
			LITTLE_BIG_SWAP(ofsBoneHie);
			LITTLE_BIG_SWAP(ofsBoneDataA);
			LITTLE_BIG_SWAP(ofsBoneDataB);
			LITTLE_BIG_SWAP(ofsBoneHieB);
			LITTLE_BIG_SWAP(numMaterials);
			LITTLE_BIG_SWAP(ofsMaterialsOfs);
			LITTLE_BIG_SWAP(ofsMaterials);
			LITTLE_BIG_SWAP(numMeshes);
			LITTLE_BIG_SWAP(ofsMeshOfs);
			LITTLE_BIG_SWAP(ofsMeshes);
			LITTLE_BIG_SWAP(unknownK);
			LITTLE_BIG_SWAP(unknownL);
			LITTLE_BIG_SWAP(ofsUnknownJ);
			LITTLE_BIG_SWAP(ofsBoneSymmetries);
			LITTLE_BIG_SWAP(ofsBoneFlags);
			for(int i=0; i<4; i++) LITTLE_BIG_SWAP(exMatInfo[i]);
		}
	}
};
typedef struct wmbMesh_s
{
	WORD				id;
	WORD				numBatch;
	int					unknownB;
	int					batchOfs;
	int					unknownD;
	int					unknownE[4];
	char				name[32];
	float				mat[12];
} wmbMesh_t;
template <bool big>
struct wmbMesh : public wmbMesh_s {
	wmbMesh( wmbMesh_t * ptr ): wmbMesh_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(id);
			LITTLE_BIG_SWAP(numBatch);
			LITTLE_BIG_SWAP(unknownB);
			LITTLE_BIG_SWAP(batchOfs);
			LITTLE_BIG_SWAP(unknownD);
			for(int i=0; i<4; i++) LITTLE_BIG_SWAP(unknownE[i]);
			for(int i=0; i<12; i++) LITTLE_BIG_SWAP(mat[i]);
		}
	}
};
typedef struct wmbBatch_s
{
	WORD				unknownA;
	WORD				id;
	WORD				unknownB;
	WORD				unknownC;
	BYTE				texID;
	BYTE				unknownDB;
	BYTE				unknownE1;
	BYTE				unknownE2;
	int					vertStart;
	int					vertEnd;
	int					primType;
	int					ofsIndices;
	int					numIndices;
	int					vertOfs;
	int					unknownI[7];
} wmbBatch_t;
template <bool big>
struct wmbBatch : public wmbBatch_s {
	wmbBatch( wmbBatch_t * ptr ): wmbBatch_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(id);
			LITTLE_BIG_SWAP(unknownB);
			LITTLE_BIG_SWAP(unknownC);
			LITTLE_BIG_SWAP(texID);
			LITTLE_BIG_SWAP(unknownDB);
			LITTLE_BIG_SWAP(unknownE1);
			LITTLE_BIG_SWAP(unknownE2);
			LITTLE_BIG_SWAP(vertStart);
			LITTLE_BIG_SWAP(vertEnd);
			LITTLE_BIG_SWAP(primType);
			LITTLE_BIG_SWAP(ofsIndices);
			LITTLE_BIG_SWAP(numIndices);
			LITTLE_BIG_SWAP(vertOfs);
			for(int i=0; i<7; i++) LITTLE_BIG_SWAP(unknownI[i]);
		}
	}
};
typedef struct bayoTex_s
{
	BYTE				tex_idx;
	BYTE				tex_flagA;
	WORD				tex_flagB;
} bayoTex_t;
typedef struct wmbMat_s
{
	WORD				matFlags;
	WORD				unknownB;
	bayoTex_t			texs[5];
} wmbMat_t;
template <bool big>
struct wmbMat : public wmbMat_s {
	wmbMat( wmbMat_t *ptr ): wmbMat_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(matFlags);
			LITTLE_BIG_SWAP(unknownB);
			for( int i = 0; i < 5; i++ ) {
				LITTLE_BIG_SWAP(texs[i]);
			}
		}
	}
};
typedef struct bayo2EXPHdr_s
{
	BYTE				id[4];
	int					version;
	unsigned int		offsetRecords;
	int					numRecords;
	unsigned int		offsetInterpolationData;
	int					numtInterpolationData;
} bayo2EXPHdr_t;
template <bool big>
struct bayo2EXPHdr : public bayo2EXPHdr_s
{
	bayo2EXPHdr(bayo2EXPHdr_t *ptr) : bayo2EXPHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(version);
			LITTLE_BIG_SWAP(offsetRecords);
			LITTLE_BIG_SWAP(numRecords);
			LITTLE_BIG_SWAP(offsetInterpolationData);
			LITTLE_BIG_SWAP(numtInterpolationData);
		}
	}
};
typedef struct bayoEXPHdr_s
{
	BYTE				id[4];
	int					unknownA;
	unsigned int		offsetRecords;
	unsigned int		numRecords;
} bayoEXPHdr_t;
template <bool big>
struct bayoEXPHdr : public bayoEXPHdr_s
{
	bayoEXPHdr(bayoEXPHdr_t *ptr) : bayoEXPHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(offsetRecords);
			LITTLE_BIG_SWAP(numRecords);
		}
	}
};
typedef struct bayo2EXPRecord_s
{
	short int			boneIndex;
	char				animationTrack;
	char				unknownA;
	short int			valueCount;
	short int			unknwonB;
	unsigned int		offset;
} bayo2EXPRecord_t;
template <bool big>
struct bayo2EXPRecord : public bayo2EXPRecord_s
{
	bayo2EXPRecord(bayo2EXPRecord_t *ptr) : bayo2EXPRecord_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(boneIndex);
			LITTLE_BIG_SWAP(valueCount);
			LITTLE_BIG_SWAP(unknwonB);
			LITTLE_BIG_SWAP(offset);
		}
	}
};

typedef struct bayoEXPRecord_s
{
	short int			flags;
	short int			boneIndex;
	char				animationTrack;
	char				entryCount;
	char				interpolationEntryCount;
	char				interpolationType;
	short int			numPoints;
	short int			unknownC;
	unsigned int		offset;
	unsigned int		offsetInterpolation;
} bayoEXPRecord_t;
template <bool big>
struct bayoEXPRecord : public bayoEXPRecord_s
{
	bayoEXPRecord(bayoEXPRecord_t * ptr) : bayoEXPRecord_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(flags);
			LITTLE_BIG_SWAP(boneIndex);
			LITTLE_BIG_SWAP(numPoints);
			LITTLE_BIG_SWAP(unknownC);
			LITTLE_BIG_SWAP(offset);
			LITTLE_BIG_SWAP(offsetInterpolation);
		}
	}
};
typedef struct bayoEXPInterpolationPoint2_s {
	float value;
	float p;
	float m0;
	float m1;
}bayoEXPInterpolationPoint2_t;
template <bool big>
struct bayoEXPInterpolationPoint2 : public bayoEXPInterpolationPoint2_s
{
	bayoEXPInterpolationPoint2(bayoEXPInterpolationPoint2_t * ptr) : bayoEXPInterpolationPoint2_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(value);
			LITTLE_BIG_SWAP(p);
			LITTLE_BIG_SWAP(m0);
			LITTLE_BIG_SWAP(m1);
		}
	}
};
typedef struct bayoEXPInterpolationData4_s
{
	float p;
	float dp;
	float m0;
	float dm0;
	float m1;
	float dm1;
} bayoEXPInterpolationData4_t;
template <bool big>
struct bayoEXPInterpolationData4 : public bayoEXPInterpolationData4_s
{
	bayoEXPInterpolationData4(bayoEXPInterpolationData4_t * ptr) : bayoEXPInterpolationData4_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(p);
			LITTLE_BIG_SWAP(dp);
			LITTLE_BIG_SWAP(m0);
			LITTLE_BIG_SWAP(dm0);
			LITTLE_BIG_SWAP(m1);
			LITTLE_BIG_SWAP(dm1);
		}
	}
};
typedef struct bayoEXPInterpolationPoint4_s {
	float			v;
	unsigned short	dummy;
	unsigned short	cp;
	unsigned short	cm0;
	unsigned short	cm1;
} bayoEXPInterpolationPoint4_t;
template <bool big>
struct bayoEXPInterpolationPoint4 : public bayoEXPInterpolationPoint4_s
{
	bayoEXPInterpolationPoint4(bayoEXPInterpolationPoint4_t * ptr) : bayoEXPInterpolationPoint4_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(v);
			LITTLE_BIG_SWAP(dummy);
			LITTLE_BIG_SWAP(cp);
			LITTLE_BIG_SWAP(cm0);
			LITTLE_BIG_SWAP(cm1);
		}
	}
};
typedef short unsigned int pghalf;
typedef struct bayoEXPInterpolationData6_s
{
	pghalf p;
	pghalf dp;
	pghalf m0;
	pghalf dm0;
	pghalf m1;
	pghalf dm1;
} bayoEXPInterpolationData6_t;
template <bool big>
struct bayoEXPInterpolationData6 : public bayoEXPInterpolationData6_s
{
	bayoEXPInterpolationData6(bayoEXPInterpolationData6_t * ptr) : bayoEXPInterpolationData6_s(*ptr) {
	}
};
typedef struct bayoEXPInterpolationPoint6_s {
	unsigned char	v;
	unsigned char	cp;
	unsigned char	cm0;
	unsigned char	cm1;
} bayoEXPInterpolationPoint6_t;
template <bool big>
struct bayoEXPInterpolationPoint6 : public bayoEXPInterpolationPoint6_s
{
	bayoEXPInterpolationPoint6(bayoEXPInterpolationPoint6_t * ptr) : bayoEXPInterpolationPoint6_s(*ptr) {
	}
};
typedef struct bayo2EXPValue_s
{
	char			type;
	char			animationTrack;
	short int		boneIndex;
	float			value;
} bayo2EXPValue_t;
template <bool big>
struct bayo2EXPValue : public bayo2EXPValue_s
{
	bayo2EXPValue(bayo2EXPValue_t * ptr) : bayo2EXPValue_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(boneIndex);
			LITTLE_BIG_SWAP(value);
		}
	}
};
typedef struct bayo2EXPInterpolationData_s
{
	short int		numPoints;
	short int		unknownA;
	unsigned int	offset;
} bayo2EXPInterpolationData_t;
template <bool big>
struct bayo2EXPInterpolationData : public bayo2EXPInterpolationData_s
{
	bayo2EXPInterpolationData(bayo2EXPInterpolationData_t * ptr) : bayo2EXPInterpolationData_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(numPoints);
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(offset);
		}
	}
};
typedef struct bayo2EXPInterpolationPoint_s
{
	float v;
	float p;
	float m0;
	float m1;
} bayo2EXPInterpolationPoint_t;
template <bool big>
struct bayo2EXPInterpolationPoint : public bayo2EXPInterpolationPoint_s
{
	bayo2EXPInterpolationPoint(bayo2EXPInterpolationPoint_t *ptr) : bayo2EXPInterpolationPoint_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(v);
			LITTLE_BIG_SWAP(p);
			LITTLE_BIG_SWAP(m0);
			LITTLE_BIG_SWAP(m1);
		}
	}
};

typedef struct bayoExpEntryBoneTrackInfo_s {
	short int			boneIndex;
	char				animationTrack;
	char				padding;
} bayoExpEntryBoneTrackInfo_t;
typedef union bayoExpEntryUnion_u {
	float value;
	bayoExpEntryBoneTrackInfo_t boneTrackInfo;
} bayoExpEntryUnion_t;
typedef struct bayoEXPEntry_s
{
	unsigned int		flags;
	bayoExpEntryUnion_t entryUnion;
} bayoEXPEntry_t;
template <bool big>
struct bayoEXPEntry : public bayoEXPEntry_s
{
	bayoEXPEntry(bayoEXPEntry_t*ptr) : bayoEXPEntry_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(flags);
			if (flags & 0x80000000) {
				LITTLE_BIG_SWAP(entryUnion.boneTrackInfo.boneIndex);
			}
			else {
				LITTLE_BIG_SWAP(entryUnion.value);
			}
		}
	}
};
typedef struct bayoMOTHdr_s
{
	BYTE				id[4];
	short int			unknownA;
	short int			frameCount;
	int					ofsMotion;
	int					numEntries;
} bayoMOTHdr_t;
template <bool big>
struct bayoMOTHdr : public bayoMOTHdr_s {
	bayoMOTHdr( bayoMOTHdr_t *ptr ): bayoMOTHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(frameCount);
			LITTLE_BIG_SWAP(ofsMotion);
			LITTLE_BIG_SWAP(numEntries);
		}
	}
};
typedef struct bayo2MOTHdr_s
{
	BYTE				id[4];
	int					hash;
	short int			unknownA;
	short int			frameCount;
	int					ofsMotion;
	int					numEntries;
} bayo2MOTHdr_t;
template <bool big>
struct bayo2MOTHdr : public bayo2MOTHdr_s {
	bayo2MOTHdr( bayo2MOTHdr_t *ptr ): bayo2MOTHdr_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(hash);
			LITTLE_BIG_SWAP(unknownA);
			LITTLE_BIG_SWAP(frameCount);
			LITTLE_BIG_SWAP(ofsMotion);
			LITTLE_BIG_SWAP(numEntries);
		}
	}
};
typedef union bayoMotField_u
{
	float flt;
	int offset;
} bayoMotField_t;
typedef struct bayoMotItem_s
{
	short int			boneIndex;
	char				index;
	BYTE				flag;
	short int			elem_number;
	short int			unknown;
	bayoMotField_t		value;
} bayoMotItem_t;
template <bool big>
struct bayoMotItem : public bayoMotItem_s {
	bayoMotItem( bayoMotItem_t *ptr ): bayoMotItem_s(*ptr) {
		if(big) {
			LITTLE_BIG_SWAP(boneIndex);
			LITTLE_BIG_SWAP(index);
			LITTLE_BIG_SWAP(flag);
			LITTLE_BIG_SWAP(elem_number);
			LITTLE_BIG_SWAP(unknown);
			LITTLE_BIG_SWAP(value);
		}
	}
};
typedef struct bayo2InterpolKeyframe4_s {
	unsigned short int index;
	unsigned short int dummy;
	float p;
	float m0;
	float m1;
} bayo2InterpolKeyframe4_t;
template <bool big>
struct bayo2InterpolKeyframe4 : public bayo2InterpolKeyframe4_s {
	bayo2InterpolKeyframe4( bayo2InterpolKeyframe4_t *ptr ): bayo2InterpolKeyframe4_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(index);
			LITTLE_BIG_SWAP(dummy);
			LITTLE_BIG_SWAP(p);
			LITTLE_BIG_SWAP(m0);
			LITTLE_BIG_SWAP(m1);
		}
	}
};
typedef struct bayoInterpolHeader4_s {
	float values[6];
} bayoInterpolHeader4_t;
typedef struct bayoInterpolKeyframe4_s {
	unsigned short int index;
	unsigned short int coeffs[3];
} bayoInterpolKeyframe4_t;
template <bool big>
struct bayoInterpolKeyframe4 : public bayoInterpolKeyframe4_s {
	bayoInterpolKeyframe4( bayoInterpolKeyframe4_t *ptr): bayoInterpolKeyframe4_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(index);
			for(int i = 0; i < 3; i++) LITTLE_BIG_SWAP(coeffs[i]);
		}
	}
};
typedef struct bayoInterpolHeader6_s {
	pghalf values[6];
} bayoInterpolHeader6_t;
typedef struct bayoInterpolKeyframe6_s {
	BYTE index;
	BYTE coeffs[3];
} bayoInterpolKeyframe6_t;
template <bool big>
struct bayoInterpolKeyframe6 : public bayoInterpolKeyframe6_s {
	bayoInterpolKeyframe6( bayoInterpolKeyframe6_t *ptr): bayoInterpolKeyframe6_s(*ptr) {
	}
};
typedef struct bayoInterpolHeader7_s {
	pghalf values[6];
} bayoInterpolHeader7_t;
typedef struct bayoInterpolKeyframe7_s {
	unsigned short int index;
	BYTE dummy;
	BYTE coeffs[3];
} bayoInterpolKeyframe7_t;
template <bool big>
struct bayoInterpolKeyframe7 : public bayoInterpolKeyframe7_s {
	bayoInterpolKeyframe7( bayoInterpolKeyframe7_t *ptr): bayoInterpolKeyframe7_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(index);
		}
	}
};
typedef struct bayoInterpolHeader8_s {
	pghalf values[6];
} bayoInterpolHeader8_t;
typedef struct bayoInterpolKeyframe8_s {
	unsigned short int index;
	BYTE coeffs[3];
} bayoInterpolKeyframe8_t;
template <bool big>
struct bayoInterpolKeyframe8 : public bayoInterpolKeyframe8_s {
	bayoInterpolKeyframe8( bayoInterpolKeyframe8_t *ptr): bayoInterpolKeyframe8_s(*ptr) {
		if (big) {
			LITTLE_BIG_SWAP(index);
		}
		else {
			LITTLE_BIG_SWAP(index);
		}
	}
};
typedef struct bayoVertexData_s {
	float position[3];			//00
	short uv[2];				//0C //half float really
	char normals[4];			//0F
	int unknwownA;				//14
	unsigned char boneIndex[4];	//18
	unsigned char boneWeight[4];//1C
} bayoVertexData_t;

//thanks Phernost (stackoverflow)
class FloatDecompressor
{
	union Bits
	{
		float f;
		int si;
		unsigned int ui;
	};

	static int const significandFBits = 23;
	static int const exponentFBits = 8;
	static int const biasF = 127;
	static int const exponentF    = 0x7F800000;
	static int const significandF = 0x007fffff;
	static int const signF        = 0x80000000;

	int const significandHBits;
	int const exponentHBits;
	int const biasH;
    int exponentH;
	int significandH;
    static int const signH        = 0x8000;

	int shiftSign;
	int shiftBits;

public:

	FloatDecompressor(int eHBits, int sHBits, int bH): exponentHBits(eHBits), significandHBits(sHBits), biasH(bH) {
		int tmp = 0;
		for(int i = 0; i < eHBits; i++) {
			tmp <<= 1;
			tmp += 1;
		}
		tmp <<= sHBits;
		exponentH = tmp;

		tmp = 0;
		for(int i = 0; i < sHBits; i++) {
			tmp <<= 1;
			tmp += 1;
		}
		significandH = tmp;
		shiftSign = significandFBits + exponentFBits - significandHBits - exponentHBits;
		shiftBits = significandFBits - significandHBits;
	}

	float decompress(short unsigned int value)
	{
		Bits v;
		v.ui = value;
		int sign = v.si & signH;
		v.si ^= sign;

		sign <<=  shiftSign;
		int exponent = v.si & exponentH;
		int significand = v.si ^ exponent;
		significand <<= shiftBits;

        v.si = sign | significand;
		if( exponent == exponentH ) {
			v.si |= exponentF;
		} else if ( exponent != 0 ) {
			exponent >>= significandHBits;
			exponent += biasF - biasH;
			exponent <<= significandFBits;
			v.si |= exponent;
		} else if ( significand ) { //denorm
			Bits magic;
			magic.ui = (biasF - biasH + biasF) << significandFBits;
			v.f *= magic.f;
		}
		return v.f;
	}
	float decompress(short unsigned int value, bool big) {
		short unsigned int v2 = value;
		if (big) LITTLE_BIG_SWAP(v2);
		return decompress(v2);
	}

};

/*typedef struct bayoMatType_s {
	bool known;
	short size;
	char texture_number;
	char lightmap_index;
	char normalmap_index;
	char texture2_index;
	char reflection_index;
} bayoMatType_t;

bayoMatType_t bayoMatTypes[256];

static void bayoSetMatType(bayoMatType_t &mat,
						   short size,
						   char texture_number,
						   char lightmap_index,
						   char normalmap_index,
						   char texture2_index,
						   char reflection_index) {
	mat.known = true;
	mat.size = size;
	mat.texture_number = texture_number;
	mat.lightmap_index = lightmap_index;
	mat.normalmap_index = normalmap_index;
	mat.texture2_index = texture2_index;
	mat.reflection_index = reflection_index;
}
static void bayoUnsetMatType(bayoMatType_t &mat) {
	mat.known = false;
	mat.size = 0;
	mat.texture_number = 1;
	mat.lightmap_index = -1;
	mat.normalmap_index = -1;
	mat.texture2_index = -1;
	mat.reflection_index = -1;
}
static void bayoSetMatTypes(void) {
	for(int i=0; i<256; i++) {
		bayoUnsetMatType(bayoMatTypes[i]);
	}
	bayoSetMatType(bayoMatTypes[0x31], 0xC0, 3,  1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x32], 0xE4, 4,  1, -1, -1,  3);
	bayoSetMatType(bayoMatTypes[0x33], 0xD4, 4,  2, -1,  1, -1);
	bayoSetMatType(bayoMatTypes[0x34], 0xF8, 5,  2, -1,  1,  4);
	bayoSetMatType(bayoMatTypes[0x38], 0xD4, 4, -1,  2, -1, -1);
	bayoSetMatType(bayoMatTypes[0x3A], 0xD4, 4,  1,  2, -1, -1);
	bayoSetMatType(bayoMatTypes[0x3C], 0xD4, 4, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x40], 0xC4, 4, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x42], 0xAC, 2, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x44], 0xE4, 4,  1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x47], 0x68, 1, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x48], 0xC0, 3,  1, -1,  2, -1);
	bayoSetMatType(bayoMatTypes[0x4A], 0xD4, 4,  2, -1,  1, -1);
	bayoSetMatType(bayoMatTypes[0x4B], 0xD4, 4, -1,  2, -1, -1);
	bayoSetMatType(bayoMatTypes[0x4C], 0xAC, 2, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x53], 0x68, 1, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x54], 0xD4, 4,  1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x59], 0xD4, 4,  1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x60], 0x68, 1, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x68], 0xAC, 2, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x6B], 0xD0, 3, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x6D], 0xD0, 3, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x6E], 0xD4, 4, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x71], 0xE4, 4,  1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x72], 0xD4, 4, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x75], 0xAC, 2, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x7C], 0xEA, 4,  1, -1, -1,  3);
	bayoSetMatType(bayoMatTypes[0x7F], 0x124,4, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x81], 0x120,3, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x83], 0xAC, 2, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x87], 0xD4, 4, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0x89], 0xC0, 3,  1, -1, -1,  2);
	bayoSetMatType(bayoMatTypes[0x8F], 0xD4, 4,  1, -1,  2,  3);
	bayoSetMatType(bayoMatTypes[0x97], 0x114,4, -1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0xA1], 0xB0, 3,  1, -1, -1, -1);
	bayoSetMatType(bayoMatTypes[0xA3], 0xE4, 4, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0xB2], 0xD4, 4, -1,  1, -1, -1);
	bayoSetMatType(bayoMatTypes[0xB3], 0x124,4, -1,  1, -1, -1);

}*/

#include "bayonetta_materials.h"

//see if something is a valid bayonetta .dat
template <bool big, game_e game>
bool Model_Bayo_Check(BYTE *fileBuffer, int bufferLen, noeRAPI_t *rapi)
{
	DBGLOG("Checking %s %s\n", big? "big" : "little", game == BAYONETTA ? "Bayonetta" : game == BAYONETTA2 ? "Bayonetta2" : game == VANQUISH ? "Vanquish" : "Nier Automata");
	if (bufferLen < sizeof(bayoDat_t))
	{
		return false;
	}
	bayoDat<big> dat((bayoDat_t *)fileBuffer);
	if (memcmp(dat.id, "DAT\0", 4))
	{
		return false;
	}
	if (dat.numRes <= 0 ||
		dat.ofsRes <= 0 || dat.ofsRes >= bufferLen ||
		dat.ofsType <= 0 || dat.ofsType >= bufferLen ||
		dat.ofsNames <= 0 || dat.ofsNames >= (bufferLen-4) ||
		dat.ofsSizes <= 0 || dat.ofsSizes >= bufferLen)
	{
		return false;
	}

	BYTE *namesp = fileBuffer+dat.ofsNames;
	int strSize = *((int *)namesp);
	if (big) LITTLE_BIG_SWAP(strSize);

	namesp += sizeof(int);
	if (strSize <= 0 || strSize >= bufferLen || dat.ofsNames+(int)sizeof(int)+(strSize*dat.numRes) > bufferLen)
	{
		return false;
	}
	int numWMB = 0;
	int numMOT = 0;
	int numSCR = 0;
	DBGLOG("Found %d resources\n", dat.numRes);
	for (int i = 0; i < dat.numRes; i++)
	{
		char *name = (char *)namesp;
		if (name[strSize-1])
		{ //incorrectly terminated string
			return false;
		}
		DBGLOG("\t%s\n", name);
		if (rapi->Noesis_CheckFileExt(name, ".wmb"))
		{
			numWMB++;
			//Try to rule out non bayonetta file.
			int sizeWmb = ((int*)(fileBuffer + dat.ofsSizes))[i];
			if (big) {
				LITTLE_BIG_SWAP(sizeWmb);
			}
			if ( sizeWmb > 12) {
				int offWmb = ((int*)(fileBuffer + dat.ofsRes))[i];
				if (big) {
					LITTLE_BIG_SWAP(offWmb);
				}
				unsigned int version = ((int*)(fileBuffer + offWmb))[1];
				if (big) {
					LITTLE_BIG_SWAP(version);
				}
				if (game != NIER_AUTOMATA && version == 0x20160116) {
					DBGLOG("Found Nier Automata File!\n");
					return false;
				}
				if (game == NIER_AUTOMATA && version != 0x20160116) {
					DBGLOG("Found non Nier Automata File!\n");
					return false;
				}
				unsigned int vertex_type = ((int*)(fileBuffer + offWmb))[2];
				if (big) {
					LITTLE_BIG_SWAP(vertex_type);
				}
				if( game == BAYONETTA && vertex_type & 0x400000) {
					DBGLOG("Found Bayonetta 2 or Vanquish File!\n");
					return false;
				}
			}
		}
		else if (game == BAYONETTA2 && rapi->Noesis_CheckFileExt(name, ".wtb"))
		{
			DBGLOG("Found Bayonetta or Vanquish File!\n");
			return false;
		}
		else if ((game == BAYONETTA || game == VANQUISH) && (rapi->Noesis_CheckFileExt(name, ".wta") || rapi->Noesis_CheckFileExt(name, ".wtp")))
		{
			DBGLOG("Found Bayonetta 2 File!\n");
			return false;
		}
		else if ((game == BAYONETTA || game == BAYONETTA2) && rapi->Noesis_CheckFileExt(name, ".hkx"))
		{
			DBGLOG("Found Vanquish File!\n");
			return false;
		}
		//todo - figure out and add support for animation data
		/*
		else if (rapi->Noesis_CheckFileExt(name, ".mot"))
		{
			numMOT++;
		}
		*/
		else if (rapi->Noesis_CheckFileExt(name, ".scr"))
		{
			numSCR++;
		}

		namesp += strSize;
	}
	if (numWMB <= 0 && numMOT <= 0 && numSCR <= 0)
	{ //nothing of interest in here
		return false;
	}
	if (game == BAYONETTA2 && numSCR > 0) {
		namesp = fileBuffer + dat.ofsNames + sizeof(int);
		bool found = false;
		for (int i = 0; i < dat.numRes; i++) {
			char *name = (char *)namesp;
			//DBGLOG("name: %s", name);
			if (rapi->Noesis_CheckFileExt(name, ".wta")) {
				found = true;
			}
			namesp += strSize;
		}
		if (!found) {
			return false;
		}
	}
	DBGLOG("Found %d wmb files\n", numWMB);
	DBGLOG("Found %d scr files\n", numSCR);
	return true;
}

//get a texture bundle file for a specific model
template <game_t game>
static void Model_Bayo_GetTextureBundle(CArrayList<bayoDatFile_t *> &texFiles, CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi);
template <>
static void Model_Bayo_GetTextureBundle<BAYONETTA>(CArrayList<bayoDatFile_t *> &texFiles, CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi)
{
	char texName[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(texName, df.name);
	strcat_s(texName, MAX_NOESIS_PATH, ".wtb");
	DBGLOG("texName: %s\n", texName);
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, texName))
		{
			texFiles.Append(&dft);
		}
	}
}
template <>
static void Model_Bayo_GetTextureBundle<VANQUISH>(CArrayList<bayoDatFile_t *> &texFiles, CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi) {
	Model_Bayo_GetTextureBundle<BAYONETTA>(texFiles, dfiles, df, rapi);
}
template <>
static void Model_Bayo_GetTextureBundle<BAYONETTA2>(CArrayList<bayoDatFile_t *> &texFiles, CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi)
{
	char texName[MAX_NOESIS_PATH];
	char texName2[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(texName, df.name);
	strcat_s(texName, MAX_NOESIS_PATH, ".wta");
	rapi->Noesis_GetExtensionlessName(texName2, df.name);
	strcat_s(texName2, MAX_NOESIS_PATH, ".wtp");
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, texName))
		{
			texFiles.Append(&dft);
		}
	}
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, texName2))
		{
			texFiles.Append(&dft);
		}
	}
	if (texFiles.Num() < 2) {
		texFiles.Clear();
	}
}
template <>
static void Model_Bayo_GetTextureBundle<NIER_AUTOMATA>(CArrayList<bayoDatFile_t *> &texFiles, CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi) {
	Model_Bayo_GetTextureBundle<BAYONETTA2>(texFiles, dfiles, df, rapi);
}
static void Model_Bayo2_GetSCRTextureBundle(CArrayList<bayoDatFile_t *> &texFiles, CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi)
{
	char texName[MAX_NOESIS_PATH];
	char texName2[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(texName, df.name);
	strcat_s(texName, MAX_NOESIS_PATH, "scr.wta");
	rapi->Noesis_GetExtensionlessName(texName2, df.name);
	strcat_s(texName2, MAX_NOESIS_PATH, "scr.wtp");
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, texName))
		{
			texFiles.Append(&dft);
		}
	}
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, texName2))
		{
			texFiles.Append(&dft);
		}
	}
	if (texFiles.Num() < 2) {
		texFiles.Clear();
	}
	rapi->Noesis_GetExtensionlessName(texName, df.name);
	strcat_s(texName, MAX_NOESIS_PATH, "shared.wta");
	rapi->Noesis_GetExtensionlessName(texName2, df.name);
	strcat_s(texName2, MAX_NOESIS_PATH, "shared.wtp");
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, texName))
		{
			texFiles.Append(&dft);
		}
	}
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, texName2))
		{
			texFiles.Append(&dft);
		}
	}
	if (texFiles.Num() == 3 || texFiles.Num() == 1) {
		texFiles.Pop();
	}
}
// get motion files
static void Model_Bayo_GetMotionFiles(CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<bayoDatFile_t *> &motfiles, const char *prefix = NULL)
{
	char motName[MAX_NOESIS_PATH];
	if (prefix)
	{
		sprintf_s(motName, MAX_NOESIS_PATH, "%s", prefix);
	}
	else
	{
		rapi->Noesis_GetExtensionlessName(motName, df.name);
	}
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (rapi->Noesis_CheckFileExt(dft.name, ".mot") && !_strnicoll(dft.name, motName, strlen(motName)))
		{
			DBGLOG("Found motion file: %s\n", dft.name);
			motfiles.Append(&dft);
		}
	}
}
// get exp file
static void Model_Bayo_GetEXPFile(CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<bayoDatFile_t *> &expfile)
{
	char expName[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(expName, df.name);
	strcat_s(expName, MAX_NOESIS_PATH, ".exp");
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &dft = dfiles[i];
		if (!_stricmp(dft.name, expName))
		{
			expfile.Append(&dft);
		}
	}
}
//load texture bundle
template <bool big, game_t game>
static void Model_Bayo_LoadTextures(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi);
template <>
static void Model_Bayo_LoadTextures<false, BAYONETTA2>(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi)
{
	const bool big = false;
	int dataSize = texFiles[0]->dataSize;
	BYTE * data = texFiles[0]->data;
	int dataSize2 = texFiles[1]->dataSize;
	BYTE * data2 = texFiles[1]->data;
	char texName[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(texName, texFiles[0]->name);
	if (dataSize < sizeof(bayoWTAHdr_t))
	{
		return;
	}
	bayoWTAHdr<big> hdr((bayoWTAHdr_t *)data);
	if (memcmp(hdr.id, "WTB\0", 4))
	{ //not a valid texture bundle
		return;
	}
	if (hdr.numTex <= 0 || hdr.ofsTexOfs <= 0 || hdr.ofsTexOfs >= dataSize ||
		hdr.ofsTexSizes <= 0 || hdr.ofsTexSizes >= dataSize)
	{
		return;
	}
	DBGLOG("found valid texture header file, containing %d textures, headers offset: %x\n", hdr.numTex, hdr.texInfoOffset);
	int *tofs = (int *)(data + hdr.ofsTexOfs);
	int *tsizes = (int *)(data + hdr.ofsTexSizes);
	for (int i = 0; i < hdr.numTex; i++)
	{
		int globalIdx;
		char fname[8192];
		char fnamebntx[8192];
		rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());

		char nameStr[MAX_NOESIS_PATH];
		sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%s%s%03i", rapi->Noesis_GetOption("texpre"), texName, i);
		strcat_s(fname, MAX_NOESIS_PATH, nameStr);
		sprintf_s(fnamebntx, MAX_NOESIS_PATH, "%s.bntx", fname);

		if (hdr.texIdxOffset)
		{
			int *ip = (int  *)(data + hdr.texIdxOffset + sizeof(int)*i);
			globalIdx = *ip;
		}
		DBGLOG("%s: 0x%0x\n", fname, globalIdx);
		FILE  * fbntx = fopen(fnamebntx, "wb");
		fwrite(data2 + tofs[i], 1, tsizes[i], fbntx);
		fclose(fbntx);

		noesisTex_t	*nt = rapi->Noesis_LoadExternalTex(fnamebntx);
		if (nt) {
			nt->filename = rapi->Noesis_PooledString(fname);
			textures.Append(nt);
		}
		else {
			DBGLOG("Could not load texture %s\n", fnamebntx);
			nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, false);
			textures.Append(nt);
		}
		remove(fnamebntx);
		nt->globalIdx = globalIdx;

	}
	//insert a flat normal map placeholder
	char fname[MAX_NOESIS_PATH];
	rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
	char nameStr[MAX_NOESIS_PATH];
	sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayoflatnormal", rapi->Noesis_GetOption("texpre"));
	strcat_s(fname, MAX_NOESIS_PATH, nameStr);
	noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, true);
	textures.Append(nt);
}
template <>
static void Model_Bayo_LoadTextures<true, BAYONETTA2>(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi)
{
	const bool big = true;
	int dataSize = texFiles[0]->dataSize;
	BYTE * data = texFiles[0]->data;
	int dataSize2 = texFiles[1]->dataSize;
	BYTE * data2 = texFiles[1]->data;
	char texName[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(texName, texFiles[0]->name);
	if (dataSize < sizeof(bayoWTAHdr_t))
	{
		return;
	}
	bayoWTAHdr<true> hdr((bayoWTAHdr_t *)data);
	if (memcmp(hdr.id, "WTB\0", 4))
	{ //not a valid texture bundle
		return;
	}
	if (hdr.numTex <= 0 || hdr.ofsTexOfs <= 0 || hdr.ofsTexOfs >= dataSize ||
		hdr.ofsTexSizes <= 0 || hdr.ofsTexSizes >= dataSize)
	{
		return;
	}
	DBGLOG("found valid texture header file, containing %d textures, headers offset: %x\n", hdr.numTex, hdr.texInfoOffset);
	int *tofs = (int *)(data+hdr.ofsTexOfs);
	int *tsizes = (int *)(data+hdr.ofsTexSizes);
	int *mofs = (int *)(data+hdr.ofsMipmapOfs);
	for (int i = 0; i < hdr.numTex; i++)
	{
		int globalIdx;
		char fname[8192];
		char fnamegtx[8192];
		char fnamedds[8192];
		char cmd[8192];
		rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());

		char nameStr[MAX_NOESIS_PATH];
		sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%s%s%03i", rapi->Noesis_GetOption("texpre"), texName, i);
		strcat_s(fname, MAX_NOESIS_PATH, nameStr);
		sprintf_s(fnamegtx, MAX_NOESIS_PATH, "%s.gtx", fname);
		sprintf_s(fnamedds, MAX_NOESIS_PATH, "%s.dds", fname);

		GX2Hdr<big> texHdr((GX2Hdr_t *)(data + hdr.texInfoOffset + i*0xc0));
		//DBGLOG("%d: dim: %d, width %d, height %d, depth %d, numMimap: %d, format: %x\n", i, texHdr.dimension, texHdr.width, texHdr.height, texHdr.depth, texHdr.numMipmaps, texHdr.format);
		//DBGLOG("\tusage: %x, length: %d, mipmapDataLength: %d, tileMode: %x\n", texHdr.usage, texHdr.dataLength, texHdr.mipmapsDataLength, texHdr.tileMode);
		//DBGLOG("\tswizzle: %x, alignment %d, pitch %d, first mipmap: %d\n", texHdr.swizzleValue, texHdr.alignment, texHdr.pitch, texHdr.firstMipmap);
		//DBGLOG("\tcomponents: %d, %d, %d, %d\n", texHdr.component[0], texHdr.component[1], texHdr.component[2], texHdr.component[3]);

		if (hdr.texIdxOffset)
		{
			int *ip = (int  *)(data+hdr.texIdxOffset+sizeof(int)*i);
			globalIdx = *ip;
		}
		DBGLOG("%s: 0x%0x\n", fname, globalIdx);
		//GX2_SURFACE_FORMAT_T_BC1_UNORM = 0x31
		//GX2_SURFACE_FORMAT_T_BC3_UNORM = 0x33
		FILE  * fgtx = fopen(fnamegtx, "wb");
		// gtx header (http://mk8.tockdom.com/wiki/GTX%5CGSH_(File_Format))
		fwrite("\x47\x66\x78\x32\x00\x00\x00\x20\x00\x00\x00\x07\x00\x00\x00\x01\x00\x00\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 1, 0x20, fgtx);
		// block header (GX2 surface)
		fwrite("\x42\x4C\x4B\x7B\x00\x00\x00\x20\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0B\x00\x00\x00\x9C\x00\x00\x00\x00\x00\x00\x00\x00", 1, 0x20, fgtx);
		// write GX2 surface
		fwrite(data + hdr.texInfoOffset + i*0xc0, 1, 0x9c, fgtx);
		// block header (data)
		fwrite("\x42\x4C\x4B\x7B\x00\x00\x00\x20\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0C", 1, 0x14, fgtx);
		DWORD size = texHdr.dataLength;
		if (big) LITTLE_BIG_SWAP(size);
		fwrite(&size, 4, 1,  fgtx);
		fwrite("\x00\x00\x00\x00\x00\x00\x00\x00", 1, 0x8, fgtx);
		// write pixel data
		int tof = tofs[i];
		LITTLE_BIG_SWAP(tof);
		fwrite(data2 + tof, 1, texHdr.dataLength, fgtx);
		if ( texHdr.numMipmaps > 1 ) {
			// block header (mipmap data)
			fwrite("\x42\x4C\x4B\x7B\x00\x00\x00\x20\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x0D", 1, 0x14, fgtx);
			size = texHdr.mipmapsDataLength;
			if (big) LITTLE_BIG_SWAP(size);
			fwrite(&size, 4, 1,  fgtx);
			fwrite("\x00\x00\x00\x00\x00\x00\x00\x00", 1, 0x8, fgtx);
			// write mipmap data
			int mof = mofs[i];
			LITTLE_BIG_SWAP(mof);
			fwrite(data2 + mof, 1, texHdr.mipmapsDataLength, fgtx);
		}
		// write ending header
		fwrite("\x42\x4C\x4B\x7B\x00\x00\x00\x20\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x01\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00", 1, 0x20, fgtx);

		fclose(fgtx);
		sprintf_s(cmd, 8192, "TexConv2.exe -i \"%s\" -o \"%s\"", fnamegtx, fnamedds);

		STARTUPINFOW si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		wchar_t wcmd[8192];
		mbstowcs(wcmd, cmd, strlen(cmd)+1);

		if (CreateProcess(L"Texconv2.exe", wcmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		//system(cmd);
		noesisTex_t	*nt = rapi->Noesis_LoadExternalTex(fnamedds);
		if (nt) {
			nt->filename = rapi->Noesis_PooledString(fname);
			textures.Append(nt);
		} else {
			DBGLOG("Could not load texture %s\n", fnamedds);
			nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, false);
			textures.Append(nt);
		}
		remove(fnamedds);
		remove(fnamegtx);
		nt->globalIdx = globalIdx;

	}
	//insert a flat normal map placeholder
	char fname[MAX_NOESIS_PATH];
	rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
	char nameStr[MAX_NOESIS_PATH];
	sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayoflatnormal", rapi->Noesis_GetOption("texpre"));
	strcat_s(fname, MAX_NOESIS_PATH, nameStr);
	noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, true);
	textures.Append(nt);
}
template <>
static void Model_Bayo_LoadTextures<true, BAYONETTA>(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi)
{
	const bool big = true;
	int dataSize = texFiles[0]->dataSize;
	BYTE * data = texFiles[0]->data;

	if (dataSize < sizeof(bayoWTBHdr_t))
	{
		return;
	}
	bayoWTBHdr<true> hdr((bayoWTBHdr_t *)data);
	if (memcmp(hdr.id, "WTB\0", 4))
	{ //not a valid texture bundle
		return;
	}
	if (hdr.numTex <= 0 || hdr.ofsTexOfs <= 0 || hdr.ofsTexOfs >= dataSize ||
		hdr.ofsTexSizes <= 0 || hdr.ofsTexSizes >= dataSize)
	{
		return;
	}
	DBGLOG("found valid texture header file, containing %d textures\n", hdr.numTex);
	int *tofs = (int *)(data+hdr.ofsTexOfs);
	int *tsizes = (int *)(data+hdr.ofsTexSizes);
	for (int i = 0; i < hdr.numTex; i++)
	{
		int globalIdx = 0;
		char fname[8192];
		char fnamegtx[8192];
		char fnamedds[8192];
		char cmd[8192];
		rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());

		char nameStr[MAX_NOESIS_PATH];
		sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayotex%03i", rapi->Noesis_GetOption("texpre"), i);
		strcat_s(fname, MAX_NOESIS_PATH, nameStr);
		sprintf_s(fnamegtx, MAX_NOESIS_PATH, "%s.gtx", fname);
		sprintf_s(fnamedds, MAX_NOESIS_PATH, "%s.dds", fname);

		if (hdr.texIdxOffset)
		{
			int *ip = (int  *)(data+hdr.texIdxOffset+sizeof(int)*i);
			globalIdx = *ip;
		}
		DBGLOG("%s: 0x%0x\n", fname, globalIdx);
		//GX2_SURFACE_FORMAT_T_BC1_UNORM = 0x31
		//GX2_SURFACE_FORMAT_T_BC3_UNORM = 0x33
		FILE  * fgtx = fopen(fnamegtx, "wb");
		int tof = tofs[i];
		int tsize = tsizes[i];
		LITTLE_BIG_SWAP(tof);
		LITTLE_BIG_SWAP(tsize);
		fwrite(data + tof, 1, tsize, fgtx);
		fclose(fgtx);

		sprintf_s(cmd, 8192, "TexConv2.exe -i \"%s\" -o \"%s\"", fnamegtx, fnamedds);

		STARTUPINFOW si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));
		wchar_t wcmd[8192];
		mbstowcs(wcmd, cmd, strlen(cmd)+1);

		if (CreateProcess(L"Texconv2.exe", wcmd, NULL, NULL, FALSE, CREATE_NO_WINDOW, NULL, NULL, &si, &pi))
		{
			WaitForSingleObject(pi.hProcess, INFINITE);
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
		//system(cmd);
		noesisTex_t	*nt = rapi->Noesis_LoadExternalTex(fnamedds);
		if (nt) {
			nt->filename = rapi->Noesis_PooledString(fname);
			textures.Append(nt);
		} else {
			DBGLOG("Could not load texture %s\n", fnamedds);
			nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, false);
			textures.Append(nt);
		}
		remove(fnamedds);
		remove(fnamegtx);
		nt->globalIdx = globalIdx;

	}
	//insert a flat normal map placeholder
	char fname[MAX_NOESIS_PATH];
	rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
	char nameStr[MAX_NOESIS_PATH];
	sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayoflatnormal", rapi->Noesis_GetOption("texpre"));
	strcat_s(fname, MAX_NOESIS_PATH, nameStr);
	noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, true);
	textures.Append(nt);
}
/*
template <>
static void Model_Bayo_LoadTextures<true, BAYONETTA>(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi)
{
	int dataSize = texFiles[0]->dataSize;
	BYTE * data = texFiles[0]->data;
	if (dataSize < sizeof(bayoWTBHdr_t))
	{
		return;
	}
	bayoWTBHdr<true> hdr((bayoWTBHdr_t *)data);
	if (memcmp(hdr.id, "\0BTW", 4))
	{ //not a valid texture bundle
		return;
	}
	if (hdr.numTex <= 0 || hdr.ofsTexOfs <= 0 || hdr.ofsTexOfs >= dataSize ||
		hdr.ofsTexSizes <= 0 || hdr.ofsTexSizes >= dataSize)
	{
		return;
	}

	int *tofs = (int *)(data+hdr.ofsTexOfs);
	int *tsizes = (int *)(data+hdr.ofsTexSizes);
	for (int i = 0; i < hdr.numTex; i++)
	{
		char fname[8192];
		rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
		char nameStr[MAX_NOESIS_PATH];
		sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayotex%03i", rapi->Noesis_GetOption("texpre"), i);
		strcat_s(fname, MAX_NOESIS_PATH, nameStr);

		int ofs = tofs[i];
		int size = tsizes[i];
		LITTLE_BIG_SWAP(ofs);
		LITTLE_BIG_SWAP(size);
		if (ofs < 0 || ofs > dataSize)
		{
			continue;
		}
		BYTE *texData = data+ofs;
		BYTE *pix;
		int globalIdx = -1;
		wtbTexHdr_t tex;
		if (hdr.texIdxOffset)
		{ //global id's (probably generated as checksums)
			int *ip = (int  *)(data+hdr.texIdxOffset+sizeof(int)*i);
			globalIdx = *ip;
			LITTLE_BIG_SWAP(globalIdx);
		}
		if (hdr.texInfoOffset)
		{ //texture info is contiguous in its own section
			wtbTexHdr_t *thdr = (wtbTexHdr_t  *)(data+hdr.texInfoOffset+sizeof(wtbTexHdr_t)*i);
			tex = *thdr;
			pix = texData;
		}
		else
		{
			tex = *((wtbTexHdr_t *)texData);
			pix = texData + sizeof(wtbTexHdr_t);
		}
		LITTLE_BIG_SWAP(tex.unknownJ);
		LITTLE_BIG_SWAP(tex.unknownO);
		LITTLE_BIG_SWAP(tex.unknownP);
		LITTLE_BIG_SWAP(tex.unknownQ);
		LITTLE_BIG_SWAP(tex.texFmt);
		LITTLE_BIG_SWAP(tex.heightBits);
		LITTLE_BIG_SWAP(tex.widthBits);
		if (tex.texFmt == 0)
		{
			noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, false);
			textures.Append(nt);
			continue;
		}
		int width = (((tex.widthBits>>5) & 127)+1)<<5;
		int height = ((tex.heightBits & 1023)+1) << 3;

		bool endianSwap = true;
		bool untile = !!(tex.unknownJ & 32768);
		bool uncompressed = false;
		bool channelSwiz = false;
		int dxtFmt = NOESISTEX_RGBA32;
		int texFlags = 0;
		if (tex.unknownJ > 0)
		{ //just a guess
			texFlags |= NTEXFLAG_SEGMENTED;
		}
		switch (tex.texFmt)
		{
		case 82:
			dxtFmt = NOESISTEX_DXT1;
			break;
		case 83:
			dxtFmt = NOESISTEX_DXT3;
			break;
		case 84:
			dxtFmt = NOESISTEX_DXT5;
			break;
		case 134:
			dxtFmt = NOESISTEX_RGBA32;
			uncompressed = true;
			channelSwiz = true;
			break;
		default:
			assert(0);
			rapi->LogOutput("WARNING: Unknown texture format %i.\n", tex.texFmt);
			dxtFmt = NOESISTEX_DXT1;
			break;
		}
		int mipSize;
		BYTE *untiledMip;
		if (uncompressed)
		{
			int bytesPerPixel = (dxtFmt == NOESISTEX_RGB24) ? 3 : 4;
			mipSize = (width*height)*bytesPerPixel;
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageRAW(untiledMip, pix, mipSize, width, height, bytesPerPixel);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = bytesPerPixel;
				for (int i = 0; i < mipSize-swapCount; i += swapCount)
				{
					LittleBigSwap(untiledMip+i, swapCount);
				}
			}
			if (channelSwiz)
			{
				int swapCount = 4;
				for (int i = 0; i < mipSize-swapCount; i += swapCount)
				{
					BYTE *p = untiledMip+i;
					BYTE t = p[0];
					p[0] = p[2];
					p[2] = t;
					p[3] = 255;
					//if i knew which textures were normal maps, they could be made consistent with this
					//rapi->Noesis_SwizzleNormalPix(p, false, true, false);
				}
			}
		}
		else
		{
			mipSize = (dxtFmt == NOESISTEX_DXT1) ? (width*height)/2 : (width*height);
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageDXT(untiledMip, pix, mipSize, width, height, (dxtFmt == NOESISTEX_DXT1) ? 8 : 16);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = 2;
				for (int j = 0; j < mipSize-swapCount; j += swapCount)
				{
					LittleBigSwap(untiledMip+j, swapCount);
				}
			}
		}
		noesisTex_t *nt = rapi->Noesis_TextureAlloc(fname, width, height, untiledMip, dxtFmt);
		nt->flags |= texFlags;
		nt->shouldFreeData = false; //because the untiledMip data is pool-allocated, it does not need to be freed
		nt->globalIdx = globalIdx;
		textures.Append(nt);
	}

	//insert a flat normal map placeholder
	char fname[MAX_NOESIS_PATH];
	rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
	char nameStr[MAX_NOESIS_PATH];
	sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayoflatnormal", rapi->Noesis_GetOption("texpre"));
	strcat_s(fname, MAX_NOESIS_PATH, nameStr);
	noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, true);
	textures.Append(nt);
}*/
template <>
static void Model_Bayo_LoadTextures<false, NIER_AUTOMATA>(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi) {
	const bool big = true;
	int dataSize = texFiles[0]->dataSize;
	BYTE * data = texFiles[0]->data;
	int dataSize2 = texFiles[1]->dataSize;
	BYTE * data2 = texFiles[1]->data;
	char texName[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(texName, texFiles[0]->name);
	if (dataSize < sizeof(bayoWTAHdr_t))
	{
		return;
	}
	bayoWTBHdr<false> hdr((bayoWTBHdr_t *)data);
	if (memcmp(hdr.id, "WTB\0", 4))
	{ //not a valid texture bundle
		return;
	}
	if (hdr.numTex <= 0 || hdr.ofsTexOfs <= 0 || hdr.ofsTexOfs >= dataSize ||
		hdr.ofsTexSizes <= 0 || hdr.ofsTexSizes >= dataSize)
	{
		return;
	}
	DBGLOG("found valid texture header file, containing %d textures\n", hdr.numTex);
	int *tofs = (int *)(data + hdr.ofsTexOfs);
	int *tsizes = (int *)(data + hdr.ofsTexSizes);
	int *tflags = (int *)(data + hdr.ofsTexFlags);
	int *tidx = (int *)(data + hdr.texIdxOffset);
	for (int i = 0; i < hdr.numTex; i++)
	{
		char fname[8192];
		rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
		char nameStr[MAX_NOESIS_PATH];
		sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%s%0x", rapi->Noesis_GetOption("texpre"), tidx[i]);
		strcat_s(fname, MAX_NOESIS_PATH, nameStr);

		int ofs = tofs[i];
		int size = tsizes[i];
		if (ofs < 0 || ofs > dataSize2)
		{
			continue;
		}
		BYTE *texData = data2 + ofs;
		BYTE *pix;
		int globalIdx = -1;
		ddsTexHdr_t tex;
		globalIdx = tidx[i];
		tex = *((ddsTexHdr_t *)texData);
		pix = texData + sizeof(ddsTexHdr_t);
		if (memcmp(tex.id, "DDS ", 4))
		{
			noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, false);
			textures.Append(nt);
			continue;
		}
		int width = tex.width;
		int height = tex.height;

		bool endianSwap = false;
		bool untile = false;//!!(tex.unknownJ & 32768);
		bool uncompressed = false;
		bool channelSwiz = false;
		int dxtFmt = NOESISTEX_RGBA32;
		int texFlags = 0;
		/*
		if (tex.unknownJ > 0)
		{ //just a guess
		texFlags |= NTEXFLAG_SEGMENTED;
		}
		*/
		DBGLOG("\t%03d (idx %x, flags %x): ", i, globalIdx, tflags[i]);
		if (tex.pixelFormat.flags & 0x4) { //DDPF_FOURCC
			DBGLOG("%4s\n", (char *)&(tex.pixelFormat.id));
			switch (tex.pixelFormat.id)
			{
			case 0x31545844: //"DXT1"
				dxtFmt = NOESISTEX_DXT1;
				break;
			case 0x33545844: //"DXT3"
				dxtFmt = NOESISTEX_DXT3;
				break;
			case 0x34545844: //"DXT4"
			case 0x35545844: //"DXT5"
				dxtFmt = NOESISTEX_DXT5;
				break;
				/*		case 134:
				dxtFmt = NOESISTEX_RGBA32;
				uncompressed = true;
				channelSwiz = true;
				break;*/
			default:
				assert(0);
				rapi->LogOutput("WARNING: Unknown texture format %x.\n", tex.pixelFormat.id);
				dxtFmt = NOESISTEX_DXT1;
				break;
			}
		}
		else if (tex.pixelFormat.flags & 0x41) {
			DBGLOG("RGBA\n");
			dxtFmt = NOESISTEX_RGBA32;
			uncompressed = true;
			channelSwiz = true;
		}
		else {
			assert(0);
			rapi->LogOutput("WARNING: Unknown texture format %x.\n", tex.pixelFormat.id);
			dxtFmt = NOESISTEX_DXT1;
		}
		int mipSize;
		BYTE *untiledMip;
		if (uncompressed)
		{
			int bytesPerPixel = (dxtFmt == NOESISTEX_RGB24) ? 3 : 4;
			mipSize = (width*height)*bytesPerPixel;
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageRAW(untiledMip, pix, mipSize, width, height, bytesPerPixel);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = bytesPerPixel;
				for (int i = 0; i < mipSize - swapCount; i += swapCount)
				{
					LittleBigSwap(untiledMip + i, swapCount);
				}
			}
			if (channelSwiz)
			{
				int swapCount = 4;
				for (int i = 0; i < mipSize - swapCount; i += swapCount)
				{
					BYTE *p = untiledMip + i;
					BYTE t = p[0];
					p[0] = p[2];
					p[2] = t;
					p[3] = 255;
					//if i knew which textures were normal maps, they could be made consistent with this
					//rapi->Noesis_SwizzleNormalPix(p, false, true, false);
				}
			}
		}
		else
		{
			mipSize = (dxtFmt == NOESISTEX_DXT1) ? (width*height) / 2 : (width*height);
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageDXT(untiledMip, pix, mipSize, width, height, (dxtFmt == NOESISTEX_DXT1) ? 8 : 16);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = 2;
				for (int j = 0; j < mipSize - swapCount; j += swapCount)
				{
					LittleBigSwap(untiledMip + j, swapCount);
				}
			}
		}
		noesisTex_t *nt = rapi->Noesis_TextureAlloc(fname, width, height, untiledMip, dxtFmt);
		nt->flags |= texFlags;
		nt->shouldFreeData = false; //because the untiledMip data is pool-allocated, it does not need to be freed
		nt->globalIdx = globalIdx;
		textures.Append(nt);
	}
	//insert a flat normal map placeholder
	char fname[MAX_NOESIS_PATH];
	rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
	char nameStr[MAX_NOESIS_PATH];
	sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayoflatnormal", rapi->Noesis_GetOption("texpre"));
	strcat_s(fname, MAX_NOESIS_PATH, nameStr);
	noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, true);
	textures.Append(nt);
}
template <>
static void Model_Bayo_LoadTextures<false, BAYONETTA>(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi)
{
	int dataSize = texFiles[0]->dataSize;
	BYTE * data = texFiles[0]->data;
	if (dataSize < sizeof(bayoWTBHdr_t))
	{
		return;
	}
	bayoWTBHdr<false> hdr((bayoWTBHdr_t *)data);
	if (memcmp(hdr.id, "WTB\0", 4))
	{ //not a valid texture bundle
		return;
	}
	if (hdr.numTex <= 0 || hdr.ofsTexOfs <= 0 || hdr.ofsTexOfs >= dataSize ||
		hdr.ofsTexSizes <= 0 || hdr.ofsTexSizes >= dataSize)
	{
		return;
	}
	DBGLOG("Found %d textures\n", hdr.numTex);
	int *tofs = (int *)(data+hdr.ofsTexOfs);
	int *tsizes = (int *)(data+hdr.ofsTexSizes);
	int *tflags = (int *)(data+hdr.ofsTexFlags);
	for (int i = 0; i < hdr.numTex; i++)
	{
		char fname[8192];
		rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
		char nameStr[MAX_NOESIS_PATH];
		sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayotex%03i", rapi->Noesis_GetOption("texpre"), i);
		strcat_s(fname, MAX_NOESIS_PATH, nameStr);

		int ofs = tofs[i];
		int size = tsizes[i];
		if (ofs < 0 || ofs > dataSize)
		{
			continue;
		}
		BYTE *texData = data+ofs;
		BYTE *pix;
		int globalIdx = -1;
		ddsTexHdr_t tex;
		if (hdr.texIdxOffset)
		{ //global id's (probably generated as checksums)
			int *ip = (int  *)(data+hdr.texIdxOffset+sizeof(int)*i);
			globalIdx = *ip;
		}
		if (hdr.texInfoOffset)
		{ //texture info is contiguous in its own section
			ddsTexHdr_t *thdr = (ddsTexHdr_t  *)(data+hdr.texInfoOffset+sizeof(ddsTexHdr_t)*i);
			tex = *thdr;
			pix = texData;
		}
		else
		{
			tex = *((ddsTexHdr_t *)texData);
			pix = texData + sizeof(ddsTexHdr_t);
		}
		if (memcmp(tex.id, "DDS ", 4))
		{
			noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, false);
			textures.Append(nt);
			continue;
		}
		int width = tex.width;
		int height = tex.height;

		bool endianSwap = false;
		bool untile = false;//!!(tex.unknownJ & 32768);
		bool uncompressed = false;
		bool channelSwiz = false;
		int dxtFmt = NOESISTEX_RGBA32;
		int texFlags = 0;
/*
		if (tex.unknownJ > 0)
		{ //just a guess
			texFlags |= NTEXFLAG_SEGMENTED;
		}
		*/
		DBGLOG("\t%03d (idx %x, flags %x): ", i, globalIdx, tflags[i]);
		if (tex.pixelFormat.flags & 0x4) { //DDPF_FOURCC
			DBGLOG("%4s\n", (char *)&(tex.pixelFormat.id));
			switch (tex.pixelFormat.id)
			{
			case 0x31545844: //"DXT1"
				dxtFmt = NOESISTEX_DXT1;
				break;
			case 0x33545844: //"DXT3"
				dxtFmt = NOESISTEX_DXT3;
				break;
			case 0x34545844: //"DXT4"
			case 0x35545844: //"DXT5"
				dxtFmt = NOESISTEX_DXT5;
				break;
/*		case 134:
			dxtFmt = NOESISTEX_RGBA32;
			uncompressed = true;
			channelSwiz = true;
			break;*/
			default:
				assert(0);
				rapi->LogOutput("WARNING: Unknown texture format %x.\n", tex.pixelFormat.id);
				dxtFmt = NOESISTEX_DXT1;
				break;
			}
		} else if (tex.pixelFormat.flags & 0x41) {
			DBGLOG("RGBA\n");
			dxtFmt = NOESISTEX_RGBA32;
			uncompressed = true;
			channelSwiz = true;
		} else {
			assert(0);
			rapi->LogOutput("WARNING: Unknown texture format %x.\n", tex.pixelFormat.id);
			dxtFmt = NOESISTEX_DXT1;
		}
		int mipSize;
		BYTE *untiledMip;
		if (uncompressed)
		{
			int bytesPerPixel = (dxtFmt == NOESISTEX_RGB24) ? 3 : 4;
			mipSize = (width*height)*bytesPerPixel;
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageRAW(untiledMip, pix, mipSize, width, height, bytesPerPixel);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = bytesPerPixel;
				for (int i = 0; i < mipSize-swapCount; i += swapCount)
				{
					LittleBigSwap(untiledMip+i, swapCount);
				}
			}
			if (channelSwiz)
			{
				int swapCount = 4;
				for (int i = 0; i < mipSize-swapCount; i += swapCount)
				{
					BYTE *p = untiledMip+i;
					BYTE t = p[0];
					p[0] = p[2];
					p[2] = t;
					p[3] = 255;
					//if i knew which textures were normal maps, they could be made consistent with this
					//rapi->Noesis_SwizzleNormalPix(p, false, true, false);
				}
			}
		}
		else
		{
			mipSize = (dxtFmt == NOESISTEX_DXT1) ? (width*height)/2 : (width*height);
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageDXT(untiledMip, pix, mipSize, width, height, (dxtFmt == NOESISTEX_DXT1) ? 8 : 16);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = 2;
				for (int j = 0; j < mipSize-swapCount; j += swapCount)
				{
					LittleBigSwap(untiledMip+j, swapCount);
				}
			}
		}
		noesisTex_t *nt = rapi->Noesis_TextureAlloc(fname, width, height, untiledMip, dxtFmt);
		nt->flags |= texFlags;
		nt->shouldFreeData = false; //because the untiledMip data is pool-allocated, it does not need to be freed
		nt->globalIdx = globalIdx;
		textures.Append(nt);
	}

	//insert a flat normal map placeholder
	char fname[MAX_NOESIS_PATH];
	rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
	char nameStr[MAX_NOESIS_PATH];
	sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayoflatnormal", rapi->Noesis_GetOption("texpre"));
	strcat_s(fname, MAX_NOESIS_PATH, nameStr);
	noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, true);
	textures.Append(nt);
}
template <>
static void Model_Bayo_LoadTextures<false, VANQUISH>(CArrayList<noesisTex_t *> &textures, CArrayList<bayoDatFile_t *> &texFiles, noeRAPI_t *rapi) {
	int dataSize = texFiles[0]->dataSize;
	BYTE * data = texFiles[0]->data;
	if (dataSize < sizeof(bayoWTBHdr_t))
	{
		return;
	}
	bayoWTBHdr<false> hdr((bayoWTBHdr_t *)data);
	if (memcmp(hdr.id, "WTB\0", 4))
	{ //not a valid texture bundle
		return;
	}
	if (hdr.numTex <= 0 || hdr.ofsTexOfs <= 0 || hdr.ofsTexOfs >= dataSize ||
		hdr.ofsTexSizes <= 0 || hdr.ofsTexSizes >= dataSize)
	{
		return;
	}
	DBGLOG("Found %d textures\n", hdr.numTex);
	int *tofs = (int *)(data + hdr.ofsTexOfs);
	int *tsizes = (int *)(data + hdr.ofsTexSizes);
	int *tflags = (int *)(data + hdr.ofsTexFlags);
	for (int i = 0; i < hdr.numTex; i++)
	{
		char fname[8192];
		rapi->Noesis_GetDirForFilePath(fname, rapi->Noesis_GetOutputName());
		char nameStr[MAX_NOESIS_PATH];
		sprintf_s(nameStr, MAX_NOESIS_PATH, ".\\%sbayotex%03i", rapi->Noesis_GetOption("texpre"), i);
		strcat_s(fname, MAX_NOESIS_PATH, nameStr);

		int ofs = tofs[i];
		int size = tsizes[i];
		if (ofs < 0 || ofs > dataSize)
		{
			continue;
		}
		BYTE *texData = data + ofs;
		BYTE *pix;
		int globalIdx = -1;
		ddsTexHdr_t tex;
		if (hdr.texIdxOffset)
		{ //global id's (probably generated as checksums)
			int *ip = (int  *)(data + hdr.texIdxOffset + sizeof(int)*i);
			globalIdx = *ip;
		}
		if (hdr.texInfoOffset)
		{ //texture info is contiguous in its own section
			ddsTexHdr_t *thdr = (ddsTexHdr_t  *)(data + hdr.texInfoOffset + sizeof(ddsTexHdr_t)*i);
			tex = *thdr;
			pix = texData;
		}
		else
		{
			tex = *((ddsTexHdr_t *)texData);
			pix = texData + sizeof(ddsTexHdr_t);
		}
		if (memcmp(tex.id, "DDS ", 4))
		{
			noesisTex_t *nt = rapi->Noesis_AllocPlaceholderTex(fname, 32, 32, false);
			textures.Append(nt);
			continue;
		}
		int width = tex.width;
		int height = tex.height;

		bool endianSwap = false;
		bool untile = false;//!!(tex.unknownJ & 32768);
		bool uncompressed = false;
		bool channelSwiz = false;
		int dxtFmt = NOESISTEX_RGBA32;
		int texFlags = 0;
		/*
		if (tex.unknownJ > 0)
		{ //just a guess
		texFlags |= NTEXFLAG_SEGMENTED;
		}
		*/
		DBGLOG("\t%03d (idx %x, flags %x): ", i, globalIdx, tflags[i]);
		if (tex.pixelFormat.flags & 0x4) { //DDPF_FOURCC
			DBGLOG("%4s\n", (char *)&(tex.pixelFormat.id));
			switch (tex.pixelFormat.id)
			{
			case 0x31545844: //"DXT1"
				dxtFmt = NOESISTEX_DXT1;
				break;
			case 0x33545844: //"DXT3"
				dxtFmt = NOESISTEX_DXT3;
				break;
			case 0x34545844: //"DXT4"
			case 0x35545844: //"DXT5"
				dxtFmt = NOESISTEX_DXT5;
				break;
				/*		case 134:
				dxtFmt = NOESISTEX_RGBA32;
				uncompressed = true;
				channelSwiz = true;
				break;*/
			default:
				assert(0);
				rapi->LogOutput("WARNING: Unknown texture format %x.\n", tex.pixelFormat.id);
				dxtFmt = NOESISTEX_DXT1;
				break;
			}
		}
		else if (tex.pixelFormat.flags & 0x41) {
			DBGLOG("RGBA\n");
			dxtFmt = NOESISTEX_RGBA32;
			uncompressed = true;
			channelSwiz = true;
		}
		else {
			assert(0);
			rapi->LogOutput("WARNING: Unknown texture format %x.\n", tex.pixelFormat.id);
			dxtFmt = NOESISTEX_DXT1;
		}
		int mipSize;
		BYTE *untiledMip;
		if (uncompressed)
		{
			int bytesPerPixel = (dxtFmt == NOESISTEX_RGB24) ? 3 : 4;
			mipSize = (width*height)*bytesPerPixel;
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageRAW(untiledMip, pix, mipSize, width, height, bytesPerPixel);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = bytesPerPixel;
				for (int i = 0; i < mipSize - swapCount; i += swapCount)
				{
					LittleBigSwap(untiledMip + i, swapCount);
				}
			}
			if (channelSwiz)
			{
				int swapCount = 4;
				for (int i = 0; i < mipSize - swapCount; i += swapCount)
				{
					BYTE *p = untiledMip + i;
					BYTE t = p[0];
					p[0] = p[2];
					p[2] = t;
					p[3] = 255;
					//if i knew which textures were normal maps, they could be made consistent with this
					//rapi->Noesis_SwizzleNormalPix(p, false, true, false);
				}
			}
		}
		else
		{
			mipSize = (dxtFmt == NOESISTEX_DXT1) ? (width*height) / 2 : (width*height);
			untiledMip = (BYTE *)rapi->Noesis_PooledAlloc(mipSize);
			if (untile)
			{
				rapi->Noesis_UntileImageDXT(untiledMip, pix, mipSize, width, height, (dxtFmt == NOESISTEX_DXT1) ? 8 : 16);
			}
			else
			{
				memcpy(untiledMip, pix, mipSize);
			}
			if (endianSwap)
			{
				int swapCount = 2;
				for (int j = 0; j < mipSize - swapCount; j += swapCount)
				{
					LittleBigSwap(untiledMip + j, swapCount);
				}
			}
		}
		noesisTex_t *nt = rapi->Noesis_TextureAlloc(fname, width, height, untiledMip, dxtFmt);
		nt->flags |= texFlags;
		nt->shouldFreeData = false; //because the untiledMip data is pool-allocated, it does not need to be freed
		nt->globalIdx = globalIdx;
		textures.Append(nt);
	}
}
//decode motion index (simpler thanks to Alquazar(zenhax))
template <bool big>
static inline short int Model_Bayo_DecodeMotionIndex(const short int *table, const short int boneIndex) {
	short int index = table[(boneIndex >> 8) & 0xf];
	if (big) LITTLE_BIG_SWAP(index);
	if ( index != -1 ) {
		index = table[((boneIndex >> 4) & 0xf) + index];
		if (big) LITTLE_BIG_SWAP(index);
		if ( index != -1 ) {
			index = table[(boneIndex & 0xf) + index];
			if (big) LITTLE_BIG_SWAP(index);
			return index;
		}
	}
	return 0x0fff;
}
//initialize motion matrix
template <bool big, game_t game>
static void Model_Bayo_InitMotions(modelMatrix_t * &matrixes, float * &tmpValues, float * &tmpCumulScale, modelBone_t *bones, const int boneNumber, const short int frameCount, noeRAPI_t *rapi, void * extraBoneInfo) {
	const int maxCoeffs = 16;
	matrixes = (modelMatrix_t *)rapi->Noesis_UnpooledAlloc(sizeof(modelMatrix_t) * (boneNumber + 1) * frameCount);
	tmpValues = (float *)rapi->Noesis_UnpooledAlloc(sizeof(float) * (boneNumber + 1) * frameCount * maxCoeffs);
	tmpCumulScale = (float *)rapi->Noesis_UnpooledAlloc(sizeof(float) * (boneNumber + 1) * frameCount * 3);
	memset(tmpValues, 0, sizeof(float) * (boneNumber + 1) * frameCount * maxCoeffs);

	for (int i = 0; i < boneNumber; i++) {
		float translate[3] = { 0.0, 0.0, 0.0 };
		float rotate[3] = { 0.0, 0.0, 0.0 };
		float scale[3] = { 1.0, 1.0, 1.0 };
		if (game == NIER_AUTOMATA) {
			nierBone<big> nBone((nierBone_t *)((BYTE*)extraBoneInfo + i * sizeof(nierBone_t)));
			translate[0] = nBone.localPosition.x;
			translate[1] = nBone.localPosition.y;
			translate[2] = nBone.localPosition.z;
			rotate[0] = nBone.localRotation.x;
			rotate[1] = nBone.localRotation.y;
			rotate[2] = nBone.localRotation.z;
			scale[0] = nBone.localScale.x;
			scale[1] = nBone.localScale.y;
			scale[2] = nBone.localScale.z;
			if (big) {
				for (int k = 0; k < 3; k++) {
					LITTLE_BIG_SWAP(translate[k]);
					LITTLE_BIG_SWAP(rotate[k]);
					LITTLE_BIG_SWAP(scale[k]);
				}
			}
		}
		else {
			float *relativePos = (float *)extraBoneInfo;
			for (int k = 0; k < 3; k++) {
				translate[k] = relativePos[k + i * 3];
				if (big) {
					LITTLE_BIG_SWAP(translate[k]);
				}
			}
		}
		for( int j = 0; j < frameCount; j++) {
			matrixes[i + j * (boneNumber + 1)] = g_identityMatrix;
			for( int k = 0; k < 3; k++) {
				tmpValues[j + k * frameCount + i *  frameCount * maxCoeffs] = translate[k];
			}
			for (int k = 3, l = 0; k < 6; k++, l++) {
				tmpValues[j + k * frameCount + i * frameCount * maxCoeffs] = rotate[l];
			}
			for( int k = 7, l = 0; k < 10; k++, l++) {
				tmpValues[j + k * frameCount + i *  frameCount * maxCoeffs] = scale[l];
			}
			for (int k = 0; k < 3; k++) {
				tmpCumulScale[j + k * frameCount + i * frameCount * 3] = 1.0f;
			}
			//g_mfn->Math_TranslateMatrix(&matrixes[i + j*bone_number], bones[i].mat.o);
			//float zero[3] = {0.0f, 0.0f, 0.0f};
			//g_mfn->Math_RotationMatrix(0.0, 0, &matrixes[i + j*bone_number]);
		}
	}
	// bone -1
	for (int j = 0; j < frameCount; j++) {
		matrixes[boneNumber + j * (boneNumber + 1)] = g_identityMatrix;
		for (int k = 0; k < 3; k++) {
			tmpCumulScale[j + k * frameCount + boneNumber * frameCount * 3] = 1.0f;
		}
		for (int k = 7, l = 0; k < 10; k++, l++) {
			tmpValues[j + k * frameCount + boneNumber * frameCount * maxCoeffs] = 1.0f;
		}
	}
}
template <bool big>
static float Model_Bayo_Interpolate2EXP_Value(float value, BYTE *interpol, short int numPoints) {
	BYTE *data = interpol;
	float outValue = 0.0;
	if (numPoints == 1) {
		bayoEXPInterpolationPoint2<big> point((bayoEXPInterpolationPoint2_t *)(data));
		outValue = point.value;
		return outValue;
	}
	for (int i = 0; i < numPoints - 1; i++) {
		bayoEXPInterpolationPoint2<big> leftPoint((bayoEXPInterpolationPoint2_t *)(data + i * sizeof(bayoEXPInterpolationPoint2_t)));
		bayoEXPInterpolationPoint2<big> rightPoint((bayoEXPInterpolationPoint2_t *)(data + (i + 1) * sizeof(bayoEXPInterpolationPoint2_t)));
		if (value <= leftPoint.value) {
			outValue = leftPoint.value;
			return outValue;
		}
		if (leftPoint.value < value && rightPoint.value > value) {
			float p0, p1, m0, m1;
			float t;
			p0 = leftPoint.p;
			p1 = rightPoint.p;
			m0 = leftPoint.m1;
			m1 = rightPoint.m0;
			t = (value - leftPoint.value) / (rightPoint.value - leftPoint.value);
			outValue = (2 * t*t*t - 3 * t*t + 1)*p0 + (t*t*t - 2 * t*t + t)*m0 + (-2 * t*t*t + 3 * t*t)*p1 + (t*t*t - t * t)*m1;
			return outValue;
		}
		if (value >= rightPoint.value) {
			outValue = rightPoint.value;
		}
	}
	return outValue;
}
template <bool big>
static float Model_Bayo_Interpolate4EXP_Value(float value, BYTE *interpol, short int numPoints) {
	BYTE *data = interpol;
	bayoEXPInterpolationData4<big> interpolData((bayoEXPInterpolationData4_t *)data);
	float outValue = 0.0;
	data += sizeof(bayoEXPInterpolationData4_t);
	if (numPoints == 1) {
		bayoEXPInterpolationPoint4<big> point((bayoEXPInterpolationPoint4_t *)(data));
		outValue = interpolData.p + point.cp * interpolData.dp;
		return outValue;
	}
	for (int i = 0; i < numPoints - 1; i++) {
		bayoEXPInterpolationPoint4<big> leftPoint((bayoEXPInterpolationPoint4_t *)(data + i * sizeof(bayoEXPInterpolationPoint4_t)));
		bayoEXPInterpolationPoint4<big> rightPoint((bayoEXPInterpolationPoint4_t *)(data + (i + 1) * sizeof(bayoEXPInterpolationPoint4_t)));
		if (value <= leftPoint.v) {
			outValue = interpolData.p + leftPoint.cp * interpolData.dp;
			return outValue;
		}
		if (leftPoint.v < value && rightPoint.v > value) {
			float p0, p1, m0, m1;
			float t;
			p0 = interpolData.p + leftPoint.cp * interpolData.dp;
			p1 = interpolData.p + rightPoint.cp * interpolData.dp;
			m0 = interpolData.m1 + leftPoint.cm1 * interpolData.dm1;
			m1 = interpolData.m0 + rightPoint.cm0 * interpolData.dm0;
			t = (value - leftPoint.v) / (rightPoint.v - leftPoint.v);
			outValue = (2 * t*t*t - 3 * t*t + 1)*p0 + (t*t*t - 2 * t*t + t)*m0 + (-2 * t*t*t + 3 * t*t)*p1 + (t*t*t - t * t)*m1;
			return outValue;
		}
		if (value >= rightPoint.v) {
			outValue = interpolData.p + rightPoint.cp * interpolData.dp;
		}
	}
	return outValue;
}
template <bool big>
static float Model_Bayo_Interpolate6EXP_Value(float value, BYTE *interpol, short int numPoints) {
	BYTE *data = interpol;
	bayoEXPInterpolationData6<big> interpolData((bayoEXPInterpolationData6_t *)data);
	float p = C.decompress(interpolData.p, big);
	float dp = C.decompress(interpolData.dp, big);
	float m_1 = C.decompress(interpolData.m1, big);
	float dm_1 = C.decompress(interpolData.dm1, big);
	float m_0 = C.decompress(interpolData.m0, big);
	float dm_0 = C.decompress(interpolData.dm0, big);
	float outValue = 0.0;
	float pointValue = 0.0;
	data += sizeof(bayoEXPInterpolationData6_t);
	if (numPoints == 1) {
		bayoEXPInterpolationPoint6<big> point((bayoEXPInterpolationPoint6_t *)(data));
		outValue = p + point.cp * dp;
		return outValue;
	}
	for (int i = 0; i < numPoints - 1; i++) {
		bayoEXPInterpolationPoint6<big> leftPoint((bayoEXPInterpolationPoint6_t *)(data + i * sizeof(bayoEXPInterpolationPoint6_t)));
		bayoEXPInterpolationPoint6<big> rightPoint((bayoEXPInterpolationPoint6_t *)(data + (i + 1) * sizeof(bayoEXPInterpolationPoint6_t)));
		pointValue = pointValue + leftPoint.v;
		if (value <= pointValue) {
			outValue = p + leftPoint.cp * dp;
			return outValue;
		}
		if (pointValue < value && (pointValue + rightPoint.v) > value) {
			float p0, p1, m0, m1;
			float t;
			p0 = p + leftPoint.cp * dp;
			p1 = p + rightPoint.cp * dp;
			m0 = m_1 + leftPoint.cm1 * dm_1;
			m1 = m_0 + rightPoint.cm0 * dm_0;
			t = (value - pointValue) / (rightPoint.v);
			outValue = (2 * t*t*t - 3 * t*t + 1)*p0 + (t*t*t - 2 * t*t + t)*m0 + (-2 * t*t*t + 3 * t*t)*p1 + (t*t*t - t * t)*m1;
			return outValue;
		}
		if (value >= (pointValue + rightPoint.v)) {
			outValue = p + rightPoint.cp * dp;
		}
	}
	return outValue;
}
struct expState_s {
	float fArray[4];
	int iArray[4];
	BYTE fCount;
	BYTE iCount;
	short unknown;
};

static void init_expState(struct expState_s &expState) {
	expState.fCount = 0;
	expState.iCount = 0;
	expState.unknown = 0;
	for (int i = 0; i < 4; i++) {
		expState.fArray[i] = 0.0;
		expState.iArray[i] = 0;
	}
}
template <bool big>
static float getExpEntryValue(bayoEXPEntry<big> &entry, int fi, float * tmpValues, const short int frameCount, short int * animBoneTT) {
	static int maxCoeffs = 16;
	if (entry.flags & 0x80000000) {
		short int sourceBone;
		char sourceTrack;
		sourceBone = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, entry.entryUnion.boneTrackInfo.boneIndex);
		sourceTrack = entry.entryUnion.boneTrackInfo.animationTrack;
		if (fi == 0) {
			DBGLOG("\t\t\t\t\tsrcBone: %d\n", sourceBone);
			DBGLOG("\t\t\t\t\tsrcTrack: %d\n", sourceTrack);
		}
		return  tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs];
	}
	else {
		return entry.entryUnion.value;
	}
}

template <bool big>
static float modifyExpEntryValue(struct expState_s &expState, float value, int fi) {
	float res = value;
	float tmp;
	expState.iCount--;
	int flags = expState.iArray[expState.iCount] & 0xFF00;
	if (fi == 0)
		DBGLOG("\t\t\t\t\tflags: %x\n", flags);
	if (flags) {
		if (flags & 0x100) {
			res = fabsf(res);
		}
		else if (flags & 0x200) {
			res = ceilf(res);
		}
		else if (flags & 0x400) {
			res = floorf(res);
		}
		else if (flags & 0x800) {
			res = roundf(res);
		}
	}
	expState.fArray[expState.fCount] = res;
	int fCount = expState.fCount;
	int iCount = expState.iCount;

	expState.fCount = expState.iCount;

	res = expState.fArray[iCount];
	if (fCount > iCount) {
		do {
			flags = expState.iArray[iCount];
			tmp = expState.fArray[iCount + 1];
			if (flags & 1) {
				res = res + tmp;
			}
			else if (flags & 2) {
				res = res - tmp;
			}
			else if (flags & 4) {
				res = res * tmp;
			}
			else if ((flags & 8) && tmp != 0.0f) {
				res = res / tmp;
			}
			iCount++;
		} while (fCount > iCount);
	}
	return res;
}

template <bool big>
static float applyExpEntry(struct expState_s &expState, bayoEXPEntry<big> &entry, int fi, float value, float * tmpValues, const short int frameCount, short int * animBoneTT) {
	float res = 0.0;
	if (fi == 0)
		DBGLOG("\t\t\tentry flags: %x\n", entry.flags);
	if (entry.flags & 0x1FF00) {
		if (fi == 0)
			DBGLOG("\t\t\t\tspecial:\n");
		unsigned int flags = entry.flags;
		unsigned int iValue = 0;
		if (entry.flags & 0x40000 && flags & 0x8000)
		{
			iValue = entry.flags & 0xFF00;
			flags = ~iValue & entry.flags;
		}
		expState.fArray[expState.fCount++] = value;
		expState.iArray[expState.iCount++] = flags;
		res = getExpEntryValue<big>(entry, fi, tmpValues, frameCount, animBoneTT);
		if (fi == 0)
			DBGLOG("\t\t\t\t\tvalue: %f\n", res);
		if (entry.flags & 0x40000) {
			expState.fArray[expState.fCount++] = 0.0;
			expState.iArray[expState.iCount++] = iValue | 1;
		}
	}
	else {
		if (fi == 0)
			DBGLOG("\t\t\t\tnormal:\n");
		res = getExpEntryValue<big>(entry, fi, tmpValues, frameCount, animBoneTT);
		if (fi == 0)
			DBGLOG("\t\t\t\t\tvalue: %f\n", res);
		if (entry.flags & 1) {
			res = res + value;
		}
		else if (entry.flags & 2) {
			res = res - value;
		}
		else if (entry.flags & 4) {
			res = res * value;
		}
		else if ((entry.flags & 8) && value != 0.0f) {
			res = res / value;
		}
		if (entry.flags & 0x20000) {
			if (fi == 0)
				DBGLOG("\t\t\t\tmodifying:\n");
			res = modifyExpEntryValue<big>(expState, res, fi);
			if (entry.flags & 0x80000) {
				res = modifyExpEntryValue<big>(expState, res, fi);
			}
		}
	}
	return res;
}

template <bool big>
static void Model_Bayo1_ApplyEXP(CArrayList<bayoDatFile_t *> & expfile, float * tmpValues, const int bone_number, const short int frameCount, short int * animBoneTT) {
	static int maxCoeffs = 16;
	if (expfile.Num() > 0) {
		DBGLOG("\tapplying: %s\n", expfile[0]->name);
		BYTE *data = expfile[0]->data;
		size_t dataSize = expfile[0]->dataSize;
		if (dataSize < sizeof(bayoEXPHdr_t))
		{
			return;
		}
		bayoEXPHdr<big> hdr((bayoEXPHdr_t *)data);
		for (unsigned int i = 0; i < hdr.numRecords - 1; i++) {
			DBGLOG("\t\trecord: %d\n", i);
			bayoEXPRecord<big> record((bayoEXPRecord_t *)(data + hdr.offsetRecords + i * sizeof(bayoEXPRecord_t)));
			short int targetBone = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, record.boneIndex);
			char targetTrack = record.animationTrack;
			DBGLOG("\t\t\tbone: %d, track: %d\n", (int)targetBone, (int)targetTrack);
			for (int fi = 0; fi < frameCount; fi++) {
				struct expState_s expState;
				init_expState(expState);
				float res = 0.0f;
				for (int j = 0; j < record.entryCount; j++) {
					if (fi == 0)
						DBGLOG("\t\t\t: entry: %d\n", j);
					bayoEXPEntry<big> entry((bayoEXPEntry_t *)(data + record.offset + j * sizeof(bayoEXPEntry_t)));
					res = applyExpEntry<big>(expState, entry, fi, res, tmpValues, frameCount, animBoneTT);
					if (fi == 0)
						DBGLOG("\t\t\tres: %f\n", res);
				}
				if (record.flags & 1) {
					if (fi == 0)
						DBGLOG("\t\t\t: interpolating %d\n", record.interpolationType);
					if (record.interpolationType == 2) {
						res = Model_Bayo_Interpolate2EXP_Value<big>(res, data + record.offsetInterpolation, record.numPoints);
					}
					else if (record.interpolationType == 4) {
						res = Model_Bayo_Interpolate4EXP_Value<big>(res, data + record.offsetInterpolation, record.numPoints);
					}
					else if (record.interpolationType == 6) {
						res = Model_Bayo_Interpolate6EXP_Value<big>(res, data + record.offsetInterpolation, record.numPoints);
					}
					if (fi == 0)
						DBGLOG("\t\t\tres: %f\n", res);
					if (record.flags & 2) {
						for (int j = 0; j < record.interpolationEntryCount; j++) {
							if (fi == 0)
								DBGLOG("\t\t\t: interpolation entry: %d\n", j);
							bayoEXPEntry<big> entry((bayoEXPEntry_t *)(data + record.offset + (record.entryCount + j) * sizeof(bayoEXPEntry_t)));
							res = applyExpEntry<big>(expState, entry, fi, res, tmpValues, frameCount, animBoneTT);
							if (fi == 0)
								DBGLOG("\t\t\tres: %f\n", res);
						}
					}
				}
				if (fi == 0)
					DBGLOG("\t\t\tvalue: %f\n", res);
				tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = res;
			}
		}
	}
}
//apply constrained bone motions
/*
template <bool big>
static void Model_Bayo1_ApplyEXP(CArrayList<bayoDatFile_t *> & expfile, float * tmpValues, const int bone_number, const short int frameCount, short int * animBoneTT) {
	static int maxCoeffs = 16;
	if (expfile.Num() > 0) {
		DBGLOG("\tapplying: %s\n", expfile[0]->name);
		BYTE *data = expfile[0]->data;
		size_t dataSize = expfile[0]->dataSize;
		if (dataSize < sizeof(bayoEXPHdr_t))
		{
			return;
		}
		bayoEXPHdr<big> hdr((bayoEXPHdr_t *)data);
		for (unsigned int i = 0; i < hdr.numRecords; i++) {
			bayoEXPRecord<big> record((bayoEXPRecord_t *)(data + hdr.offsetRecords + i * sizeof(bayoEXPRecord_t)));
			short int targetBone = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, record.boneIndex);
			char targetTrack = record.animationTrack;
			short int sourceBone;
			char sourceTrack;
			DBGLOG("\t\trecord type: %d\n", record.entryType);
			if (record.entryType == 1) {
				bayoEXPEntry1<big> entry1((bayoEXPEntry1_t *)(data + record.offset));
				sourceBone = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, entry1.boneIndex);
				sourceTrack = entry1.animationTrack;
				DBGLOG("\t\ttrgtBone: %d, trgtTrack: %d, srcBone: %d, srcTrack: %d\n", targetBone, targetTrack, sourceBone, sourceTrack);
				for (int fi = 0; fi < frameCount; fi++) {
					tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs];
				}
				if (record.interpolationType == 4) {
					for (int fi = 0; fi < frameCount; fi++) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = Model_Bayo_Interpolate4EXP_Value<big>(tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs], data + record.offsetInterpolation, record.numPoints);
					}
				}
			}
			else if (record.entryType == 2) {
				bayoEXPEntry2<big> entry2((bayoEXPEntry2_t *)(data + record.offset));
				sourceBone = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, entry2.boneIndex);
				sourceTrack = entry2.animationTrack;
				DBGLOG("\t\ttrgtBone: %d, trgtTrack: %d, srcBone: %d, srcTrack: %d, flag: %x, value: %f\n", targetBone, targetTrack, sourceBone, sourceTrack, entry2.coefficient.flag, entry2.coefficient.value);
				for (int fi = 0; fi < frameCount; fi++) {
					if (entry2.coefficient.flag == 0x4) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = entry2.coefficient.value * tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs];
					}
					else if (entry2.coefficient.flag == 0x20004) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = entry2.coefficient.value * abs(tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs]);
					}
					else if (entry2.coefficient.flag == 0x1) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = entry2.coefficient.value + tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs];
					}
				}
			}
			else if (record.entryType == 3) {
				bayoEXPEntry3<big> entry3((bayoEXPEntry3_t *)(data + record.offset));
				sourceBone = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, entry3.boneIndex);
				sourceTrack = entry3.animationTrack;
				DBGLOG("\t\ttrgtBone: %d, trgtTrack: %d, srcBone: %d, srcTrack: %d, flag1: %x, value1: %f, flag2: %x, value2: %f\n", targetBone, targetTrack, sourceBone, sourceTrack, entry3.coefficients[0].flag, entry3.coefficients[0].value, entry3.coefficients[1].flag, entry3.coefficients[1].value);
				for (int fi = 0; fi < frameCount; fi++) {
					if (entry3.coefficients[0].flag == 0x4) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = entry3.coefficients[0].value * tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs];
					}
					else if (entry3.coefficients[0].flag == 0x20004) { //dubious
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = entry3.coefficients[0].value * abs(tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs]);
					}
					else if (entry3.coefficients[0].flag == 0x1) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = entry3.coefficients[0].value + tmpValues[fi + sourceTrack * frameCount + sourceBone * frameCount * maxCoeffs];
					}
					if (entry3.coefficients[1].flag == 0x4) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] *= entry3.coefficients[1].value;
					}
					else if (entry3.coefficients[1].flag == 0x1) {
						tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] += entry3.coefficients[1].value;
					}
				}
			}
		}
	}
}
*/
template <bool big>
static float Model_Bayo2_InterpolateEXP_Value(short int interpolFunction, float value, BYTE *interpol) {
	BYTE *data = interpol + interpolFunction * sizeof(bayo2EXPInterpolationData_t);
	bayo2EXPInterpolationData<big> interpolData((bayo2EXPInterpolationData_t *)data);
	short int numPoints = interpolData.numPoints;
	float outValue = 0.0;
	data += interpolData.offset;
	for (int i = 0; i < numPoints - 1; i++) {
		bayo2EXPInterpolationPoint<big> leftPoint((bayo2EXPInterpolationPoint_t *)(data + i * sizeof(bayo2EXPInterpolationPoint_t)));
		bayo2EXPInterpolationPoint<big> rightPoint((bayo2EXPInterpolationPoint_t *)(data + (i + 1) * sizeof(bayo2EXPInterpolationPoint_t)));
		if (leftPoint.v <= value && rightPoint.v >= value) {
			float p0, p1, m0, m1;
			float t;
			p0 = leftPoint.p;
			p1 = rightPoint.p;
			m0 = leftPoint.m1;
			m1 = rightPoint.m0;
			t = (value - leftPoint.v) / (rightPoint.v - leftPoint.v);
			outValue = (2 * t*t*t - 3 * t*t + 1)*p0 + (t*t*t - 2 * t*t + t)*m0 + (-2 * t*t*t + 3 * t*t)*p1 + (t*t*t - t * t)*m1;
			DBGLOG("{t = %f}", t);
		}
	}
	return outValue;
}
template <bool big>
static float Model_Bayo2_DecodeEXP_Value( float * tmpValues, const int bone_number, const short int frameCount, short int * animBoneTT, BYTE *interpol, BYTE *&values, int &valueCount, int frame) {
	//DBGLOG("\t\tDecoding, remaining: %d\n\t\t\t", valueCount);
	static int maxCoeffs = 16;
	std::stack<float> s;
	char o = -1;
	while( valueCount > 0 ) {
		if (o != -1 && s.size() >= 2) {
			float v2 = s.top();
			s.pop();
			float v1 = s.top();
			s.pop();
			if (o == '+') {
				v1 += v2;
			}
			else if (o == '*') {
				v1 *= v2;
			}
			s.push(v1);
			o = -1;
		}
		bayo2EXPValue<big> v((bayo2EXPValue_t *) values);
		values += sizeof(bayo2EXPValue_t);
		valueCount -= 1;
		switch (v.type) {
		case 0: //terminator
			DBGLOG(" = ");
			return s.top();
			break;
		case 1: //Parenthesis
			DBGLOG("( ");
			s.push( Model_Bayo2_DecodeEXP_Value<big>(tmpValues, bone_number, frameCount, animBoneTT, interpol, values, valueCount, frame) );
			break;
		case 2: //Closing Parenthesis
			DBGLOG(") ");
			return s.top();
			break;
		case 3: //Animation track
			s.push(tmpValues[frame + v.animationTrack * frameCount + Model_Bayo_DecodeMotionIndex<big>(animBoneTT, v.boneIndex) * frameCount * maxCoeffs]);
			DBGLOG("[%d:%d](%f)", Model_Bayo_DecodeMotionIndex<big>(animBoneTT, v.boneIndex), v.animationTrack, s.top());
			break;
		case 4: //Immediate
			DBGLOG("%f ", v.value);
			s.push(v.value);
			break;
		case 5: // Arithmetic
			if (v.boneIndex == 0) {
				o = '+';
			}
			else if (v.boneIndex == 2) {
				o = '*';
			}
			DBGLOG("%c ", o);
			break;
		case 6: // Function call
			DBGLOG("abs( ");
			s.push( abs( Model_Bayo2_DecodeEXP_Value<big>(tmpValues, bone_number, frameCount, animBoneTT, interpol, values, valueCount, frame) ) );
			break;
		case 7: // End function call
			DBGLOG(") ");
			return s.top();
			break;
		case 8: // interpolate
			DBGLOG("Interpolate%d( ", v.boneIndex);
			s.push(Model_Bayo2_InterpolateEXP_Value<big>(v.boneIndex,
				Model_Bayo2_DecodeEXP_Value<big>(tmpValues, bone_number, frameCount, animBoneTT, interpol, values, valueCount, frame),
				interpol)
			);
			break;
		}

	}
	return s.top();
}
template <bool big>
static void Model_Bayo2_ApplyEXP(CArrayList<bayoDatFile_t *> & expfile, float * tmpValues, const int bone_number, const short int frameCount, short int * animBoneTT) {
	static int maxCoeffs = 16;
	if (expfile.Num() > 0) {
		DBGLOG("\tapplying: %s\n", expfile[0]->name);
		BYTE *data = expfile[0]->data;
		size_t dataSize = expfile[0]->dataSize;
		if (dataSize < sizeof(bayo2EXPHdr_t))
		{
			return;
		}
		bayo2EXPHdr<big> hdr((bayo2EXPHdr_t *)data);
		BYTE *interpol = data + hdr.offsetInterpolationData;
		for (int i = 0; i < hdr.numRecords; i++) {
			bayo2EXPRecord<big> record((bayo2EXPRecord_t *)(data + hdr.offsetRecords + i * sizeof(bayo2EXPRecord_t)));
			short int targetBone = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, record.boneIndex);
			char targetTrack = record.animationTrack;
			DBGLOG("\t\tfound bone: %d, track: %d, numValues: %d\n", targetBone, targetTrack, record.valueCount);
			for (int fi = 0; fi < frameCount; fi++) {
				BYTE *vals = data + hdr.offsetRecords + i * sizeof(bayo2EXPRecord_t) + record.offset;
				int count = record.valueCount;
				float value = Model_Bayo2_DecodeEXP_Value<big>(
					tmpValues,
					bone_number,
					frameCount,
					animBoneTT,
					interpol,
					vals,
					count,
					fi);
				DBGLOG("%f\n", value);
				tmpValues[fi + targetTrack * frameCount + targetBone * frameCount * maxCoeffs] = value;
			}
		}
	}
}
template <bool big, game_t game>
inline static void Model_Bayo_ApplyEXP(CArrayList<bayoDatFile_t *> & expfile, float * tmpValues, const int bone_number, const short int frameCount, short int * animBoneTT) {
	if (game == BAYONETTA) {
		Model_Bayo1_ApplyEXP<big>(expfile, tmpValues, bone_number, frameCount, animBoneTT);
	}
	else if (game == BAYONETTA2) {
		Model_Bayo2_ApplyEXP<big>(expfile, tmpValues, bone_number, frameCount, animBoneTT);
	}
}
//apply rotate/translate to model matrix
static void Model_Bayo_ApplyMotions(modelMatrix_t * matrixes, float * tmpValues, float *tmpCumulScale, modelBone_t *bones, const int boneNumber, const short int frameCount) {
	const int maxCoeffs = 16;
	DBGALOG("-------------------------------\n");
	for(int bi = 0; bi < boneNumber + 1; bi++) {
		DBGALOG("bone %d (%d)", bi, bones[bi].index);
		DBGALOG(" parent %d\n", bones[bi].eData.parent ? bones[bi].eData.parent->index : -1);
		//DBGLOG("\ttranslate: %f, %f, %f\n", bones[bi].mat.o[0], bones[bi].mat.o[1], bones[bi].mat.o[2]);
		DBGALOG("\trelative: %f, %f, %f\n",
			bones[bi].mat.o[0] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[0] : 0.0),
			bones[bi].mat.o[1] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[1] : 0.0),
			bones[bi].mat.o[2] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[2] : 0.0));
	}
	DBGALOG("-------------------------------\n");
	for(int bi = 0; bi < boneNumber + 1; bi++) {
		DBGALOG("bone %d\n", bi);

		for( int fi = 0; fi < frameCount; fi++) {
			DBGALOG("\tframe %d\n", fi);
			float translate[3];
			float rotate[3];
			float scale[3];
			//int rotate_order[3] = {2,1,0};
			float rotate_coeff[3] = {-1.0, 1.0, -1.0};
			float translate_coeff[3] = {1.0, 1.0, 1.0};
			float scale_coeff[3] = {1.0, 1.0, 1.0};

			for ( int i = 0; i < 3; i++) {
				translate[i] = translate_coeff[i] * tmpValues[fi + i * frameCount + bi *  frameCount * maxCoeffs];
			}
			for ( int i = 3, j = 0; i < 6; i++, j++) {
				rotate[j] = rotate_coeff[j] * tmpValues[fi + i * frameCount + bi *  frameCount * maxCoeffs];
			}
			for ( int i = 7, j = 0; i < 10; i++, j++) {
				scale[j] = scale_coeff[j] * tmpValues[fi + i * frameCount + bi *  frameCount * maxCoeffs];
			}
			DBGALOG("\t\ttranslate: %f, %f, %f\n", translate[0], translate[1], translate[2]);
			DBGALOG("\t\tref relative: %f, %f, %f\n",
			bones[bi].mat.o[0] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[0] : 0.0),
			bones[bi].mat.o[1] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[1] : 0.0),
			bones[bi].mat.o[2] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[2] : 0.0));

			g_mfn->Math_TranslateMatrix(&matrixes[bi + (boneNumber + 1) * fi], translate);

			if (bones[bi].eData.parent) {
				int pi = bones[bi].eData.parent->index;
				float parentCumulScale[3];
				for (int i = 0; i < 3; i++)
					parentCumulScale[i] = tmpCumulScale[fi + i * frameCount + pi * frameCount * 3];
				for (int i = 0; i < 3; i++)
					scale[i] *= parentCumulScale[i];
				for (int i = 0; i < 3; i++) {
					matrixes[bi + (boneNumber + 1) * fi].x1[i] /= parentCumulScale[i];
					matrixes[bi + (boneNumber + 1) * fi].x2[i] /= parentCumulScale[i];
					matrixes[bi + (boneNumber + 1) * fi].x3[i] /= parentCumulScale[i];
				}
			}
			for (int i = 0; i < 3; i++)
				tmpCumulScale[fi + i * frameCount + bi * frameCount * 3] = scale[i];
			DBGALOG("\t\trotate: %f, %f, %f (order %d)\n", rotate[0], rotate[1], rotate[2], bones[bi].userIndex);
			switch (bones[bi].userIndex) {
			case 0:
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[0], 1.0f, 0.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[1], 0.0f, 1.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[2], 0.0f, 0.0f, 1.0f);
				break;
			case 1:
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[0], 1.0f, 0.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[2], 0.0f, 0.0f, 1.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[1], 0.0f, 1.0f, 0.0f);
				break;
			case 2:
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[1], 0.0f, 1.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[0], 1.0f, 0.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[2], 0.0f, 0.0f, 1.0f);
				break;
			case 3:
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[1], 0.0f, 1.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[2], 0.0f, 0.0f, 1.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[0], 1.0f, 0.0f, 0.0f);
				break;
			case 4:
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[2], 0.0f, 0.0f, 1.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[0], 1.0f, 0.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[1], 0.0f, 1.0f, 0.0f);
				break;
			default:
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[2], 0.0f, 0.0f, 1.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[1], 0.0f, 1.0f, 0.0f);
				g_mfn->Math_RotateMatrix(&matrixes[bi + (boneNumber + 1) * fi], rotate[0], 1.0f, 0.0f, 0.0f);
			}

			DBGALOG("\t\tscale: %f, %f, %f\n", scale[0], scale[1], scale[2]);
			for ( int i = 0; i < 3; i++ ) {
				matrixes[bi + (boneNumber + 1) * fi].x1[i] *= scale[i];
				matrixes[bi + (boneNumber + 1) * fi].x2[i] *= scale[i];
				matrixes[bi + (boneNumber + 1) * fi].x3[i] *= scale[i];
			}
		}
	}
}
template <bool big>
static inline void Model_Bayo_DecodeInterpolateHeader(float * fvals, bayoInterpolHeader4_t *h) {
	for(int j = 0; j < 6; j++) {
		fvals[j] = h->values[j];
		if (big) LITTLE_BIG_SWAP(fvals[j]);
	}
}
static FloatDecompressor C(6, 9, 47);
static FloatDecompressor H(5, 10, 15);
template <bool big>
static inline void Model_Bayo_DecodeInterpolateHeader(float * fvals, bayoInterpolHeader6_t *h) {
	for(int j = 0; j < 6; j++) {
		fvals[j] = C.decompress(h->values[j], big);
	}
}
template <bool big>
static inline void Model_Bayo_DecodeInterpolateHeader(float * fvals, bayoInterpolHeader7_t *h) {
	for(int j = 0; j < 6; j++) {
		fvals[j] = C.decompress(h->values[j], big);
	}
}
template <bool big>
static inline void Model_Bayo_DecodeInterpolateHeader(float * fvals, bayoInterpolHeader8_t *h) {
	for(int j = 0; j < 6; j++) {
		fvals[j] = C.decompress(h->values[j], big);
	}
}
template <bool big, game_e game>
static inline void Model_Bayo_DecodeFrameIndex(short int &firstFrame, short int &lastFrame, short int, bayoInterpolKeyframe4<big> &p_v, bayoInterpolKeyframe4<big> &v) {
	firstFrame = p_v.index;
	lastFrame = v.index;
}
template <bool big, game_e game>
static inline void Model_Bayo_DecodeFrameIndex(short int &firstFrame, short int &lastFrame, short int frameCount, bayoInterpolKeyframe6<big> &p_v, bayoInterpolKeyframe6<big> &v) {
	if (game == BAYONETTA2) {
		firstFrame = p_v.index;
		lastFrame = v.index;
	}else {
		firstFrame = frameCount - 1;
		lastFrame = frameCount - 1 + v.index;
	}
}
template <bool big, game_e game>
static inline void Model_Bayo_DecodeFrameIndex(short int &firstFrame, short int &lastFrame, short int, bayoInterpolKeyframe7<big> &p_v, bayoInterpolKeyframe7<big> &v) {
	firstFrame = p_v.index;
	lastFrame = v.index;
}
template <bool big, game_e game>
static inline void Model_Bayo_DecodeFrameIndex(short int &firstFrame, short int &lastFrame, short int, bayoInterpolKeyframe8<big> &p_v, bayoInterpolKeyframe8<big> &v) {
	firstFrame = p_v.index;
	lastFrame = v.index;
}
template <class T>
static void Model_Bayo_HermitInterpolate(float * tmpValues, float *fvals, const T &p_v, const T &v, short int &frameCount, const short int first_frame, const short int last_frame) {
	float p0, p1, m0, m1;
	p0 = fvals[0] + fvals[1] * p_v.coeffs[0];
	p1 = fvals[0] + fvals[1] * v.coeffs[0];
	m0 = fvals[4] + fvals[5] * p_v.coeffs[2];
	m1 = fvals[2] + fvals[3] * v.coeffs[1];

	for(; frameCount <= last_frame; frameCount++) {
		float t;
		t = (float)(frameCount - first_frame)/(last_frame - first_frame);
		tmpValues[frameCount] = (2*t*t*t - 3*t*t + 1)*p0 + (t*t*t - 2*t*t + t)*m0 + (-2*t*t*t + 3*t*t)*p1 + (t*t*t - t*t)*m1;
		DBGALOG("%f, %d, %f\n\t", t, frameCount, tmpValues[frameCount]);
	}
	DBGALOG("%3d %5d %5d %5d (%+f %+f %+f)\n\t", v.index, v.coeffs[0], v.coeffs[1], v.coeffs[2],
			fvals[0] + fvals[1] * v.coeffs[0],
			fvals[2] + fvals[3] * v.coeffs[1],
			fvals[4] + fvals[5] * v.coeffs[2]);
}
//interpolate motion
template <bool big, game_e game, class T1, class T2, class T3>
static void Model_Bayo_Interpolate(float * tmpValues, BYTE * data, const short int elemNumber, short int length) {
	short int frameCount;
	T1 *h = (T1 *)(data);
	T2 *v_p = (T2 *)(h+1);
	T2 *p_v_p;

	float fvals[6];
	Model_Bayo_DecodeInterpolateHeader<big>(fvals, h);
	DBGALOG("\t");
	for(int j = 0; j < 6; j++)
	{
		DBGALOG("%#g ", fvals[j]);
	}
	DBGALOG("\n\t(");
	for(int j = 0; j < sizeof(*h); j++)
	{
		DBGALOG("%02x ", ((BYTE *)h)[j]);
	}
	DBGALOG(")\n\t");

	T3 t_v(v_p);
	DBGALOG("%3d %5d %5d %5d (%+f %+f %+f)\n\t", t_v.index, t_v.coeffs[0], t_v.coeffs[1], t_v.coeffs[2],
			fvals[0] + fvals[1] * t_v.coeffs[0],
			fvals[2] + fvals[3] * t_v.coeffs[1],
			fvals[4] + fvals[5] * t_v.coeffs[2]);
	frameCount = t_v.index;
	float f = fvals[0] + fvals[1] * t_v.coeffs[0];
	for (int j = 0; j < frameCount; j++) {
		tmpValues[j] = f;
		DBGALOG("%d, %f\n\t", j, f);
	}
	tmpValues[frameCount] = f;
	DBGALOG("%f, %d, %f\n\t", 0.0, frameCount, f);

	p_v_p = v_p;
	v_p++;
	frameCount++;

	for (int j = 1; j < elemNumber; j++, p_v_p++, v_p++)
	{
		T3 p_v(p_v_p);
		T3 v(v_p);
		short int first_frame, last_frame;
		Model_Bayo_DecodeFrameIndex<big, game>(first_frame, last_frame, frameCount, p_v, v);
		Model_Bayo_HermitInterpolate(tmpValues, fvals, p_v, v, frameCount, first_frame, last_frame);
	}
	for (int j = frameCount; j < length; j++) {
		tmpValues[j] = tmpValues[frameCount-1];
		DBGALOG("%d, %f\n\t", j, tmpValues[j]);
	}

	DBGALOG("\n");
}
template <bool big, game_t game>
static void Model_Bayo_CreateRestPoseAnim(CArrayList<noesisAnim_t *> &animList, modelBone_t *bones, int bone_number, noeRAPI_t *rapi, void * extraBoneInfo)
{
	const int frameCount = 1;
	const int maxCoeffs = 16;
	modelMatrix_t * matrixes;
	float * tmp_values;
	float * tmp_cumul_scale;
	Model_Bayo_InitMotions<big,game>(matrixes, tmp_values, tmp_cumul_scale, bones, bone_number, frameCount, rapi, extraBoneInfo);
	for (int bi = 0; bi < bone_number + 1; bi++) {
		for (int fi = 0; fi < frameCount; fi++) {
			// convert to degrees
			tmp_values[fi + 3 * frameCount + bi * frameCount * maxCoeffs] *= g_flRadToDeg;
			tmp_values[fi + 4 * frameCount + bi * frameCount * maxCoeffs] *= g_flRadToDeg;
			tmp_values[fi + 5 * frameCount + bi * frameCount * maxCoeffs] *= g_flRadToDeg;
		}
	}
	Model_Bayo_ApplyMotions(matrixes, tmp_values, tmp_cumul_scale, bones, bone_number, frameCount);
	noesisAnim_t *anim = rapi->rpgAnimFromBonesAndMatsFinish(bones, bone_number+1, matrixes, frameCount, 60);

	anim->filename = rapi->Noesis_PooledString("restpose");
	anim->flags |= NANIMFLAG_FILENAMETOSEQ;
	anim->aseq = rapi->Noesis_AnimSequencesAlloc(1, frameCount);
	anim->aseq->s->startFrame = 0;
	anim->aseq->s->endFrame = frameCount - 1;
	anim->aseq->s->frameRate = 60;
	anim->aseq->s->name = rapi->Noesis_PooledString("restpose");
	if (anim)
	{
		animList.Append(anim);
	}
	rapi->Noesis_UnpooledFree(matrixes);
	rapi->Noesis_UnpooledFree(tmp_values);
	rapi->Noesis_UnpooledFree(tmp_cumul_scale);
}
//loat motion file
template <bool big, game_t game>
static void Model_Bayo_LoadMotions(CArrayList<noesisAnim_t *> &animList, CArrayList<bayoDatFile_t *> &motfiles, CArrayList<bayoDatFile_t *> &expfile, modelBone_t *bones, int bone_number, noeRAPI_t *rapi, short int * animBoneTT, void * extraBoneInfo)
{
  const int maxCoeffs = 16;
  if (motfiles.Num() > 0) {
	  Model_Bayo_CreateRestPoseAnim<big, game>(animList, bones, bone_number, rapi, extraBoneInfo);
  }
  else {
	  return;
  }
  for(int mi = 0; mi < motfiles.Num(); mi++)// motfiles.Num(); mi++)
  {
	DBGLOG("Loading %s\n", motfiles[mi]->name);

	BYTE * data     = motfiles[mi]->data;
	size_t dataSize = motfiles[mi]->dataSize;
	int unknownA;
	int frameCount;
	int ofsMotion;
	int numEntries;
	if (game != BAYONETTA) {
		if (dataSize < sizeof(bayo2MOTHdr_t))
		{
			continue;
		}
		bayo2MOTHdr<big> hdr((bayo2MOTHdr_t *)data);
		if (memcmp(hdr.id, "mot\0", 4))
		{ //not a valid motion file
			continue;
		}
		unknownA = hdr.unknownA;
		frameCount = hdr.frameCount;
		ofsMotion = hdr.ofsMotion;
		numEntries = hdr.numEntries;
	} else {
		if (dataSize < sizeof(bayoMOTHdr_t))
		{
			continue;
		}
		bayoMOTHdr<big> hdr((bayoMOTHdr_t *)data);
		if (memcmp(hdr.id, "mot\0", 4))
		{ //not a valid motion file
			continue;
		}
		unknownA = hdr.unknownA;
		frameCount = hdr.frameCount;
		ofsMotion = hdr.ofsMotion;
		numEntries = hdr.numEntries;
	}

	char fname[MAX_NOESIS_PATH];
	sprintf_s(fname, MAX_NOESIS_PATH, "%s", motfiles[mi]->name);

	modelMatrix_t * matrixes;
	float * tmp_values;
	float * tmp_cumul_scale;
	Model_Bayo_InitMotions<big, game>(matrixes, tmp_values, tmp_cumul_scale, bones, bone_number, frameCount, rapi, extraBoneInfo);

	bayoMotItem_t * items = (bayoMotItem_t*)(data + ofsMotion);
	DBGALOG("unknown flag: 0x%04x, frame count: %d, data offset: 0x%04x, record number: %d\n", unknownA, frameCount, ofsMotion, numEntries);
	for(int i=0; i < numEntries; i++) {
		bayoMotItem<big> it(&items[i]);
		if (game != BAYONETTA) data = (BYTE *)&items[i];
		if( it.boneIndex == 0x7fff) {
			DBGALOG("%5d %3d 0x%02x %3d %3d %+f (0x%08x)\n",it.boneIndex, it.index, it.flag, it.elem_number, it.unknown, it.value.flt, it.value.offset);
			continue;
		} else if ( it.boneIndex >= 0xf60 ) {
			DBGALOG("%5d %3d 0x%02x %3d %3d %+f (0x%08x) special flag 0x2 index\n", it.boneIndex, it.index, it.flag, it.elem_number, it.unknown, it.value.flt, it.value.offset);
		    continue;
		}

		short int boneIndex;
		if (it.boneIndex == -1) {
			boneIndex = bone_number;
		}
		else {
			boneIndex = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, it.boneIndex);
		}
		if( boneIndex == 0x0fff ) {
			DBGALOG("%5d %3d 0x%02x %3d %3d %+f (0x%08x) cannot translate bone\n", it.boneIndex, it.index, it.flag, it.elem_number, it.unknown, it.value.flt, it.value.offset);
			continue;
		}

		//float tmp_values[65536];
		DBGALOG("%5d (%5d) %3d 0x%02x %3d %3d", boneIndex, it.boneIndex, it.index, it.flag, it.elem_number, it.unknown);
		DBGALOG(" %+f (0x%08x)\n", it.value.flt, it.value.offset);
		if ( boneIndex > bone_number ) {
			DBGALOG(" out of bone bounds\n");
			continue;
		}
		if ( it.flag == 1 ) {
			float *fdata = (float *)(data + it.value.offset);
			for(int frame_number=0; frame_number < it.elem_number; frame_number++) {
				float f = fdata[frame_number];
				if (big) LITTLE_BIG_SWAP(f);
				tmp_values[frame_number + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = f;
				DBGALOG("\t%3d %+f\n", frame_number, f);
			}
			float v = tmp_values[it.elem_number - 1 + it.index * frameCount + boneIndex *  frameCount * maxCoeffs];
			for (int frame_number = it.elem_number; frame_number < frameCount; frame_number++) {
				tmp_values[frame_number + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = v;
				DBGALOG("\t%d, %f\n", frame_number, v);
			}
		} else if ( it.flag == 2 ) {
			float *fData = (float *)(data + it.value.offset);
			short unsigned int *suiData = (short unsigned int *)&fData[2];
			float fbase = fData[0];
			float fdelta = fData[1];
			if (big) LITTLE_BIG_SWAP(fbase);
			if (big) LITTLE_BIG_SWAP(fdelta);
			DBGALOG("\t%f %f\n", fbase, fdelta);
			for(int frame_count=0; frame_count < it.elem_number; frame_count++) {
				short unsigned int val;
				float f;
				val = suiData[frame_count];
				if (big) LITTLE_BIG_SWAP(val);
				f = fbase + fdelta*val;
				DBGALOG("\t%d %d %f\n", frame_count, val, f);
				tmp_values[frame_count + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = f;
			}
			float v = tmp_values[it.elem_number - 1 + it.index * frameCount + boneIndex *  frameCount * maxCoeffs];
			for (int j = it.elem_number; j < frameCount; j++) {
				tmp_values[j + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = v;
				DBGALOG("\t%d, %f\n", j, v);
			}
		} else if ( it.flag == 3 ) {
			short unsigned int *suiData = (short unsigned int *)(data + it.value.offset);
			BYTE * bData = (BYTE *)&suiData[2];
			float fbase = C.decompress(suiData[0], big);
			float fdelta = C.decompress(suiData[1], big);
			DBGALOG("\t%f %f\n", fbase, fdelta);
			for(int frame_count=0; frame_count < it.elem_number; frame_count++) {
				BYTE val = bData[frame_count];
				float f = fbase + fdelta*val;
				DBGALOG("\t%d %d %f\n", frame_count, val, f);
				tmp_values[frame_count + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = f;
			}
			float v = tmp_values[it.elem_number - 1 + it.index * frameCount + boneIndex *  frameCount * maxCoeffs];
			for (int j = it.elem_number; j < frameCount; j++) {
				tmp_values[j + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = v;
				DBGALOG("\t%d, %f\n", j, v);
			}
		} else if ( game != BAYONETTA && it.flag == 4 ) {
			bayo2InterpolKeyframe4_t *v_p = (bayo2InterpolKeyframe4_t *)(data + it.value.offset);
			bayo2InterpolKeyframe4_t *p_v_p;
			bayo2InterpolKeyframe4<big> t_v(v_p);
			int frame_count = t_v.index;

			DBGALOG("\t%d %f (%f %f)\n", t_v.index, t_v.p, t_v.m0, t_v.m1);
			for (int j = 0; j < frame_count; j++) {
				tmp_values[j + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = t_v.p;
				DBGALOG("\t%d, %f\n", j, t_v.p);
			}
			tmp_values[frame_count + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = t_v.p;
			DBGALOG("\t%f, %d, %f\n", 0.0, frame_count, t_v.p);
			frame_count++;
			p_v_p = v_p;
			v_p++;
			for(int j = 1; j < it.elem_number; j++, p_v_p++, v_p++)	{
				bayo2InterpolKeyframe4<big> v(v_p);
				bayo2InterpolKeyframe4<big> p_v(p_v_p);
				float p0, p1, m0, m1;
				p0 = p_v.p;
				m0 = p_v.m1;
				p1 = v.p;
				m1 = v.m0;
				for(; frame_count <= v.index; frame_count++) {
					float t;
					float f;
					t = (float)(frame_count - p_v.index)/(v.index - p_v.index);
		            f = (2*t*t*t - 3*t*t + 1)*p0 + (t*t*t - 2*t*t + t)*m0 + (-2*t*t*t + 3*t*t)*p1 + (t*t*t - t*t)*m1;
					tmp_values[frame_count + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = f;
					DBGALOG("\t%f, %d, %f\n", 0.0, frame_count, f);
				}
				DBGALOG("\t%d %f (%f %f)\n", v.index, v.p, v.m0, v.m1);
			}
			float v = tmp_values[frame_count - 1 + it.index * frameCount + boneIndex *  frameCount * maxCoeffs];
			for (int j = frame_count; j < frameCount; j++) {
				tmp_values[j + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = v;
				DBGALOG("\t%d, %f\n", j, v);
			}
			DBGALOG("\n");
		} else if ( it.flag == 4 || (game != BAYONETTA && it.flag == 5) ) {

			Model_Bayo_Interpolate<big, game, bayoInterpolHeader4_t, bayoInterpolKeyframe4_t, bayoInterpolKeyframe4<big>>(tmp_values + it.index * frameCount + boneIndex *  frameCount * maxCoeffs,
									data + it.value.offset,
									it.elem_number, frameCount);
		} else if ( game != BAYONETTA && it.flag == 6 ) {

			Model_Bayo_Interpolate<big, BAYONETTA2, bayoInterpolHeader6_t, bayoInterpolKeyframe6_t, bayoInterpolKeyframe6<big>>(tmp_values + it.index * frameCount + boneIndex *  frameCount * maxCoeffs,
									data + it.value.offset,
									it.elem_number, frameCount);

		} else if ( it.flag == 6 || (game != BAYONETTA && it.flag == 7) ) {

			Model_Bayo_Interpolate<big, BAYONETTA, bayoInterpolHeader6_t, bayoInterpolKeyframe6_t, bayoInterpolKeyframe6<big>>(tmp_values + it.index * frameCount + boneIndex *  frameCount * maxCoeffs,
									data + it.value.offset,
									it.elem_number, frameCount);

		} else if ( it.flag == 7 ) { //diff from 6 because frame delta would be > 255

			Model_Bayo_Interpolate<big, game, bayoInterpolHeader7_t, bayoInterpolKeyframe7_t, bayoInterpolKeyframe7<big>>(tmp_values + it.index * frameCount + boneIndex *  frameCount * maxCoeffs,
									data + it.value.offset,
									it.elem_number, frameCount);

		} else if ( it.flag == 8 ) {

			Model_Bayo_Interpolate<big, game, bayoInterpolHeader8_t, bayoInterpolKeyframe8_t, bayoInterpolKeyframe8<big>>(tmp_values + it.index * frameCount + boneIndex *  frameCount * maxCoeffs,
									data + it.value.offset,
									it.elem_number, frameCount);

		} else if ( it.flag ==  0xff ) {
			continue;
		} else if ( it.flag != 0 ) {
			DBGLOG("WARNING: Unknown motion flag %02x.\n", it.flag);
			assert(0);
			continue;
			rapi->LogOutput("WARNING: Unknown motion flag %02x.\n", it.flag);
		} else {
			for (int j = 0; j < frameCount; j++) {
				tmp_values[j + it.index * frameCount + boneIndex *  frameCount * maxCoeffs] = it.value.flt;
			}
		}

	}

	for (int bi = 0; bi < bone_number + 1; bi++) {
		for (int fi = 0; fi < frameCount; fi++) {
			// convert to degrees
			tmp_values[fi + 3 * frameCount + bi * frameCount * maxCoeffs] *= g_flRadToDeg;
			tmp_values[fi + 4 * frameCount + bi * frameCount * maxCoeffs] *= g_flRadToDeg;
			tmp_values[fi + 5 * frameCount + bi * frameCount * maxCoeffs] *= g_flRadToDeg;
		}
	}

	if (game == BAYONETTA2) {
		for (int bi = 0; bi < bone_number + 1; bi++) {
			for (int fi = 0; fi < frameCount; fi++) {
				// convert to dm: ugly fix for animations, doubtfull to be legit...
				tmp_values[fi + 0 * frameCount + bi * frameCount * maxCoeffs] *= 10.0f;
				tmp_values[fi + 1 * frameCount + bi * frameCount * maxCoeffs] *= 10.0f;
				tmp_values[fi + 2 * frameCount + bi * frameCount * maxCoeffs] *= 10.0f;
			}
		}
	}

	Model_Bayo_ApplyEXP<big, game>(expfile, tmp_values, bone_number, frameCount, animBoneTT);

	if (game == BAYONETTA2) {
		for (int bi = 0; bi < bone_number + 1; bi++) {
			for (int fi = 0; fi < frameCount; fi++) {
				// convert back to m
				tmp_values[fi + 0 * frameCount + bi * frameCount * maxCoeffs] *= 0.1f;
				tmp_values[fi + 1 * frameCount + bi * frameCount * maxCoeffs] *= 0.1f;
				tmp_values[fi + 2 * frameCount + bi * frameCount * maxCoeffs] *= 0.1f;
			}
		}
	}

	Model_Bayo_ApplyMotions(matrixes, tmp_values, tmp_cumul_scale, bones, bone_number, frameCount);

	noesisAnim_t *anim = rapi->rpgAnimFromBonesAndMatsFinish(bones, bone_number + 1, matrixes, frameCount, 60);
	if (!anim) {
		DBGLOG("Could not create anim for %s (out of memory?)!\n", fname);
		rapi->Noesis_UnpooledFree(matrixes);
		rapi->Noesis_UnpooledFree(tmp_values);
		continue;
	}

	anim->filename = rapi->Noesis_PooledString(fname);
	anim->flags |= NANIMFLAG_FILENAMETOSEQ;
	anim->aseq = rapi->Noesis_AnimSequencesAlloc(1, frameCount);
	anim->aseq->s->startFrame = 0;
	anim->aseq->s->endFrame = frameCount - 1;
	anim->aseq->s->frameRate = 60;
	anim->aseq->s->name = rapi->Noesis_PooledString(fname);
	if (anim)
	{
		animList.Append(anim);
	}
	rapi->Noesis_UnpooledFree(matrixes);
	rapi->Noesis_UnpooledFree(tmp_values);
	rapi->Noesis_UnpooledFree(tmp_cumul_scale);
  }

}

//gather entries from a dat file
template <bool big>
static void Model_Bayo_GetDATEntries(CArrayList<bayoDatFile_t> &dfiles, BYTE *fileBuffer, int bufferLen)
{
	bayoDat<big> dat((bayoDat_t *)fileBuffer);

	BYTE *namesp = fileBuffer+dat.ofsNames;
	int strSize = *((int *)namesp);
	if (big) LITTLE_BIG_SWAP(strSize);
	namesp += sizeof(int);

	int *ofsp = (int *)(fileBuffer+dat.ofsRes);
	int *sizep = (int *)(fileBuffer+dat.ofsSizes);
	DBGLOG("Found %d entries\n", dat.numRes);
	for (int i = 0; i < dat.numRes; i++)
	{
		bayoDatFile_t df;
		memset(&df, 0, sizeof(df));
		df.name = (char *)namesp;
		DBGLOG("\t%s\n", df.name);
		namesp += strSize;

		df.dataSize = *sizep;
		if (big) LITTLE_BIG_SWAP(df.dataSize);
		sizep++;

		int ofs = *ofsp;
		if (big) LITTLE_BIG_SWAP(ofs);
		df.data = fileBuffer+ofs;
		ofsp++;

		dfiles.Append(df);
	}
}
template <bool big, game_t game>
static void Model_Bayo_LoadSharedTextures(CArrayList<noesisTex_t *> &textures, char* name, noeRAPI_t *rapi) {
	noeUserPromptParam_t promptParams;
	char texturePrompt[MAX_NOESIS_PATH];
	char defaultValue[MAX_NOESIS_PATH];
	sprintf_s(texturePrompt, MAX_NOESIS_PATH, "Load shared textures (%s) in other files? (specify prefix if different)", name);
	sprintf_s(defaultValue, MAX_NOESIS_PATH, "%s", "Just click!");
	promptParams.titleStr = "Load shared textures?";
	promptParams.promptStr = texturePrompt;
	promptParams.defaultValue = defaultValue;
	promptParams.valType = NOEUSERVAL_NONE;
	promptParams.valHandler = NULL;
	wchar_t noepath[MAX_NOESIS_PATH];
	GetCurrentDirectory(MAX_NOESIS_PATH, noepath);

	while (g_nfn->NPAPI_UserPrompt(&promptParams)) {
		int dataLength;
		BYTE* data = rapi->Noesis_LoadPairedFile("Scenery Model", ".dat", dataLength, NULL);
		SetCurrentDirectory(noepath);
		if (data) {
			CArrayList<bayoDatFile_t> datfiles;
			Model_Bayo_GetDATEntries<big>(datfiles, data, dataLength);
			if (datfiles.Num() > 0) {
				CArrayList<bayoDatFile_t *> texfiles;
				if (game == BAYONETTA2) {
					for (int i = 0; i < datfiles.Num(); i++) {
						if (strstr(datfiles[i].name, "shared.wta")) {
							DBGLOG("Found shared texture bundle %s\n", datfiles[i].name);
							texfiles.Append(&datfiles[i]);
						}
					}
					for (int i = 0; i < datfiles.Num(); i++) {
						if (strstr(datfiles[i].name, "shared.wtp")) {
							DBGLOG("Found shared texture bundle %s\n", datfiles[i].name);
							texfiles.Append(&datfiles[i]);
						}
					}
					if (texfiles.Num() == 2) {
						Model_Bayo_LoadTextures<big, game>(textures, texfiles, rapi);
					}
				}
				else {
					for (int i = 0; i < datfiles.Num(); i++) {
						if (strstr(datfiles[i].name, "cmn.wtb")) {
							DBGLOG("Found shared texture bundle %s\n", datfiles[i].name);
							texfiles.Append(&datfiles[i]);
						}
					}
					if (texfiles.Num() > 0) {
						Model_Bayo_LoadTextures<big, game>(textures, texfiles, rapi);
					}
				}
				texfiles.Clear();
			}
			datfiles.Clear();
		}
		rapi->Noesis_UnpooledFree(data);
	}
	SetCurrentDirectory(noepath);
}
template <bool big, game_t game>
static void Model_Bayo_LoadExternalMotions(CArrayList<noesisAnim_t *> &animList, bayoDatFile_t &df, CArrayList<bayoDatFile_t *> &expfile, modelBone_t *bones, int bone_number, noeRAPI_t *rapi, short int * animBoneTT, void* extraBoneInfo){
	noeUserPromptParam_t promptParams;
	char wmbName[MAX_NOESIS_PATH];
	char motionPrompt[MAX_NOESIS_PATH];
	char defaultValue[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(wmbName, df.name);
	sprintf_s(motionPrompt, MAX_NOESIS_PATH, "Load motions for %s in other files? (specify prefix if different)", wmbName);
	sprintf_s(defaultValue, MAX_NOESIS_PATH, "%s", wmbName);
	promptParams.titleStr = "Load motions?";
	promptParams.promptStr = motionPrompt;
	promptParams.defaultValue = defaultValue;
	promptParams.valType = NOEUSERVAL_STRING;
	promptParams.valHandler = NULL;
	wchar_t noepath[MAX_NOESIS_PATH];
	GetCurrentDirectory(MAX_NOESIS_PATH, noepath);

	while( g_nfn->NPAPI_UserPrompt(&promptParams) ) {
		int dataLength;
		BYTE* data = rapi->Noesis_LoadPairedFile("Bayonetta PC Model", ".dat", dataLength, NULL);
		if (data) {
			CArrayList<bayoDatFile_t> datfiles;
			Model_Bayo_GetDATEntries<big>(datfiles, data, dataLength);
			if(datfiles.Num() > 0) {
				CArrayList<bayoDatFile_t *> motfiles;
				Model_Bayo_GetMotionFiles(datfiles, df, rapi, motfiles, (char *)promptParams.valBuf);
				if(motfiles.Num() > 0) {
					Model_Bayo_LoadMotions<big, game>(animList, motfiles, expfile, bones, bone_number, rapi, animBoneTT, extraBoneInfo);
				}
				motfiles.Clear();
			}
			datfiles.Clear();
		}
		rapi->Noesis_UnpooledFree(data);
	}
	SetCurrentDirectory(noepath);
}
//decode bayonetta x10y10z10 normals
template <bool big, game_t game>
static void Model_Bayo_CreateNormal(char *src, float *dst);
template <>
static void Model_Bayo_CreateNormal<true, BAYONETTA2>(char *src, float *dst) {
	DWORD r;
	memcpy(&r, src, sizeof(r));
	LITTLE_BIG_SWAP(r);
	int xBits = 10;
	int yBits = 10;
	int zBits = 10;
	int x = ((r>>0) & ((1<<xBits)-1));
	int y = ((r>>xBits) & ((1<<yBits)-1));
	int z = ((r>>(xBits+yBits)) & ((1<<zBits)-1));
	dst[0] = (float)SignedBits(x, xBits) / (float)((1<<(xBits-1))-1);
	dst[1] = (float)SignedBits(y, yBits) / (float)((1<<(yBits-1))-1);
	dst[2] = (float)SignedBits(z, zBits) / (float)((1<<(zBits-1))-1);
}
template <>
static void Model_Bayo_CreateNormal<false, BAYONETTA2>(char *src, float *dst) {
	DWORD r;
	memcpy(&r, src, sizeof(r));
	int xBits = 10;
	int yBits = 10;
	int zBits = 10;
	int x = ((r >> 0) & ((1 << xBits) - 1));
	int y = ((r >> xBits) & ((1 << yBits) - 1));
	int z = ((r >> (xBits + yBits)) & ((1 << zBits) - 1));
	dst[0] = (float)SignedBits(x, xBits) / (float)((1 << (xBits - 1)) - 1);
	dst[1] = (float)SignedBits(y, yBits) / (float)((1 << (yBits - 1)) - 1);
	dst[2] = (float)SignedBits(z, zBits) / (float)((1 << (zBits - 1)) - 1);
}
template <>
static void Model_Bayo_CreateNormal<true, BAYONETTA>(char *src, float *dst) {
	return Model_Bayo_CreateNormal<true, BAYONETTA2>(src, dst);
}
template <>
static void Model_Bayo_CreateNormal<false, BAYONETTA>(char *src, float *dst) {
	for (int j = 0; j < 3;  j++) {
		dst[j] = (float)src[3-j]/(float)127;
	}
}
template <>
static void Model_Bayo_CreateNormal<false, VANQUISH>(char *src, float *dst) {
	Model_Bayo_CreateNormal<false, BAYONETTA>(src, dst);
}
template <bool big, game_t game>
static void Model_Bayo_CreateNormals(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m)
{
	for (int i = 0; i < numVerts; i++)
	{
		char *src = (char *)(data + stride*i);
		float *dst = dsts+i*3;
		float tmp[3];
		Model_Bayo_CreateNormal<big, game>(src, dst);
		if (m) {
			g_mfn->Math_TransformPointByMatrixNoTrans(m, dst, tmp);
			g_mfn->Math_VecCopy(tmp, dst);
		}
		g_mfn->Math_VecNorm(dst);
		if(big) {
			LITTLE_BIG_SWAP(dst[0]);
			LITTLE_BIG_SWAP(dst[1]);
			LITTLE_BIG_SWAP(dst[2]);
		}
	}
}
template <bool big>
static void Model_Bayo_PretransformPositions(BYTE *data, int numVerts, int stride, modelMatrix_t *m)
{
	for (int i = 0; i < numVerts; i++)
	{
		float * src = (float *)(data + stride*i);
		float tmp[3];
		if (big) {
			LITTLE_BIG_SWAP(src[0]);
			LITTLE_BIG_SWAP(src[1]);
			LITTLE_BIG_SWAP(src[2]);
		}
		g_mfn->Math_TransformPointByMatrix(m, src, tmp);
		g_mfn->Math_VecCopy(tmp, src);
		if (big) {
			LITTLE_BIG_SWAP(src[0]);
			LITTLE_BIG_SWAP(src[1]);
			LITTLE_BIG_SWAP(src[2]);
		}
	}
}
template <bool big>
static void Model_Bayo_PretransformNormals(BYTE *data, int numVerts, int stride, modelMatrix_t *m)
{
	for (int i = 0; i < numVerts; i++)
	{
		float * src = (float *)(data + stride * i);
		float tmp[3];
		if (big) {
			LITTLE_BIG_SWAP(src[0]);
			LITTLE_BIG_SWAP(src[1]);
			LITTLE_BIG_SWAP(src[2]);
		}
		g_mfn->Math_TransformPointByMatrixNoTrans(m, src, tmp);
		g_mfn->Math_VecCopy(tmp, src);
		if (big) {
			LITTLE_BIG_SWAP(src[0]);
			LITTLE_BIG_SWAP(src[1]);
			LITTLE_BIG_SWAP(src[2]);
		}
	}
}
template <bool big, game_t game>
static void Model_Bayo_CreateTangents(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m);
template <>
static void Model_Bayo_CreateTangents<false, BAYONETTA>(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m)
{
	for (int i = 0; i < numVerts; i++)
	{
		BYTE *src = data + stride * i;
		float tmp[3];
		float *dst = dsts + i * 4;
		for (int j = 0; j < 4; j++) {
			dst[j] = ( src[j] - (float)127 ) / (float)127;
		}
		if (m) {
			g_mfn->Math_TransformPointByMatrixNoTrans(m, dst, tmp);
			g_mfn->Math_VecCopy(tmp, dst);
		}
	}
}
template <>
static void Model_Bayo_CreateTangents<false, VANQUISH>(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m) {
	Model_Bayo_CreateTangents<false, BAYONETTA>(data, dsts, numVerts, stride, m);
}
template <>
static void Model_Bayo_CreateTangents<false, NIER_AUTOMATA>(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m) {
	Model_Bayo_CreateTangents<false, BAYONETTA>(data, dsts, numVerts, stride, m);
}
template <>
static void Model_Bayo_CreateTangents<true, BAYONETTA2>(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m) {
	for (int i = 0; i < numVerts; i++)
	{
		BYTE *src = data + stride * i;
		float tmp[3];
		float *dst = dsts + i * 4;
		for (int j = 0; j < 4; j++) {
			dst[j] = (src[j] - (float)127) / (float)127;
		}
		// handedness is reverse here:
		if (m) {
			g_mfn->Math_TransformPointByMatrixNoTrans(m, dst, tmp);
			g_mfn->Math_VecCopy(tmp, dst);
		}
		for (int j = 0; j < 4; j++) {
			LITTLE_BIG_SWAP(dst[j]);
		}
	}
}
template <>
static void Model_Bayo_CreateTangents<false, BAYONETTA2>(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m) {
	for (int i = 0; i < numVerts; i++)
	{
		BYTE *src = data + stride * i;
		float tmp[3];
		float *dst = dsts + i * 4;
		for (int j = 0; j < 4; j++) {
			dst[j] = (src[j] - (float)127) / (float)127;
		}
		// handedness is reverse here:
		if (m) {
			g_mfn->Math_TransformPointByMatrixNoTrans(m, dst, tmp);
			g_mfn->Math_VecCopy(tmp, dst);
		}
	}
}
template <>
static void Model_Bayo_CreateTangents<true, BAYONETTA>(BYTE *data, float *dsts, int numVerts, int stride, modelMatrix_t *m)
{
	for (int i = 0; i < numVerts; i++)
	{
		BYTE *src = data + stride * i;
		float tmp[3];
		float *dst = dsts + i * 4;
		for (int j = 0; j < 4; j++) {
			dst[j] = (src[3-j] - (float)127) / (float)127;
		}
		// handedness is reverse here:
		if (m) {
			g_mfn->Math_TransformPointByMatrixNoTrans(m, dst, tmp);
			g_mfn->Math_VecCopy(tmp, dst);
		}
		for (int j = 0; j < 4; j++) {
			LITTLE_BIG_SWAP(dst[j]);
		}
	}
}
template <bool big>
modelBone_t *Model_Nier_CreateBones(nierWMBHdr<big> &hdr, BYTE *data, noeRAPI_t *rapi, int &numBones, short int * &animBoneTT)
{
	numBones = 0;
	if (hdr.numBones <= 0 || hdr.ofsBones <= 0 || hdr.ofsBoneIndexTT <= 0) {
		return NULL;
	}
	numBones = hdr.numBones;
	animBoneTT = (short int *)(data + hdr.ofsBoneIndexTT);
	DBGLOG("Found %d bones\n", numBones);
	modelBone_t *bones = rapi->Noesis_AllocBones(numBones+1);
	for (int i = 0; i < numBones; i++)
	{
		nierBone<big> nBone((nierBone_t *)(data + hdr.ofsBones + i*sizeof(nierBone_t)));
		modelBone_t *bone = bones + i;
		short parent = nBone.parentIndex;
		assert( parent < numBones);
		bone->index = i;
		if (parent == -1) {
			bone->eData.parent = bones + numBones;
		}
		else if (parent >= 0) {
			bone->eData.parent = bones + parent;
		}
		else {
			bone->eData.parent = NULL;
		}
		sprintf_s(bone->name, 30, "bone%03i", nBone.id);
		bone->mat = g_identityMatrix;

		float pos[3];
		pos[0] = nBone.localPosition.x;
		pos[1] = nBone.localPosition.y;
		pos[2] = nBone.localPosition.z;
		g_mfn->Math_TranslateMatrix(&bone->mat, pos);
		g_mfn->Math_RotateMatrix(&bone->mat, -nBone.localRotation.z * g_flRadToDeg, 0.0f, 0.0f, 1.0f);
		g_mfn->Math_RotateMatrix(&bone->mat,  nBone.localRotation.y * g_flRadToDeg, 0.0f, 1.0f, 0.0f);
		g_mfn->Math_RotateMatrix(&bone->mat, -nBone.localRotation.x * g_flRadToDeg, 1.0f, 0.0f, 0.0f);

		float scale[3];
		scale[0] = nBone.localScale.x;
		scale[1] = nBone.localScale.y;
		scale[2] = nBone.localScale.z;
		for (int i = 0; i < 3; i++) {
			bone->mat.x1[i] *= scale[i];
			bone->mat.x2[i] *= scale[i];
			bone->mat.x3[i] *= scale[i];
		}
		bone->userIndex = 5;
		/*
		pos[0] = nBone.position.x;
		pos[1] = nBone.position.y;
		pos[2] = nBone.position.z;
		g_mfn->Math_VecCopy(pos, bone->mat.o);*/
	}
	bones[numBones].index = numBones;
	bones[numBones].eData.parent = NULL;
	sprintf_s(bones[numBones].name, 30, "bone-1");
	bones[numBones].mat = g_identityMatrix;
	bones[numBones].userIndex = 5;
	rapi->rpgMultiplyBones(bones, numBones);
	return bones;
}
//convert the bones
template <bool big>
modelBone_t *Model_Bayo_CreateBones(bayoWMBHdr<big> &hdr, BYTE *data, noeRAPI_t *rapi, int &numBones, short int * &animBoneTT)
{
	numBones = 0;
	if (hdr.numBones <= 0 || hdr.ofsBoneHie <= 0 || hdr.ofsBoneDataA <= 0 || hdr.ofsBoneDataB <= 0)
	{
		return NULL;
	}
	short *parentList = (short *)(data+hdr.ofsBoneHie);
	float *posList = (float *)(data+hdr.ofsBoneDataB);
	float *relPosList = (float *)(data+hdr.ofsBoneDataA); //actually relative positions
	char *boneFlags = nullptr;
	if(hdr.ofsBoneFlags)
		boneFlags = (char *)(data + hdr.ofsBoneFlags);
	animBoneTT = (short int *)(data+hdr.ofsBoneHieB);
	std::map<short int, short int> boneMap;
	for (short int i = 0; i < 0x1000; i++) {
		short int decoded_index = Model_Bayo_DecodeMotionIndex<big>(animBoneTT, i);
		if (decoded_index != 0x0fff) {
			boneMap.insert(std::pair <short int, short int>(decoded_index, i));
		}
	}

	numBones = hdr.numBones;
	DBGLOG("Found %d bones\n", numBones);
	modelBone_t *bones = rapi->Noesis_AllocBones(numBones+1);
	for (int i = 0; i < numBones; i++)
	{
		modelBone_t *bone = bones + i;
		float *ppos = posList + i*3;
		short parent = parentList[i];
		if (big) LITTLE_BIG_SWAP(parent);
		assert(parent < numBones);
		bone->index = i;
		if (parent == -1) {
			bone->eData.parent = bones + numBones;
		}
		else if (parent >= 0) {
			bone->eData.parent = bones + parent;
		}
		else {
			bone->eData.parent = NULL;
		}
		sprintf_s(bone->name, 30, "bone%03i", boneMap.at((short int)i));
		bone->mat = g_identityMatrix;
		float pos[3];

		memcpy(pos, ppos, sizeof(pos));
		if (big)  {
			LITTLE_BIG_SWAP(pos[0]);
			LITTLE_BIG_SWAP(pos[1]);
			LITTLE_BIG_SWAP(pos[2]);
		}
		g_mfn->Math_VecCopy(pos, bone->mat.o);
		if (boneFlags)
			bone->userIndex = boneFlags[i];
		else
			bone->userIndex = 5;
	}
	bones[numBones].index = numBones;
	bones[numBones].eData.parent = NULL;
	sprintf_s(bones[numBones].name, 30, "bone-1");
	bones[numBones].mat = g_identityMatrix;
	bones[numBones].userIndex = 5;

	DBGALOG("-------------------------------\n");
	for(int bi = 0; bi < numBones; bi++) {
		DBGALOG("bone %d (%d)\n", bi, bones[bi].index);
		DBGALOG("parent %d\n", bones[bi].eData.parent ? bones[bi].eData.parent->index : -1);
		DBGALOG("\ttranslate: %f, %f, %f\n", bones[bi].mat.o[0], bones[bi].mat.o[1], bones[bi].mat.o[2]);
		DBGALOG("\trelative_c: %f, %f, %f\n",
			bones[bi].mat.o[0] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[0] : 0.0),
			bones[bi].mat.o[1] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[1] : 0.0),
			bones[bi].mat.o[2] - (bones[bi].eData.parent ? bones[bi].eData.parent->mat.o[2] : 0.0));
		float relPos[3];
		memcpy(relPos, relPosList + 3*bi, sizeof(relPos));
		if (big)  {
			LITTLE_BIG_SWAP(relPos[0]);
			LITTLE_BIG_SWAP(relPos[1]);
			LITTLE_BIG_SWAP(relPos[2]);
		}
		DBGALOG("\trelative_g: %f, %f, %f\n", relPos[0], relPos[1], relPos[2]);
	}
	//bones come pre-transformed
	//rapi->rpgMultiplyBones(bones, numBones);
	return bones;
}
template <bool big, game_t game>
static void Model_Vanquish_LoadExtraTex(char *texDir, int texId, noeRAPI_t *rapi, CArrayList<noesisTex_t *> &textures) {
	//Try to load from texture directory
	char texName[MAX_NOESIS_PATH];
	snprintf(texName, MAX_NOESIS_PATH, "%s%0x.wtb", texDir, texId);
	DBGLOG("Trying to load %s...\n", texName);
	bayoDatFile_t wtbFile;
	wtbFile.name = texName;
	wtbFile.data = (BYTE *)rapi->Noesis_ReadFile(texName, &wtbFile.dataSize);
	if (wtbFile.data && wtbFile.dataSize > 0)
	{
		CArrayList<bayoDatFile_t *> texFiles;
		int numText = textures.Num();
		texFiles.Append(&wtbFile);
		DBGLOG("...Found Texture!\n");
		Model_Bayo_LoadTextures<big, game>(textures, texFiles, rapi);
	}
	else {
		DBGLOG("...Texture not found!\n");
	}
}

template<bool big>
static void Model_Bayo_SetTile(BYTE * matData, int tileOffset, noesisMaterial_t * mat, noeRAPI_t *rapi) {
	if (tileOffset != -1) {
		bayoV4F<big> tile((bayoV4F_t *)(matData + tileOffset));
		char myexpr[128];
		if (tile.x != 1.0f) {
			snprintf(myexpr, 128, "vert_uv_u * %f", tile.x);
			mat->expr->v_uvExpr[0] = rapi->Express_Parse(myexpr);
		}
		if (tile.y != 1.0f) {
			snprintf(myexpr, 128, "vert_uv_v * %f", tile.y);
			mat->expr->v_uvExpr[1] = rapi->Express_Parse(myexpr);
		}
	}
}
template<bool big>
static int Model_Bayo_ReadTextureIndex(wmbMat<big> &mat, CArrayList<noesisTex_t *> &textures, int textureOffset, int &sharedtextureoffset, bool default0, noeRAPI_t * rapi) {
	DBGLOG("texture offset %d ", textureOffset);
	if (textureOffset != -1 || default0) {
		int offset;
		if (textureOffset != -1) {
			offset = textureOffset / 4 - 1;
		}
		else {
			offset = 0;
		}
		DBGLOG(" offset %d ", offset);
		if ((mat.texs[offset].tex_flagB & 0xff00) == 0xa000) {
			if (sharedtextureoffset == -1) {
				sharedtextureoffset = textures.Num();
				char scrName[10];
				snprintf(scrName, 10, "r%2x0", mat.texs[offset].tex_flagB & 0xff);
				Model_Bayo_LoadSharedTextures<big, BAYONETTA>(textures, scrName, rapi);
				if (textures.Num() == sharedtextureoffset) {
					sharedtextureoffset = -2;
				}
			}
			if (sharedtextureoffset >= 0) {
				return mat.texs[offset].tex_idx + sharedtextureoffset;
			}
			else {
				return -1;
			}
		}
		else if (!mat.texs[offset].tex_flagB || (mat.texs[offset].tex_flagB & 0xff00) == 0x9000) {
			return mat.texs[offset].tex_idx;
		}
		else {
			return -1;
		}
	}
	else {
		return -1;
	}
}
//load Bayonetta Material
#define BAYONETTA_MULTIPASS (1<<0)
template <bool big, game_t game>
static void Model_Bayo_LoadMaterials(bayoWMBHdr<big> &hdr,
                                     CArrayList<noesisTex_t *> &textures,
                                     bool &hasExMatInfo,
                                     CArrayList<noesisMaterial_t *> &matList,
                                     CArrayList<noesisMaterial_t *> &matListLightMap,
                                     CArrayList<noesisMaterial_t *> &totMatList,
                                     BYTE *data,
                                     noeRAPI_t *rapi,
                                     int sharedtextureoffset = -1) {
	char *inFile = rapi->Noesis_GetInputName();
	char texDir[MAX_NOESIS_PATH] = "";
	if (game == VANQUISH) {
		rapi->Noesis_GetDirForFilePath(texDir, inFile);
		strcat_s(texDir, MAX_NOESIS_PATH, "\\..\\HTextures\\");
	}
	int *matOfsList = (int *)(data + hdr.ofsMaterialsOfs);
	hasExMatInfo = (hdr.exMatInfo[0] && hdr.exMatInfo[1] && hdr.exMatInfo[3]);
	int *matIDs = (hasExMatInfo) ? (int *)(data + hdr.exMatInfo[1]) : NULL;
	int numMatIDs = 0;
	if (matIDs)
	{ //got a global reference list (this seems redundant since it's also provided in the texture bundle)
		numMatIDs = *matIDs;
		if (big) LITTLE_BIG_SWAP(numMatIDs);
		matIDs++;
	}
	DBGLOG("Found %d materials, shared textures offset: %d\n", hdr.numMaterials, sharedtextureoffset);
	for (int i = 0; i < hdr.numMaterials; i++)
	{
		DBGLOG("\t%03d:", i);
		int matOfs = matOfsList[i];
		if (big) LITTLE_BIG_SWAP(matOfs);
		BYTE *matData = data + hdr.ofsMaterials + matOfs;
		//create a noesis material entry
		char matName[128];
		sprintf_s(matName, 128, "bayomat%i", i);
		DBGLOG(" name: %s,", matName);
		noesisMaterial_t *nmat = rapi->Noesis_GetMaterialList(1, true);
		noesisMaterial_t *nmatLightMap = NULL;
		nmat->name = rapi->Noesis_PooledString(matName);
		//nmat->flags |= NMATFLAG_TWOSIDED;

		if (hasExMatInfo && numMatIDs > 0)
		{ //search by global index values
			DBGLOG("vanquish style\n");
			nmat->noDefaultBlend = true;
			nmat->texIdx = -1;
			nmat->normalTexIdx = -1; //default to flat normal
			char *shaderName = (char *)(data + hdr.exMatInfo[0] + 16 * i);
			if (_strnicmp(shaderName, "har", 3) == 0 || _strnicmp(shaderName, "gla", 3) == 0 || _strnicmp(shaderName, "cnm", 3) == 0 || _strnicmp(shaderName, "alp", 3) == 0)
			{ //blend hair
				nmat->noDefaultBlend = false;
			}
			bool isSkin = (_strnicmp(shaderName, "skn0", 4) == 0);
			bool isPHG = (_strnicmp(shaderName, "phg05", 5) == 0);
			bool isLBT = (_strnicmp(shaderName, "lbt00", 5) == 0);
			bool raSwap = (_strnicmp(shaderName, "max32", 5) == 0); //hacky way to see if the normal needs red and alpha swapped, probably not reliable
			int difTexId = *((int *)(matData + 4));
			int nrmOfs = (isPHG || isLBT) ? 8 : 16;
			int nrmTexId = *((int *)(matData + nrmOfs)); //this is kinda happenstance, i think the only right way to know what to do is to check the pixel shader
			DBGLOG("\t\tshader: %s, diff: 0x%0x, nrm: 0x%0x\n", shaderName, difTexId, nrmTexId);
			for (int j = 0; j < textures.Num(); j++)
			{
				noesisTex_t *tex = textures[j];
				if (tex && tex->globalIdx == difTexId)
				{
					nmat->texIdx = j;
				}
			}
			if (game == VANQUISH && nmat->texIdx == -1) {
				//Try to load from texture directory
				int numTex = textures.Num();
				Model_Vanquish_LoadExtraTex<big, game>(texDir, difTexId, rapi, textures);
				if (numTex < textures.Num()) {
					nmat->texIdx = textures.Num() - 1;
				}
			}
			for (int j = 0; j < textures.Num(); j++)
			{
				noesisTex_t *tex = textures[j];
				if (tex && tex->globalIdx == nrmTexId)
				{
					nmat->normalTexIdx = j;
					if (raSwap)
					{
						nmat->flags |= NMATFLAG_NMAPSWAPRA;
					}
				}
			}
			if (game == VANQUISH && nmat->normalTexIdx == -1) {
				//Try to load from texture directory
				int numTex = textures.Num();
				Model_Vanquish_LoadExtraTex<big, game>(texDir, nrmTexId, rapi, textures);
				if (numTex < textures.Num()) {
					nmat->normalTexIdx = textures.Num() - 1;
				}
			}
			else if (nmat->normalTexIdx == -1) {
				nmat->normalTexIdx = textures.Num() - 1;
			}
		}
		else
		{ //bayonetta-style
			wmbMat<big> mat((wmbMat_t *)matData);

			if (!bayoMatTypes[mat.matFlags].known) {
				DBGLOG(" unknown material id: %2x, %4x", mat.matFlags, mat.unknownB);
				for (int j = 0; j < 5; j++) {
					DBGLOG(", tex%d: %d, %2x, %4x", j, mat.texs[j].tex_idx, mat.texs[j].tex_flagA, mat.texs[j].tex_flagB);
				}
				DBGLOG("\n");
				DBGFLUSH();
			}
			else {
				DBGLOG(" material id: %2x, %4x, offset: %x",
					mat.matFlags, mat.unknownB, hdr.ofsMaterials + matOfs);
			}

			nmat->normalTexIdx = -1;
			int color1 = Model_Bayo_ReadTextureIndex(mat, textures, bayoMatTypes[mat.matFlags].color_1_sampler, sharedtextureoffset, true, rapi);
			int color2 = Model_Bayo_ReadTextureIndex(mat, textures, bayoMatTypes[mat.matFlags].color_2_sampler, sharedtextureoffset, false, rapi);
			int color3 = Model_Bayo_ReadTextureIndex(mat, textures, bayoMatTypes[mat.matFlags].color_3_sampler, sharedtextureoffset, false, rapi);
			DBGLOG(", color1: %d", color1);
			if (color2 != -1) {
				DBGLOG(", color2: %d", color2);
			}
			if (color3 != -1) {
				DBGLOG(", color3: %d", color3);
			}
			if (color3 != -1 && color2 != -1) {
				// r: 1 - ratio of color1 and (color2 + color3)
				// g: 1 - ratio of color2 and color 3
				// (1 - r) * (g*color3 + (1-g)*color2) +  r * color1
				//(1-r) * g * color3 + (1-r)*(1-g)*color2 + r * color1
				nmat->ex->userTag[0] = BAYONETTA_MULTIPASS;
				nmat->texIdx = color3;
				nmat->expr = rapi->Noesis_AllocMaterialExpressions(NULL);
				nmat->expr->v_clrExpr[0] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[1] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[2] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[3] = rapi->Express_Parse("1.0");

				char matName2[128];
				sprintf_s(matName2, 128, "bayomat2p%i", i);
				DBGLOG(" name: %s,", matName2);
				noesisMaterial_t *nmat2 = rapi->Noesis_GetMaterialList(1, true);
				nmat2->name = rapi->Noesis_PooledString(matName2);
				totMatList.Append(nmat2);
				nmat2->expr = rapi->Noesis_AllocMaterialExpressions(NULL);
				nmat2->expr->v_clrExpr[0] = rapi->Express_Parse("1.0");
				nmat2->expr->v_clrExpr[1] = rapi->Express_Parse("1.0");
				nmat2->expr->v_clrExpr[2] = rapi->Express_Parse("1.0");
				nmat2->expr->v_clrExpr[3] = rapi->Express_Parse("vert_clr_g");
				nmat2->texIdx = color2;
				nmat->nextPass = nmat2;
				nmat2->blendSrc = NOEBLEND_ONE_MINUS_SRC_ALPHA;
				nmat2->blendDst = NOEBLEND_SRC_ALPHA;

				char matName3[128];
				sprintf_s(matName3, 128, "bayomat3p%i", i);
				DBGLOG(" name: %s,", matName3);
				noesisMaterial_t *nmat3 = rapi->Noesis_GetMaterialList(1, true);
				nmat3->name = rapi->Noesis_PooledString(matName3);
				totMatList.Append(nmat3);
				nmat3->expr = rapi->Noesis_AllocMaterialExpressions(NULL);
				nmat3->expr->v_clrExpr[0] = rapi->Express_Parse("1.0");
				nmat3->expr->v_clrExpr[1] = rapi->Express_Parse("1.0");
				nmat3->expr->v_clrExpr[2] = rapi->Express_Parse("1.0");
				nmat3->expr->v_clrExpr[3] = rapi->Express_Parse("vert_clr_r");
				nmat3->texIdx = color1;
				nmat2->nextPass = nmat3;
				nmat3->blendSrc = NOEBLEND_ONE_MINUS_SRC_ALPHA;
				nmat3->blendDst = NOEBLEND_SRC_ALPHA;

				Model_Bayo_SetTile<big>(matData, bayoMatTypes[mat.matFlags].color_1_tile, nmat3, rapi);
				Model_Bayo_SetTile<big>(matData, bayoMatTypes[mat.matFlags].color_2_tile, nmat2, rapi);
				Model_Bayo_SetTile<big>(matData, bayoMatTypes[mat.matFlags].color_3_tile, nmat, rapi);
			}
			else if (color2 != -1) {
				nmat->ex->userTag[0] = BAYONETTA_MULTIPASS;
				nmat->texIdx = color1;
				nmat->expr = rapi->Noesis_AllocMaterialExpressions(NULL);
				nmat->expr->v_clrExpr[0] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[1] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[2] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[3] = rapi->Express_Parse("1.0");
				//nmat->alphaTest = 0.0f;
				//nmat->noDefaultBlend = true ;
				//DBGLOG(", blendSrc: %d", nmat->blendSrc);
				//DBGLOG(", blendDst: %d", nmat->blendDst);
				char matName2[128];
				sprintf_s(matName2, 128, "bayomat2p%i", i);
				DBGLOG(" name: %s,", matName2);
				noesisMaterial_t *nmat2 = rapi->Noesis_GetMaterialList(1, true);
				nmat2->name = rapi->Noesis_PooledString(matName2);
				totMatList.Append(nmat2);

				nmat2->expr = rapi->Noesis_AllocMaterialExpressions(NULL);
				nmat2->expr->v_clrExpr[0] = rapi->Express_Parse("1.0");
				nmat2->expr->v_clrExpr[1] = rapi->Express_Parse("1.0");
				nmat2->expr->v_clrExpr[2] = rapi->Express_Parse("1.0");
				nmat2->expr->v_clrExpr[3] = rapi->Express_Parse("vert_clr_r");
				nmat2->texIdx = color2;
				nmat->nextPass = nmat2;

				nmat2->blendSrc = NOEBLEND_SRC_ALPHA;
				nmat2->blendDst = NOEBLEND_ONE_MINUS_SRC_ALPHA;

				Model_Bayo_SetTile<big>(matData, bayoMatTypes[mat.matFlags].color_1_tile, nmat, rapi);
				Model_Bayo_SetTile<big>(matData, bayoMatTypes[mat.matFlags].color_2_tile, nmat2, rapi);
			}
			else {
				nmat->texIdx = color1;
				//avoid using material expressions for simple materials.

				/*if (bayoMatTypes[mat.matFlags].color_1_tile != -1) {
					bayoV4F<big> tile((bayoV4F_t *)(matData + bayoMatTypes[mat.matFlags].color_1_tile));

					if (tile.x != 1.0f || tile.y != 1.0f) {
						float *scaleBias = (float *)rapi->Noesis_PooledAlloc(4 * sizeof(float));
						scaleBias[0] = tile.x;
						scaleBias[1] = tile.y;
						scaleBias[2] = 0.0f;
						scaleBias[3] = 0.0f;
						nmat->ex->pUvScaleBias = scaleBias;
					}
				}*/
				nmat->expr = rapi->Noesis_AllocMaterialExpressions(NULL);
				/*nmat->expr->v_clrExpr[0] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[1] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[2] = rapi->Express_Parse("1.0");
				nmat->expr->v_clrExpr[3] = rapi->Express_Parse("1.0");*/
				Model_Bayo_SetTile<big>(matData, bayoMatTypes[mat.matFlags].color_1_tile, nmat, rapi);
			}
			if (bayoMatTypes[mat.matFlags].shader_name && strstr(bayoMatTypes[mat.matFlags].shader_name, "modelshaderpls06_bxnxx")) {
				//nmat->flags |= NMATFLAG_NORMAL_UV1;
			}
			if (bayoMatTypes[mat.matFlags].shader_name && strstr(bayoMatTypes[mat.matFlags].shader_name, "modelshaderbgs")) {
				nmat->bumpTexIdx = Model_Bayo_ReadTextureIndex(mat, textures, bayoMatTypes[mat.matFlags].reliefmap_sampler, sharedtextureoffset, false, rapi);
				DBGLOG(", bump: %d", nmat->bumpTexIdx);
			}
			else {
				nmat->normalTexIdx = Model_Bayo_ReadTextureIndex(mat, textures, bayoMatTypes[mat.matFlags].reliefmap_sampler, sharedtextureoffset, false, rapi);
				DBGLOG(", normal: %d", nmat->normalTexIdx);
			}
			if (nmat->normalTexIdx == -1 && textures.Num() > 0) {
				nmat->normalTexIdx = textures.Num() - 1;
			}

			nmat->specularTexIdx = Model_Bayo_ReadTextureIndex(mat, textures, bayoMatTypes[mat.matFlags].spec_mask_sampler, sharedtextureoffset, false, rapi);

			if (bayoMatTypes[mat.matFlags].specular != -1) {
				bayoV4F<big> spec((bayoV4F_t *)(matData + bayoMatTypes[mat.matFlags].specular));
				nmat->specular[0] = spec.x;
				nmat->specular[1] = spec.y;
				nmat->specular[2] = spec.z;
			}

			if (bayoMatTypes[mat.matFlags].diffuse != -1) {
				bayoV4F<big> diff((bayoV4F_t *)(matData + bayoMatTypes[mat.matFlags].diffuse));
				nmat->diffuse[0] = diff.x;
				nmat->diffuse[1] = diff.y;
				nmat->diffuse[2] = diff.z;
			}

			nmat->envTexIdx = Model_Bayo_ReadTextureIndex(mat, textures, bayoMatTypes[mat.matFlags].envmap_sampler, sharedtextureoffset, false, rapi);
/*			if (nmat->envTexIdx != -1 && nmat->specularTexIdx == -1) {
				nmat->specularTexIdx = nmat->texIdx;
				nmat->flags |= NMATFLAG_PBR_SPEC;
				nmat->specular[0] = 1.0;
				nmat->specular[1] = 1.0;
				nmat->specular[2] = 1.0;
				nmat->specular[3] = 1.0;
				nmat->noDefaultBlend = true;
			}*/
			//todo - some materials also do a scale+bias+rotation on the uv's at runtime to transform the texture coordinates into a
			//specific region of the normal page. i would think the uv transform data is buried in the giant chunk of floats that
			//follows the material data, but i don't see it in there. maybe it's related to some texture bundle flags.
			short lightmap_offset = bayoMatTypes[mat.matFlags].lightmap_sampler;
			if ( lightmap_offset != -1 && !mat.texs[lightmap_offset / 4 - 1].tex_flagB) {
				char matNameLightMap[128];
				sprintf_s(matNameLightMap, 128, "bayomat_light%i", i);
				nmatLightMap = rapi->Noesis_GetMaterialList(1, true);
				nmatLightMap->name = rapi->Noesis_PooledString(matNameLightMap);
				nmatLightMap->texIdx = mat.texs[lightmap_offset / 4 - 1].tex_idx;
				nmatLightMap->normalTexIdx = nmat->normalTexIdx;
				//nmatLightMap->noLighting = true;
				nmatLightMap->flags |= NMATFLAG_USELMUVS;
				nmatLightMap->blendDst = NOEBLEND_ZERO;
				nmatLightMap->blendSrc = NOEBLEND_DST_COLOR;
				//nmatLightMap->noDefaultBlend = true;
				//nmatLightMap->alphaTest = 0.5;

				if (nmat->ex->userTag[0] & BAYONETTA_MULTIPASS) {
					nmatLightMap->expr = rapi->Noesis_AllocMaterialExpressions(NULL);
					nmatLightMap->expr->v_clrExpr[0] = rapi->Express_Parse("1.0");
					nmatLightMap->expr->v_clrExpr[1] = rapi->Express_Parse("1.0");
					nmatLightMap->expr->v_clrExpr[2] = rapi->Express_Parse("1.0");
					nmatLightMap->expr->v_clrExpr[3] = rapi->Express_Parse("1.0");
				}

				totMatList.Append(nmatLightMap);
				//nmat->noLighting = true;
				if (color3 != -1 && color2 != -1) {
					nmat->nextPass->nextPass->nextPass = nmatLightMap;
				}
				else if (color2 != -1) {
					nmat->nextPass->nextPass = nmatLightMap;
				}
				else {
					nmat->nextPass = nmatLightMap;
				}
			}
			/*copy properties to next passes*/
			if (color3 != -1 && color2 != -1) {
				nmat->nextPass->nextPass->normalTexIdx = nmat->normalTexIdx;
				nmat->nextPass->nextPass->noLighting = nmat->noLighting;
				g_mfn->Math_VecCopy(nmat->diffuse, nmat->nextPass->nextPass->diffuse);
				g_mfn->Math_VecCopy(nmat->specular, nmat->nextPass->nextPass->specular);
				nmat->nextPass->normalTexIdx = nmat->normalTexIdx;
				nmat->nextPass->noLighting = nmat->noLighting;
				g_mfn->Math_VecCopy(nmat->diffuse, nmat->nextPass->diffuse);
				g_mfn->Math_VecCopy(nmat->specular, nmat->nextPass->specular);
			}
			else if (color2 != -1) {
				nmat->nextPass->normalTexIdx = nmat->normalTexIdx;
				nmat->nextPass->noLighting = nmat->noLighting;
				g_mfn->Math_VecCopy(nmat->diffuse, nmat->nextPass->diffuse);
				g_mfn->Math_VecCopy(nmat->specular, nmat->nextPass->specular);
			}
			DBGLOG("\n");



		}
		matListLightMap.Append(nmatLightMap);
		matList.Append(nmat);
		totMatList.Append(nmat);
	}
}
//load Bayonetta Material
template <bool big, game_t game>
static void Model_Nier_LoadMaterials(nierWMBHdr<big> &hdr,
	CArrayList<noesisTex_t *> &textures,
	CArrayList<noesisMaterial_t *> &matList,
	CArrayList<noesisMaterial_t *> &matListLightMap,
	CArrayList<noesisMaterial_t *> &totMatList,
	BYTE *data,
	noeRAPI_t *rapi) {

	nierMaterial_t *matPtr = (nierMaterial_t *)(data + hdr.ofsMaterials);

	DBGLOG("Found %d materials\n", hdr.numMaterials);
	for (int i = 0; i < hdr.numMaterials; i++)
	{
		nierMaterial<big> mat(matPtr + i);
		DBGLOG("\tFound %s, %s, %s\n", data + mat.ofsName, data + mat.ofsShaderName, data + mat.ofsTechniqueName);
		noesisMaterial_t *nmat = rapi->Noesis_GetMaterialList(1, true);
		nmat->name = rapi->Noesis_PooledString((char*)(data + mat.ofsName));
		nmat->noDefaultBlend = false;
		nmat->texIdx = -1;
		nmat->normalTexIdx = textures.Num() - 1;
		nmat->specularTexIdx = -1;
		int difTexId = 0;
		int nrmTexId = 0;
		int specTexId = 0;
		DBGLOG("\tFound %d textures\n", mat.numTextures);
		nierTexture_t * texPtr = (nierTexture_t *)(data + mat.ofsTextures);
		for (unsigned int j = 0; j < mat.numTextures; j++) {
			nierTexture<big> tex(texPtr + j);
			if (strcmp((char*)(data + tex.ofsName), "g_AlbedoMap") == 0 || strcmp((char*)(data + tex.ofsName), "g_AlbedoMap1") == 0) {
				difTexId = tex.id;
			}
			if (strcmp((char*)(data + tex.ofsName), "g_NormalMap") == 0 || strcmp((char*)(data + tex.ofsName), "g_NormalMap1") == 0) {
				nrmTexId = tex.id;
			}
			if (strcmp((char*)(data + tex.ofsName), "g_MaskMap2") == 0) {
				specTexId = tex.id;
			}
		}
		for (int j = 0; j < textures.Num(); j++) {
			noesisTex_t *tex = textures[j];
			if (difTexId && tex->globalIdx == difTexId) {
				DBGLOG("Found matching texture %d\n", j);
				nmat->texIdx = j;
			}
			if (nrmTexId && tex->globalIdx == nrmTexId) {
				DBGLOG("Found matching normal %d\n", j);
				nmat->normalTexIdx = j;
			}
			if (specTexId && tex->globalIdx == specTexId) {
				DBGLOG("Found matching specular %d\n", j);
				nmat->specularTexIdx = j;
				nmat->flags |= NMATFLAG_PBR_SPEC_IR_RG;
			}
		}
		matListLightMap.Append(NULL);
		matList.Append(nmat);
		totMatList.Append(nmat);
	}
}
typedef struct buffer_s {
	BYTE				*address = NULL;
	unsigned int		stride = 0;
	rpgeoDataType_e		type = RPGEODATA_FLOAT;
} buffer_t;
typedef struct buffers_s {
	buffer_t position;
	buffer_t mapping;
	buffer_t tangents;
	buffer_t normal;
	buffer_t bone_indexes;
	buffer_t bone_weights;
	buffer_t color;
	buffer_t mapping2;
	buffer_t position2;
} buffers_t;
typedef struct nierBuffers_s : public buffers_s {
	buffer_t indices;
	buffer_t mapping3;
	buffer_t mapping4;
	buffer_t mapping5;
} nierBuffers_t;
template <bool big, game_t game>
static void __set_position(buffer_t &position, BYTE *address, unsigned int stride, unsigned int count, modelMatrix_t * pretransform = NULL) {
	position.address = address;
	position.stride = stride;
	position.type = RPGEODATA_FLOAT;
	if (pretransform)
	{
		Model_Bayo_PretransformPositions<big>(position.address, count, stride, pretransform);
	}
}
template <bool big, game_t game>
static void __set_fnormal(buffer_t &normal, BYTE *address, unsigned int stride, unsigned int count, modelMatrix_t * pretransform = NULL) {
	normal.address = address;
	normal.stride = stride;
	normal.type = RPGEODATA_FLOAT;
	if (pretransform)
	{
		Model_Bayo_PretransformNormals<big>(normal.address, count, stride, pretransform);
	}
}
template <bool big, game_t game>
static void __set_hnormal(buffer_t &normal, BYTE *address, unsigned int stride, unsigned int count, noeRAPI_t *rapi, modelMatrix_t * pretransform = NULL) {
	normal.address = (BYTE *)rapi->Noesis_PooledAlloc(sizeof(float) * 3 * count);
	float *dst = (float *)normal.address;
	for (unsigned int i = 0; i < count; i++) {
		dst[3 * i] = H.decompress(*(unsigned short *)(address + i * stride), big);
		dst[3 * i + 1] = H.decompress(*(unsigned short *)(address + i * stride + 2), big);
		dst[3 * i + 2] = H.decompress(*(unsigned short *)(address + i * stride + 4), big);
	}
	normal.stride = 12;
	normal.type = RPGEODATA_FLOAT;
	if (pretransform)
	{
		Model_Bayo_PretransformNormals<big>(normal.address, count, stride, pretransform);
	}
}
template <bool big, game_t game>
static void __set_mapping(buffer_t &mapping, BYTE *address, unsigned int stride) {
	mapping.address = address;
	mapping.stride = stride;
	mapping.type = RPGEODATA_HALFFLOAT;
}
template <bool big, game_t game>
static void __set_fmapping(buffer_t &mapping, BYTE *address, unsigned int stride) {
	mapping.address = address;
	mapping.stride = stride;
	mapping.type = RPGEODATA_FLOAT;
}
template <bool big, game_t game>
static void __set_normal(buffer_t &normal, BYTE *address, unsigned int stride, unsigned int count, noeRAPI_t *rapi, modelMatrix_t * pretransform = NULL) {
	normal.address = (BYTE *)rapi->Noesis_PooledAlloc(sizeof(float) * 3 * count);
	Model_Bayo_CreateNormals<big, game>(address, (float *)normal.address, count, stride, pretransform);
	normal.stride = 12;
	normal.type = RPGEODATA_FLOAT;
}
template <bool big, game_t game>
static void __set_tangents(buffer_t &tangents, BYTE *address, unsigned int stride, unsigned int count, noeRAPI_t *rapi, modelMatrix_t * pretransform = NULL) {
	tangents.address = (BYTE *)rapi->Noesis_PooledAlloc(sizeof(float) * 4 * count);
	Model_Bayo_CreateTangents<big, game>(address, (float *)tangents.address, count, stride, pretransform);
	tangents.stride = 16;
	tangents.type = RPGEODATA_FLOAT;
}
template <bool big, game_t game>
static void __set_bone_infos(buffer_t &bone_indexes, BYTE *address, unsigned int stride) {
	bone_indexes.address = address;
	bone_indexes.stride = stride;
	bone_indexes.type = RPGEODATA_UBYTE;
}
template <bool big, game_t game>
static void __set_color(buffer_t &color, BYTE *address, unsigned int stride) {
	color.address = address;
	color.stride = stride;
	color.type = RPGEODATA_UBYTE;
}
template <bool big, game_t game>
static void __set_indices(buffer_t &indices, BYTE *address, unsigned int stride) {
	indices.address = address;
	indices.type = RPGEODATA_UINT;
	indices.stride = stride;
}
template <bool big, game_t game>
static void __set_sindices(buffer_t &indices, BYTE *address, unsigned int stride) {
	indices.address = address;
	indices.type = RPGEODATA_USHORT;
	indices.stride = stride;
}
template <bool big, game_t game>
static void Model_Bayo_SetBuffers(bayoDatFile_t &df, noeRAPI_t *rapi, bayoWMBHdr<big> &hdr, buffers_t &buffers, modelMatrix_t * pretransform = NULL) {
	BYTE *data = df.data;
	unsigned int bayoVertSize;
	unsigned int bayoVertExSize;
	unsigned int numVerts = hdr.numVerts;
	BYTE *pos = data + hdr.ofsPositions;
	BYTE *verts = data + hdr.ofsVerts;
	BYTE *vertsEx = data + hdr.ofsVertExData;
	DBGLOG("Vertex format: <%0x, %d, %d>!!!\n", hdr.vertexFormat, hdr.numMapping, hdr.unknownD);
	// Bayonetta
	if (hdr.vertexFormat == 0x6800001f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1){
		bayoVertSize = 32;
		bayoVertExSize = 8;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
		__set_mapping<big, game>(buffers.mapping2, vertsEx + 4, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x6800001f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1){
		bayoVertSize = 32;
		bayoVertExSize = 4;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x4800000f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
		__set_mapping<big, game>(buffers.mapping2, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x4800000f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 28;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x6800003f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 16;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
		__set_position<big, game>(buffers.position2, vertsEx + 4, bayoVertExSize, numVerts, pretransform);
	}
	else if (hdr.vertexFormat == 0x6000001f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1) {
		bayoVertSize = 44;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, pos, 12, numVerts, pretransform);
		__set_fnormal<big, game>(buffers.normal, verts, bayoVertSize, numVerts, pretransform);
		__set_color<big, game>(buffers.color, verts + 12, bayoVertSize);
		__set_tangents<big, game>(buffers.tangents, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 20, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 24, bayoVertSize);
		__set_fmapping<big, game>(buffers.mapping, verts + 28, bayoVertSize);
		__set_fmapping<big, game>(buffers.mapping2, verts + 36, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x4800001f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 8;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
		__set_mapping<big, game>(buffers.mapping2, vertsEx + 4, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x4800001f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 4;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x4000001f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 36;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, pos, 12, numVerts, pretransform);
		__set_fnormal<big, game>(buffers.normal, verts, bayoVertSize, numVerts, pretransform);
		__set_color<big, game>(buffers.color, verts + 12, bayoVertSize);
		__set_tangents<big, game>(buffers.tangents, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 20, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 24, bayoVertSize);
		__set_fmapping<big, game>(buffers.mapping, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x6000001f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 36;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, pos, 12, numVerts, pretransform);
		__set_fnormal<big, game>(buffers.normal, verts, bayoVertSize, numVerts, pretransform);
		__set_color<big, game>(buffers.color, verts + 12, bayoVertSize);
		__set_tangents<big, game>(buffers.tangents, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 20, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 24, bayoVertSize);
		__set_fmapping<big, game>(buffers.mapping, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x5800000f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
		__set_mapping<big, game>(buffers.mapping2, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x5800000f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 28;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x5800002f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 40;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
		__set_position<big, game>(buffers.position2, verts + 28, bayoVertSize, numVerts, pretransform);
	}
	else if (hdr.vertexFormat == 0x4800000b && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
	}
	else if (hdr.vertexFormat == 0x4800002f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 40;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
		__set_position<big, game>(buffers.position2, verts + 28, bayoVertSize, numVerts, pretransform);
	}
	//Bayonetta 2
	else if (hdr.vertexFormat == 0x6b40001f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 4;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x4b40000f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 28;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x6b40001f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 8;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
		__set_mapping<big, game>(buffers.mapping2, vertsEx + 4, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x4b40000f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_color<big, game>(buffers.color, verts + 24, bayoVertSize);
		__set_mapping<big, game>(buffers.mapping2, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x4b40001f && hdr.numMapping == 0x2 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 8;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
		__set_mapping<big, game>(buffers.mapping2, vertsEx + 4, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x6b40001d && hdr.numMapping == 0x1 && hdr.unknownD == 0x0) {
		bayoVertSize = 32;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x4b40001d && hdr.numMapping == 0x1 && hdr.unknownD == 0x0) {
		bayoVertSize = 32;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x4b40001f && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 4;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x4b40001d && hdr.numMapping == 0x2 && hdr.unknownD == 0x0) {
		bayoVertSize = 32;
		bayoVertExSize = 8;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
		__set_mapping<big, game>(buffers.mapping2, vertsEx + 4, bayoVertExSize);
	}
	else if (hdr.vertexFormat == 0x4b40000d && hdr.numMapping == 0x1 && hdr.unknownD == 0x0) {
		bayoVertSize = 24;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
	}
	else if (hdr.vertexFormat == 0x6b40001d && hdr.numMapping == 0x2 && hdr.unknownD == 0x0) {
		bayoVertSize = 32;
		bayoVertExSize = 8;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
		__set_color<big, game>(buffers.color, vertsEx, bayoVertExSize);
		__set_mapping<big, game>(buffers.mapping2, vertsEx + 4, bayoVertExSize);
	}
	//Vanquish
	else if (hdr.vertexFormat == 0x4b40001d && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x6b40001d && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 32;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
		__set_bone_infos<big, game>(buffers.bone_indexes, verts + 24, bayoVertSize);
		__set_bone_infos<big, game>(buffers.bone_weights, verts + 28, bayoVertSize);
	}
	else if (hdr.vertexFormat == 0x4b40000d && hdr.numMapping == 0x1 && hdr.unknownD == 0x1) {
		bayoVertSize = 24;
		bayoVertExSize = 0;
		__set_position<big, game>(buffers.position, verts, bayoVertSize, numVerts, pretransform);
		__set_mapping<big, game>(buffers.mapping, verts + 12, bayoVertSize);
		__set_normal<big, game>(buffers.normal, verts + 16, bayoVertSize, numVerts, rapi, pretransform);
		__set_tangents<big, game>(buffers.tangents, verts + 20, bayoVertSize, numVerts, rapi, pretransform);
	}
	else {
		DBGLOG("Unknown vertex format: <%0x, %d, %d>!!!\n", hdr.vertexFormat, hdr.numMapping, hdr.unknownD);
	}
}
template <bool big, game_t game>
static void Model_Nier_SetBuffers(bayoDatFile_t &df, noeRAPI_t *rapi, nierWMBHdr<false> &hdr, nierBuffers_t *buffers, modelMatrix_t * pretransform ) {
	BYTE *data = df.data;
	for (int i = 0; i < hdr.numVertexGroups; i++) {
		nierVertexGroup<big> vg((nierVertexGroup_t*)(data + hdr.ofsVertexGroups + i * sizeof(nierVertexGroup_t)));
		int bayoVertSize = vg.sizeVert;
		int bayoVertExSize = vg.sizeVertExData;
		int numVerts = vg.numVerts;
		BYTE *indices = data + vg.ofsIndexBuffer;
		BYTE *verts = data + vg.ofsVerts;
		BYTE *vertsEx = data + vg.ofsVertsExData;
		if (bayoVertSize != 28) {
			DBGLOG("Unknown vertex size format: %d!!!\n", bayoVertSize);
#ifdef _DEBUG
			g_nfn->NPAPI_PopupDebugLog(0);
#endif
			continue;
		}
		DBGLOG("Found vertex groups %d %d %x\n", bayoVertSize, bayoVertExSize, vg.vertExDataFlag);
		DBGLOG("Found unknownB %x\n", hdr.unknownB);
		if (hdr.unknownB == 0xa || hdr.unknownB == 0x8) {
			__set_indices<big, game>(buffers[i].indices, indices, 4);
		}
		else if (hdr.unknownB == 0x2) {
			__set_sindices<big, game>(buffers[i].indices, indices, 2);
		}
		else {
			DBGLOG("Found unknown unknownB %x!!!\n", hdr.unknownB);
#ifdef _DEBUG
			g_nfn->NPAPI_PopupDebugLog(0);
#endif
			continue;
		}
		DBGLOG("Found vertex groups %d %d %x\n", bayoVertSize, bayoVertExSize, vg.vertExDataFlag);
		if (bayoVertExSize == 20 && vg.vertExDataFlag == 0xb) {
			__set_position<big, game>(buffers[i].position, verts, bayoVertSize, numVerts, pretransform);
			__set_tangents<big, game>(buffers[i].tangents, verts + 12, bayoVertSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping, verts + 16, bayoVertSize);
			__set_bone_infos<big, game>(buffers[i].bone_indexes, verts + 20, bayoVertSize);
			__set_bone_infos<big, game>(buffers[i].bone_weights, verts + 24, bayoVertSize);

			__set_mapping<big, game>(buffers[i].mapping2, vertsEx, bayoVertExSize);
			__set_color<big, game>(buffers[i].color, vertsEx + 4, bayoVertExSize);
			__set_hnormal<big, game>(buffers[i].normal, vertsEx + 8, bayoVertExSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping3, vertsEx + 16, bayoVertExSize);
		}
		else if (bayoVertExSize == 12 && vg.vertExDataFlag == 0x7) {
			__set_position<big, game>(buffers[i].position, verts, bayoVertSize, numVerts, pretransform);
			__set_tangents<big, game>(buffers[i].tangents, verts + 12, bayoVertSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping, verts + 16, bayoVertSize);
			__set_bone_infos<big, game>(buffers[i].bone_indexes, verts + 20, bayoVertSize);
			__set_bone_infos<big, game>(buffers[i].bone_weights, verts + 24, bayoVertSize);

			__set_mapping<big, game>(buffers[i].mapping2, vertsEx, bayoVertExSize);
			__set_hnormal<big, game>(buffers[i].normal, vertsEx + 4, bayoVertExSize, numVerts, rapi, pretransform);
		}
		else if (bayoVertExSize == 16 && vg.vertExDataFlag == 0xa) {
			__set_position<big, game>(buffers[i].position, verts, bayoVertSize, numVerts, pretransform);
			__set_tangents<big, game>(buffers[i].tangents, verts + 12, bayoVertSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping, verts + 16, bayoVertSize);
			__set_bone_infos<big, game>(buffers[i].bone_indexes, verts + 20, bayoVertSize);
			__set_bone_infos<big, game>(buffers[i].bone_weights, verts + 24, bayoVertSize);

			__set_mapping<big, game>(buffers[i].mapping2, vertsEx, bayoVertExSize);
			__set_color<big, game>(buffers[i].color, vertsEx + 4, bayoVertExSize);
			__set_hnormal<big, game>(buffers[i].normal, vertsEx + 8, bayoVertExSize, numVerts, rapi, pretransform);
		}
		else if (bayoVertExSize == 12 && vg.vertExDataFlag == 0x5) {
			__set_position<big, game>(buffers[i].position, verts, bayoVertSize, numVerts, pretransform);
			__set_tangents<big, game>(buffers[i].tangents, verts + 12, bayoVertSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping, verts + 16, bayoVertSize);
			__set_mapping<big, game>(buffers[i].mapping2, verts + 20, bayoVertSize);
			__set_color<big, game>(buffers[i].color, verts + 24, bayoVertSize);

			__set_hnormal<big, game>(buffers[i].normal, vertsEx, bayoVertExSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping3, vertsEx + 8, bayoVertExSize);
		}
		else if (bayoVertExSize == 8 && vg.vertExDataFlag == 0x4) {
			__set_position<big, game>(buffers[i].position, verts, bayoVertSize, numVerts, pretransform);
			__set_tangents<big, game>(buffers[i].tangents, verts + 12, bayoVertSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping, verts + 16, bayoVertSize);
			__set_mapping<big, game>(buffers[i].mapping2, verts + 20, bayoVertSize);
			__set_color<big, game>(buffers[i].color, verts + 24, bayoVertSize);

			__set_hnormal<big, game>(buffers[i].normal, vertsEx, bayoVertExSize, numVerts, rapi, pretransform);
		} 
		else if (bayoVertExSize == 16 && vg.vertExDataFlag == 0xe) {
			__set_position<big, game>(buffers[i].position, verts, bayoVertSize, numVerts, pretransform);
			__set_tangents<big, game>(buffers[i].tangents, verts + 12, bayoVertSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping, verts + 16, bayoVertSize);
			__set_mapping<big, game>(buffers[i].mapping2, verts + 20, bayoVertSize);
			__set_color<big, game>(buffers[i].color, verts + 24, bayoVertSize);

			__set_hnormal<big, game>(buffers[i].normal, vertsEx, bayoVertExSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping3, vertsEx + 8, bayoVertExSize);
			__set_mapping<big, game>(buffers[i].mapping4, vertsEx + 12, bayoVertExSize);
		}
		else if (bayoVertExSize == 20 && vg.vertExDataFlag == 0xc) {
			__set_position<big, game>(buffers[i].position, verts, bayoVertSize, numVerts, pretransform);
			__set_tangents<big, game>(buffers[i].tangents, verts + 12, bayoVertSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping, verts + 16, bayoVertSize);
			__set_mapping<big, game>(buffers[i].mapping2, verts + 20, bayoVertSize);
			__set_color<big, game>(buffers[i].color, verts + 24, bayoVertSize);

			__set_hnormal<big, game>(buffers[i].normal, vertsEx, bayoVertExSize, numVerts, rapi, pretransform);
			__set_mapping<big, game>(buffers[i].mapping3, vertsEx + 8, bayoVertExSize);
			__set_mapping<big, game>(buffers[i].mapping4, vertsEx + 12, bayoVertExSize);
			__set_mapping<big, game>(buffers[i].mapping5, vertsEx + 16, bayoVertExSize);
		}
		else {
			DBGLOG("Unknown vertex EX size format: %d %x!!!\n", bayoVertExSize, vg.vertExDataFlag);
#ifdef _DEBUG
			g_nfn->NPAPI_PopupDebugLog(0);
#endif
		}
	}
}

//load a single model from a dat set
template <bool big, game_t game>
static void Model_Bayo_LoadModel(CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models, CArrayList<noesisTex_t *> &givenTextures, modelMatrix_t * pretransform, int sharedtextureoffset = -1)
{
	DBGLOG("Loading %s\n", df.name);
	BYTE *data = df.data;
	int dataSize = df.dataSize;
	if (dataSize < sizeof(bayoWMBHdr_t))
	{
		return;
	}
	bayoWMBHdr<big> hdr((bayoWMBHdr_t *)data);
	if (memcmp(hdr.id, "WMB\0", 4))
	{ //invalid header
		return;
	}
	bool isVanqModel = (hdr.unknownA < 0);
	DBGLOG("Vanquish: %s\n", isVanqModel ? "true" : "false");

	CArrayList<bayoDatFile_t *> motfiles;
	CArrayList<bayoDatFile_t *> texFiles;
	CArrayList<bayoDatFile_t *> expfile;
	CArrayList<noesisTex_t *> textures;
	CArrayList<noesisMaterial_t *> matList;
	CArrayList<noesisMaterial_t *> matListLightMap;
	CArrayList<noesisMaterial_t *> totMatList;
	CArrayList<noesisAnim_t *> animList;

	buffers_t buffers;

	bool hasExMatInfo;

	if (givenTextures.Num() == 0) {
		Model_Bayo_GetTextureBundle<game>(texFiles, dfiles, df, rapi);
		if (texFiles.Num() > 0)
		{
			for (int i = 0; i < texFiles.Num(); i++) {
				DBGLOG("Found texture bundle %s\n", texFiles[i]->name);
			}
			if (game == BAYONETTA2 || game == NIER_AUTOMATA) {
				CArrayList<bayoDatFile_t *> newTexFiles;
				for (int i = 0; i < texFiles.Num(); i += 2) {
					CArrayList<bayoDatFile_t *> newTexFiles;
					newTexFiles.Append(texFiles[i]);
					newTexFiles.Append(texFiles[i + 1]);
					Model_Bayo_LoadTextures<big, game>(textures, newTexFiles, rapi);
				}
			}
			else {
				Model_Bayo_LoadTextures<big, game>(textures, texFiles, rapi);
			}
		}
	}
	else {
		for (int i = 0; i < givenTextures.Num(); i++) {
			textures.Append(givenTextures[i]);
		}
	}


	Model_Bayo_LoadMaterials<big,game>(hdr, textures, hasExMatInfo, matList, matListLightMap, totMatList, data, rapi, sharedtextureoffset);
	void *pgctx = rapi->rpgCreateContext();
	rapi->rpgSetOption(RPGOPT_BIGENDIAN, big);
	rapi->rpgSetOption(RPGOPT_TRIWINDBACKWARD, true);

	int numBones;
	short int * animBoneTT;
	modelBone_t *bones = Model_Bayo_CreateBones<big>(hdr, data, rapi, numBones, animBoneTT);

	Model_Bayo_GetMotionFiles(dfiles, df, rapi, motfiles);
	Model_Bayo_GetEXPFile(dfiles, df, rapi, expfile);
	if (bones) {
		Model_Bayo_LoadMotions<big, game>(animList, motfiles, expfile, bones, numBones, rapi, animBoneTT, data + hdr.ofsBoneDataA);
		Model_Bayo_LoadExternalMotions<big, game>(animList, df, expfile, bones, numBones, rapi, animBoneTT, data + hdr.ofsBoneDataA);
	}

	Model_Bayo_SetBuffers<big,game>(df, rapi, hdr, buffers, pretransform);

	BYTE *meshStart = data + hdr.ofsMeshes;
	int *meshOfsList = (int *)(data + hdr.ofsMeshOfs);
	DBGLOG("Found %d meshes\n", hdr.numMeshes);
	for (int i = 0; i < hdr.numMeshes; i++)
	{
		//if (i != 0) continue;
		int meshOfs = meshOfsList[i];
		if (big) LITTLE_BIG_SWAP(meshOfs);
		wmbMesh<big> mesh((wmbMesh_t *)(meshStart+meshOfs));
		DBGLOG("\t%3d: id: %d, offset: %x, name: %s\n", i, mesh.id , hdr.ofsMeshes+meshOfs, mesh.name);
		int *batchOfsList = (int *)(meshStart+meshOfs+mesh.batchOfs);
		for (int j = 0; j < mesh.numBatch; j++)
		{
			char batch_name[256];
			DBGLOG("\t\t%3d: ", j);
			int batchOfs = batchOfsList[j];
			if (big) LITTLE_BIG_SWAP(batchOfs);
			BYTE *batchData = (BYTE *)batchOfsList + batchOfs;
			wmbBatch<big> batch((wmbBatch_t *)batchData);
			DBGLOG("%d, %x, %x, %x, %x, %x, ", batch.id, batch.unknownB, batch.unknownC, batch.unknownDB, batch.unknownE1, batch.unknownE2);
			if ((game == BAYONETTA && batch.unknownE1 == 0x20 && batch.unknownE2 == 0x0f) || (game == BAYONETTA2 && batch.unknownE1 == 0x30)) {
				sprintf_s(batch_name, 256, "%02d(%s)_%02d_s", i, mesh.name, j);
			}
			else {
				sprintf_s(batch_name, 256, "%02d(%s)_%02d", i, mesh.name, j);
			}
			rapi->rpgSetName(rapi->Noesis_PooledString(batch_name));
			/*if (batch.unknownE == 0xf20) {
				DBGLOG("skipped (shadow model)\n");
				continue; // shadow meshes
			}*/
			int numBoneRefs = *((int *)(batchData+sizeof(wmbBatch_t)));
			if (big) LITTLE_BIG_SWAP(numBoneRefs);
			int *boneRefDst = NULL;
			if (numBoneRefs > 0 && bones && buffers.bone_indexes.address)
			{ //set up reference map
				BYTE *boneRefs = batchData+sizeof(wmbBatch_t)+sizeof(int);
				boneRefDst = (int *)rapi->Noesis_UnpooledAlloc(sizeof(int)*numBoneRefs);
				for (int k = 0; k < numBoneRefs; k++)
				{
					boneRefDst[k] = boneRefs[k];
				}
			}
			rapi->rpgSetBoneMap(boneRefDst);
			int vertOfs = batch.vertOfs;
			DBGLOG("vertAddr: %p, ", buffers.position.address + vertOfs*buffers.position.stride);
			//bind positions
			rapi->rpgBindPositionBuffer(buffers.position.address + vertOfs * buffers.position.stride, buffers.position.type, buffers.position.stride);
			//bind normals
			rapi->rpgBindNormalBuffer(buffers.normal.address + vertOfs * buffers.normal.stride, buffers.normal.type, buffers.normal.stride);
			//bind tangents
			if (false && batch.primType == 4) {
				DBGLOG("\n");
				modelTan4_t	*tangents = rapi->rpgCalcTangents(batch.vertEnd - vertOfs, batch.numIndices, batchData + batch.ofsIndices,
					RPGEODATA_USHORT, sizeof(unsigned short) * (batch.primType == 4 ? 3 : 1),
					buffers.position.address + vertOfs * buffers.position.stride, buffers.position.type, buffers.position.stride,
					buffers.normal.address + vertOfs * buffers.normal.stride, buffers.normal.type, buffers.normal.stride,
					buffers.mapping.address + vertOfs * buffers.mapping.stride, buffers.mapping.type, buffers.mapping.stride,
					NULL, true);
				rapi->rpgBindTangentBuffer(tangents, RPGEODATA_FLOAT, 16);
				for (int k = batch.vertStart - batch.vertOfs; k < batch.vertEnd - batch.vertOfs; k++) {
					modelTan4_t * ta = (modelTan4_t *)(buffers.tangents.address + vertOfs * buffers.tangents.stride);
					modelVert_t * n = (modelVert_t *)(buffers.normal.address + vertOfs * buffers.normal.stride);
					DBGLOG("%d no: %f %f %f\n", k, n[k].x, n[k].y, n[k].z);
					DBGLOG("%d gt: %f %f %f %f\n", k, ta[k].v[0], ta[k].v[1], ta[k].v[2], ta[k].v[3]);
					DBGLOG("%d nt: %f %f %f %f\n", k, tangents[k].v[0], tangents[k].v[1], tangents[k].v[2], tangents[k].v[3]);
					DBGLOG("%d  d: %f %f %f %f\n", k, abs(ta[k].v[0] - tangents[k].v[0]), abs(ta[k].v[1] - tangents[k].v[1]), abs(ta[k].v[2] - tangents[k].v[2]), abs(ta[k].v[3] - tangents[k].v[3]));
					modelVert_t bt;
					g_mfn->Math_CrossProduct((float*)&n[k], ta[k].v, (float *)&bt);
					g_mfn->Math_VecScale((float *)&bt, ta[k].v[3]);
					DBGLOG("%d gb: %f %f %f\n", k, bt.x, bt.y, bt.z);
					g_mfn->Math_CrossProduct((float*)&n[k], tangents[k].v, (float *)&bt);
					g_mfn->Math_VecScale((float *)&bt, tangents[k].v[3]);
					DBGLOG("%d nb: %f %f %f\n", k, bt.x, bt.y, bt.z);
				}
			}
			rapi->rpgBindTangentBuffer(buffers.tangents.address + vertOfs * buffers.tangents.stride, buffers.tangents.type, buffers.tangents.stride);
			//bind uv's
			rapi->rpgBindUV1Buffer(buffers.mapping.address + vertOfs * buffers.mapping.stride, buffers.mapping.type, buffers.mapping.stride);
			if (buffers.mapping2.address) {
				rapi->rpgBindUV2Buffer(buffers.mapping2.address + vertOfs * buffers.mapping2.stride, buffers.mapping2.type, buffers.mapping2.stride);
			}
			else {
				rapi->rpgBindUV2Buffer(NULL, RPGEODATA_HALFFLOAT, 0);
			}
			if (bones && buffers.bone_indexes.address)
			{ //bind weight data
				rapi->rpgBindBoneIndexBuffer(buffers.bone_indexes.address + vertOfs * buffers.bone_indexes.stride, buffers.bone_indexes.type, buffers.bone_indexes.stride, 4);
				rapi->rpgBindBoneWeightBuffer(buffers.bone_weights.address + vertOfs * buffers.bone_weights.stride, buffers.bone_weights.type, buffers.bone_weights.stride, 4);
			}

			int texID = (hasExMatInfo) ? batch.unknownC : batch.texID;
			char *matName = (texID < matList.Num()) ? matList[texID]->name : NULL;
			//bind color if needed for multipass materials for now
			if (matName && buffers.color.address && (matList[texID]->ex->userTag[0] & BAYONETTA_MULTIPASS)) {
				rapi->rpgBindColorBuffer(buffers.color.address + vertOfs * buffers.color.stride, buffers.color.type, buffers.color.stride, 4);
			}
			else {
				rapi->rpgBindColorBuffer(NULL, RPGEODATA_BYTE, 0, 0);
			}
			DBGLOG("matName: %s\n", matName);
			rapi->rpgSetMaterial(matName);
			if(texID < matList.Num() && matList[texID]->ex->pUvScaleBias) {
				rapi->rpgSetUVScaleBias(matList[texID]->ex->pUvScaleBias, matList[texID]->ex->pUvScaleBias + 2);
			} else {
				rapi->rpgSetUVScaleBias(NULL, NULL);
			}
			DBGLOG("primType: %d, numIndices: %d\n", batch.primType, batch.numIndices);
			rpgeoPrimType_e primType = (batch.primType == 4) ? RPGEO_TRIANGLE : RPGEO_TRIANGLE_STRIP;
			rapi->rpgCommitTriangles(batchData+batch.ofsIndices, RPGEODATA_USHORT, batch.numIndices, primType, true);
			rapi->rpgSmoothTangents(NULL);
			if (boneRefDst)
			{ //reference map is no longer needed once triangles have been committed
				rapi->rpgSetBoneMap(NULL);
				rapi->Noesis_UnpooledFree(boneRefDst);
			}
		}
	}

	noesisMatData_t *md = rapi->Noesis_GetMatDataFromLists(totMatList, textures);
	rapi->rpgSetExData_Materials(md);
	if (bones) {
		rapi->rpgSetExData_Bones(bones, numBones + 1);
	}

	int anims_num = animList.Num();
    noesisAnim_t *anims = rapi->Noesis_AnimFromAnimsList(animList, anims_num);
	//for(int i = 0; i < anims_num; i++) {
	//	DBGLOG("anim: %s, size: %d, flag: %d\n", animList[i]->filename, animList[i]->dataLen, animList[i]->flags & NANIMFLAG_FILENAMETOSEQ);
	//	DBGLOG("seq: %p, size: %d\n", (anims+i)->aseq, (anims+i)->dataLen );
	//}
    rapi->rpgSetExData_AnimsNum(anims, 1);
	DBGLOG("Found %d anims\n", anims_num);
	noesisModel_t *mdl = rapi->rpgConstructModel();
	if( mdl ) {
		models.Append(mdl);
	}
	
	rapi->rpgDestroyContext(pgctx);

	animList.Clear();
	matList.Clear();
	motfiles.Clear();
	texFiles.Clear();
	textures.Clear();
	matListLightMap.Clear();
	totMatList.Clear();
}
template <>
static void Model_Bayo_LoadModel<false, NIER_AUTOMATA>(CArrayList<bayoDatFile_t> &dfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models, CArrayList<noesisTex_t *> &givenTextures, modelMatrix_t * pretransform, int sharedtextureoffset) {
	static const bool big = false;
	static const game_t game = NIER_AUTOMATA;
	DBGLOG("Loading %s\n", df.name);
	BYTE *data = df.data;
	int dataSize = df.dataSize;
	if (dataSize < sizeof(nierWMBHdr_t))
	{
		return;
	}
	nierWMBHdr<big> hdr((nierWMBHdr_t *)data);
	if (memcmp(hdr.id, "WMB3", 4))
	{ //invalid header
		return;
	}

	CArrayList<bayoDatFile_t *> motfiles;
	CArrayList<bayoDatFile_t *> texFiles;
	CArrayList<bayoDatFile_t *> expfile;
	CArrayList<noesisTex_t *> textures;
	CArrayList<noesisMaterial_t *> matList;
	CArrayList<noesisMaterial_t *> matListLightMap;
	CArrayList<noesisMaterial_t *> totMatList;
	CArrayList<noesisAnim_t *> animList;

	nierBuffers_t *buffers = (nierBuffers_t *)rapi->Noesis_UnpooledAlloc( hdr.numVertexGroups * sizeof(nierBuffers_t));

	if (givenTextures.Num() == 0) {
		Model_Bayo_GetTextureBundle<game>(texFiles, dfiles, df, rapi);
		if (texFiles.Num() > 0)
		{
			for (int i = 0; i < texFiles.Num(); i++) {
				DBGLOG("Found texture bundle %s\n", texFiles[i]->name);
			}
	
			CArrayList<bayoDatFile_t *> newTexFiles;
			for (int i = 0; i < texFiles.Num(); i += 2) {
				CArrayList<bayoDatFile_t *> newTexFiles;
				newTexFiles.Append(texFiles[i]);
				newTexFiles.Append(texFiles[i + 1]);
				Model_Bayo_LoadTextures<big, game>(textures, newTexFiles, rapi);
			}
		}
	}
	else {
		for (int i = 0; i < givenTextures.Num(); i++) {
			textures.Append(givenTextures[i]);
		}
	}

	Model_Nier_LoadMaterials<big, game>(hdr, textures, matList, matListLightMap, totMatList, data, rapi);
	Model_Nier_SetBuffers<big, game>(df, rapi, hdr, buffers, pretransform);

	void *pgctx = rapi->rpgCreateContext();
	rapi->rpgSetOption(RPGOPT_BIGENDIAN, big);
	rapi->rpgSetOption(RPGOPT_TRIWINDBACKWARD, true);

	int numBones;
	short int * animBoneTT;
	modelBone_t *bones = Model_Nier_CreateBones<big>(hdr, data, rapi, numBones, animBoneTT);
	Model_Bayo_GetMotionFiles(dfiles, df, rapi, motfiles);
	Model_Bayo_GetEXPFile(dfiles, df, rapi, expfile);
	if (bones) {
		Model_Bayo_LoadMotions<big, game>(animList, motfiles, expfile, bones, numBones, rapi, animBoneTT, data + hdr.ofsBones);
		Model_Bayo_LoadExternalMotions<big, game>(animList, df, expfile, bones, numBones, rapi, animBoneTT, data + hdr.ofsBones);
	}

	DBGLOG("Found %d lods\n", hdr.numLods);
	DBGLOG("Found %d meshes\n", hdr.numMeshes);
	DBGLOG("Found %d batches\n", hdr.numBatches);

	nierMesh_t *meshes = (nierMesh_t *)(data + hdr.ofsMeshes);
	nierLod_t *lods = (nierLod_t *)(data + hdr.ofsLods);
	nierBatch_t *batches = (nierBatch_t *)(data + hdr.ofsBatches);
	nierBoneSet_t *boneSets = (nierBoneSet_t *)(data + hdr.ofsBoneSets);
	unsigned int *boneMap = (unsigned int *)(data + hdr.ofsBoneMap);
	for (int i = 0; i < std::min(hdr.numLods, 1); i++)
	{
		nierLod<big> lod(lods + i);
		DBGLOG("LOD %d: name: %s\n", i, (char*)(data + lod.ofsName));
		unsigned int batchStart = lod.batchStart;
		nierBatchInfo_t * batchInfos = (nierBatchInfo_t *)(data + lod.ofsBatchInfos);
		for (unsigned int j = 0; j < lod.numBatchInfos; j++) {
			char batch_name[256];
			nierBatchInfo<big> batchInfo(batchInfos + j);
			nierBatch<big> batch(batches + batchStart + j);
			nierMesh<big> mesh(meshes + batchInfo.meshIndex);
			nierBoneSet<big> boneSet(boneSets + batch.boneSetIndex);

			unsigned int vertexGroupIndex = batchInfo.vertexGroupIndex;
			unsigned int materialIndex = batchInfo.materialIndex;

			sprintf_s(batch_name, 256, "%s_%d_%s", (char *)(data + lod.ofsName), j, (char *)(data + mesh.ofsName));
			DBGLOG("\t%s\n", batch_name);
			int *boneIndices = NULL;
			if (bones && batch.boneSetIndex >= 0) {
				boneIndices = (int *)rapi->Noesis_UnpooledAlloc(boneSet.numBoneIndices * sizeof(int));
				unsigned short *originalBoneIndices = (unsigned short *)(data + boneSet.ofsBoneSet);
				for (unsigned int j = 0; j < boneSet.numBoneIndices; j++) {
					unsigned short sourceIndex;
					sourceIndex = originalBoneIndices[j];
					if (big) {
						LITTLE_BIG_SWAP(sourceIndex);
					}
					boneIndices[j] = boneMap[sourceIndex];
				}
			}

			rapi->rpgSetName(rapi->Noesis_PooledString(batch_name));
			rapi->rpgSetBoneMap(boneIndices);
			if (bones && buffers[vertexGroupIndex].bone_indexes.address)
			{
				rapi->rpgBindBoneIndexBuffer(buffers[vertexGroupIndex].bone_indexes.address, buffers[vertexGroupIndex].bone_indexes.type, buffers[vertexGroupIndex].bone_indexes.stride, 4);
				rapi->rpgBindBoneWeightBuffer(buffers[vertexGroupIndex].bone_weights.address, buffers[vertexGroupIndex].bone_weights.type, buffers[vertexGroupIndex].bone_weights.stride, 4);
			}
			rapi->rpgBindPositionBuffer(buffers[vertexGroupIndex].position.address, buffers[vertexGroupIndex].position.type, buffers[vertexGroupIndex].position.stride);
			rapi->rpgBindNormalBuffer(buffers[vertexGroupIndex].normal.address, buffers[vertexGroupIndex].normal.type, buffers[vertexGroupIndex].normal.stride);
			rapi->rpgBindUV1Buffer(buffers[vertexGroupIndex].mapping.address, buffers[vertexGroupIndex].mapping.type, buffers[vertexGroupIndex].mapping.stride);
			rapi->rpgSetMaterial(matList[materialIndex]->name);
			rapi->rpgCommitTriangles(buffers[vertexGroupIndex].indices.address + batch.indexStart * buffers[vertexGroupIndex].indices.stride, buffers[vertexGroupIndex].indices.type, batch.numIndices, RPGEO_TRIANGLE, true);

			if (bones) {
				rapi->rpgSetBoneMap(NULL);
				rapi->Noesis_UnpooledFree(boneIndices);
			}
		}

	}

	noesisMatData_t *md = rapi->Noesis_GetMatDataFromLists(totMatList, textures);
	rapi->rpgSetExData_Materials(md);
	if (bones) {
		rapi->rpgSetExData_Bones(bones, numBones + 1);
	}
	int anims_num = animList.Num();
	DBGLOG("Found %d anims\n", anims_num);
	if (anims_num > 700) {
		DBGLOG("Only displaying 700 first animations");
		anims_num = 700;
	}

	noesisAnim_t *anims = rapi->Noesis_AnimFromAnimsList(animList, anims_num);
	if(anims) {
		rapi->rpgSetExData_AnimsNum(anims, 1);
	} else if (animList.Num() > 0) {
		DBGLOG("Could not create animation block\n");
	}

	noesisModel_t *mdl = rapi->rpgConstructModel();
	if (mdl) {
		models.Append(mdl);
	}

	rapi->rpgDestroyContext(pgctx);

	rapi->Noesis_UnpooledFree(buffers);
	animList.Clear();
	matList.Clear();
	motfiles.Clear();
	texFiles.Clear();
	textures.Clear();
	matListLightMap.Clear();
	totMatList.Clear();
}
static void Model_Bayo_CreatePreTransformMatrix(float * transform, modelMatrix_t &m) {
	float translate[3];
	float rotate[3];
	float scale[3];
	//int rotate_order[3] = {2,1,0};
	float rotate_coeff[3] = { -1.0, 1.0, -1.0 };
	float translate_coeff[3] = { 1.0, 1.0, 1.0 };
	float scale_coeff[3] = { 1.0, 1.0, 1.0 };

	m = g_identityMatrix;

	for (int i = 0; i < 3; i++) {
		translate[i] = translate_coeff[i] * transform[i];
	}
	for (int i = 3, j = 0; i < 6; i++, j++) {
		rotate[j] = rotate_coeff[j] * transform[i] * g_flRadToDeg;
	}
	for (int i = 6, j = 0; i < 9; i++, j++) {
		scale[j] = scale_coeff[j] * transform[i];
	}

	DBGALOG("\tpre translate: %f, %f, %f\n", translate[0], translate[1], translate[2]);
	g_mfn->Math_TranslateMatrix(&m, translate);

	DBGALOG("\tpre rotate: %f, %f, %f\n", rotate[0], rotate[1], rotate[2]);

	g_mfn->Math_RotateMatrix(&m, rotate[2], 0.0f, 0.0f, 1.0f);
	g_mfn->Math_RotateMatrix(&m, rotate[1], 0.0f, 1.0f, 0.0f);
	g_mfn->Math_RotateMatrix(&m, rotate[0], 1.0f, 0.0f, 0.0f);

	DBGALOG("\tpre scale: %f, %f, %f\n", scale[0], scale[1], scale[2]);
	for (int i = 0; i < 3; i++) {
		m.x1[i] *= scale[i];
		m.x2[i] *= scale[i];
		m.x3[i] *= scale[i];
	}
}
template <bool big, game_t game>
static void Model_Bayo_LoadScenery(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models);
template <bool big>
static void Model_Bayo2_LoadScenery(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
	DBGLOG("Loading %s\n", df.name);
	bayo2SCRHdr<big> hdr((bayo2SCRHdr_t *)df.data);
	if (memcmp(hdr.id, "SCR\0", 4))
	{ //invalid header
		DBGLOG("Invalid SCR file\n");
		return;
	}
	CArrayList<bayoDatFile_t *> texFiles;
	CArrayList<noesisTex_t *> textures;
	Model_Bayo2_GetSCRTextureBundle(texFiles, olddfiles, df, rapi);
	if (texFiles.Num() > 0)
	{
		for (int i = 0; i < texFiles.Num(); i++) {
			DBGLOG("Found texture bundle %s\n", texFiles[i]->name);
		}
		Model_Bayo_LoadTextures<big, BAYONETTA2>(textures, texFiles, rapi);
	}
	char scrName[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(scrName, df.name);
	Model_Bayo_LoadSharedTextures<big, BAYONETTA2>(textures, scrName, rapi);

	CArrayList<bayoDatFile_t> dfiles;

	unsigned int * ofsOffsetsModels = (unsigned int *)(df.data + hdr.ofsOffsetsModels);

	DBGLOG("found %d models in %s\n", hdr.numModels, df.name);
	for (int i = 0; i < hdr.numModels; i++) {
		bayoDatFile_t modelFile;
		int dscrOffset = ofsOffsetsModels[i];
		if (big) {
			LITTLE_BIG_SWAP(dscrOffset);
		}
		bayo2SCRModelDscr<big> modelDscr((bayo2SCRModelDscr_t *)(df.data + dscrOffset));
		char modelName[69];
		char fileName[69];
		memset(modelName, 0, 69);
		for (int j = 0; j < 64; j++) {
			modelName[j] = modelDscr.name[j];
		}
		snprintf(fileName, 69, "%s.wmb", modelName);
		DBGLOG(" model name: %s, ", fileName);
		modelFile.name = rapi->Noesis_PooledString(fileName);
		modelFile.data = df.data + modelDscr.offset;
		if (i < (hdr.numModels - 1)) {
			int nextDscrOffset = ofsOffsetsModels[i + 1];
			if (big) {
				LITTLE_BIG_SWAP(nextDscrOffset);
			}
			modelFile.dataSize = nextDscrOffset - modelDscr.offset;
		}
		else {
			modelFile.dataSize = df.dataSize - modelDscr.offset;
		}
		DBGLOG("start: %d, size: %d\n", modelDscr.offset, modelFile.dataSize);
		modelMatrix_t m;
		Model_Bayo_CreatePreTransformMatrix(modelDscr.transform, m);
		Model_Bayo_LoadModel<big, BAYONETTA2>(dfiles, modelFile, rapi, models, textures, &m);
	}
	rapi->SetPreviewOption("drawAllModels", "1");
}
template <>
static void Model_Bayo_LoadScenery<true, BAYONETTA2>(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
	Model_Bayo2_LoadScenery<true>(olddfiles, df, rapi, models);
}
template <>
static void Model_Bayo_LoadScenery<false, BAYONETTA2>(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
	Model_Bayo2_LoadScenery<false>(olddfiles, df, rapi, models);
}
template <bool big>
static void Model_Bayo_LoadSceneryBayo1(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
	DBGLOG("Loading %s\n", df.name);
	bayoSCRHdr<big> hdr((bayoSCRHdr_t *)df.data);
	if (memcmp(hdr.id, "SCR\0", 4))
	{ //invalid header
		DBGLOG("Invalid SCR file\n");
		return;
	}
	bayoDatFile_t textureFile;
	textureFile.name = rapi->Noesis_PooledString("dummy.wtb");
	textureFile.data = df.data + hdr.ofsTextures;
	textureFile.dataSize = df.dataSize - hdr.ofsTextures;
	CArrayList<bayoDatFile_t> dfiles;
	CArrayList<bayoDatFile_t *> texfiles;
	texfiles.Append(&textureFile);
	CArrayList<noesisTex_t *> textures;
	int sharedtexturesoffset = -1;
	Model_Bayo_LoadTextures<big, BAYONETTA>(textures, texfiles, rapi);
	sharedtexturesoffset = textures.Num();
	char scrName[MAX_NOESIS_PATH];
	rapi->Noesis_GetExtensionlessName(scrName, df.name);
	Model_Bayo_LoadSharedTextures<big, BAYONETTA>(textures, scrName, rapi);
	DBGLOG("found %d models in %s\n", hdr.numModels, df.name);
	for (int i = 0; i < hdr.numModels; i++) {
		bayoDatFile_t modelFile;
		int dscrOffset = sizeof(bayoSCRHdr_t) + i * sizeof(bayoSCRModelDscr_t);
		DBGLOG("offset: %x\n", dscrOffset);
		bayoSCRModelDscr<big> modelDscr((bayoSCRModelDscr_t *)(df.data + dscrOffset));
		char modelName[21];
		char fileName[21];
		memset(modelName, 0, 21);
		for (int j = 0; j < 16; j++) {
			modelName[j] = modelDscr.name[j];
		}
		snprintf(fileName, 21, "%s.wmb", modelName);
		DBGLOG("%d - %d: model name: %s, ", i, models.Num(), fileName);
		modelFile.name = rapi->Noesis_PooledString(fileName);
		modelFile.data = df.data + dscrOffset + modelDscr.offset;
		if (i < (hdr.numModels - 1)) {
			int nextDscrOffset = sizeof(bayoSCRHdr_t) + (i + 1) * sizeof(bayoSCRModelDscr_t);
			bayoSCRModelDscr<big> nextModelDscr((bayoSCRModelDscr_t *)(df.data + nextDscrOffset));
			modelFile.dataSize = (nextDscrOffset + nextModelDscr.offset) - (dscrOffset + modelDscr.offset);
		}
		else {
			modelFile.dataSize = hdr.ofsTextures - (dscrOffset + modelDscr.offset);
		}
		DBGLOG("start: %d, size: %d\n", dscrOffset + modelDscr.offset, modelFile.dataSize);
		modelMatrix_t m;
		Model_Bayo_CreatePreTransformMatrix(modelDscr.transform, m);
		Model_Bayo_LoadModel<big, BAYONETTA>(dfiles, modelFile, rapi, models, textures, &m, sharedtexturesoffset);
	}
	rapi->SetPreviewOption("drawAllModels", "1");
}
template <>
static void Model_Bayo_LoadScenery<false, BAYONETTA>(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
	Model_Bayo_LoadSceneryBayo1<false>(olddfiles, df, rapi, models);
}
template <>
static void Model_Bayo_LoadScenery<true, BAYONETTA>(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
	Model_Bayo_LoadSceneryBayo1<true>(olddfiles, df, rapi, models);
}
template <>
static void Model_Bayo_LoadScenery<false, VANQUISH>(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
}
template <>
static void Model_Bayo_LoadScenery<false, NIER_AUTOMATA>(CArrayList<bayoDatFile_t> &olddfiles, bayoDatFile_t &df, noeRAPI_t *rapi, CArrayList<noesisModel_t *> &models) {
}
//load it
template <bool big, game_t game>
noesisModel_t *Model_Bayo_Load(BYTE *fileBuffer, int bufferLen, int &numMdl, noeRAPI_t *rapi)
{
	CArrayList<bayoDatFile_t> dfiles;
	DBGLOG("Loading model\n");
	//create a list of resources
	Model_Bayo_GetDATEntries<big>(dfiles, fileBuffer, bufferLen);
	//for vanquish, append any matching dtt files (they're just paired dat files)
	char *inFile = rapi->Noesis_GetInputName();
	BYTE *dttFile = NULL;
	if (inFile && inFile[0] && game != NIER_AUTOMATA)
	{
		char fn[MAX_NOESIS_PATH];
		rapi->Noesis_GetExtensionlessName(fn, inFile);
		strcat_s(fn, MAX_NOESIS_PATH, ".dtt");
		int dttLen = 0;
		dttFile = (BYTE *)rapi->Noesis_ReadFile(fn, &dttLen);
		if (dttFile && dttLen > 0)
		{
			Model_Bayo_GetDATEntries<big>(dfiles, dttFile, dttLen);
		}
	}
	//for Nier, append any matching dat files (they're just paired dtt files) :)
	if (inFile && inFile[0] && game == NIER_AUTOMATA)
	{
		char fn[MAX_NOESIS_PATH];
		rapi->Noesis_GetExtensionlessName(fn, inFile);
		strcat_s(fn, MAX_NOESIS_PATH, ".dat");
		int dttLen = 0;
		dttFile = (BYTE *)rapi->Noesis_ReadFile(fn, &dttLen);
		if (dttFile && dttLen > 0)
		{
			Model_Bayo_GetDATEntries<big>(dfiles, dttFile, dttLen);
		}
	}
	CArrayList<noesisModel_t *> models;
	DBGLOG("Have %d files\n", dfiles.Num());
	for (int i = 0; i < dfiles.Num(); i++)
	{
		bayoDatFile_t &df = dfiles[i];
		if (rapi->Noesis_CheckFileExt(df.name, ".wmb"))
		{ //it's a model
			CArrayList<noesisTex_t *> textures;
			Model_Bayo_LoadModel<big, game>(dfiles, df, rapi, models, textures, NULL);
		}
		else if (rapi->Noesis_CheckFileExt(df.name, ".scr"))
		{ //it's a scenery
			Model_Bayo_LoadScenery<big, game>(dfiles, df, rapi, models);
		}
	}
	DBGFLUSH();
	if (models.Num() <= 0)
	{
		return NULL;
	}
	noesisModel_t *mdlList = rapi->Noesis_ModelsFromList(models, numMdl);
	models.Clear();
	if (dttFile)
	{ //free the dtt file memory
		rapi->Noesis_UnpooledFree(dttFile);
	}
	return mdlList;
}

//called by Noesis to init the plugin
bool NPAPI_InitLocal(void)
{
	int fh = g_nfn->NPAPI_Register("Bayonetta PC Model", ".dat");
	if (fh < 0)
	{
		return false;
	}
	int fh_b2 = g_nfn->NPAPI_Register("Bayonetta 2 Big Endian Model", ".dat");
	if (fh_b2 < 0)
	{
		return false;
	}
	int fh_2 = g_nfn->NPAPI_Register("Bayonetta 2 Switch Model", ".dat");
	if (fh_2 < 0)
	{
		return false;
	}
	int fh_b = g_nfn->NPAPI_Register("Bayonetta Big Endian Model (WiiU)", ".dat");
	if (fh_b < 0)
	{
		return false;
	}
	int fh_v = g_nfn->NPAPI_Register("Vanquish PC Model", ".dat");
	if (fh_v < 0)
	{
		return false;
	}
	int fh_n = g_nfn->NPAPI_Register("Nier Automata PC Model", ".dtt");
	if (fh_v < 0)
	{
		return false;
	}
	OPENLOG();
	bayoSetMatTypes();
	//set the data handlers for this format
	g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh, Model_Bayo_Check<false, BAYONETTA>);
	g_nfn->NPAPI_SetTypeHandler_LoadModel(fh, Model_Bayo_Load<false, BAYONETTA>);
	g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh_b, Model_Bayo_Check<true, BAYONETTA>);
	g_nfn->NPAPI_SetTypeHandler_LoadModel(fh_b, Model_Bayo_Load<true, BAYONETTA>);
	g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh_b2, Model_Bayo_Check<true, BAYONETTA2>);
	g_nfn->NPAPI_SetTypeHandler_LoadModel(fh_b2, Model_Bayo_Load<true, BAYONETTA2>);
	g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh_2, Model_Bayo_Check<false, BAYONETTA2>);
	g_nfn->NPAPI_SetTypeHandler_LoadModel(fh_2, Model_Bayo_Load<false, BAYONETTA2>);
	g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh_v, Model_Bayo_Check<false, VANQUISH>);
	g_nfn->NPAPI_SetTypeHandler_LoadModel(fh_v, Model_Bayo_Load<false, VANQUISH>);
	g_nfn->NPAPI_SetTypeHandler_TypeCheck(fh_n, Model_Bayo_Check<false, NIER_AUTOMATA>);
	g_nfn->NPAPI_SetTypeHandler_LoadModel(fh_n, Model_Bayo_Load<false, NIER_AUTOMATA>);
	//g_nfn->NPAPI_PopupDebugLog(0);
	return true;
}

//called by Noesis before the plugin is freed
void NPAPI_ShutdownLocal(void)
{
	CLOSELOG();
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	return TRUE;
}

//! Predefined filter operations.
enum FilterMorphOperation
{
	MORPH_ADD = 0,			//!< add to existing
	MORPH_NOISE,			//!< add noise to existing
	MORPH_EXACT,			//!< set exact value
	MORPH_SMOOTH,			//!< smoothing
	MORPH_COMBINE			//!< combine with other map, take map from TerrainFilterDesc.GetCombinePtr()
}

/*!
Height shape of filter. Describes how values between outer (= 0) and inner
(= 1) radius will be modifed.
*/
enum FilterMorphLerpFunc
{
	FUNC_LINEAR = 0,		//!< linear interpolation
	FUNC_SIN,				//!< sine
	FUNC_INV_COS,			//!< inverse cosine
	FUNC_SMOOTH,			//!< hermit interpolation
	FUNC_SPHERE,			//!< sphere interpolation
	FUNC_INV_SPHERE			//!< inverse sphere interpolation
}

//! 2D shape of filter.
enum FilterMorphShape
{
	SHAPE_ROUND = 0,		//!< generate round kernel
	SHAPE_SQUARE,			//!< generate square kernel
	SHAPE_USER				//!< user shape
}

//! Filtering type of given user shape.
enum UserShapeFilter
{
	NEAREST = 0,	//!< nearest
	BILINEAR,		//!< bilinear
	BICUBIC			//!< bicubic
}

enum TerrainToolType
{
	TTT_NONE,

	TTT_HEIGHT_ADD,
	TTT_HEIGHT_EXACT,
	TTT_HEIGHT_SMOOTH,
	TTT_HEIGHT_NOISE,
	TTT_HEIGHT_USER,

	TTT_LAYER_PAINT,

	TTT_COUNT,
}

enum ETerrainNoiseType
{
	RANDOM,
	PERLIN
}

class TerrainToolDesc : Managed
{
	float fOuterSize;
	float fInnerSizePercent;
	float fAngle;
}

//--------------------------------------------------------------
class TerrainToolDesc_HeightAdd : TerrainToolDesc
{
	float fAdd;
}

//--------------------------------------------------------------
class TerrainToolDesc_HeightExact : TerrainToolDesc
{
	float fExactHeight;
}

//--------------------------------------------------------------
class TerrainToolDesc_HeightSmooth : TerrainToolDesc
{
	float fPower;
}

//--------------------------------------------------------------
class TerrainToolDesc_HeightNoise : TerrainToolDesc
{
	float fMaxSize;
	ETerrainNoiseType eNoiseType;
}

//--------------------------------------------------------------
class TerrainToolDesc_HeightUser : TerrainToolDesc
{
}

//--------------------------------------------------------------
class TerrainToolDesc_LayerAdd : TerrainToolDesc
{
	float		fAddNormalized;
	Material	pMaterial;
}

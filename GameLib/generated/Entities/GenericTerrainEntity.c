/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class GenericTerrainEntityClass: GenericEntityClass
{
}

class GenericTerrainEntity: GenericEntity
{
	/*!
	create sphere
	\param x		world X of touch with ground
	\param z		world X of touch with ground
	\param radius		radius
	\param centerLerp01	lerp of values inside rasterization
	\param timeDown01	relative speed of going down, 1 is usually 0.5secs
	\param maxHeight01	maximum relative height of flattening, 0 = no flatten
	*/
	proto external void FlattenGrassSphere(float x, float z, float radius, float centerLerp01, float timeDown01, float maxHeight01);
	/*!
	create ellipse
	\param x		world X of touch with ground
	\param z		world X of touch with ground
	\param sideX		radius in X coord before rotate
	\param sideZ		radius in Z coord before rotate
	\param offset		offset, 0 = default center, <-1, 1>
	\param angleRAD		rotation
	\param centerLerp01	lerp of values inside rasterization
	\param timeDown01	relative speed of going down, 1 is usually 0.5secs
	\param maxHeight01	maximum relative height of flattening, 0 = no flatten
	*/
	proto external void FlattenGrassEllipse(float x, float z, float sideX, float sideZ, float offset, float angleRAD, float centerLerp01, float timeDown01, float maxHeight01);
	/*!
	create box
	\param x		world X of touch with ground
	\param z		world X of touch with ground
	\param side		size of side
	\param angleRAD		rotation
	\param centerLerp01	lerp of values inside rasterization
	\param timeDown01	relative speed of going down, 1 is usually 0.5secs
	\param maxHeight01	maximum relative height of flattening, 0 = no flatten
	*/
	proto external void FlattenGrassBox(float x, float z, float side, float angleRAD, float centerLerp01, float timeDown01, float maxHeight01);
	/*!
	create rectangle
	\param x		world X of touch with ground
	\param z		world X of touch with ground
	\param sideX	size of side in X
	\param sideZ	size of side in Z
	\param offset		offset, 0 = default center, <-1, 1>
	\param angleRAD		rotation
	\param centerLerp01	lerp of values inside rasterization
	\param timeDown01	relative speed of going down, 1 is usually 0.5secs
	\param maxHeight01	maximum relative height of flattening, 0 = no flatten
	*/
	proto external void FlattenGrassRect(float x, float z, float sideX, float sideZ, float offset, float angleRAD, float centerLerp01, float timeDown01, float maxHeight01);
	proto bool GetTileTextureResName(int tile, out ResourceName textureResName);
	/*!
	get linear tile number from texccords
	\param terrx	x terrain coords
	\param terry	y terrain coords
	\return tile number in linear pos
	*/
	proto external int GetTileNumber(int terrx, int terry);
	/*!
	convert point from world to terr coords
	\param worldpos		worldpos
	\return terrpos
	*/
	proto external vector WorldToTerrCoord(vector worldpos);
	/*!
	convert point from terr to worldpos coords
	\param terrpos	terrpos
	\return worldpos
	*/
	proto external vector TerrToWorldCoord(vector terrpos);
	proto bool TryGetHeightTC(float x, float z, out float y);
	/*!
	number of vertices in tile
	*/
	proto external int GetTileVerticesCount();
}

/*!
\}
*/

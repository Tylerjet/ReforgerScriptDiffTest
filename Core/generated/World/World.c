/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup World
\{
*/

/*!
derived class connected to gamelib to implement game related features
*/
sealed class World: BaseWorld
{
	/*!
	Creates single visual mark, e.g. from shots, version with projection vector and angle
	//when lifetime=0, pointer to decal is returned, that can be removed by RemoveDecal then
	\param entity				entity where the Track should be created
	\param origin				first point of the decal, nothing is done now
	\param project			projection direction (length is far clipping distance)
	\param nearclip			near clipping distance
	\param farclip			far clipping distance
	\param materialName Material used for decal
	\param size 				size of decal
	\param stretch 			stretch of decal
	\param lifetime			Lifetime in seconds, if created with zero or negative value it's used as static and the pointer to decal is returned
	\param color				color of decal
	\return Decal pointer for static decals or null if the decal is dynamic or wasn't created for some reason (invalid material, NoDecal flag, ...)
	*/
	proto external Decal CreateDecal(notnull IEntity entity, vector origin, vector project, float nearclip, float farclip, float angle, float size, float stretch, string materialName, float lifetime, int color);
	/*!
	Creates single visual mark, e.g. from shots, version with exact matrix
	//when lifetime=0, pointer to decal is returned, that can be removed by RemoveDecal then
	\param entity				entity where the Track should be created
	\param matrix				projection matrix of decal
	\param nearclip			near clipping distance
	\param farclip			far clipping distance
	\param materialName Material used for decal
	\param size 				size of decal
	\param stretch 			stretch of decal
	\param lifetime			Lifetime in seconds, if created with zero or negative value it's used as static and the pointer to decal is returned
	\param color				color of decal
	\return Decal pointer for static decals or null if the decal is dynamic or wasn't created for some reason (invalid material, NoDecal flag, ...)
	*/
	proto external Decal CreateDecal2(notnull IEntity entity, vector matrix[4], float nearclip, float farclip, float size, float stretch, string materialName, float lifetime, int color);
}

/*!
\}
*/

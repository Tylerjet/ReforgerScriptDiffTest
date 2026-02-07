/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Entities
\{
*/

class BaseGameEntityClass: GenericEntityClass
{
}

class BaseGameEntity: GenericEntity
{
	/*!
	Teleport the entity to a coordinate
	\param mat The matrix used for the teleportation
	*/
	proto external void Teleport(vector mat[4]);
	//! Returns the replication component associated to this entity
	proto external RplComponent GetRplComponent();
	/*!
	Returns a matrix that transforms a point in ancestor-space to local-space.
	\param ancestor The Entity whose space is the source of the transformation. Can't be null.
	\param[out] outResult The output matrix transformation.
	\return true, if ancestor-descendant relationship was valid. Else false, in which case outResult is undefined.
	*/
	proto external bool GetAncestorToLocalTransform(IEntity ancestor, out vector result[4]);
	/*!
	Returns a matrix that transforms a point in local-space to ancestor-space.
	\param ancestor The Entity whose space is the target of the transformation. Can't be null.
	\param[out] outResult The output matrix transformation.
	\return true, if ancestor-descendant relationship was valid. Else false, in which case outResult is undefined.
	*/
	proto external bool GetLocalToAncestorTransform(IEntity ancestor, out vector result[4]);
}

/*!
\}
*/

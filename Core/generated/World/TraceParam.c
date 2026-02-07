/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup World
* @{
*/

/*!
Input&Output structure for collision tests.
*/
class TraceParam: Managed
{
	vector Start;
	vector End;
	int LayerMask = 0xffffffff;
	TraceFlags Flags;
	//use either Exclude or ExcludeArray. Never both, it has some performance penalty
	IEntity Exclude;
	array<IEntity> ExcludeArray;
	IEntity TraceEnt; ///<[out] traced entity
	vector TraceNorm; ///<[out] traced polygon normal (X,Y,Z)
	float TraceDist; ///<[out] traced polygon plane distace
	int NodeIndex; ///<[out] bone associated with traced collider
	int ColliderIndex; ///<[out] trace collider index
	SurfaceProperties SurfaceProps; ///<[out] traced surface properties
	owned string TraceMaterial; ///<[out] traced surface material (usualy only on terrain)
	owned string ColliderName; ///<[out] traced collider name
	
};

/** @}*/

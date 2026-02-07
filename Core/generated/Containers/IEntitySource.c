/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Containers
* @{
*/

sealed class IEntitySource: BaseContainer
{
	proto external IEntitySource GetChild(int n);
	proto external int GetNumChildren();
	proto external IEntitySource GetParent();
	proto external int GetSubScene();
	proto external int GetLayerID();
	proto external int GetComponentCount();
	proto external IEntityComponentSource GetComponent(int index);
	proto external EntityID GetID();
};

/** @}*/

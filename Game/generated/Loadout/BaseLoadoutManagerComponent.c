/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Loadout
* @{
*/

class BaseLoadoutManagerComponentClass: GameComponentClass
{
};

class BaseLoadoutManagerComponent: GameComponent
{
	proto external void Wear(IEntity pCloth);
	proto external void Unwear(IEntity pCloth);
	proto external IEntity GetClothByArea(typename pAreaType);
	proto external bool IsAreaAvailable(typename pAreaType);
};

/** @}*/

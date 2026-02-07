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
	proto external IEntity GetClothByArea(int pArea);
	proto external bool IsAreaAvailable(int pArea);
};

/** @}*/

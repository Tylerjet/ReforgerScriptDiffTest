/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Weapon
* @{
*/

class BaseMagazineComponentClass: GameComponentClass
{
};

class BaseMagazineComponent: GameComponent
{
	private IEntity m_Owner;
	IEntity GetOwner()
	{
		return m_Owner;
	}
	
	void BaseMagazineComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		m_Owner = ent;
	}
	
	proto external bool IsUsed();
	// remaining ammo
	proto external int GetAmmoCount();
	// maximum amount of ammo in this magazine
	proto external int GetMaxAmmoCount();
	proto external BaseMagazineWell GetMagazineWell();
	proto external UIInfo GetUIInfo();
};

/** @}*/

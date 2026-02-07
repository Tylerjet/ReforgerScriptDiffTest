/*
===========================================
Do not modify, this script is generated
===========================================
*/

/**
* \addtogroup Weapon
* @{
*/

class BaseMuzzleComponentClass: AttachmentSlotComponentClass
{
};

class BaseMuzzleComponent: AttachmentSlotComponent
{
	proto external EMuzzleType GetMuzzleType();
	proto external int GetNextFireModeIndex();
	proto external int GetFireModesCount();
	proto external int GetFireModeIndex();
	proto external int GetFireModesList(out notnull array<BaseFireMode> outFireModes);
	proto external BaseFireMode GetCurrentFireMode();
	// Barrels
	proto external int GetBarrelsCount();
	proto external int GetCurrentBarrelIndex();
	proto external bool IsBarrelChambered(int barrelIndex);
	proto external bool IsCurrentBarrelChambered();
	proto external bool IsChamberingPossible();
	// Disposable
	proto external bool IsDisposable();
	// Magazines
	proto external int GetAmmoCount();
	proto external int GetMaxAmmoCount();
	proto external BaseMagazineComponent GetMagazine();
	proto external BaseMagazineWell GetMagazineWell();
	// UI Info
	proto external UIInfo GetUIInfo();
};

/** @}*/

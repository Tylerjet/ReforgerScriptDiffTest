/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup Weapon
\{
*/

class BaseMuzzleComponentClass: GameComponentClass
{
}

class BaseMuzzleComponent: GameComponent
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
	//Deletes the bullet from selected chamber as well as Invoking OnAmmoCountChanged
	proto external void ClearChamber(int barrelIndex);
	// Disposable
	proto external bool IsDisposable();
	// Magazines
	proto external int GetAmmoCount();
	proto external int GetMaxAmmoCount();
	proto external BaseMagazineComponent GetMagazine();
	proto external BaseMagazineWell GetMagazineWell();
	// UI Info
	proto external UIInfo GetUIInfo();
}

/*!
\}
*/

// Class which fully describes weapon state
class SCR_WeaponState
{
	BaseWeaponComponent m_Weapon;
	BaseMuzzleComponent m_Muzzle;
	BaseMagazineComponent m_Magazine;
	
	SCR_2DSightsComponent m_Sights;
	SCR_SightsZoomFOVInfo m_SightsZoomFOVInfo;
	
	WeaponUIInfo m_WeaponUi;
	MagazineUIInfo m_MagazineUi;
	MuzzleUIInfo m_MuzzleUi;
	GrenadeUIInfo m_GrenadeUi;
	
	SCR_MagazineIndicatorConfiguration m_MagazineConfig;
	
	int m_iMagCount;
	int m_iMagAmmoCount;
	int m_iMagMaxAmmoCount;
	float m_fMagAmmoPerc;
	int m_iZeroing;
	float m_fZoom;
	bool m_bShowFiremode;
	EWeaponFiremodeType m_FireModeType;
	EAmmoType m_eAmmoTypeFlags;
	bool m_bBarrelChambered;
	bool m_bBarrelCanBeChambered;
	bool m_bInADS;
	bool m_bInInspectionMode;
	bool m_bIsGrenade;
	bool m_bHasSpecialAmmo;
	
	void Init()
	{
		m_Weapon = null;
		m_Muzzle = null;
		m_Magazine = null;
		
		m_Sights = null;
		m_SightsZoomFOVInfo = null;
		
		m_WeaponUi = null;
		m_MagazineUi = null;
		m_MuzzleUi = null;
		m_GrenadeUi = null;
		
		m_MagazineConfig = null;
		
		m_iMagCount = 0;
		m_iMagAmmoCount = 0;
		m_iMagMaxAmmoCount = 1;
		m_fMagAmmoPerc = 1;
		m_iZeroing = 0;
		m_fZoom = 0;
		m_bInADS = false;
		m_bInInspectionMode = false;
		m_bIsGrenade = false;
		m_bShowFiremode = false;
		m_FireModeType = EWeaponFiremodeType.Semiauto;
		m_eAmmoTypeFlags = EAmmoType.FMJ;
		m_bBarrelChambered = false;
		m_bBarrelCanBeChambered = false;
		m_bHasSpecialAmmo = false;
	}
};
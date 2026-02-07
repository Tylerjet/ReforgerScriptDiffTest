//! Forward declaration of UIInfo
//! Serves as a container of data to be displayed on player UI
class MuzzleUIInfo : UIInfo
{
	// Magazine icon behaviour in weapon UI
	[Attribute("true", UIWidgets.CheckBox, "Show firemode indicator.")]
	protected bool m_bShowFiremode;	

	bool ShowFiremodeIcon()
	{
		return m_bShowFiremode;
	}		

	/*
	{793CF85523BFB09F}UI/Textures/WeaponInfo/icons_weaponInfo-050.imageset
	{1394D968ED53BF7A}UI/Textures/WeaponInfo/icons_weaponInfo-050-glow.imageset
	
	{4A44FEA03A779FED}UI/Textures/WeaponInfo/icons_weaponInfo-075.imageset
	{DF3045B20D6C6C5F}UI/Textures/WeaponInfo/icons_weaponInfo-075-glow.imageset
	
	{93B6FB242078875D}UI/Textures/WeaponInfo/icons_weaponInfo-100.imageset	
	{9369077A21E066EB}UI/Textures/WeaponInfo/icons_weaponInfo-100-glow.imageset
	*/		
			
	//! Firemode imageset with foreground icons
	[Attribute("{793CF85523BFB09F}UI/Textures/WeaponInfo/icons_weaponInfo-050.imageset", UIWidgets.ResourceNamePicker, "Imageset with most of weapon info textures", "imageset")]
	protected ResourceName m_sFiremodeIconImageset;

	ResourceName GetFiremodeIconImageset()
	{
		return m_sFiremodeIconImageset;
	}	
	
	//! Firemode imageset with glow icons
	[Attribute("{1394D968ED53BF7A}UI/Textures/WeaponInfo/icons_weaponInfo-050-glow.imageset", UIWidgets.ResourceNamePicker, "Imageset with most of weapon info textures", "imageset")]
	protected ResourceName m_sFiremodeGlowImageset;	

	ResourceName GetFiremodeGlowImageset()
	{
		return m_sFiremodeGlowImageset;
	}	

	//! Firemode texture - single shot
	[Attribute("firemode-rifle-single", "auto", "Firemode indicator - single shot")]
	protected string m_sFiremodeSingle;

	//! Firemode texture - burst
	[Attribute("firemode-rifle-burst3", "auto", "Firemode indicator - burst")]
	protected string m_sFiremodeBurst;	

	//! Firemode texture - full auto
	[Attribute("firemode-rifle-auto", "auto", "Firemode indicator - full auto")]
	protected string m_sFiremodeAuto;	
			
	//! Firemode texture - safety
	[Attribute("firemode-safety", "auto", "Firemode indicator - safety", "edds")]
	protected string m_sFiremodeSafety;
	
	string GetFiremodeIconName(EWeaponFiremodeType firemode)
	{
		string icon = "";
		
		switch (firemode)
		{
			case EWeaponFiremodeType.Semiauto:
				icon = m_sFiremodeSingle;
				break;
			
			case EWeaponFiremodeType.Burst:
				icon = m_sFiremodeBurst;
				break;

			case EWeaponFiremodeType.Auto:
				icon = m_sFiremodeAuto;
				break;		
										
			case EWeaponFiremodeType.Safety:
				icon = 	m_sFiremodeSafety;
				break;
			
			default:
				icon = m_sFiremodeSingle;
				break;
		}		
		
		return icon;
	}
	
	[Attribute("", UIWidgets.Object)]
	ref SCR_MagazineIndicatorConfiguration m_MagIndicator;
};
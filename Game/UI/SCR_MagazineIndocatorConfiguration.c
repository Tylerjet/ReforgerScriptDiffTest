// Class for one entry of the magazine configuration
// It contains images for mag outline, progress bar and alpha mask.
[BaseContainerProps(configRoot: true)]
class SCR_MagazineIndicatorConfiguration 
{
	// ---- Imageset with magazine standard textures
	[Attribute("{4A44FEA03A779FED}UI/Textures/WeaponInfo/icons_weaponInfo-075.imageset", UIWidgets.ResourceNamePicker, "Imageset with foreground textures", "imageset")]
	ResourceName m_sImagesetIcons;
	
	// ---- Imageset with magazine glow textures
	[Attribute("{DF3045B20D6C6C5F}UI/Textures/WeaponInfo/icons_weaponInfo-075-glow.imageset", UIWidgets.ResourceNamePicker, "Imageset with glow textures", "imageset")]
	ResourceName m_sImagesetGlows;
	
	// ---- Magazine outline image ----
	[Attribute("magazine-default-outline", UIWidgets.EditBox, "Magazine icon/outline image")]
	string m_sOutline;
	
	[Attribute("true", UIWidgets.EditBox, "Use progress bar. If false, only outline image will be used.", params: "imageset")]
	bool m_bProgressBar;	
	
	[Attribute("magazine-default-background", UIWidgets.EditBox, "Magazine background image")]
	string m_sBackground;	
		
	// ---- Magazine progress image ----
	[Attribute("magazine-default-fill", UIWidgets.EditBox, "Magazine progress bar image")]
	string m_sProgress;	
	
	
	// ---- Magazine alpha mask image ----
	[Attribute("{0EF2203DF4502FB7}UI/Textures/WeaponInfo/alphamasks/alpha-mag-default.edds", UIWidgets.ResourceNamePicker, "Magazine progress bar alpha mask image", params: "edds")]
	ResourceName m_sProgressAlphaMask;
};
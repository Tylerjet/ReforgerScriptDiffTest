[BaseContainerProps()]
class SCR_ArmoredClothItemData : BaseItemAttributeData
{
	[Attribute("Only hitzones listed here will be affected by this item (by HitZoneName)", params: "")]
	ref array<string> m_aProtectedHitZones;
	
	[Attribute("0", UIWidgets.ComboBox, "EDamageTypes against which the hitzones will be protected", enums: ParamEnumArray.FromEnum(EDamageType))]
	ref array<EDamageType> m_aProtectedDamageTypes;
	
	[Attribute(defvalue: SCR_Enum.GetDefault(SCR_EArmorLevels.NONE), uiwidget: UIWidgets.ComboBox, "Armor protection level", enums: ParamEnumArray.FromEnum(SCR_EArmorLevels))]
	SCR_EArmorLevels m_eProtection;
	
	//-----------------------------------------------------------------------------------------------------------
	void SCR_ArmoredClothItemData(SCR_EArmorLevels protection, array<string> protectedHitZones, array<EDamageType> protectedDamageTypes)
	{
		m_eProtection = protection;
		m_aProtectedHitZones = protectedHitZones;
		m_aProtectedDamageTypes = protectedDamageTypes;
	}
};
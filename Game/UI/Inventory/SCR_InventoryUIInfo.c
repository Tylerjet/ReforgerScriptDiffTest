[BaseContainerProps()]
class SCR_InventoryUIInfo : UIInfo
{
	[Attribute("0", desc: "Set the icon visible in inventory hint.")]
	protected bool m_bShowIconInInventory;

	[Attribute("")]
	protected ref Color m_IconColor;

	bool IsIconVisible()
	{
		return m_bShowIconInInventory;
	}

	Color GetIconColor()
	{
		return m_IconColor;
	}
}
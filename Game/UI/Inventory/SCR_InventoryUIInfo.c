[BaseContainerProps()]
class SCR_InventoryUIInfo : UIInfo
{
	[Attribute("0", desc: "Set the icon visible in inventory hint.")]
	protected bool m_bShowIconInInventory;

	[Attribute("0 0 0 1")]
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
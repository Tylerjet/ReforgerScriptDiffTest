[BaseContainerProps(), SCR_BaseContainerLocalizedTitleField("Name")]
class SCR_DamageStateUIInfo : SCR_UIInfo
{	
	[Attribute("1.0 1.0 1.0 1.0", desc: "Main color of outline and icon")]
	protected ref Color m_Color;
	
	[Attribute("1.0 1.0 1.0 1.0", desc: "Main color of background")]
	protected ref Color m_BackgroundColor;
	
	[Attribute("{4C13A0801C18ED27}UI/Textures/InventoryIcons/Medical/outline_UI.edds", params: "edds", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sOutlineImage;
	
	[Attribute("{758F6E249DD20A02}UI/Textures/InventoryIcons/Medical/medical-con-BG_UI.edds", params: "edds", uiwidget: UIWidgets.ResourcePickerThumbnail)]
	protected ResourceName m_sBackgroundImage;
	
	/*!
	Get color of Icon and Outline
	\return Color
	*/
	Color GetColor()
	{
		return m_Color;
	}
	
	/*!
	Get color of background
	\return Color
	*/
	Color GetBackgroundColor()
	{
		return m_BackgroundColor;
	}
	
	/*!
	Get ResourceName of outline image
	\return Outline image
	*/
	ResourceName GetOutlineImage()
	{
		return m_sOutlineImage;
	}
	
	/*!
	Get ResourceName of outline image
	\return Outline image
	*/
	ResourceName GetBackgroundImage()
	{
		return m_sBackgroundImage;
	}
};

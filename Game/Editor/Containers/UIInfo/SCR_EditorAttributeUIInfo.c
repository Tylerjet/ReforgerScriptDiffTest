/*!
UIInfo used by editor attribute system
*/
[BaseContainerProps(), BaseContainerCustomTitleField("Name")]
class SCR_EditorAttributeUIInfo : SCR_UIInfo
{
	[Attribute("1 1 1 1", UIWidgets.ColorPicker, desc: "Description Icon Color")]
	protected ref Color m_cDescriptionIconColor;
	
	
	Color GetDescriptionIconColor()
	{
		return m_cDescriptionIconColor;
	}
	
	void SetDescription(string description)
	{
		Description = description;
	}	
};
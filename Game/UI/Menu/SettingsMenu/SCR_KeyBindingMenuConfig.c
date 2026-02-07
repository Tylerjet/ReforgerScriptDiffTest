/*
Configuration of keybindings visible in the keybindings menu.
*/

//------------------------------------------------------------------------------------------------


[BaseContainerProps(configRoot: true)]
class SCR_KeyBindingMenuConfig : Managed
{			
	[Attribute("", UIWidgets.Object, "Description", "")]
	ref array<ref SCR_KeyBindingCategory> keyBindingCategories;
};


[BaseContainerProps("", "Entry of one key binding"), BaseContainerCustomTitleField("actionName")]
class SCR_KeyBindingEntry
{
	[Attribute("", UIWidgets.EditBox, "Action name as defined in the ActionManager config")]
	string actionName;
	
	[Attribute("", UIWidgets.LocaleEditBox, "Visible name of the action, long texts are not supported, name shouldn't be over 15 characters ")]
	LocalizedString displayName;
	
	[Attribute("", UIWidgets.EditBox, "Show only Action sources with this preset.\nUsed for example when multiple directions are mapped to one action, like moving forward with value +1 and backward with value -1.")]
	string preset;
};

[BaseContainerProps("", "Category of key bindings"), BaseContainerCustomTitleField("name")]
class SCR_KeyBindingCategory
{
	[Attribute("", UIWidgets.EditBox, "Technical name of the category")]
	string name;
	
	[Attribute("", UIWidgets.LocaleEditBox, "Visible name of the category")]
	LocalizedString displayName;
	
	[Attribute("", UIWidgets.Object, "Description", "")]
	ref array<ref SCR_KeyBindingEntry> keyBindingEntries;
	
	
};
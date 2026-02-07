/*
Configuration of keybindings visible in the keybindings menu.
*/

//------------------------------------------------------------------------------------------------


[BaseContainerProps(configRoot: true)]
class SCR_KeyBindingMenuConfig : Managed
{			
	[Attribute("", UIWidgets.Object, "Description", "")]
	ref array<ref SCR_KeyBindingCategory> keyBindingCategories;
	
	[Attribute("", UIWidgets.Object, "Input filters available", "")]
	ref array<ref SCR_KeyBindingFilter> inputFilters;
	
	[Attribute("", UIWidgets.Object, "Addition binds able to be manually bound", "")]
	ref array<ref SCR_KeyBindingBind> inputBinds;
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

[BaseContainerProps("", "Input filters"), BaseContainerCustomTitleField("name")]
class SCR_KeyBindingFilter
{	
	[Attribute("", UIWidgets.Auto, "Name of the filter use the same name as defined in enfusion", "")]
	string filterString;
	
	[Attribute("", UIWidgets.Auto, "Display name of filter", "")]
	string filterDisplayName;
}

[BaseContainerProps("", "Custom keybinds"), BaseContainerCustomTitleField("name")]
class SCR_KeyBindingBind
{	
	[Attribute("", UIWidgets.Auto, "Name of the bind use the same name as defined in enfusion", "")]
	string bindString;
	
	[Attribute("", UIWidgets.Auto, "Display name of bind", "")]
	string bindDisplayName;
}
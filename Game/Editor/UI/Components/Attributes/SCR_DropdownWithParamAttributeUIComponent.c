
class SCR_DropdownWithParamAttributeUIComponent: SCR_DropdownEditorAttributeUIComponent
{	
	[Attribute("")]
	protected LocalizedString m_sParam;
	
	protected ref array<string> m_aTaskNameArray = new ref array<string>;
	
	override void Init(Widget w, SCR_BaseEditorAttribute attribute)
	{		
		super.Init(w, attribute);
		
		SCR_BaseEditorAttributeVar var = attribute.GetVariableOrCopy();
		if (!var)
			return;
		
		array<ref SCR_BaseEditorAttributeEntry> entries = new array<ref SCR_BaseEditorAttributeEntry>;
		attribute.GetEntries(entries);
		
		int entriesCount = entries.Count();
		for (int i = 0; i < entriesCount; i++)
		{
			SCR_BaseEditorAttributeEntryText entry = SCR_BaseEditorAttributeEntryText.Cast(entries[i]);
			if (entry) 
				m_aTaskNameArray.Insert(entry.GetText());
		}
		
		if (m_ComboBoxComponent)
			m_ComboBoxComponent.m_OnOpened.Insert(OnOpened);	
	}
	
	//Var init or reset
	override void SetFromVar(SCR_BaseEditorAttributeVar var)
	{
		super.SetFromVar(var);
		
		if (!m_ComboBoxComponent)
			return;
		
		SetCurrentElementName(m_ComboBoxComponent.GetCurrentIndex());	
	}
	
	//Changed current element
	protected override void OnChangeComboBox(SCR_ComboBoxComponent comboBox, int value)
	{
		super.OnChangeComboBox(comboBox, value);
		SetCurrentElementName(value);
	}
	
	//Drop box opened so set element names
	protected void OnOpened(SCR_ComboBoxComponent combobox)
	{		
		array<Widget> elementWidgets = new array<Widget>;
		int count = combobox.GetElementWidgets(elementWidgets);
		TextWidget text;
		
		if (count != m_aTaskNameArray.Count())
			return;
		
		for(int i = 0; i < count; i++)
        {
			text = TextWidget.Cast(elementWidgets[i].FindAnyWidget("Text"));
			if (!text)
				continue;
           
			text.SetTextFormat(m_aTaskNameArray[i], m_sParam);
        }		
	}
	
	//Set the current element name
	protected void SetCurrentElementName(int value)
	{
		if (!m_ComboBoxComponent)
			return;
		
		if (value >= m_aTaskNameArray.Count())
			return;
		
		TextWidget selectedElement = TextWidget.Cast(m_ComboBoxComponent.GetRootWidget().FindAnyWidget("Content"));

		if (selectedElement)
			selectedElement.SetTextFormat(m_aTaskNameArray[value], m_sParam);
	}
	
};
//------------------------------------------------------------------------------------------------
class SCR_MapMenuEntry : ScriptedSelectionMenuEntry
{
	int m_iHash;			// hash indentifier
	string m_sIdentifier;	// string identifier
	string m_sParentCategory;
	
	ResourceName m_sImageResource;
	string m_sImageName;
	
	ref ScriptInvoker<> m_OnClick = new ScriptInvoker();
	
	//------------------------------------------------------------------------------------------------
	void SetIcon(ResourceName imageResource, string imageName = "")
	{
		m_sImageResource = imageResource;
		m_sImageName = imageName;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateVisuals()
	{
		if (!m_EntryComponent)	
			return;
		
		m_EntryComponent.SetTextureData(m_sImageResource, m_sImageName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event void OnPerform(IEntity user, BaseSelectionMenu sourceMenu)
	{
		m_OnClick.Invoke();
		
		super.OnPerform(user, sourceMenu);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		outName = GetName();

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryDescriptionScript(out string outDescription)
	{
		outDescription = "";
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapMenuEntry(string name, string category = "", string identifier = "")
	{
		SetName(name);
		m_iHash = name.Hash();
		m_sParentCategory = category;
		m_sIdentifier = identifier;
	}
	
	//------------------------------------------------------------------------------------------------
	string GetEntryIdentifier()
	{
		return m_sIdentifier;
	}
};

//------------------------------------------------------------------------------------------------
class SCR_MapMenuRequestedTaskEntry : SCR_MapMenuEntry
{
	SCR_RequestedTaskSupportEntity m_SupportClass;
	
	//------------------------------------------------------------------------------------------------
	void SetSupportClass(SCR_RequestedTaskSupportEntity supportClass)
	{
		m_SupportClass = supportClass;
	}
	
	override bool CanBeShownScript(IEntity user, BaseSelectionMenu sourceMenu)
	{
		return m_SupportClass.CanRequest();
	}
};

//------------------------------------------------------------------------------------------------
class SCR_MapMenuCategory : BaseSelectionMenuCategory
{
	int m_iHash;
	
	ResourceName m_sImageResource;
	string m_sImageName;
	
	//------------------------------------------------------------------------------------------------
	void SetIcon(ResourceName imageResource, string imageName = "")
	{
		m_sImageResource = imageResource;
		m_sImageName = imageName;
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateVisuals()
	{
		if (!m_EntryComponent)	
			return;
		
		m_EntryComponent.SetTextureData(m_sImageResource, m_sImageName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override bool GetEntryNameScript(out string outName)
	{
		outName = GetName();
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_MapMenuCategory(string name)
	{
		SetName(name);
		m_iHash = name.Hash();
	}
};

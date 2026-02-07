class SCR_GroupFlagSelectionMenu : DialogUI
{		
	protected const int MAX_COLUMNS = 5; 
	
	[Attribute("160")]
	protected float m_fImgWidht;
	
	[Attribute("90")]
	protected float m_fImgHeight;
	
	protected const ResourceName BUTTON_IMAGE = "{4A119D28E23A3999}UI/layouts/WidgetLibrary/Buttons/WLib_ButtonImage.layout";			
		
	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		InitGroupFlagSelectionMenu(MAX_COLUMNS, BUTTON_IMAGE);			
	}
	
	//------------------------------------------------------------------------------------------------
	void CloseDialog()
	{			
		CloseAnimated();
	}	
	
	//------------------------------------------------------------------------------------------------
	void InitGroupFlagSelectionMenu(int maxColumns, ResourceName widget)
	{	
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();		
		if(!groupManager)
			return;
			
		int row = 1;
		int col = 1;	
		
		array<ResourceName> flags = {};
		groupManager.GetGroupFlags(flags);
		
		foreach(ResourceName flag : flags)
		{		
			GridLayoutWidget content = GridLayoutWidget.Cast( GetRootWidget().FindAnyWidget("Content"));				
						
			Widget testButton = GetGame().GetWorkspace().CreateWidgets(widget, content);		
			SCR_ButtonImageComponent imageButton = SCR_ButtonImageComponent.Cast(testButton.FindHandler(SCR_ButtonImageComponent));	
			
			if (col > maxColumns)
			{
				row++;
				col = 1;
			}
			
			GridSlot.SetRow(testButton, row);
			GridSlot.SetColumn(testButton, col);
			
			imageButton.SetImage(flag);
			imageButton.GetImageWidget().SetSize(m_fImgWidht, m_fImgHeight);			
			imageButton.m_OnClicked.Insert(SetGroupFlag);
			
			col++;
		}			
		
		Widget cancelButton = GetRootWidget().FindAnyWidget("Cancel");		
		
		SCR_NavigationButtonComponent cancel = SCR_NavigationButtonComponent.Cast(cancelButton.FindHandler(SCR_NavigationButtonComponent));		
		cancel.m_OnClicked.Insert(CloseDialog);		
	}
	
	//------------------------------------------------------------------------------------------------
	void SetGroupFlag(SCR_ButtonBaseComponent button)
	{		
		
		SCR_GroupsManagerComponent groupManager = SCR_GroupsManagerComponent.GetInstance();		
		if(!groupManager)
			return;
			
		SCR_PlayerControllerGroupComponent playerGroupController = SCR_PlayerControllerGroupComponent.GetLocalPlayerControllerGroupComponent();		
		SCR_AIGroup group =  groupManager.FindGroup(playerGroupController.GetGroupID());
		
		array<ResourceName> flags = {};
		groupManager.GetGroupFlags(flags);
		
		SCR_ButtonImageComponent imageButton = SCR_ButtonImageComponent.Cast(button.GetRootWidget().FindHandler(SCR_ButtonImageComponent));					
		playerGroupController.RequestSetGroupFlag(group.GetGroupID(), flags.Find(imageButton.m_sTexture));			
		
		CloseAnimated();				
	}
}
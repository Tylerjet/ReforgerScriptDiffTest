//------------------------------------------------------------------------------------------------
[BaseContainerProps()]
class NavigationButtonEntry
{
	[Attribute(desc: "Name of input action")]
	string m_sAction;
	[Attribute(desc: "Name of the button - ID")]
	string m_sButtonID;
	
	SCR_NavigationButtonComponent m_Component;
};

//------------------------------------------------------------------------------------------------
//! UI Script
//! Inventory navigation bar handler
[BaseContainerProps()]
class SCR_NavigationBarUI : ScriptedWidgetComponent
{
	//#define BUTTON_LAYOUT				"{19A9CC7487AAD442}UI/layouts/Common/Buttons/NavigationButton.layout"
	
	const string LOCALIZATION_PREFIX = "#AR-";
	
	[Attribute("{08CF3B69CB1ACBC4}UI/layouts/WidgetLibrary/WLib_NavigationButton.layout", UIWidgets.ResourceNamePicker, "Layout", "layout")]
	protected ResourceName m_Layout;
	
	[Attribute(uiwidget: UIWidgets.Object, desc: "Action Buttons")]
	ref array<ref NavigationButtonEntry> m_aEntries;
	
	ref ScriptInvoker m_OnAction = new ScriptInvoker;
			
	//------------------------------------------------------------------------ USER METHODS ------------------------------------------------------------------------							
	
	//------------------------------------------------------------------------------------------------
	void Refresh();
	
	//------------------------------------------------------------------------------------------------
	protected SCR_NavigationButtonComponent GetButton( string name )
	{
		foreach( NavigationButtonEntry entry : m_aEntries )
		{
			SCR_NavigationButtonComponent comp = entry.m_Component;
			if (entry.m_sButtonID == name && comp)
				return comp;
		}
	
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnNavigation(SCR_NavigationButtonComponent comp, string action)
	{
		m_OnAction.Invoke(comp, action, null, -1);
	}

	//------------------------------------------------------------------------------------------------
	void SetButtonEnabled( string sButtonName, bool bEnable = true, string sName = "" )
	{
		SCR_NavigationButtonComponent pActionButton = GetButton( sButtonName );
		if( !pActionButton )
			return;
		pActionButton.SetEnabled( bEnable );
		pActionButton.GetRootWidget().SetVisible( bEnable );
		
		if( !sName.IsEmpty() )
			pActionButton.SetLabel( LOCALIZATION_PREFIX + sName );
	}		
	
	//------------------------------------------------------------------------------------------------
	void SetButtonActionName( string sButtonName, string sName )
	{
		SCR_NavigationButtonComponent pActionButton = GetButton( sButtonName );
		if( !pActionButton )
			return;
		pActionButton.SetLabel( LOCALIZATION_PREFIX + sName );
	}
	
	//------------------------------------------------------------------------------------------------
	void SetAllButtonEnabled( bool bEnable = true )
	{
		foreach( NavigationButtonEntry entry: m_aEntries )
		{
			SCR_NavigationButtonComponent comp = entry.m_Component;
			if (!comp)
				return;
			
			entry.m_Component.SetEnabled( bEnable );
			entry.m_Component.GetRootWidget().SetVisible( bEnable );
		}
	}
	
	//------------------------------------------------------------------------ COMMON METHODS ----------------------------------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached( Widget w )
	{
		WorkspaceWidget workspace = GetGame().GetWorkspace(); 
		
		foreach ( NavigationButtonEntry entry : m_aEntries )
		{
			if ( entry.m_sAction == "Inventory_Selected" )
				continue;
			
			Widget button = workspace.CreateWidgets(m_Layout, w);
			if (!button)
				continue;
			
			SCR_NavigationButtonComponent comp = SCR_NavigationButtonComponent.Cast(button.FindHandler(SCR_NavigationButtonComponent));
			if (!comp)
				continue;
			
			entry.m_Component = comp;
			comp.SetAction(entry.m_sAction);
			comp.SetLabel(LOCALIZATION_PREFIX + entry.m_sAction);
			comp.m_OnActivated.Insert(OnNavigation);
			comp.SetClickedSound("");
		}
	}
};
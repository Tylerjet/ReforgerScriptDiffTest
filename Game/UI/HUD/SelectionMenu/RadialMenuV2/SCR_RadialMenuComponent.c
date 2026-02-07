//! Component runs functions and interaction of radial menu
class SCR_RadialMenuComponentClass: ScriptComponentClass
{
	
};

class SCR_RadialMenuComponent : ScriptComponent
{		
	protected BaseHUDComponent m_HUDManager;
	
	//------------------------------------------------------------------------------------------------
	[Attribute("", UIWidgets.Object, "Setting for scripted behavior - can be all SCR_RadialMenuHandler children", category:"Behavior")]
	ref SCR_RadialMenuHandler m_pRadialMenu;
	
	[Attribute("", UIWidgets.Object, "Setting for menu visualization", category:"Behavior")]
	protected ref SCR_RadialMenuVisuals m_RadialMenuVisuals;
	
	[Attribute("1", UIWidgets.Object, "Set handler id - hadnler and visual class needs to match to function together", category:"Behavior")]
	protected int m_iHandlerId;
	
	//------------------------------------------------------------------------------------------------
	// Interaction modification
	[Attribute("CharacterSwitchWeaponRadial", UIWidgets.CheckBox, "Opening / closing input reference", category:"Interaction")]
	string m_sInput_Toggle;
	
	[Attribute("1", UIWidgets.CheckBox, "Using thumb stick for navigation - True = left | False = right", category:"Interaction")]
	protected bool m_bUsingLeftStick;
	
	[Attribute("0", UIWidgets.CheckBox, "True = button must be hold to keep menu open | False = menu cen be on/off with button click", category:"Interaction")]
	protected bool m_bHoldToggleToOpen;
	
	[Attribute("1", UIWidgets.ComboBox, "The way the entry is performed", category: "Interaction", ParamEnumArray.FromEnum(ERadialMenuPerformType))]
	protected ERadialMenuPerformType m_iEntryPerformType; 
	
	[Attribute(SCR_RadialMenuInteractions.INPUT_PAGE_NEXT, UIWidgets.ComboBox, "Input to switch next page.", category: "Interaction", ParamEnumArray.FromEnum(ERadialMenuPerformType))]
	protected string m_sInputPageNext; 
	
	[Attribute(SCR_RadialMenuInteractions.INPUT_PAGE_PREVIOUS, UIWidgets.ComboBox, "Input to switch previous page.", category: "Interaction", ParamEnumArray.FromEnum(ERadialMenuPerformType))]
	protected string m_sInputPagePrevious; 
	
	//------------------------------------------------------------------------------------------------
	// Entries modification
	
	[Attribute("", UIWidgets.CheckBox, "Placement type of entries", category:"Entries")]
	protected bool m_bEvenlyPlacedEntries;
	
	[Attribute("90.0", "0-360", "Distance angle in degrees between each entry", category:"Entries")]
	protected float m_fEntryDistance;
	
	[Attribute("0", "0-360", "Offset of entries initial position in degrees", category:"Entries")]
	protected float m_fEntryInitialOffset;
	
	//[Attribute("0", "0-360", "Offset of entries initial position in degrees", category:"Entries")]
	protected float m_fEntryOffset;
	
	[Attribute("1", UIWidgets.CheckBox, "alow to show entries that has no data", category:"Entries")]
	protected bool m_bShowEmptyEntries;
	
	//------------------------------------------------------------------------------------------------
	// Selector modification
	[Attribute("1", UIWidgets.ComboBox, "", category:"Selector", ParamEnumArray.FromEnum(ERadialMenuSelectionBehavior))]
	protected ERadialMenuSelectionBehavior m_iSelectionBehavior;
	
	[Attribute("0.25", UIWidgets.EditBox, "Delay for holding selection with free selection", category:"Selector")]
	protected float m_fSelectFreeDelay;
	
	/*!
	Get RadialMenuHandler defined on this component
	\return RadialMenuHandler instance
	*/
	SCR_RadialMenuHandler GetRadialMenuHandler()
	{
		return m_pRadialMenu;
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{ 
		if(!m_pRadialMenu)
			return;
		
		SetupValues(owner);
		m_pRadialMenu.Init(owner);
	}

	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if(!m_pRadialMenu)
			return;
		
		m_pRadialMenu.Update(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetupValues(IEntity owner)
	{
		if(!m_pRadialMenu)
			return;
		
		SCR_RadialMenuInteractions interactions = m_pRadialMenu.GetRadialMenuInteraction();
		if(!interactions)
			return;
		
		// Interaction 
		interactions.SetHoldToggleToOpen(m_bHoldToggleToOpen);
		interactions.SetEntryPerformType(m_iEntryPerformType);
		interactions.SetHandling(owner, m_bUsingLeftStick, m_sInput_Toggle);
		interactions.SetHandlingPaging();
		
		// Entries 
		m_pRadialMenu.SetEvenlyPlacedEntries(m_bEvenlyPlacedEntries);
		m_pRadialMenu.SetEntryDistance(m_fEntryDistance);
		m_pRadialMenu.SetEntryOffset(m_fEntryInitialOffset, m_fEntryOffset);
		m_pRadialMenu.SetShowEmptyEntries(m_bShowEmptyEntries);
		
		// Selector
		m_pRadialMenu.SetRadialMenuSelectionBehavior(m_iSelectionBehavior);
		m_pRadialMenu.SetSelectFreeDelay(m_fSelectFreeDelay);
		
		interactions.SetHandlerId(m_iHandlerId);
		
		// Visuals 
		if(!m_RadialMenuVisuals)
			return;
		
		m_RadialMenuVisuals.SetMenuHandler(m_pRadialMenu);
		
		// Check for hud maanger 
		/*m_HUDManager = SCR_BaseHUDComponent.Cast(owner.FindComponent(SCR_BaseHUDComponent));
		if (!m_HUDManager)
			return;
		
		Print("owner: " + owner.GetName());*/
		//m_HUDManager.RegisterHUDElement(m_RadialMenuVisuals);
		//m_HUDManager.CreateLayout();
	}
	
	//------------------------------------------------------------------------------------------------
    void SCR_RadialMenuComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
    {
		
    }
      
    //------------------------------------------------------------------------------------------------
    void ~SCR_RadialMenuComponent()
    {
		
    }
};
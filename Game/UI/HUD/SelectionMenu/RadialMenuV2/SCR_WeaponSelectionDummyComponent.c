// This script serves only as component class fpr testing radial weapon MenuBase
class SCR_WeaponSelectionDummyComponentClass: ScriptComponentClass
{
};

class SCR_WeaponSelectionDummyComponent : ScriptComponent
{
	
	[Attribute("", UIWidgets.Object)]
	protected ref SCR_RadialMenuData m_pRadialMenuData;
	
	//[Attribute("", UIWidgets.Object)]
	//protected ref SCR_RadialMenuVisuals m_pRadialMenuVisuals;
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{ 
		
		if(m_pRadialMenuData != null)
		{
			//m_pRadialMenuData.Init(owner);
			//m_pRadialMenuInteractions.Init();
			//m_pRadialMenuVisuals.Init();
			
			// Add listeners
			//m_pRadialMenuInteractions.onPerformInputCallInvoker.Insert(TryPerformEntry);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnDeactivate(IEntity owner)
	{
		// Remove listeners
		//m_pRadialMenuInteractions.onPerformInputCallInvoker.Remove(TryPerformEntry);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		/*if(m_pRadialMenuData != null)
			m_pRadialMenuData.Update(owner, timeSlice);*/
	}
	
	//------------------------------------------------------------------------------------------------
    void SCR_WeaponSelectionDummyComponent( IEntityComponentSource src, IEntity ent, IEntity parent )
    {
		/*SetEventMask(ent, EntityEvent.INIT);
        SetEventMask(ent, EntityEvent.FRAME);              //setting event mask, so EOnInit event will be called
        ent.SetFlags(EntityFlags.ACTIVE, true);  */   
    }
      
    //------------------------------------------------------------------------------------------------
    void ~SCR_WeaponSelectionDummyComponent()
    {
    }
};
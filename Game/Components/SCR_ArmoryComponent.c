[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "Component handling Armory and ammo storage compositions.", color: "0 0 255 255")]
class SCR_ArmoryComponentClass: ScriptComponentClass
{
};

class SCR_ArmoryComponent: ScriptComponent
{
	protected ref array <SCR_FactionControlComponent> m_aFactionControlComponents = {};
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
	
	//------------------------------------------------------------------------------------------------
	void ChangeOwningFaction(notnull Faction faction)
	{
		if (IsProxy())
			return;
		
		FactionKey key = faction.GetFactionKey();
		
		foreach (SCR_FactionControlComponent factionComp : m_aFactionControlComponents)
		{
			factionComp.SetFaction(key);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Goes though entity children (recursively) and finds all faction control components
	protected void FindFactionComponentsInHiearchy(IEntity entity)
	{
		IEntity child = entity.GetChildren();
		SCR_FactionControlComponent factionComp;
		
		while (child)
		{
			if (child.GetChildren())
				FindFactionComponentsInHiearchy(child);
			
			factionComp = SCR_FactionControlComponent.Cast(child.FindComponent(SCR_FactionControlComponent));
			if (factionComp)
				m_aFactionControlComponents.Insert(factionComp);
			
			child = child.GetSibling();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{	
		if (!GetGame().InPlayMode())
			return;
		
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
		if (!m_RplComponent)
			return;
		
		if (IsProxy())
			return;
		
		FindFactionComponentsInHiearchy(GetOwner());
	}
	
	//------------------------------------------------------------------------------------------------
	// PostInit
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}
}



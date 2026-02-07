[EntityEditorProps(category: "GameScripted", description: "Set faction over network", color: "0 0 255 255")]
class SCR_FactionControlComponentClass: ScriptComponentClass
{
};

/*!
Component to set faction over network.
Used when entity it's attached to is not replicated by default.
Calls SetFaction function on parent entity:
~~~~
void SetFaction(Faction faction)
{
};
~~~~
*/
class SCR_FactionControlComponent : ScriptComponent
{
	[Attribute(desc: "Default faction.\nLeave empty if you don't want to set any default, but want to set faction in script instead.")]
	protected FactionKey m_DefaultFaction;
	
	[RplProp(onRplName: "OnFactionIndex")]
	protected int m_iFactionIndex = -1;
	
	protected IEntity m_Owner;
	
	protected ref ScriptInvoker m_OnFactionChanged = new ScriptInvoker();
	
	/*
	Get ScriptInvoker on faction change
	\returns ScriptInvoker invoked on all clients when faction of this entity changes
	*/
	ScriptInvoker GetOnFactionChanged()
	{
		return m_OnFactionChanged;
	}
	
	/*
	Set faction and broadcast it to all machines.
	\param factionKey Name of desired faction.
	*/
	void SetFaction(FactionKey factionKey)
	{
		FactionManager manager = GetGame().GetFactionManager();
		if (!manager)
			return;
		
		Faction faction = manager.GetFactionByKey(factionKey);
		if (!faction)
			return;
		
		SetFaction(faction);
	}
	/*
	Set faction and broadcast it to all machines.
	\param faction Desired faction
	*/
	void SetFaction(Faction faction)
	{
		FactionManager manager = GetGame().GetFactionManager();
		if (!manager)
			return;
		
		int index = manager.GetFactionIndex(faction);
		if (index == -1)
			return;
		
		SetFaction(index);
	}
	/*
	Set faction and broadcast it to all machines.
	\param factionKey Index of desired faction from faction manager
	*/
	void SetFaction(int factionIndex)
	{
		m_iFactionIndex = factionIndex;
		OnFactionIndex();
		Replication.BumpMe();
	}
	/*
	Set faction and broadcast it to all machines.
	\param IEntity Owner
	\param faction Desired faction
	*/
	static void SetFaction(IEntity owner, Faction faction)
	{
		if (!owner || !faction )
			return;
		
		SCR_FactionControlComponent factionControl = SCR_FactionControlComponent.Cast(owner.FindComponent(SCR_FactionControlComponent));
		if (factionControl)
			factionControl.SetFaction(faction);
	}
	/*
	Get assigned faction.
	\return Faction
	*/
	Faction GetFaction()
	{
		FactionManager manager = GetGame().GetFactionManager();
		if (manager)
			return manager.GetFactionByIndex(m_iFactionIndex);
		else
			return null;
	}
	
	protected void OnFactionIndex()
	{
		FactionManager manager = GetGame().GetFactionManager();
		if (!manager)
			return;
		
		Faction faction = manager.GetFactionByIndex(m_iFactionIndex);
		if (!faction)
			return;
		
		if (m_OnFactionChanged)
		{
			m_OnFactionChanged.Invoke(faction);
		}
	}
	
	override void OnPostInit(IEntity owner)
	{
		m_Owner = owner;
		
		if (!m_DefaultFaction.IsEmpty())
			SetFaction(m_DefaultFaction);
	}
};
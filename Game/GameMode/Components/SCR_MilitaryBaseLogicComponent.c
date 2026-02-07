class SCR_MilitaryBaseLogicComponentClass : ScriptComponentClass
{
};

class SCR_MilitaryBaseLogicComponent : ScriptComponent
{
	protected ref array<SCR_MilitaryBaseComponent> m_aBases = {};
	
	//------------------------------------------------------------------------------------------------
	void RegisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (m_aBases.Contains(base))
			return;
		
		m_aBases.Insert(base);
		OnBaseRegistered(base);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBaseRegistered(notnull SCR_MilitaryBaseComponent base)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	void UnregisterBase(notnull SCR_MilitaryBaseComponent base)
	{
		if (!m_aBases.Contains(base))
			return;
		
		m_aBases.RemoveItem(base);
		OnBaseUnregistered(base);
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBaseUnregistered(notnull SCR_MilitaryBaseComponent base)
	{
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (GetGame().InPlayMode())
			SCR_MilitaryBaseManager.GetInstance().RegisterLogicComponent(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_MilitaryBaseLogicComponent()
	{
		SCR_MilitaryBaseManager.UnregisterLogicComponentStatic(this);
		
		foreach (SCR_MilitaryBaseComponent base : m_aBases)
		{
			if (base)
				base.UnregisterLogicComponent(this);
		}
	}
};
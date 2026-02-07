[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class SCR_MineWeaponComponentClass : WeaponComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_MineWeaponComponent : WeaponComponent
{
	protected ResourceName m_sFlagPrefab;
	protected IEntity m_FlagEntity;
/*
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
	}
*/
	//------------------------------------------------------------------------------------------------
	event protected bool RplLoad(ScriptBitReader reader)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	event protected bool RplSave(ScriptBitWriter writer)
	{
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RPC_AddFlag(ResourceName prefabName)
	{
		m_sFlagPrefab = prefabName;
		UpdateFlag();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Call only on server!
	void AddFlag(ResourceName prefabName)
	{
		RPC_AddFlag(prefabName);
		Rpc(RPC_AddFlag, prefabName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateFlag()
	{
		if (m_FlagEntity)
			delete m_FlagEntity; //Local only
		
		Resource resource = Resource.Load(m_sFlagPrefab);
		if (!resource.IsValid())
			return;
		
		IEntity owner = GetOwner();
		
		EntitySpawnParams params = new EntitySpawnParams();
		params.Transform[3] = owner.GetOrigin() + "0 0.1 0";
		
		m_FlagEntity = GetGame().SpawnEntityPrefabLocal(resource, owner.GetWorld(), params);
	}
	
	//------------------------------------------------------------------------------------------------
	/*override void EOnInit(IEntity owner)
	{
	}*/
	
	//------------------------------------------------------------------------------------------------
	void SCR_MineWeaponComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MineWeaponComponent()
	{
	}

};

[EntityEditorProps(category: "Game/Building", description: "Component attached to compositions.")]
class SCR_CampaignBuildingCompositionComponentClass : ScriptComponentClass
{
};

	
//------------------------------------------------------------------------------------------------
class SCR_CampaignBuildingCompositionComponent : ScriptComponent
{
	// Could be for example a base to which this composition belongs to.
	protected IEntity m_ProviderEntity;
	protected RplId m_RplCompId;
	private SCR_EditorModeEntity m_EditorModeEntity;
	
	//------------------------------------------------------------------------------------------------
	void SetProviderEntity(IEntity newOwner)
	{
		m_ProviderEntity = newOwner;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetProviderEntity()
	{
		return m_ProviderEntity;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove provider entity when the provider is not a base and the building mode was terminated.
	void RemoveProviderEntity()
	{
		m_ProviderEntity = null;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetProviderEntityServer(IEntity newOwner)
	{
		SetProviderEntity(newOwner);
		
		RplId id = RplId.Invalid();
		
		if (newOwner)
		{
			RplComponent comp = RplComponent.Cast(newOwner.FindComponent(RplComponent));
			if (comp)
				id = comp.Id();
		}
		
		Rpc(RpcDo_SetProviderEntity, id);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Remove provider entity when the provider is not a base and the building mode was terminated. Called on server from invoker
	void RemoveProviderEntityServer()
	{
		RemoveProviderEntity();
		m_EditorModeEntity.GetOnClosedServer().Remove(RemoveProviderEntityServer);
		Rpc(RpcDo_RemoveProviderEntity);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set an event to remove a provider from composition component when the building mode is terminated.
	void SetClearProviderEvent(notnull SCR_EditorModeEntity ent)
	{	
		m_EditorModeEntity = ent;
		ent.GetOnClosedServer().Insert(RemoveProviderEntityServer);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Set a provider, loaded from RPL ID
	void SetProviderFromRplID()
	{
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(m_RplCompId));
		if (!rplComp)
			return;
		
		SCR_FreeCampaignBuildingTrigger.GetOnProviderCreated().Remove(SetProviderFromRplID);
		SetProviderEntity(rplComp.GetEntity());
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_RemoveProviderEntity()
	{	
 		RemoveProviderEntity();
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_SetProviderEntity(RplId rplCompId)
	{	
		IEntity newOwner = null;
		
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(rplCompId));
		if (rplComp)
			newOwner = IEntity.Cast(rplComp.GetEntity());
				
 		SetProviderEntity(newOwner);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		if (!m_ProviderEntity)
		{
			writer.WriteBool(0);
			return true;
		}
	
		RplComponent rplComp = RplComponent.Cast(m_ProviderEntity.FindComponent(RplComponent));
		if (!rplComp)
		{
			writer.WriteBool(0);
			return true;
		}
	
		writer.WriteBool(1);
		writer.WriteRplId(rplComp.Id());
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		bool providerExist;
		reader.ReadBool(providerExist);
		if (!providerExist)
			return true;
		
		reader.ReadRplId(m_RplCompId);
		RplComponent rplComp = RplComponent.Cast(Replication.FindItem(m_RplCompId));
		if (!rplComp)
		{
			SCR_FreeCampaignBuildingTrigger.GetOnProviderCreated().Insert(SetProviderFromRplID);
			return true;
		}
		
		m_ProviderEntity = rplComp.GetEntity();
		return true;
	}
};

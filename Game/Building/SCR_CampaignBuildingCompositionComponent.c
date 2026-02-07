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
	protected int m_iBuilderId = -1;
	protected ref ScriptInvoker m_OnBuilderSet;

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (System.IsConsoleApp())
			return;

		SCR_EditableEntityComponent editableEnt = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEnt)
			return;

		if (!CanSendNotification(editableEnt))
			return;

		GetOnBuilderSet().Insert(SendBuildNotification);

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;

		core.Event_OnEntityUnregistered.Insert(SendDissasamblyNotification);
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvoker GetOnBuilderSet()
	{
		if (!m_OnBuilderSet)
			m_OnBuilderSet = new ScriptInvoker();

		return m_OnBuilderSet;
	}

	//------------------------------------------------------------------------------------------------
	void SetProviderEntity(IEntity newOwner)
	{
		m_ProviderEntity = newOwner;
		if (m_OnBuilderSet)
			m_OnBuilderSet.Invoke();
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
	void SetBuilderId(int id)
	{
		m_iBuilderId = id;
	}

	//------------------------------------------------------------------------------------------------
	int GetBuilderId()
	{
		return m_iBuilderId;
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
		if (m_EditorModeEntity)
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

		SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Remove(SetProviderFromRplID);
		SetProviderEntity(rplComp.GetEntity());
	}

	//------------------------------------------------------------------------------------------------
	void SendBuildNotification()
	{
		SCR_EditableEntityComponent editableEnt = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEnt || !CanSendNotification(editableEnt))
			return;

		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(m_ProviderEntity.FindComponent(SCR_CampaignMilitaryBaseComponent));

		if (!IsPlayerAndCompositionFactionSame(editableEnt))
			return;

		if (!base)
			return;

		SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SERVICE_BUILD, GetBuilderId(), Replication.FindId(editableEnt), base.GetCallsign());
		if (m_OnBuilderSet)
			m_OnBuilderSet.Remove(SendBuildNotification);
	}

	//------------------------------------------------------------------------------------------------
	void SendDissasamblyNotification(SCR_EditableEntityComponent editableEnt)
	{
		if (!editableEnt || !CanSendNotification(editableEnt))
			return;
		
		SCR_CampaignMilitaryBaseComponent base = SCR_CampaignMilitaryBaseComponent.Cast(m_ProviderEntity.FindComponent(SCR_CampaignMilitaryBaseComponent));
		if (!base)
			return;

		if (!IsPlayerAndCompositionFactionSame(editableEnt))
			return;

		BaseGameMode gameMode = GetGame().GetGameMode();
		SCR_CampaignBuildingManagerComponent buildingManagerComponent = SCR_CampaignBuildingManagerComponent.Cast(gameMode.FindComponent(SCR_CampaignBuildingManagerComponent));
		if (!buildingManagerComponent)
			return;

		int initiator;
		if (!buildingManagerComponent.TryRemoveFromList(editableEnt, initiator))
			return;

		SCR_EditableEntityCore core = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		if (!core)
			return;

		SCR_NotificationsComponent.SendLocal(ENotification.EDITOR_SERVICE_DISASSEMBLED, initiator, Replication.FindId(editableEnt), base.GetCallsign());
		core.Event_OnEntityUnregistered.Remove(SendDissasamblyNotification);
	}

	//------------------------------------------------------------------------------------------------
	bool IsPlayerAndCompositionFactionSame(SCR_EditableEntityComponent editableEnt)
	{
		IEntity player = EntityUtils.GetPlayer();
		if (!player)
			return false;

		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(editableEnt.GetInfo());
		if (!editableEntityUIInfo)
			return false;

		return editableEntityUIInfo.GetFaction() == SCR_EntityHelper.GetEntityFaction(player);
	}

	//------------------------------------------------------------------------------------------------
	bool CanSendNotification(notnull SCR_EditableEntityComponent editableEnt)
	{
		// The event was triggered by a deleting of the preview entity.
		if (!editableEnt.HasEntityFlag(EEditableEntityFlag.VIRTUAL))
			return false;

		SCR_EditableEntityComponent editableEntOwner = SCR_EditableEntityComponent.Cast(GetOwner().FindComponent(SCR_EditableEntityComponent));
		if (!editableEntOwner || editableEntOwner != editableEnt)
			return false;

		SCR_EditableEntityUIInfo editableEntityUIInfo = SCR_EditableEntityUIInfo.Cast(editableEnt.GetInfo());
		if (!editableEntityUIInfo)
			return false;

		array<EEditableEntityLabel> entityLabels = {};
		editableEntityUIInfo.GetEntityLabels(entityLabels);
		return entityLabels.Contains(EEditableEntityLabel.TRAIT_SERVICE);
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
		writer.WriteInt(m_iBuilderId);
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
			SCR_CampaignBuildingProviderComponent.GetOnProviderCreated().Insert(SetProviderFromRplID);
			return true;
		}

		reader.ReadInt(m_iBuilderId);
		SetProviderEntity(rplComp.GetEntity());

		return true;
	}
};

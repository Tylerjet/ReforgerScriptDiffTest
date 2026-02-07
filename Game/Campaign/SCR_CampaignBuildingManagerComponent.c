[ComponentEditorProps(category: "GameScripted/GameMode/Components", description: "Base for gamemode scripted component.")]
class SCR_CampaignBuildingManagerComponentClass : SCR_BaseGameModeComponentClass
{
};

//------------------------------------------------------------------------------------------------
//! Interface for game mode extending components.
//! Must be attached to a GameMode entity.
class SCR_CampaignBuildingManagerComponent : SCR_BaseGameModeComponent
{
	[Attribute("0", UIWidgets.ComboBox, "", enums: ParamEnumArray.FromEnum(EEditableEntityBudget))]
	protected EEditableEntityBudget m_BudgetType;
	
	[Attribute("25",  "Refund percentage", "")]
	protected int m_iCompositionRefundPercentage;	
	protected SCR_EditableEntityCore m_EntityCore;
	protected IEntity m_TemporaryProvider;
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	void SetTemporaryProvider(IEntity ent)
	{
		m_TemporaryProvider = ent;
	}
	
	//------------------------------------------------------------------------------------------------
	IEntity GetTemporaryProvider()
	{
		return m_TemporaryProvider;
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool GetProviderEntity(IEntity ownerEntity, out SCR_CampaignSuppliesComponent suppliesComponent)
	{
		SCR_CampaignBuildingCompositionComponent campaignCompositionComponent = SCR_CampaignBuildingCompositionComponent.Cast(ownerEntity.FindComponent(SCR_CampaignBuildingCompositionComponent));
		if (!campaignCompositionComponent)
			return false;
		
		IEntity providerEntity;
		providerEntity = campaignCompositionComponent.GetProviderEntity();
		if (!providerEntity)
		{			
			providerEntity = GetTemporaryProvider();
			SetTemporaryProvider(null);
		}

		if (!providerEntity)
			return false;
		
		suppliesComponent = SCR_CampaignSuppliesComponent.Cast(providerEntity.FindComponent(SCR_CampaignSuppliesComponent));
		return suppliesComponent != null;
	}
	
	/*!
	Get supplies from the composition cost that gets refunded on removal of composition
	\return Percentage of supplies refunded
	*/
	int GetCompositionRefundPercentage()
	{
		return m_iCompositionRefundPercentage;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnEntityCoreBudgetUpdated(EEditableEntityBudget entityBudget, int originalBudgetValue, int budgetChange, int updatedBudgetValue, SCR_EditableEntityComponent entity)
	{
		if (IsProxy())
			return;
		
		if (entityBudget != m_BudgetType)
			return;
		
		IEntity entityOwner = entity.GetOwnerScripted();
		SCR_CampaignSuppliesComponent suppliesComponent;
		IEntity providerEntity;
		if (!GetProviderEntity(entityOwner, suppliesComponent))
			return;
		
		if (budgetChange < 0)
			budgetChange = Math.Round(budgetChange * (m_iCompositionRefundPercentage / 100));

		suppliesComponent.AddSupplies(-budgetChange);
	}
	
	//------------------------------------------------------------------------------------------------
	//! GetNetworkManager
	protected  SCR_CampaignBuildingNetworkComponent GetNetworkManager()
	{
		PlayerController playerController = GetGame().GetPlayerController();
		if (!playerController)
			return null;
		
		return SCR_CampaignBuildingNetworkComponent.Cast(playerController.FindComponent(SCR_CampaignBuildingNetworkComponent));
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		m_EntityCore = SCR_EditableEntityCore.Cast(SCR_EditableEntityCore.GetInstance(SCR_EditableEntityCore));
		m_RplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		
		if (GetGameMode().IsMaster())
			m_EntityCore.Event_OnEntityBudgetUpdated.Insert(OnEntityCoreBudgetUpdated);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		if (m_EntityCore && GetGameMode().IsMaster())
			m_EntityCore.Event_OnEntityBudgetUpdated.Remove(OnEntityCoreBudgetUpdated);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		if (SCR_Global.IsEditMode())
			return;
		
		SetEventMask(owner, EntityEvent.INIT);
	}
	
	//------------------------------------------------------------------------------------------------	
	protected bool IsProxy()
	{
		return (m_RplComponent && m_RplComponent.IsProxy());
	}
};

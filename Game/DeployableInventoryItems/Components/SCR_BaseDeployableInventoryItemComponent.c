[EntityEditorProps(category: "GameScripted/DeployableItems", description: "")]
class SCR_BaseDeployableInventoryItemComponentClass : ScriptComponentClass
{
}

//! Base class which all deployable inventory items inherit from
class SCR_BaseDeployableInventoryItemComponent : ScriptComponent
{
	[RplProp(onRplName: "OnRplDeployed")] 
	protected bool m_bIsDeployed;
	
	protected int m_iItemOwnerID = -1;
	
	protected RplComponent m_RplComponent;
	
	//------------------------------------------------------------------------------------------------
	//! Gets called when deploy action is executed by player
	//! \param[in] userEntity
	void Deploy(IEntity userEntity = null)
	{						
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		PlayerManager playerManager = GetGame().GetPlayerManager();
		if (!playerManager)
			return;
		
		if (userEntity)
			m_iItemOwnerID = playerManager.GetPlayerIdFromControlledEntity(userEntity);
		
		// Put deploy logic here
		
		m_bIsDeployed = true;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Gets called when dismantle action is executed by player
	//! \param[in] userEntity
	void Dismantle(IEntity userEntity = null)
	{
		if (!m_RplComponent || m_RplComponent.IsProxy())
			return;
		
		m_iItemOwnerID = -1; // Reset owner ID
		
		// Put dismantle logic here
		
		m_bIsDeployed = false;
		Replication.BumpMe();
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	protected void OnRplDeployed();
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] userEntity
	//! \return
	bool CanDeployBeShown(notnull IEntity userEntity)
	{
		return !m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] userEntity
	//! \return
	bool CanDismantleBeShown(notnull IEntity userEntity)
	{
		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	//!
	//! \return
	bool IsDeployed()
	{
		return m_bIsDeployed;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \return
	int GetItemOwnerID()
	{
		return m_iItemOwnerID;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(GetOwner(), EntityEvent.INIT);
		m_RplComponent = RplComponent.Cast(GetOwner().FindComponent(RplComponent));
	}
}

[EntityEditorProps(category: "GameScripted/Campaign", description: "Armory component used in campaign.", color: "0 0 255 255")]
class SCR_CampaignArmoryComponentClass: SCR_BaseCampaignInstallationComponentClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_CampaignArmoryComponent : SCR_BaseCampaignInstallationComponent
{
	static ref array<SCR_CampaignArmoryComponent> s_aArmories = {};
	protected ref array<SCR_CampaignWeaponRackEntity> m_aWeaponRacks = new ref array<SCR_CampaignWeaponRackEntity>();
	
	protected SCR_CampaignBase m_Base;
	protected bool m_bCanSpawnItems = false; 
	
	//------------------------------------------------------------------------------------------------
	static SCR_CampaignArmoryComponent GetNearestArmory(vector origin)
	{
		SCR_CampaignArmoryComponent armory;
		float minDistance = float.MAX, currentDistance;
		for (int i = s_aArmories.Count() - 1; i >= 0; i--)
		{
			currentDistance = vector.Distance(origin, s_aArmories[i].GetOwner().GetOrigin());
			if (currentDistance < minDistance)
			{
				minDistance = currentDistance;
				armory = s_aArmories[i];
			}
		}
		
		return armory;
	}
	//------------------------------------------------------------------------------------------------
	SCR_CampaignBase GetBase()
	{
		return m_Base;
	}
	
	//------------------------------------------------------------------------------------------------
	array<SCR_CampaignWeaponRackEntity> GetWeaponRacks()
	{
		return m_aWeaponRacks;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBaseOwnerChanged(SCR_CampaignFaction newOwner)
	{
		if((!newOwner) || !newOwner.GetFactionKey())
			m_bCanSpawnItems = false;
		
		if(m_Base.GetStartingBaseOwner() == newOwner.GetFactionKey())
			m_bCanSpawnItems = true;
	}
	
	//------------------------------------------------------------------------------------------------
	void OnBaseReinforcementsArrived(SCR_CampaignBase base, SCR_CampaignFaction previousSuppliesFaction, SCR_CampaignFaction currentSuppliesFaction)
	{
		if (!m_bUpdate)
			return;
		
		
		//This will turn on spawning items after base was reinforced by new owner
		m_bCanSpawnItems = true;
		
		if (previousSuppliesFaction != currentSuppliesFaction)
			RespawnAllWeapons(currentSuppliesFaction);
	}
	
	//------------------------------------------------------------------------------------------------
	void RespawnAllWeapons(SCR_CampaignFaction newFaction = null)
	{
		SCR_CampaignFaction faction = newFaction;
		if (!faction)
		{
			faction = m_Base.GetSuppliesFaction();
			if (!faction)
				return;
		}
		
		for (int i = m_aWeaponRacks.Count() - 1; i >= 0; i--)
		{
			m_aWeaponRacks[i].ClearWeapons();
			
			m_aWeaponRacks[i].PeriodicalSpawn(faction);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void RegisterWeaponRack(SCR_CampaignWeaponRackEntity weaponRack)
	{
		if (m_aWeaponRacks && m_aWeaponRacks.Find(weaponRack) == -1 && weaponRack)
		{
			m_aWeaponRacks.Insert(weaponRack);
			weaponRack.ClearWeapons();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void DoPeriodicalSpawn(float timeSlice)
	{
		if (!m_bUpdate)
			return;
		
		SCR_CampaignFaction faction = m_Base.GetSuppliesFaction();
		if (!faction)
			return;
		
		for (int i = m_aWeaponRacks.Count() - 1; i >= 0; i--)
		{
			float time = m_aWeaponRacks[i].GetTimerTime();
			time -= timeSlice;
			
			if (time < 0)
			{
				m_aWeaponRacks[i].PeriodicalSpawn(faction);
				m_aWeaponRacks[i].ResetTimer();
			}
			else
				m_aWeaponRacks[i].SetTimerTime(time);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//Called each x seconds by the installation it's attached to
	override void UpdateInstallationComponent(float timeSinceLastUpdate)
	{
		if (m_bCanSpawnItems)
			DoPeriodicalSpawn(timeSinceLastUpdate);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (m_bUpdatedByInstallation)
			return;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{		
		SCR_CampaignDeliveryPoint armory = SCR_CampaignDeliveryPoint.Cast(owner);
		if (!armory)
			return;
		
		SCR_BaseCampaignInstallation campaignInstallation = armory.GetParentBase();
		
		if (campaignInstallation)
			campaignInstallation.RegisterComponent(this);
		
		m_Base = SCR_CampaignBase.Cast(campaignInstallation);
		
		IEntity parent = owner;
		bool shouldReturn = true;
		while (parent)
		{
			RplComponent rplComponent = RplComponent.Cast(parent.FindComponent(RplComponent));
			if (rplComponent && !rplComponent.IsProxy())
			{
				m_bUpdate = true;
				shouldReturn = false;
				break;
			}
			else
				m_bUpdate = false;
			
			parent = parent.GetParent();
		}
		
		if (shouldReturn)
		    return;
		
		if (m_Base)
			m_Base.m_OnReinforcementsArrived.Insert(OnBaseReinforcementsArrived);
		else
			Print("SCR_CampaignArmoryComponent not attached to SCR_CampaignBase, expect issues!", LogLevel.ERROR);
		
		if (!m_bUpdatedByInstallation)
		{
			SetEventMask(owner, EntityEvent.FRAME);
			owner.SetFlags(EntityFlags.ACTIVE, true);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_CampaignArmoryComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
		s_aArmories.Insert(this);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_CampaignArmoryComponent()
	{
		s_aArmories.RemoveItem(this);
		if (m_Base)
			m_Base.m_OnReinforcementsArrived.Remove(OnBaseReinforcementsArrived);
	}

};
[ComponentEditorProps(category: "GameScripted/SupportStation", description: "")]
class SCR_BaseDamageHealSupportStationComponentClass : SCR_BaseSupportStationComponentClass
{
	[Attribute("SOUND_VEHICLE_REPAIR_PARTIAL", desc: "Sound effect played when heal is updated and not done. Broadcast to players. Leave empty if no sfx", category: "Heal/Repair Support Station")]
	protected string m_sOnHealUpdateSoundEffectEventName;
	
	[Attribute(desc: "Sound effect played when Damage over Time is removed (Other than fire). Broadcast to players. Leave empty if no sfx", category: "Heal/Repair Support Station")]
	protected string m_sOnDoTRemovedSoundEffectEventName;
	
	protected ref SCR_AudioSourceConfiguration m_OnHealUpdateAudioSourceConfiguration;
	protected ref SCR_AudioSourceConfiguration m_OnDoTRemovedAudioSourceConfiguration;
	
	//------------------------------------------------------------------------------------------------
	/*!
	Get Audio config to play
	Will create it if it not yet exists. Returns null if no SoundFile or SoundEvent is set
	\return Sound Config
	*/
	SCR_AudioSourceConfiguration CreateSoundAudioConfig(string soundEventName)
	{		
		//~ Does not have audio assigned
		if (SCR_StringHelper.IsEmptyOrWhiteSpace(m_sOnUseSoundEffectFile) || SCR_StringHelper.IsEmptyOrWhiteSpace(soundEventName))
			return null;
		
		//~ Create Audio source
		SCR_AudioSourceConfiguration audioSourceConfiguration = new SCR_AudioSourceConfiguration();
		audioSourceConfiguration.m_sSoundProject = m_sOnUseSoundEffectFile;
		audioSourceConfiguration.m_sSoundEventName = soundEventName;
		
		if (!CanMoveWithPhysics())
			audioSourceConfiguration.m_eFlags = SCR_Enum.SetFlag(audioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static);
		else 
			audioSourceConfiguration.m_eFlags = SCR_Enum.RemoveFlag(audioSourceConfiguration.m_eFlags, EAudioSourceConfigurationFlag.Static);
		
		return audioSourceConfiguration;
	}	
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Sound Config for heal update. Will return null if no audio is assigned
	*/
	SCR_AudioSourceConfiguration GetOnHealUpdateAudioConfig()
	{		
		//~ Create Audio source if it does not yet exist
		if (!m_OnHealUpdateAudioSourceConfiguration)
			m_OnHealUpdateAudioSourceConfiguration = CreateSoundAudioConfig(m_sOnHealUpdateSoundEffectEventName);

		return m_OnHealUpdateAudioSourceConfiguration;
	}	
	
	//------------------------------------------------------------------------------------------------
	/*!
	\return Sound Config for heal update. Will return null if no audio is assigned
	*/
	SCR_AudioSourceConfiguration GetOnDoTRemovedAudioConfig()
	{		
		//~ Create Audio source if it does not yet exist
		if (!m_OnDoTRemovedAudioSourceConfiguration)
			m_OnDoTRemovedAudioSourceConfiguration = CreateSoundAudioConfig(m_sOnDoTRemovedSoundEffectEventName);

		return m_OnDoTRemovedAudioSourceConfiguration;
	}	
};

class SCR_BaseDamageHealSupportStationComponent : SCR_BaseSupportStationComponent
{
	[Attribute("500", desc: "Max damage healed each execute. If you hold the action it will heal each time the action ends", category: "Heal/Repair Support Station")]
	protected float m_iDamageHealedEachExecution;
	
	[Attribute("100", desc: "Supply cost for ever 'Damage Healed Each Execution' damage healed. If the left over damage is less then it will of course be cheaper with a min value of 1", category: "Heal/Repair Support Station")]
	protected int m_iSupplyCostDamageHealed;
	
	[Attribute("1", desc: "The max percentage that this support station can heal scaled. 1 == 100%, This is not calculated as a whole but for each hitZone. Aka if hitZone A is less than this percentage then the heal action can be executed even if total health is more than this %", params: "0.01 1", category: "Heal/Repair Support Station")]
	protected float m_fMaxHealScaled;
	
	[Attribute(desc: "The DPS types that cost supplies to be healed/repaired. It will still remove the DPS effect even if it is not included here. It simply does not cost extra", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(EDamageType), category: "Heal/Repair Support Station")]
	protected ref array<EDamageType> m_aDoTTypesHealed;
	
	[Attribute("100", desc: "Supplies needed to heal conditions such as bleeding. Ignored if 0 or m_aDoTTypesHealed is empty. Fire uses own supply calculation", params: "0 inf 1", category: "Heal/Repair Support Station")]
	protected int m_iSuppliesPerDoTHealed;
	
	[Attribute(ESupportStationReasonInvalid.HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY.ToString(), desc: "Invalid reason when healing is done but full health not reached. For repair vehicle this is HEAL_MAX_HEALABLE_HEALTH_REACHED_FIELD for repair wrench only this is HEAL_MAX_HEALABLE_HEALTH_REACHED_EMERGENCY", uiwidget: UIWidgets.SearchComboBox, enums: ParamEnumArray.FromEnum(ESupportStationReasonInvalid), category: "Heal/Repair Support Station")]
	protected ESupportStationReasonInvalid m_eMaxHealDone;
		
	//------------------------------------------------------------------------------------------------
	override bool IsValid(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, vector actionPosition, out ESupportStationReasonInvalid reasonInvalid, out int supplyCost)
	{
		if (!SCR_BaseDamageHealSupportStationAction.Cast(action))
		{
			Debug.Error2("SCR_BaseDamageHealSupportStationComponent", "'IsValid' fails as the damage heal support station is executed with a non SCR_BaseDamageHealSupportStationAction action! This is will break the support station!");
			return false;
		}
		
		if (!super.IsValid(actionOwner, actionUser, action, actionPosition, reasonInvalid, supplyCost))
			return false;
		
		EDamageType activeDot;
		array<HitZone> hitZones = {};
		float damageToHeal = GetDamageOrStateToHeal(actionOwner, actionUser, SCR_BaseDamageHealSupportStationAction.Cast(action), activeDot, hitZones);
		
		if (activeDot == -1 && damageToHeal <= 0)
		{
			foreach (HitZone hitzone : hitZones)
			{
				//~ No damage can be healed yet a hitzone is not full heath
				if (hitzone.GetHealthScaled() != 1)
				{
					reasonInvalid = m_eMaxHealDone;
					return false;
				}
			}
			
			//~ Entity has no damage
			reasonInvalid = ESupportStationReasonInvalid.HEAL_ENTITY_UNDAMAGED;
			return false;
		}

		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetDamageOrStateToHeal(IEntity actionOwner, IEntity actionUser, notnull SCR_BaseDamageHealSupportStationAction action, out EDamageType activeDoT, out notnull array<HitZone> hitZones)
	{			
		action.GetHitZonesToHeal(hitZones);
		
		activeDoT = -1;
		
		//~ Has DoT so this gets priority over healing damage (Excluding fire)
		foreach (EDamageType DoT : m_aDoTTypesHealed)
		{
			foreach (HitZone hitZone : hitZones)
			{
				if (hitZone.GetDamageOverTime(DoT))
				{
					activeDoT = DoT;
					return 1;
				}
			}
		}		
		
		//~ Get amount healed (Cap to damage healed each execute)
		return Math.Clamp(action.GetActionDamageManager().GetHitZonesDamage(m_fMaxHealScaled, hitZones), 0, m_iDamageHealedEachExecution);
	}

	//------------------------------------------------------------------------------------------------
	protected override int GetSupplyCostAction(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action)
	{
		if (!IsUsingSupplies())
			return 0;
		
		EDamageType activeDoT;
		array<HitZone> hitZones = {};
		
		float damageToHeal = GetDamageOrStateToHeal(actionOwner, actionUser, SCR_BaseDamageHealSupportStationAction.Cast(action), activeDoT, hitZones);
		
		//~ Has DoT, this is always healed first
		if (activeDoT != -1)
		{
			return m_iSuppliesPerDoTHealed + m_iBaseSupplyCostOnUse;
		}
			
		//~ No damage to heal
		if (damageToHeal <= 0)
			return 0;
		
		//~ Calculate the supply cost
		float damageHealCost = (damageToHeal / m_iDamageHealedEachExecution) * m_iSupplyCostDamageHealed;
		
		//~ There is damage so there should always be a cost of 1
		if ((damageHealCost + m_iBaseSupplyCostOnUse) < 1)
			return 1;
		
		return Math.Ceil(damageHealCost + m_iBaseSupplyCostOnUse);
	}	
	
	//------------------------------------------------------------------------------------------------
	override void OnExecutedServer(notnull IEntity actionOwner, notnull IEntity actionUser, notnull SCR_BaseUseSupportStationAction action)
	{
		//~ Consume supplies
		if (IsUsingSupplies())
		{
			//~ Failed to consume supplies, meaning there weren't enough supplies for the action
			if (!OnConsumeSuppliesServer(GetSupplyCostAction(actionOwner, actionUser, action)))
				return;
		}
		
		SCR_BaseDamageHealSupportStationAction damageHealAction = SCR_BaseDamageHealSupportStationAction.Cast(action);
		
		//~ Get damage to heal
		EDamageType activeDoT;
		array<HitZone> hitZones = {};
		float damageToHeal = GetDamageOrStateToHeal(actionOwner, actionUser, damageHealAction, activeDoT, hitZones);
		
		//~ Get broadcast ids
		RplId ownerId;
		RplId userId;
		int playerId;
		SCR_EDamageSupportStationHealState healState;
		FindEntityIds(actionOwner, actionUser, ownerId, userId, playerId);
		
		//~ Remove the active Damage over Time
		if (activeDoT != -1)
		{		
			healState = SCR_EDamageSupportStationHealState.DOT_REMOVED;
			
			foreach (HitZone hitZone : hitZones)
			{
				hitZone.SetDamageOverTime(activeDoT, 0);
			}
					
			//~ Execute default logics
			OnExecuteDamageSystem(actionOwner, actionUser, healState, SCR_BaseDamageHealSupportStationAction.Cast(action), 0);
			Rpc(OnExecuteDamageSystemBroadcast, ownerId, userId, healState, action.GetActionID(), 0);
			return;	
		}
		
		SCR_DamageManagerComponent damageManager = damageHealAction.GetActionDamageManager();
		if (!damageManager)
			return;
		
		//~ No DoT so get hitZones to heal
		damageManager.HealHitZones(damageToHeal, true, m_fMaxHealScaled, hitZones);
		
		//~ The support station can still heal more
		if (damageManager.GetHitZonesDamage(m_fMaxHealScaled, hitZones) > 0)
		{
			healState = SCR_EDamageSupportStationHealState.HEAL_UPDATE;
		}	
		else 
		{
			//~ Entity health is full health
			if (m_fMaxHealScaled >= 1)
				healState = SCR_EDamageSupportStationHealState.HEAL_DONE;
			//~ The support station reached the max health
			else 
				healState = SCR_EDamageSupportStationHealState.HEAL_DONE_NOT_FULL_HEAL;
		}
			
		//~ Get the current percentage of healing and if all hitZones are healed to the max
		float healthScaled = damageHealAction.GetActionDamageManager().GetSingleHitZonesHealthScaled(hitZones);
		
		OnExecuteDamageSystem(actionOwner, actionUser, healState, SCR_BaseDamageHealSupportStationAction.Cast(action), healthScaled);
		Rpc(OnExecuteDamageSystemBroadcast, ownerId, userId, healState, action.GetActionID(), healthScaled);
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void OnExecuteDamageSystemBroadcast(RplId ownerId, RplId userId, SCR_EDamageSupportStationHealState healState, int actionId, float healthScaled)
	{
		IEntity actionOwner, actionUser;
		GetEntitiesFromID(ownerId, userId, actionOwner, actionUser);
		
		SCR_BaseUseSupportStationAction action;
		if (actionOwner)
		{
			ActionsManagerComponent actionManager = ActionsManagerComponent.Cast(actionOwner.FindComponent(ActionsManagerComponent));
			
			if (actionManager)
				action = SCR_BaseUseSupportStationAction.Cast(actionManager.FindAction(actionId));
		}
			
		OnExecuteDamageSystem(actionOwner, actionUser, healState, SCR_BaseDamageHealSupportStationAction.Cast(action), healthScaled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnExecuteDamageSystem(IEntity actionOwner, IEntity actionUser, SCR_EDamageSupportStationHealState healState, SCR_BaseDamageHealSupportStationAction action, float healthScaled)
	{
		if (!actionOwner)
			return;
		
		//~ Play sound effect
		SCR_BaseDamageHealSupportStationComponentClass classData = SCR_BaseDamageHealSupportStationComponentClass.Cast(GetComponentData(GetOwner()));
		if (classData)
		{
			switch (healState)
			{
				case SCR_EDamageSupportStationHealState.HEAL_UPDATE :
				{
					PlaySoundEffect(classData.GetOnHealUpdateAudioConfig(), actionOwner, action);
					break;
				}
				case SCR_EDamageSupportStationHealState.HEAL_DONE_NOT_FULL_HEAL :
				{
					PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
					break;
				}
				case SCR_EDamageSupportStationHealState.HEAL_DONE :
				{
					PlaySoundEffect(GetOnUseAudioConfig(), actionOwner, action);
					
					//~ Play done voice event (if any) if character is fully healed
					SCR_DamageManagerComponent damageManager = SCR_DamageManagerComponent.GetDamageManager(actionOwner);
					if (damageManager)
					{
						if (damageManager.GetHealthScaled() >= 1)
							PlayCharacterVoiceEvent(actionOwner);
					}

					break;
				}
				case SCR_EDamageSupportStationHealState.DOT_REMOVED :
				{
					PlaySoundEffect(classData.GetOnDoTRemovedAudioConfig(), actionOwner, action);
					break;
				}
			}
		}
		
		//~ Do not send notification
		if (!GetSendNotificationOnUse())
			return;
		
		SendDamageSupportStationNotification(actionOwner, actionUser, action, healState, healthScaled);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SendDamageSupportStationNotification(IEntity actionOwner, IEntity actionUser, SCR_BaseUseSupportStationAction action, SCR_EDamageSupportStationHealState healState, float healthScaled)
	{
		//~ Override in inherited classes
	}
};

//------------------------------------------------------------------------------------------------
enum SCR_EDamageSupportStationHealState
{
	FIRE_EXTINGUISH_UPDATE, ///< To notify the system that the fire extinguish was updated but the fire state is not removed
	FIRE_EXTINGUISH_DONE, ///< To notify the system that the fire extinguish was updated but the fire state was removed
	DOT_REMOVED, ///< To notify the system that a Damage over time effect was removed
	HEAL_UPDATE, ///< To notify the system that health was updated but healing is not done
	HEAL_DONE_NOT_FULL_HEAL, ///< To notify the system that health was updated and healing is done. But the entity is not full heal aka the repair station cannot heal more
	HEAL_DONE, ///< To notify the system that health was updated and healing is done.
	BLOOD_UPDATE, ///< To notify the system that blood amount was updated but healing is not done
	BLOOD_DONE_NOT_FULL_HEAL, ///< To notify the system that blood amount was updated and healing is done. But the entity is not full blood aka the heal station cannot heal more blood
	BLOOD_DONE, ///< To notify the system that blood amount was updated and healing is done.
};
class SCR_ContainerMedicalActionTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		title = source.GetClassName();
		title.Replace("SCR_ScenarioFrameworkMedicalAction", "");
		string sOriginal = title;
		SplitStringByUpperCase(sOriginal, title);
		return true;
	}

	protected void SplitStringByUpperCase(string input, out string output)
	{
		output = "";
		bool wasPreviousUpperCase;
		int asciiChar;
		for (int i, count = input.Length(); i < count; i++)
		{
			asciiChar = input.ToAscii(i);
			bool isLowerCase = (asciiChar > 96 && asciiChar < 123);
			if (i > 0 && !wasPreviousUpperCase && !isLowerCase)
			{
				output += " " + asciiChar.AsciiToString();
				wasPreviousUpperCase = true;
			}
			else
			{
				 if (isLowerCase)
					wasPreviousUpperCase = false;
				output += asciiChar.AsciiToString();
			}
		}
	}
}

//! Medical actions that will be executed on target entity
[BaseContainerProps(), SCR_ContainerActionTitle()]
class SCR_ScenarioFrameworkActionMedical : SCR_ScenarioFrameworkActionBase
{
	[Attribute(desc: "Target entity for Medical Action")];
	ref SCR_ScenarioFrameworkGet m_Getter;

	[Attribute(desc: "Medical actions that will be executed on target entity")];
	ref array<ref SCR_ScenarioFrameworkMedicalAction> m_aMedicalActions;

	//------------------------------------------------------------------------------------------------
	override void OnActivate(IEntity object)
	{
		if (!CanActivate())
			return;

		if (!m_Getter)
		{
			Print(string.Format("ScenarioFramework Action: Getter not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ScenarioFrameworkParam<IEntity> entityWrapper = SCR_ScenarioFrameworkParam<IEntity>.Cast(m_Getter.Get());
		if (!entityWrapper)
		{
			Print(string.Format("ScenarioFramework Action: Issue with Getter detected for Action %1.", this), LogLevel.ERROR);
			return;
		}

		IEntity entity = IEntity.Cast(entityWrapper.GetValue());
		if (!entity)
		{
			Print(string.Format("ScenarioFramework Action: Entity not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		SCR_ChimeraCharacter character = SCR_ChimeraCharacter.Cast(entity);
		if (!character)
		{
			Print(string.Format("ScenarioFramework Action: Entity is not ChimeraCharacter for Action %1.", this), LogLevel.ERROR);
			return;
		}

		foreach (SCR_ScenarioFrameworkMedicalAction medicalAction : m_aMedicalActions)
		{
			medicalAction.Init(character);
		}
	}
}

//! Medical action base class
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalAction
{
	SCR_CharacterDamageManagerComponent m_DamageManager;

	//------------------------------------------------------------------------------------------------
	//!
	//! \param[in] character
	void Init(SCR_ChimeraCharacter character)
	{
		m_DamageManager = SCR_CharacterDamageManagerComponent.Cast(character.GetDamageManager());
		if (!m_DamageManager)
		{
			Print(string.Format("ScenarioFramework Action: Character Damage Manager Component not found for Action %1.", this), LogLevel.ERROR);
			return;
		}

		OnActivate();
	}

	//------------------------------------------------------------------------------------------------
	void OnActivate();
}

//! Sets blood hit zone value
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetBlood : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "6000", uiwidget: UIWidgets.Slider, desc: "Character blood hit zone value", params: "0 6000 0.001")]
	float m_fBloodValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		HitZone bloodHitZone = m_DamageManager.GetBloodHitZone();
		if (!bloodHitZone)
		{
			Print(string.Format("ScenarioFramework Action: Blood Hit Zone not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		bloodHitZone.SetHealth(m_fBloodValue);
	}
}

//! Sets resilience hit zone value
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetResilience : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "100", uiwidget: UIWidgets.Slider, desc: "Character resilience hit zone value", params: "0 100 0.001")]
	float m_fResilienceValue;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		HitZone resilienceHitZone = m_DamageManager.GetResilienceHitZone();
		if (!resilienceHitZone)
		{
			Print(string.Format("ScenarioFramework Action: Resilience Hit Zone not found for Action %1.", this), LogLevel.ERROR);
			return;
		}
		
		resilienceHitZone.SetHealth(m_fResilienceValue);
	}
}

//! Removes all bleedings
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionRemoveAllBleedings : SCR_ScenarioFrameworkMedicalAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.RemoveAllBleedings();
	}
}

//! Removes all bleedings from specific hit zone group
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionRemoveGroupBleeding : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute("10", UIWidgets.ComboBox, "Select Character hit zone group to stop bleeding from", "", ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	ECharacterHitZoneGroup	m_eCharacterHitZoneGroup;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.RemoveGroupBleeding(m_eCharacterHitZoneGroup);
	}
}

//! Adds particular bleeding to selected hit zone
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddParticularBleeding : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(uiwidget: UIWidgets.EditBox, desc: "Which hit zone will start bleeding")];
	string m_sHitZoneName;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.AddParticularBleeding(m_sHitZoneName);
	}
}

//! Adds random bleeding
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionAddRandomBleeding : SCR_ScenarioFrameworkMedicalAction
{
	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.AddRandomBleeding();
	}
}

//! Sets Character hit zone group to add/remove tourniquets
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetTourniquettedGroup : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute("10", UIWidgets.ComboBox, "Select Character hit zone group to add/remove tourniquets", "", ParamEnumArray.FromEnum(ECharacterHitZoneGroup))]
	ECharacterHitZoneGroup	m_eCharacterHitZoneGroup;
	
	[Attribute(defvalue: "{D70216B1B2889129}Prefabs/Items/Medicine/Tourniquet_01/Tourniquet_US_01.et", desc: "Resource name of the Tourniquet you want to use")]
	ResourceName m_sTourniquetPrefab;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Whether target hit zone group is tourniquetted or not")]
	bool m_bTourniquetted;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		if (!m_bTourniquetted)
		{
			SCR_TourniquetStorageComponent tourniquetStorage = SCR_TourniquetStorageComponent.Cast(m_DamageManager.GetOwner().FindComponent(SCR_TourniquetStorageComponent));
			if (!tourniquetStorage)
			{
				Print(string.Format("ScenarioFramework Action: Tourniquet Storage Component not found for Action %1.", this), LogLevel.ERROR);
				return;
			}
	
			tourniquetStorage.RemoveTourniquetFromSlot(m_eCharacterHitZoneGroup, m_DamageManager.GetOwner());
			
			return;
		}
		
		Resource resource = Resource.Load(m_sTourniquetPrefab);
		if (!resource && !resource.IsValid())
			return;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		m_DamageManager.GetOwner().GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		
		IEntity tourniquet = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (!tourniquet)
			return;
		
		SCR_ConsumableItemComponent consumableItemComp = SCR_ConsumableItemComponent.Cast(tourniquet.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableItemComp)
			return;
		
		SCR_ConsumableEffectBase consumableEffect = consumableItemComp.GetConsumableEffect();
		if (!consumableEffect)
			return;
		
		ItemUseParameters params = ItemUseParameters();
		params.SetEntity(tourniquet);
		params.SetAllowMovementDuringAction(false);
		params.SetKeepInHandAfterSuccess(false);
		params.SetIntParam(m_DamageManager.FindAssociatedBandagingBodyPart(m_eCharacterHitZoneGroup));
		
		consumableEffect.ApplyEffect(m_DamageManager.GetOwner(), m_DamageManager.GetOwner(), tourniquet, params);
	}
}

//! Sets Character hit zone group to add/remove saline bag
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetSalineBaggedGroup : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "{00E36F41CA310E2A}Prefabs/Items/Medicine/SalineBag_01/SalineBag_US_01.et", desc: "Resource name of the Saline bag you want to use")]
	ResourceName m_sSalineBagPrefab;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		Resource resource = Resource.Load(m_sSalineBagPrefab);
		if (!resource)
			return;
		
		EntitySpawnParams spawnParams = new EntitySpawnParams();
		m_DamageManager.GetOwner().GetWorldTransform(spawnParams.Transform);
		spawnParams.TransformMode = ETransformMode.WORLD;
		
		IEntity salineBag = GetGame().SpawnEntityPrefab(resource, GetGame().GetWorld(), spawnParams);
		if (!salineBag)
			return;
		
		SCR_ConsumableItemComponent consumableItemComp = SCR_ConsumableItemComponent.Cast(salineBag.FindComponent(SCR_ConsumableItemComponent));
		if (!consumableItemComp)
			return;
		
		SCR_ConsumableEffectBase consumableEffect = consumableItemComp.GetConsumableEffect();
		if (!consumableEffect)
			return;
		
		consumableEffect.ApplyEffect(m_DamageManager.GetOwner(), m_DamageManager.GetOwner(), salineBag, null);
	}
}

//! Sets whether unconsciousness is allowed for this particular character
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetPermitUnconsciousness : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "true", uiwidget: UIWidgets.CheckBox, desc: "Whether unconsciousness is allowed for this particular character")]
	bool m_bPermitUnconsciousness;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetPermitUnconsciousness(m_bPermitUnconsciousness, true);
	}
}

//! Sets character bleeding rate multiplier for this particular character
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetBleedingRate : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character bleeding rate multiplier", params: "0 5 0.001")]
	float m_fBleedingRate;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetDOTScale(m_fBleedingRate, true);
	}
}

//! Sets character regeneration rate multiplier for this particular character
[BaseContainerProps(), SCR_ContainerMedicalActionTitle()]
class SCR_ScenarioFrameworkMedicalActionSetRegenerationRate : SCR_ScenarioFrameworkMedicalAction
{
	[Attribute(defvalue: "1", uiwidget: UIWidgets.Slider, desc: "Character regeneration rate multiplier", params: "0 5 0.001")]
	float m_fRegeneration;

	//------------------------------------------------------------------------------------------------
	override void OnActivate()
	{
		m_DamageManager.SetRegenScale(m_fRegeneration, true);
	}
}

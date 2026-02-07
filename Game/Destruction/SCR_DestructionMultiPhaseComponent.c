#define ENABLE_BASE_DESTRUCTION
//------------------------------------------------------------------------------------------------
enum EMaterialSoundType
{
	NONE = 0,
	GLASS = 1,
	WOOD_SOLID = 2,
	WOOD_PLANK_SMALL = 3,
	WOOD_PLANK_LARGE = 4,
	METAL_POLE = 5,
	PLASTIC_HOLLOW = 6,
	ROCK = 7,
	METAL_LIGHT = 8,
	ROCK_SMALL = 9,
	NETFENCE = 10,
	METAL_HEAVY = 11,
	METAL_GENERATOR = 12,
	PLASTIC_SOLID = 13,
	PIANO = 14,
	MATRESS = 15,
	BREAK_WOOD_SMALL = 16,
	//BREAK_WOOD_LARGE = 17,
	//BREAK_METAL_TINNY = 18,
	BREAK_METAL_REGULAR = 19,
	BREAK_PLASTIC = 20,
	BREAK_GLASSBOTTLE = 21,
	//BREAK_METAL_CRUNCHY = 22
	BELL_SMALL = 23
};

//------------------------------------------------------------------------------------------------
[ComponentEditorProps(category: "GameScripted/Destruction", description: "Multi-Phase destruction component, for objects that go through several damage phases")]
class SCR_DestructionMultiPhaseComponentClass: SCR_DestructionBaseComponentClass
{
	[Attribute("1", UIWidgets.CheckBox, "If true, the object will be deleted after being damaged beyond the final damage phase", category: "Destruction Multi-Phase")]
	bool m_bDeleteAfterFinalPhase;
	[Attribute("", UIWidgets.Object, "List of the individual damage phases (sequential) above initial damage phase", category: "Destruction Multi-Phase")]
	ref array<ref SCR_DamagePhaseData> m_aDamagePhases;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for destruction sound", "", ParamEnumArray.FromEnum(EMaterialSoundType))]
	EMaterialSoundType m_eMaterialSoundType;
};

//------------------------------------------------------------------------------------------------
//! Multi-Phase destruction component, for objects that go through several destruction states
class SCR_DestructionMultiPhaseComponent : SCR_DestructionBaseComponent
{
#ifdef ENABLE_BASE_DESTRUCTION
	private static int s_iFirstFreeDestructionMultiPhaseData = -1;
	private static ref array<ref SCR_DestructionMultiPhaseData> s_aDestructionMultiPhaseData = {};
	
	private int m_iDestructionMultiPhaseDataIndex = -1;
	
	protected int m_iOriginalHealth;
	
	//------------------------------------------------------------------------------------------------
	protected notnull SCR_DestructionMultiPhaseData GetDestructionMultiPhaseData()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			m_iDestructionMultiPhaseDataIndex = AllocateDestructionMultiPhaseData();
		
		return s_aDestructionMultiPhaseData[m_iDestructionMultiPhaseDataIndex];
	}
	
	//------------------------------------------------------------------------------------------------
	private int AllocateDestructionMultiPhaseData()
	{
		if (s_iFirstFreeDestructionMultiPhaseData == -1)
			return s_aDestructionMultiPhaseData.Insert(new SCR_DestructionMultiPhaseData());
		else
		{
			int returnIndex = s_iFirstFreeDestructionMultiPhaseData;
			SCR_DestructionMultiPhaseData data = s_aDestructionMultiPhaseData[returnIndex];
			s_iFirstFreeDestructionMultiPhaseData = data.m_iNextFreeIndex;
			data.m_iNextFreeIndex = -1;
			return returnIndex;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	private void FreeDestructionMultiPhaseData(int index)
	{
		s_aDestructionMultiPhaseData[index].Reset();
		s_aDestructionMultiPhaseData[index].m_iNextFreeIndex = s_iFirstFreeDestructionMultiPhaseData;
		s_iFirstFreeDestructionMultiPhaseData = index;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the input damage phase data (null = initial)
	SCR_DamagePhaseData GetDamagePhaseData(int damagePhase)
	{
		if (damagePhase <= 0 || damagePhase >= GetNumDamagePhases())
			return null;
		
		return SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner())).m_aDamagePhases.Get(damagePhase - 1);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns whether the object should disable physics on destruction (before being deleted or changed)
	override bool GetDisablePhysicsOnDestroy()
	{
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns if the object is in the initial damage phase
	bool GetIsInInitialDamagePhase()
	{
		return GetDamagePhase() == 0;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns which damage phase the object is currently in (0 = initial)
	int GetDamagePhase()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			return 0;
		
		return GetDestructionMultiPhaseData().GetDamagePhase();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the amount of damage phases (including initial)
	int GetNumDamagePhases()
	{
		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		
		if (componentData.m_aDamagePhases)
			return componentData.m_aDamagePhases.Count() + 1;
		else
			return 1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Returns the target damage stage for the input current health, current phase and damage
	int CalculateDamagePhase(int damagePhase, float health, float damage)
	{
		if (damage < 0)
			return damagePhase;
		
		while (damage >= 0)
		{
			damagePhase++;
			SCR_DamagePhaseData damagePhaseData = GetDamagePhaseData(damagePhase);
			if (!damagePhaseData)
				break;
			
			damage -= damagePhaseData.m_fPhaseHealth;
		}
		
		if (SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner())).m_bDeleteAfterFinalPhase)
			return Math.ClampInt(damagePhase, 0, GetNumDamagePhases()); // Clamp between initial and 1 past final phase
		else
			return Math.ClampInt(damagePhase, 0, GetNumDamagePhases() - 1);
	}


	//------------------------------------------------------------------------------------------------
	float GetDestructibleSize()
	{
		vector mins, maxs
		GetOwner().GetBounds(mins, maxs);
		maxs = maxs - mins;
		
		return Math.Max(maxs[2], Math.Max(maxs[0], maxs[1]));
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UpdateSoundSignal()
	{
		SoundComponent soundComponent = SoundComponent.Cast(GetOwner().FindComponent(SoundComponent));
		if (!soundComponent)
			return;
		
		// Use inverse value for cases when Multiphase component is not on entity
		soundComponent.SetSignalValueStr(SCR_MPDestructionManager.DAMAGE_PHASE_SIGNAL_NAME, GetNumDamagePhases() - GetDamagePhase());
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetOriginalResourceName(ResourceName originalResourceName)
	{
		GetDestructionMultiPhaseData().SetOriginalResourceName(originalResourceName);
	}
	
	//------------------------------------------------------------------------------------------------
	protected ResourceName GetOriginalResourceName()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			return "";
		
		return GetDestructionMultiPhaseData().GetOriginalResourceName();
	}
	
	//------------------------------------------------------------------------------------------------
	protected int GetTargetDamagePhase()
	{
		if (m_iDestructionMultiPhaseDataIndex == -1)
			return 0;
		
		return GetDestructionMultiPhaseData().GetTargetDamagePhase();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetTargetDamagePhase(int targetDamagePhase)
	{
		GetDestructionMultiPhaseData().SetTargetDamagePhase(targetDamagePhase);
	}
	
	//------------------------------------------------------------------------------------------------
	protected void SetDamagePhase(int damagePhase)
	{
		GetDestructionMultiPhaseData().SetDamagePhase(damagePhase);
		
		UpdateSoundSignal();
	}
	
	//------------------------------------------------------------------------------------------------
	//! Switches to the input damage phase (or deletes if past last phase)
	void GoToDamagePhase(int damagePhase)
	{
		if (damagePhase >= GetNumDamagePhases()) // Going past final phase, so delete if we can
		{
			s_OnDestructibleDestroyed.Invoke(this);
			
			//Play sound
			if (!s_bReadingInit)
				PlaySound();

			DeleteSelf();
			return;
		}
		
		int currentDamagePhase = GetDamagePhase();
		if (currentDamagePhase == damagePhase) // Same phase, ignore
			return;
		
		RegenerateNavmeshDelayed();
		
		if (currentDamagePhase == 0)
		{
			VObject vObject = GetOwner().GetVObject();
			if (vObject)
				SetOriginalResourceName(vObject.GetResourceName());
		}
		
		SetDamagePhase(damagePhase);
		SetTargetDamagePhase(damagePhase);
		
		// Play sound before we change m_iDamagePhase, because we want to play the current phase sound
		if (!s_bReadingInit)
			PlaySound();
		
		ApplyDamagePhaseData(GetDamagePhaseData(GetDamagePhase()));
	}
	
	//------------------------------------------------------------------------------------------------
	void ApplyDamagePhaseData(SCR_DamagePhaseData pDamagePhaseData)
	{
		if (!pDamagePhaseData)
		{
			SetHitZoneHealth(m_iOriginalHealth);
			GetDestructionBaseData().SetPreviousHealth(GetHealth());
			SetModel(GetOriginalResourceName());
			return;
		}
		
		GetDestructionBaseData().SetPreviousHealth(pDamagePhaseData.m_fPhaseHealth);
		SetHitZoneHealth(pDamagePhaseData.m_fPhaseHealth);
		SetModel(pDamagePhaseData.m_PhaseModel);
	}

	//------------------------------------------------------------------------------------------------
	override void PlaySound()
	{
		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (componentData.m_eMaterialSoundType == 0)
			return;
		
		SCR_MPDestructionManager destructionManager = SCR_MPDestructionManager.GetInstance();
		if (!destructionManager)
			return;
		
		SimpleSoundComponent soundComponent = destructionManager.GetSoundComponent();
		if (!soundComponent)
			return;
		
		// Set signals
	    soundComponent.SetSignalValue(destructionManager.GetEntitySizeSignalID(), GetDestructibleSize());
		soundComponent.SetSignalValue(destructionManager.GetDamagePhaseSignalID(), GetNumDamagePhases() - GetDamagePhase());
		// Set Impulse signal to very heigh value (choosen in relationship with audio project setup), so audio system uses sound with heighest intensity
		soundComponent.SetSignalValue(destructionManager.GetImpulseSignalID(), 100);
		
		// Set position
		vector mat[4];
		
		// Get position offset for slotted entities
		vector mins;
		vector maxs;
		GetOwner().GetWorldBounds(mins, maxs);
		
		for (int i = 0; i < 3; i++)
		{
			mat[3][i] = mins[i] + Math.AbsFloat(((maxs[i] - mins[i]) * 0.5));
		}
				
		soundComponent.SetTransformation(mat);
		
		// Play sound
		soundComponent.PlayStr(SCR_SoundEvent.SOUND_MPD_ + typename.EnumToString(EMaterialSoundType, componentData.m_eMaterialSoundType));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the object receives damage, return false if damage should be ignored
	override bool GetCanBeDamaged()
	{
		SCR_DestructionMultiPhaseComponentClass componentData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!componentData)
			return true;
		
		if (componentData.m_bDeleteAfterFinalPhase)
			return GetDamagePhase() < GetNumDamagePhases();
		
		return GetDamagePhase() < GetNumDamagePhases() - 1;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Spawns objects that are meant to be created when the object is destroyed (particles, debris, etc)
	override void SpawnDestroyObjects()
	{
		Physics ownerPhysics = GetOwner().GetPhysics();
		
		if (GetIsInInitialDamagePhase()) // Initial phase
		{
			array<ref SCR_BaseSpawnable> destroySpawnObjects = SCR_DestructionBaseComponentClass.Cast(GetComponentData(GetOwner())).m_DestroySpawnObjects;
			int numSpawnOnDestroy = destroySpawnObjects.Count();
			for (int i = 0; i < numSpawnOnDestroy; i++)
			{
				SCR_BaseSpawnable spawnObject = destroySpawnObjects[i];
				spawnObject.Spawn(GetOwner(), ownerPhysics, GetDestructionHitInfo());
			}
		}
		else
		{
			SCR_DamagePhaseData damagePhaseData = GetDamagePhaseData(GetDamagePhase());
			if (damagePhaseData) // We have data for phase
			{
				int numSpawnOnDestroy = damagePhaseData.m_PhaseDestroySpawnObjects.Count();
				for (int i = 0; i < numSpawnOnDestroy; i++)
				{
					SCR_BaseSpawnable spawnObject = damagePhaseData.m_PhaseDestroySpawnObjects[i];
					spawnObject.Spawn(GetOwner(), ownerPhysics, GetDestructionHitInfo());
				}
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Called when the object should be destroyed and various effects or other things need to be performed (actual destruction handled HandleDestruction())
	override void QueueDestroy()
	{
		SCR_DestructionHitInfo hitInfo = GetDestructionHitInfo();
		if (!hitInfo)
			return;
		
		if (hitInfo.m_TotalDestruction)
			return;
		
		// Calculate which damage phase we should jump to in case we are meant to skip through multiple
		SetTargetDamagePhase(CalculateDamagePhase(GetDamagePhase(), hitInfo.m_LastHealth, hitInfo.m_HitDamage));
	}
	
	//------------------------------------------------------------------------------------------------
	//! Handle destruction
	override void HandleDestruction()
	{
		SCR_DestructionHitInfo hitInfo = GetDestructionHitInfo();
		if (!hitInfo)
			return;
		
		if (hitInfo.m_TotalDestruction) // Total destruction, so just delete
		{
			DeleteSelf();
		}
		else // Go to the stored target damage phase
		{
			SetHitZoneDamage(0);
			if (hitInfo)
				GetDestructionBaseData().DeleteHitInfo();
			GoToDamagePhase(GetTargetDamagePhase());
		}
	}
	
	//------------------------------------------------------------------------------------------------
	override void UpdateResponseIndex()
	{
		SCR_DestructionMultiPhaseComponentClass prefabData = SCR_DestructionMultiPhaseComponentClass.Cast(GetComponentData(GetOwner()));
		if (!prefabData || !prefabData.m_bDeleteAfterFinalPhase)
			return; // We don't want to drive through indestructible objects
		
		Physics physics = GetOwner().GetPhysics();
		if (!physics)
			return; // No physics, no need to update anything
		
		if (GetDamagePhase() + 1 >= GetNumDamagePhases()) // Is in last phase and will be destroyed after it
		{
			super.UpdateResponseIndex();
			return;
		}
		
		// Not the last phase - don't update the response index
	}
	
	//------------------------------------------------------------------------------------------------
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RPC_ReplicateMultiPhaseDestructionState(int phase)
	{
		GoToDamagePhase(phase);
	}
	
	//------------------------------------------------------------------------------------------------
	override void ReplicateDestructibleState(bool forceUpdate = false)
	{
		int damagePhase = GetDamagePhase();
		
		// Only send update if this destructible has changed phase
		if (forceUpdate || damagePhase != 0)
			Rpc(RPC_ReplicateMultiPhaseDestructionState, damagePhase);
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplSave(ScriptBitWriter writer)
	{
		if (!super.OnRplSave(writer))
			return false;
		
		bool writeDamagePhase = GetDamagePhase() != 0;
		writer.Write(writeDamagePhase, 1);
		
		if (writeDamagePhase)
			writer.Write(GetDamagePhase(), 8); // Write which damage phase we are in
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected override event bool OnRplLoad(ScriptBitReader reader)
	{
		if (!super.OnRplLoad(reader))
			return false;
		
		bool readDamagePhase;
		reader.Read(readDamagePhase, 1);
		
		if (!readDamagePhase)
			return true;
		
		int damagePhase;
		reader.Read(damagePhase, 8); // Read which damage phase we are meant to be in
		
		if (damagePhase != GetDamagePhase())
			GoToDamagePhase(damagePhase);
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		
		m_iOriginalHealth = GetHealth();
		
		UpdateSoundSignal();
	}
#endif
};

//------------------------------------------------------------------------------------------------
class SCR_DamagePhaseTitle : BaseContainerCustomTitle
{
	override bool _WB_GetCustomTitle(BaseContainer source, out string title)
	{
		float health = 0;
		source.Get("m_fPhaseHealth", health);
		title = "Phase | HP: " + health.ToString();
		return true;
	}
};

//------------------------------------------------------------------------------------------------
//! Multi-Phase destruction phase
[BaseContainerProps(), SCR_DamagePhaseTitle()]
class SCR_DamagePhaseData
{
	[Attribute("100", UIWidgets.Slider, "Base health value for the damage phase. Upon switching to this phase, this value replaces the base health of the object", "0.01 100000 0.01")]
	float m_fPhaseHealth;
	[Attribute(ResourceName.Empty, UIWidgets.ResourcePickerThumbnail, "Model to use for the damage phase", "xob")]
	ResourceName m_PhaseModel;
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the phase")]
	ref array<ref SCR_BaseSpawnable> m_PhaseDestroySpawnObjects;
};

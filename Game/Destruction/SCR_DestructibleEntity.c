[EntityEditorProps(category: "GameScripted/Destructibles", visible: false, dynamicBox: true)]
class SCR_DestructibleEntityClass: DestructibleEntityClass
{
	[Attribute("0.05", UIWidgets.Slider, "Contact momentum to damage multiplier", "0.01 10 0.01", category: "Destruction Setup")]
	float m_fMomentumToDamageScale;
	
	[Attribute("", UIWidgets.Object, "List of objects (particles, debris, etc) to spawn on destruction of the object", category: "Destruction FX")]
	ref array<ref SCR_BaseSpawnable> m_aDestroySpawnObjects;
	
	[Attribute("0", uiwidget: UIWidgets.ComboBox, "Type of material for destruction sound", "", ParamEnumArray.FromEnum(SCR_EMaterialSoundTypeBreak))]
	SCR_EMaterialSoundTypeBreak m_eMaterialSoundType;
}

class SCR_DestructibleEntity: DestructibleEntity
{
	protected static const int MIN_MOMENTUM_RESPONSE_INDEX = 1;
	protected static const int MIN_DESTRUCTION_RESPONSE_INDEX = 6;
	static const int TOTAL_DESTRUCTION_MAX_HEALTH_MULTIPLIER = 10;
	
	//------------------------------------------------------------------------------------------------
	float GetDamageMultiplier(EDamageType type)
	{
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (prefabData)
			return prefabData.GetDamageMultiplier(type);
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDamageReduction()
	{
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (prefabData)
			return prefabData.GetDamageReduction();
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetDamageThreshold()
	{
		DestructibleEntityClass prefabData = DestructibleEntityClass.Cast(GetPrefabData());
		if (prefabData)
			return prefabData.GetDamageThreshold();
		
		return 0;
	}
	
	//------------------------------------------------------------------------------------------------
	float GetMaxHealth()
	{
		DestructibleEntityClass prefabData = DestructibleEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return 0;
		
		return prefabData.GetMaxHealth();
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnDamage(int previousState, int newState, EDamageType type, float damageTaken, float currentHealth, inout vector hitTransform[3], ScriptBitWriter frameData)
	{
		float maxHealth = GetMaxHealth();
		SCR_DestructionUtility.UpdateResponseIndex(GetPhysics(), currentHealth, maxHealth);
		if (!frameData)
			return;
		
		// Handling double damage in one frame:
		// FrameData exists from previous damage - the object was damaged twice in a frame, the second damage didn't actually change the phase,
		// therefore not writing it into the destructionData.
		if (previousState == newState)
			return;
		
		SCR_DestructionData destructionData = new SCR_DestructionData();
		destructionData.m_vHitPosition = hitTransform[0];
		destructionData.m_vHitDirection = hitTransform[1].Normalized();
		destructionData.m_vHitNormal = hitTransform[2].Normalized();
		destructionData.m_fHitDamage = damageTaken;
		destructionData.m_eDamageType = type;
		destructionData.m_bTotalDestruction = damageTaken > maxHealth * TOTAL_DESTRUCTION_MAX_HEALTH_MULTIPLIER;
		destructionData.m_iPreviousPhase = previousState;
		destructionData.Save(frameData);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Only call from OnStateChanged, otherwise you have HUGE desync
	protected void GoToDamagePhase(int damagePhaseIndex, int previousDamagePhaseIndex, SCR_DestructionData destructionData, bool streamed)
	{
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return;
		
		SCR_BaseDestructionPhase phase = SCR_BaseDestructionPhase.Cast(prefabData.GetDestructionPhase(damagePhaseIndex));
		if (!phase)
			return; // Should never happen, unless some memory issue happens
		
		SCR_DestructionUtility.RegenerateNavmeshDelayed(this);
		SCR_DestructionUtility.SetDamagePhaseSignal(this, damagePhaseIndex);

		
		//test
		HeatmapPrototype();
		
		VObject model = phase.GetModel();
		if (!model)
			return;

		SCR_DestructionUtility.SetModel(this, model);

		if (streamed)
			return;// When streamed, we don't care about sounds or effects, we just change the model and quit
		
		bool destroyAtNoHealth;
		prefabData.GetPrefab().Get("DestroyAtNoHealth", destroyAtNoHealth);

		if (prefabData.m_eMaterialSoundType > 0)
			SCR_DestructionUtility.PlaySound(this, prefabData.m_eMaterialSoundType, prefabData.GetNumDestructionPhases(), damagePhaseIndex);
		
		if (damagePhaseIndex == prefabData.GetNumDestructionPhases() - 1 && destroyAtNoHealth) // is last phase and gets deleted
		{
			SCR_DestructionUtility.SpawnDestroyObjects(this, phase.m_aPhaseDestroySpawnObjects, SCR_DestructionHitInfo.FromDestructionData(destructionData));
			return;
		}
		
		SCR_BaseDestructionPhase previousPhase = SCR_BaseDestructionPhase.Cast(prefabData.GetDestructionPhase(previousDamagePhaseIndex));
		if (!previousPhase)
			return; // Should never happen, unless some memory issue happens
		
		SCR_DestructionUtility.SpawnDestroyObjects(this, previousPhase.m_aPhaseDestroySpawnObjects, SCR_DestructionHitInfo.FromDestructionData(destructionData));
	}
	
	void HeatmapPrototype()
	{
		ChimeraWorld world = ChimeraWorld.CastFrom(GetWorld());
		
		if(!world)
			return;
	
		DestructionManager manager = world.GetDestructionManager();	
	
		if(!manager)
			return;
		
		SCR_DestructionHeatmapEntry entry = new SCR_DestructionHeatmapEntry;
		
		vector minsOffset = "-5 -5 -5";
		vector maxsOffset = "5 5 5";
		
		minsOffset = minsOffset*1.5;
		maxsOffset = maxsOffset*1.5;
		entry.SetCategory(EDestructionHeatmapCategory.FOREST);
		entry.SetWeight(10);
		entry.SetBoundingBox(GetOrigin() + minsOffset, GetOrigin() + maxsOffset);
		manager.QueueDestructionHeatmapEntry(entry);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnStateChanged(int destructibleState, ScriptBitReader frameData, bool JIP)
	{
		SCR_DestructionData destructionData = new SCR_DestructionData();
		if (frameData)
			destructionData.Load(frameData);
		
		GoToDamagePhase(destructibleState, destructionData.m_iPreviousPhase, destructionData, JIP);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnBeforeDestroyed()
	{
		SCR_DestructionUtility.RegenerateNavmeshDelayed(this);
	}
	
	//------------------------------------------------------------------------------------------------
	protected bool FilterContact(IEntity owner, IEntity other, Contact contact)
	{	
		if (!other)
			return false;
		
		if (!owner.GetPhysics() || !other.GetPhysics())
			return false;
		
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());
		if (!prefabData)
			return false;
		
		if (other.IsInherited(SCR_DebrisSmallEntity)) // Ignore impacts from debris
			return false;
		
		if (ChimeraCharacter.Cast(other)) // Ignore impacts from characters
			return false;
		
		return true;
	}
	
	//------------------------------------------------------------------------------------------------
	protected void OnFilteredContact(IEntity owner, IEntity other, Contact contact)
	{
		Physics ownerPhysics = contact.Physics1;
		Physics otherPhysics = contact.Physics2;
		
		float ownerMass = ownerPhysics.GetMass();
		float otherMass = otherPhysics.GetMass();
		int ownerReponseIndex = ownerPhysics.GetResponseIndex();
		int otherResponseIndex = otherPhysics.GetResponseIndex();
		
		SCR_DestructibleEntityClass prefabData = SCR_DestructibleEntityClass.Cast(GetPrefabData());		
		float damage = prefabData.GetMaxHealth();
		EDamageType damageType = EDamageType.TRUE;
		// If vehicle response index is the same or higher -> automatically destroy
		// Otherwise -> apply damage
		if (otherResponseIndex - MIN_MOMENTUM_RESPONSE_INDEX < ownerReponseIndex - MIN_DESTRUCTION_RESPONSE_INDEX)
		{
			float momentum = SCR_DestructionUtility.CalculateMomentum(contact, ownerMass, otherMass);
			damage = momentum * prefabData.m_fMomentumToDamageScale;
			damageType = EDamageType.COLLISION;
		}
		
		vector outMat[3];
		vector relVel = contact.VelocityBefore2 - contact.VelocityBefore1;
		outMat[0] = contact.Position; // Hit position
		outMat[1] = relVel.Normalized(); // Hit direction
		outMat[2] = contact.Normal; // Hit normal
		
		HandleDamage(damageType, damage, outMat);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Contact
	override void EOnContact(IEntity owner, IEntity other, Contact contact)
	{	
		// Call FilterContact to check if contact should be registered
		// This is done for consistency's sake since that's how damage manager component does it. 
		// Damage manager does not have access to EOnContact however.
		if (!FilterContact(owner, other, contact))
			return;
		
		OnFilteredContact(owner, other, contact);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_DestructibleEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.CONTACT);
	}
};

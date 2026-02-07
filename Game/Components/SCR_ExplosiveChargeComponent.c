[ComponentEditorProps(category: "GameScripted/Misc", description: "")]
class SCR_ExplosiveChargeComponentClass : ScriptGameComponentClass
{
}

class SCR_ExplosiveChargeComponent : ScriptGameComponent
{
	protected float m_fTimer;
	protected RplId m_RplID;
	protected RplComponent m_RplComp;
	protected SCR_ExplosiveTriggerComponent m_Trigger;
	protected float m_fFuzeTime = 20;

	[RplProp(onRplName: "OnFuzeTypeChanged")]
	protected SCR_EFuzeType m_eUsedFuzeType = SCR_EFuzeType.NONE;

	[RplProp(onRplName: "OnArrayOfConnectedDetonatorsChanged")]
	protected ref array<RplId> m_aConnectedDetonators = {};

	//------------------------------------------------------------------------------------------------
	void OnFuzeTypeChanged()
	{
		UpdateFuzeVisibility();
	}

	//------------------------------------------------------------------------------------------------
	void OnArrayOfConnectedDetonatorsChanged()
	{
		UpdateFuzeVisibility();
	}

	//------------------------------------------------------------------------------------------------
	float GetTimeOfDetonation()
	{
		return m_fTimer;
	}

	//------------------------------------------------------------------------------------------------
	SCR_EFuzeType GetUsedFuzeType()
	{
		return m_eUsedFuzeType;
	}

	//------------------------------------------------------------------------------------------------
	SCR_ExplosiveTriggerComponent GetTrigger()
	{
		return m_Trigger;
	}

	//------------------------------------------------------------------------------------------------
	array<RplId> GetConnectedDetonators()
	{
		return m_aConnectedDetonators;
	}

	//------------------------------------------------------------------------------------------------
	int GetNumberOfConnectedDetonators()
	{
		return m_aConnectedDetonators.Count();
	}

	//------------------------------------------------------------------------------------------------
	bool IsDetonatorConnected(RplId detonatorId)
	{
		return m_aConnectedDetonators.Contains(detonatorId);
	}

	//------------------------------------------------------------------------------------------------
	float GetFuzeTime()
	{
		return m_fFuzeTime;
	}

	//------------------------------------------------------------------------------------------------
	//! Change the time which will be used for timed fuze
	void SetFuzeTime(float fuzeDelay, bool silent = false)
	{
		if (float.AlmostEqual(fuzeDelay, m_fFuzeTime))
			return;

		m_fFuzeTime = fuzeDelay;

		if (silent)
			return;

		PlaySound(SCR_SoundEvent.SOUND_EXPLOSIVE_ADJUST_TIMER);
	}

	//------------------------------------------------------------------------------------------------
	protected void PlaySound(string soundName)
	{
		SCR_SoundManagerEntity soundManager = GetGame().GetSoundManagerEntity();
		if (!soundManager)
			return;

		soundManager.CreateAndPlayAudioSource(GetOwner(), soundName);
	}

	//------------------------------------------------------------------------------------------------
	//! Changes if garbage system should manage this entity
	//! \param[in] garbageCollectable true if this entity should be inserted into garbage system
	protected void SetGarbageCollectable(bool garbageCollectable)
	{
		SCR_GarbageSystem garbageManager = SCR_GarbageSystem.GetByEntityWorld(GetOwner());
		if (!garbageManager)
			return;

		garbageManager.UpdateBlacklist(GetOwner(), !garbageCollectable);
		if (garbageCollectable)
			garbageManager.Insert(GetOwner());
	}

	//------------------------------------------------------------------------------------------------
	//! Arms the charge and starts the timer for the detonation
	void ArmWithTimedFuze(bool silent = false)
	{
		m_eUsedFuzeType = SCR_EFuzeType.TIMED;
		UpdateFuzeVisibility();
		ChangeLockState(true);

		if (m_RplComp && !m_RplComp.Role())
		{
			m_fTimer = GetGame().GetWorld().GetWorldTime() + m_fFuzeTime * 1000;
			SetEventMask(GetOwner(), EntityEvent.FRAME);
			SetGarbageCollectable(false);
			Replication.BumpMe();
		}

		if (!silent)
			PlaySound(SCR_SoundEvent.SOUND_EXPLOSIVE_ARM);
	}

	//------------------------------------------------------------------------------------------------
	//! Arms the charge with remotely detonated fuze.
	//! Adds provided detonator RplId unless its already on the list
	void ConnectDetonator(SCR_EFuzeType fuzeType, RplId detonatorId = RplId.Invalid(), bool shouldReplicate = true, bool silent = false)
	{
		if (fuzeType != m_eUsedFuzeType && m_eUsedFuzeType != SCR_EFuzeType.NONE)
			return;

		if (!m_aConnectedDetonators.Contains(detonatorId))
			m_aConnectedDetonators.Insert(detonatorId);

		m_eUsedFuzeType = fuzeType;
		UpdateFuzeVisibility();
		ChangeLockState(true);
		if (shouldReplicate && m_RplComp && !m_RplComp.Role())
		{
			SetGarbageCollectable(false);
			Replication.BumpMe();
		}

		if (silent)
			return;

		if (m_eUsedFuzeType == SCR_EFuzeType.REMOTE)
			PlaySound(SCR_SoundEvent.SOUND_EXPLOSIVE_CONNECT_WIRES);
		else
			PlaySound(SCR_SoundEvent.SOUND_EXPLOSIVE_ARM);
	}

	//------------------------------------------------------------------------------------------------
	//! Removes provided detonator RplId unless its not there in the first place
	void RemoveDetonatorFromTheList(RplId detonatorId, bool shouldReplicate = true)
	{
		int index = m_aConnectedDetonators.Find(detonatorId);
		if (index == -1)
			return;

		m_aConnectedDetonators.Remove(index);
		if (m_aConnectedDetonators.Count() < 1)
		{
			m_eUsedFuzeType = SCR_EFuzeType.NONE;
			UpdateFuzeVisibility();
			ChangeLockState();
		}

		if (shouldReplicate && m_RplComp && !m_RplComp.Role())
		{
			SetGarbageCollectable(true);
			Replication.BumpMe();
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Replaces connected detonator RplId with provided id which by default will be invalid
	void ReplaceDetonatorFromTheList(RplId detonatorIdToReplace, RplId replaceWith = RplId.Invalid())
	{
		ConnectDetonator(m_eUsedFuzeType, replaceWith, false, true);
		RemoveDetonatorFromTheList(detonatorIdToReplace, false);

		if (m_RplComp && !m_RplComp.Role())
			Replication.BumpMe();
	}

	//------------------------------------------------------------------------------------------------
	//! Change visibility of sloted fuze
	protected void UpdateFuzeVisibility()
	{
		SlotManagerComponent slotManagerComp = SlotManagerComponent.Cast(GetOwner().FindComponent(SlotManagerComponent));
		if (!slotManagerComp)
			return;

		array<EntitySlotInfo> slots = {};
		if (!slotManagerComp.GetSlotInfos(slots))
			return;

		FuzeSlotInfo fuzeSlot;
		IEntity fuzeEntity;
		foreach (EntitySlotInfo slot : slots)
		{
			fuzeSlot = FuzeSlotInfo.Cast(slot);
			if (!fuzeSlot)
				continue;

			fuzeEntity = fuzeSlot.GetAttachedEntity();
			if (!fuzeEntity)
				continue;

			if (fuzeSlot.GetFuzeType() == m_eUsedFuzeType)
				fuzeEntity.SetFlags(EntityFlags.VISIBLE);
			else
				fuzeEntity.ClearFlags(EntityFlags.VISIBLE);
		}
	}

	//------------------------------------------------------------------------------------------------
	//! Callback method to ensure that charge is disarmed when transferred but with no sound
	void DisarmChargeSilent()
	{
		DisarmCharge(false);
	}

	//------------------------------------------------------------------------------------------------
	//! Will clear the array of connected detonators, change fuze visibility and update their previously
	//! connected detonators with false info to not give players on the other end information that
	//! someone is tampering with their charges
	void DisarmCharge(bool playSound = true)
	{
		if (m_eUsedFuzeType == SCR_EFuzeType.REMOTE)
		{
			if (!m_aConnectedDetonators.Count())
				return;

			RplId triggerId = Replication.FindId(this);
			if (!triggerId.IsValid())
				return;

			SCR_DetonatorGadgetComponent detonatorComp;
			foreach (RplId detonatorId : m_aConnectedDetonators)
			{
				if (!detonatorId.IsValid())
					continue;

				detonatorComp = SCR_DetonatorGadgetComponent.Cast(Replication.FindItem(detonatorId));
				if (!detonatorComp)
					continue;

				detonatorComp.ReplaceChargeFromTheList(triggerId, RplId.Invalid());
			}
		}
		else if (m_eUsedFuzeType == SCR_EFuzeType.TIMED)
		{
			ClearEventMask(GetOwner(), EntityEvent.FRAME);
			m_fTimer = -1;
		}

		ChangeLockState();
		m_eUsedFuzeType = SCR_EFuzeType.NONE;
		m_aConnectedDetonators.Clear();
		UpdateFuzeVisibility();
		if (m_RplComp && !m_RplComp.Role())
		{
			SetGarbageCollectable(true);
			Replication.BumpMe();
		}

		if (playSound)
			PlaySound(SCR_SoundEvent.SOUND_EXPLOSIVE_DISARM);
	}

	//------------------------------------------------------------------------------------------------
	//! When item is locked then players wont be able to pick it up.
	protected void ChangeLockState(bool locked = false)
	{
		InventoryItemComponent iic = InventoryItemComponent.Cast(GetOwner().FindComponent(InventoryItemComponent));
		if (!iic)
			return;

		iic.RequestUserLock(GetOwner(), locked);
	}

	//------------------------------------------------------------------------------------------------
	//! Checks validity and returns RplId of this component
	RplId GetRplId()
	{
		if (CheckRplId())
			return m_RplID;

		return RplId.Invalid();
	}

	//------------------------------------------------------------------------------------------------
	//! Checks validaty of RplId and if that fails then tries to find correct RplId for that trigger
	protected bool CheckRplId()
	{
		if (!m_RplID.IsValid())
		{
			m_RplID = Replication.FindId(this);
			if (!m_RplID.IsValid())
				return false;
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (GetGame().GetWorld().GetWorldTime() > m_fTimer)
		{
			ClearEventMask(owner, EntityEvent.FRAME);
			m_Trigger.UseTrigger();
		}
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		m_RplID = Replication.FindId(this);
		m_RplComp = RplComponent.Cast(owner.FindComponent(RplComponent));
		m_Trigger = SCR_ExplosiveTriggerComponent.Cast(owner.FindComponent(SCR_ExplosiveTriggerComponent));
		InventoryItemComponent iic = InventoryItemComponent.Cast(owner.FindComponent(InventoryItemComponent));
		if (iic)
			iic.m_OnParentSlotChangedInvoker.Insert(DisarmChargeSilent);

		UpdateFuzeVisibility();
	}

	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);

		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	override void OnDelete(IEntity owner)
	{
		DisarmCharge(false);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplSave(ScriptBitWriter writer)
	{
		writer.WriteInt(m_eUsedFuzeType);
		writer.WriteInt(m_aConnectedDetonators.Count());
		foreach (RplId detonatorId : m_aConnectedDetonators)
		{
			writer.WriteRplId(detonatorId);
		}
		writer.WriteFloat(m_fTimer);

		return super.RplSave(writer);
	}

	//------------------------------------------------------------------------------------------------
	override bool RplLoad(ScriptBitReader reader)
	{
		reader.ReadInt(m_eUsedFuzeType);
		int numberOfConnectedDetonators;
		reader.ReadInt(numberOfConnectedDetonators);
		for (int i; i < numberOfConnectedDetonators; i++)
		{
			RplId rplIdOutput;
			reader.ReadRplId(rplIdOutput);
			if (rplIdOutput.IsValid())
				m_aConnectedDetonators.Insert(rplIdOutput);
		}
		UpdateFuzeVisibility();
		reader.ReadFloat(m_fTimer);

		return super.RplLoad(reader);
	}
}

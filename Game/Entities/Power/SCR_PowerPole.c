[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_PowerPoleClass: GenericEntityClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_PowerPole : GenericEntity
{
	[Attribute(desc: "Slots of this power pole for connecting with other power poles.")]
	ref array<ref SCR_PowerPoleSlotBase> m_aSlots;
	
	[Attribute(desc: "Draw debug shapes?")]
	bool m_bDrawDebugShapes;
	
	[Attribute(desc: "Static ID of this entity used for linking.")]
	int m_iStaticID;
	
	[Attribute(desc: "Static ID of an entity this entity is connected to.")]
	int m_iAttachedToID;
	
	ref array<ref Shape> m_aDebugShapes = new ref array<ref Shape>();
	IEntitySource m_Source;
	
	#ifdef WORKBENCH
	// every use of m_API should be wrapped by WORKBENCH define
	static WorldEditorAPI m_API;
	#endif

	//------------------------------------------------------------------------------------------------
	int GetSlotsCount(bool sameLine = true)
	{
		return m_aSlots.Count();
	}
	
	//------------------------------------------------------------------------------------------------
	vector GetSlot(int index, bool sameLine)
	{
		return m_aSlots[index].m_vSlotA;
	}
	
	//------------------------------------------------------------------------------------------------
	//! \Tries to find position of the closest compatible slot.
	//! \return vector with world origin of the closest compatible slot.
	//! \param index is the index of the other slot, this method tries to find the slot under the same index.
	//! \param otherSlot is the local position of other slot.
	vector TryGetSlot(int index, vector otherSlot, bool sameLine)
	{
		if (index >= m_aSlots.Count())
		{
			return m_aSlots[index % m_aSlots.Count()].m_vSlotA;
			return vector.Zero;
		}
		
		SCR_PowerPoleSlot dualSlot = SCR_PowerPoleSlot.Cast(m_aSlots[index]);
		if (dualSlot)
		{
			if (vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotA)) > vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotB)))
				return CoordToParent(dualSlot.m_vSlotB);
			else
				return CoordToParent(dualSlot.m_vSlotA);
		}
		
		SCR_PowerPoleSlotSingle singleSlot = SCR_PowerPoleSlotSingle.Cast(m_aSlots[index]);
		if (singleSlot)
		{
			return CoordToParent(singleSlot.m_vSlotA);
		}
		
		return vector.Zero;
	}
	
	#ifdef WORKBENCH
	//-----------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		DrawDebugShapes();
		return false;
	}
	
	override void _WB_OnCreate(IEntitySource src)
	{
		// this one is important ... any code using the WorldEditorAPI interface is safe only certain functions
		// see the gameLibEntities.c for a reference - it is mentioned in the description
		// examples of supported functions: _WB_OnKeyChanged, _WB_AfterWorldUpdate, _WB_OnCreate, etc.
		
		// the functionality of RegisterSelf should go here
		
		// this was causing the asserts and crashes yesterday
		// you were changing entity values while an entity was being created.
		// this resulted in throwing the old object away and recreating an entity -> bad things		
		
		// similar - AttachTo might be dangerous
	}
	#endif
	
	//------------------------------------------------------------------------------------------------
	//! \Attaches this entity to the other entity.
	//! \param other is the other entity we are trying to attach to.
	void AttachTo(IEntity other)
	{
		return;
		// Is the other entity a power pole?
		auto otherPowerPole = SCR_PowerPole.Cast(other);
		
		if (!otherPowerPole)
			return;
		
		// Not set to be attached automatically yet
		if (m_iAttachedToID <= 0)
		{
			#ifdef WORKBENCH
			if (m_API)
			{
				if (!m_API.UndoOrRedoIsRestoring() && m_Source)
				{
					auto containerPath = new array<ref ContainerIdPathEntry>();
					if (m_API.IsDoingEditAction())
						m_API.SetVariableValue(m_Source, containerPath, "m_iAttachedToID", SCR_StaticLinkingSystem.GetInstance().FindID(other).ToString());
					else
					{
						m_API.BeginEntityAction();
						m_API.SetVariableValue(m_Source, containerPath, "m_iAttachedToID", SCR_StaticLinkingSystem.GetInstance().FindID(other).ToString());
						m_API.EndEntityAction();
					}
					return;
				}
			}
			#endif
		}
		
		// Go through all the slots and attach them one by one
		for (int i = 0, count = m_aSlots.Count(); i < count; i ++)
		{
			vector otherSlot, thisSlot; //Slot positions in world coords
			Shape shape;
			
			m_aSlots[i].AttachTo(this, otherPowerPole, i, thisSlot, otherSlot);
			
			if (m_bDrawDebugShapes && otherSlot != vector.Zero)
			{
				shape = Shape.Create(ShapeType.LINE, ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, thisSlot, otherSlot);
				m_aDebugShapes.Insert(shape);
			}
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void DrawDebugShapes()
	{
		if (m_aDebugShapes)
			m_aDebugShapes.Clear();
		
		if (!m_bDrawDebugShapes)
			return;
		
		if (!m_aDebugShapes)
			m_aDebugShapes = new ref array<ref Shape>();
		
		foreach (SCR_PowerPoleSlotBase slot: m_aSlots)
		{
			slot.DrawDebugShapes(m_aDebugShapes, this);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void RegisterSelf(IEntitySource source)
	{
		if (m_iStaticID > 0)
			SCR_StaticLinkingSystem.GetInstance().AddEntry(this, m_iStaticID);
		else
		{
			int id = SCR_StaticLinkingSystem.GetInstance().AddEntry(this);
			#ifdef WORKBENCH
			if (m_API)
			{
				//m_API.BeginEntityAction();
				if (m_API.UndoOrRedoIsRestoring())
					return;
				if (source)
				{
					auto containerPath = new array<ref ContainerIdPathEntry>();
					if (m_API.IsDoingEditAction())
						m_API.SetVariableValue(source, containerPath, "m_iStaticID", id.ToString());
					else
					{
						m_API.BeginEntityAction();
						m_API.SetVariableValue(source, containerPath, "m_iStaticID", id.ToString());
						m_API.EndEntityAction();
					}
					return;
				}
			}
			#endif
			m_iStaticID = id;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void UnregisterSelf()
	{
		SCR_StaticLinkingSystem.GetInstance().RemoveEntry(m_iStaticID);
	}
	
	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		if (m_iAttachedToID > 0)
		{
			IEntity foundEntity = SCR_StaticLinkingSystem.GetInstance().FindEntry(m_iAttachedToID);
			if (foundEntity)
				AttachTo(foundEntity);
		}
		
		ClearFlags(EntityFlags.ACTIVE, true);
	}
	
	//------------------------------------------------------------------------------------------------
	void SCR_PowerPole(IEntitySource src, IEntity parent)
	{
		#ifdef WORKBENCH
		if (!m_API)
			m_API = _WB_GetEditorAPI();
		#endif
		DrawDebugShapes();
		//RegisterSelf(src);
		m_Source = src;
		SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_PowerPole()
	{
		if (m_aDebugShapes)
			m_aDebugShapes.Clear();
		m_aDebugShapes = null;
		
		if (m_aSlots)
			m_aSlots.Clear();
		m_aSlots = null;
		
		//UnregisterSelf();
	}

};

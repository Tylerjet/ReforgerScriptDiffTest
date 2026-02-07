[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_JunctionPowerPoleClass: SCR_PowerPoleClass
{
};

//------------------------------------------------------------------------------------------------
class SCR_JunctionPowerPole : SCR_PowerPole
{
	[Attribute(desc: "Slots of this power pole for connecting with other power poles - Junction slots.")]
	protected ref array<ref SCR_PowerPoleSlotBase> m_aJunctionSlots;

	//------------------------------------------------------------------------------------------------
	override void DrawDebugShapes()
	{
		super.DrawDebugShapes();

		if (!m_bDrawDebugShapes)
			return;

		foreach (SCR_PowerPoleSlotBase slot: m_aJunctionSlots)
		{
			slot.DrawDebugShapes(m_aDebugShapes, this);
		}
	}

	//------------------------------------------------------------------------------------------------
	override vector GetSlot(int index, bool sameLine)
	{
		if (sameLine)
			return super.GetSlot(index, sameLine);
		else
			return m_aJunctionSlots[index].m_vSlotA;
	}

	//------------------------------------------------------------------------------------------------
	override int GetSlotsCount(bool sameLine = true)
	{
		if (sameLine)
			return m_aSlots.Count();
		else
			return m_aJunctionSlots.Count();
	}

	//------------------------------------------------------------------------------------------------
	override vector TryGetSlot(int index, vector otherSlot, bool sameLine)
	{
		if (sameLine)
			return super.TryGetSlot(index, otherSlot, sameLine);

		if (index >= m_aJunctionSlots.Count())
		{
			return m_aJunctionSlots[index % m_aJunctionSlots.Count()].m_vSlotA;
			return vector.Zero;
		}

		SCR_PowerPoleSlot dualSlot = SCR_PowerPoleSlot.Cast(m_aJunctionSlots[index]);
		if (dualSlot)
		{
			if (vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotA)) > vector.Distance(otherSlot, CoordToParent(dualSlot.m_vSlotB)))
				return CoordToParent(dualSlot.m_vSlotB);
			else
				return CoordToParent(dualSlot.m_vSlotA);
		}

		SCR_PowerPoleSlotSingle singleSlot = SCR_PowerPoleSlotSingle.Cast(m_aJunctionSlots[index]);
		if (singleSlot)
			return CoordToParent(singleSlot.m_vSlotA);

		return vector.Zero;
	}

	//------------------------------------------------------------------------------------------------
	void SCR_JunctionPowerPole(IEntitySource src, IEntity parent)
	{
	}
};

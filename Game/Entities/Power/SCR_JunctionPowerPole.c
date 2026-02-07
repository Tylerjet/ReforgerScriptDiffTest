[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_JunctionPowerPoleClass : SCR_PowerPoleClass
{
}

class SCR_JunctionPowerPole : SCR_PowerPole
{
	[Attribute(desc: "[OBSOLETE (use Cable Slot Groups above)] Slots for connecting with other power poles in a junction", category: "[OLD] Power Cable Slots")]
	protected ref array<ref SCR_PowerPoleSlotBase> m_aJunctionSlots; // obsolete since 2024-04-02

#ifdef WORKBENCH

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
			return super.TryGetSlot(index, otherSlot, sameLine); // using m_aSlots

		if (index < 0)
		{
			Print("Power line slot index was negative (" + index + ")", LogLevel.WARNING);
			return vector.Zero;
		}

		int junctionCount = m_aJunctionSlots.Count();
		if (junctionCount < 1)
		{
			Print("No junction slots defined - check Prefab at " + GetOrigin(), LogLevel.WARNING);
			return vector.Zero;
		}

		if (index >= junctionCount)
			index = index % junctionCount;

		SCR_PowerPoleSlotSingle singleSlot = SCR_PowerPoleSlotSingle.Cast(m_aJunctionSlots[index]);
		if (singleSlot)
			return CoordToParent(singleSlot.m_vSlotA);

		SCR_PowerPoleSlot dualSlot = SCR_PowerPoleSlot.Cast(m_aJunctionSlots[index]);
		if (dualSlot)
		{
			// let's detect which slot is wanted: A or B
			vector avgSideA;
			vector avgSideB;
			SCR_PowerPoleSlot powerPoleJunction;
			foreach (SCR_PowerPoleSlotBase powerPoleSlot : m_aJunctionSlots)
			{
				avgSideA += powerPoleSlot.m_vSlotA;
				powerPoleJunction = SCR_PowerPoleSlot.Cast(powerPoleSlot);
				if (powerPoleJunction)
					avgSideB += powerPoleJunction.m_vSlotB;
			}

			avgSideA /= (float)junctionCount;
			avgSideB /= (float)junctionCount;

			if (avgSideB == vector.Zero || vector.Distance(otherSlot, CoordToParent(avgSideA)) <= vector.Distance(otherSlot, CoordToParent(avgSideB)))
				return CoordToParent(dualSlot.m_vSlotA);
			else
				return CoordToParent(dualSlot.m_vSlotB);
		}

		return vector.Zero;
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_JunctionPowerPole(IEntitySource src, IEntity parent)
	{
	}
}

[EntityEditorProps(category: "GameScripted/Power", description: "This is the power pole entity.", color: "0 255 0 255", visible: false, dynamicBox: true)]
class SCR_PowerPoleClass : PowerPoleEntityClass
{
	/*
		Cable Slots
	*/

	[Attribute(category: "[Prefab-Wide] Cable Slots")]
	ref array<ref SCR_PoleCableSlotGroup> m_aCableSlotGroups; // since 2024-04-02
}

class SCR_PowerPole : PowerPoleEntity
{
	/*
		[OLD] Power Cable Slots
	*/

	[Attribute(desc: "[OBSOLETE (use Cable Slot Groups above)] Slots for connecting with other power poles", category: "[OLD] Power Cable Slots")]
	protected ref array<ref SCR_PowerPoleSlotBase> m_aSlots; // obsolete since 2024-04-02

#ifdef WORKBENCH

	//------------------------------------------------------------------------------------------------
	//! \param[in] sameLine is used by SCR_JunctionPowerPole.GetSlotsCount
	int GetSlotsCount(bool sameLine = true)
	{
		return m_aSlots.Count();
	}

	//------------------------------------------------------------------------------------------------
	//! Get the closest non-empty cable slot group for each available cable type
	//! \param[in] worldPos the position from which to find the closest slots
	//! \return a cableType-slotGroup map - never returns null
	map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> GetClosestCableSlotGroupsPerCableType(vector worldPos)
	{
		map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup> result = new map<SCR_EPoleCableType, ref SCR_PoleCableSlotGroup>();
		map<SCR_EPoleCableType, float> distancesSq = new map<SCR_EPoleCableType, float>();

		SCR_PowerPoleClass prefabData = SCR_PowerPoleClass.Cast(GetPrefabData());
		if (!prefabData)
			return result;

		foreach (SCR_PoleCableSlotGroup group : prefabData.m_aCableSlotGroups)
		{
			vector avgPos = vector.Zero;
			int count = group.m_aSlots.Count();
			if (count < 1)
				continue;

			if (group.m_vAnchorOverride == vector.Zero)
			{
				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					avgPos += slot.m_vPosition;
				}
	
				avgPos /= count;
			}
			else
			{
				avgPos = group.m_vAnchorOverride;
			}

			float storedGroupDistanceSq = distancesSq.Get(group.m_eCableType);
			if (storedGroupDistanceSq == 0) // not present - I believe faster than Contains, Get, Set/Insert
				storedGroupDistanceSq = float.INFINITY;

			float distanceSq = vector.DistanceSq(worldPos, CoordToParent(avgPos));
			if (distanceSq < storedGroupDistanceSq)
			{
				distancesSq.Set(group.m_eCableType, distanceSq);
				result.Set(group.m_eCableType, group);
			}
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	//! Tries to find position of the closest compatible slot.
	//! \param[in] index is the index of the other slot, this method tries to find the slot under the same index.
	//! \param[in] otherSlot is the world position of the other pole's slot.
	//! \param[in] sameLine is used by SCR_JunctionPowerPole.TryGetSlot
	//! \return vector with world origin of the closest compatible slot.
	vector TryGetSlot(int index, vector otherSlot, bool sameLine)
	{
		if (index < 0)
			return vector.Zero;

		int slotCount = m_aSlots.Count();
		if (slotCount < 1)
			return vector.Zero;

		if (index >= slotCount)
			index = index % slotCount;

		SCR_PowerPoleSlot dualSlot = SCR_PowerPoleSlot.Cast(m_aSlots[index]);
		if (dualSlot)
		{
			vector slotAWorldPos = CoordToParent(dualSlot.m_vSlotA);
			vector slotBWorldPos = CoordToParent(dualSlot.m_vSlotB);
			if (vector.DistanceSq(otherSlot, slotAWorldPos) <= vector.DistanceSq(otherSlot, slotBWorldPos))
				return slotAWorldPos;
			else
				return slotBWorldPos;
		}

		SCR_PowerPoleSlotSingle singleSlot = SCR_PowerPoleSlotSingle.Cast(m_aSlots[index]);
		if (singleSlot)
			return CoordToParent(singleSlot.m_vSlotA);

		return vector.Zero;
	}

	//
	// temp debug shapes for cable slot placement
	//

	protected static bool s_bDisplayCableSlots;
	protected static ref SCR_DebugShapeManager s_DebugShapeManager;

	protected static const float MIN_AVG_ANCHOR_DIST = 0.5;
	protected static const int DEBUG_SLOT_POS_COLOUR_1 = Color.DARK_GREEN & 0x88FFFFFF;
	protected static const int DEBUG_SLOT_POS_COLOUR_2 = Color.DARK_GREEN & 0x55FFFFFF;
	protected static const int DEBUG_PRECISION_LINE_COLOUR = Color.RED;
	protected static const float DEBUG_PRECISION_LINE_SIZE = 0.1;

	protected static const int DEBUG_ANCHOR_POS_COLOUR = Color.ORANGE & 0xBBFFFFFF;
	protected static const int DEBUG_ANCHOR_LINE_COLOUR = Color.ORANGE & 0x66FFFFFF;
	protected static const float DEBUG_SLOT_POS_SIZE_1 = 0.05;
	protected static const float DEBUG_SLOT_POS_SIZE_2 = 0.25;
	protected static const float DEBUG_ANCHOR_POS = 0.075;

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		bool result = super._WB_OnKeyChanged(src, key, ownerContainers, parent);
		if (!src || _WB_GetEditorAPI().UndoOrRedoIsRestoring())
			return result;

		typename srcType = src.GetClassName().ToType();
		if (
			!srcType ||
			(key != "m_vPosition" && key != "m_vAnchorOverride") ||
			(!srcType.IsInherited(SCR_PoleCableSlot) && !srcType.IsInherited(SCR_PoleCableSlotGroup)))
		{
			if (s_DebugShapeManager)
				s_DebugShapeManager.Clear();

			s_bDisplayCableSlots = false;
			return result;
		}

		s_bDisplayCableSlots = true;

		if (s_DebugShapeManager)
			UpdateDebugShapes();

		return result;
	}

//	why these two do not work, IDK
//	_WB_AfterWorldUpdate	// sometimes doesn't work
//	thread				// never works / crashes WB

	//------------------------------------------------------------------------------------------------
	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		if (s_bDisplayCableSlots && _WB_GetEditorAPI() && _WB_GetEditorAPI().IsPrefabEditMode())
			UpdateDebugShapes();
	}

	//------------------------------------------------------------------------------------------------
	protected void UpdateDebugShapes()
	{
		SCR_PowerPoleClass prefabData = SCR_PowerPoleClass.Cast(GetPrefabData());
		if (!prefabData)
			return;

		const float angle = 45 * Math.DEG2RAD;
		array<vector> lines = {
			{ Math.Cos(angle), 0, Math.Sin(angle) } * DEBUG_PRECISION_LINE_SIZE,
			{ 0, 1, 0 } * DEBUG_PRECISION_LINE_SIZE,
			{ Math.Cos(-angle), 0, Math.Sin(-angle) } * DEBUG_PRECISION_LINE_SIZE,
		};

		s_DebugShapeManager.Clear();
		foreach (SCR_PoleCableSlotGroup group : prefabData.m_aCableSlotGroups)
		{
			foreach (SCR_PoleCableSlot slot : group.m_aSlots)
			{
				s_DebugShapeManager.AddSphere(slot.m_vPosition, DEBUG_SLOT_POS_SIZE_1, DEBUG_SLOT_POS_COLOUR_1, ShapeFlags.DEPTH_DITHER | ShapeFlags.NOOUTLINE);
				s_DebugShapeManager.AddSphere(slot.m_vPosition, DEBUG_SLOT_POS_SIZE_2, DEBUG_SLOT_POS_COLOUR_2, ShapeFlags.DEPTH_DITHER | ShapeFlags.NOOUTLINE);
				foreach (vector line : lines)
				{
					s_DebugShapeManager.AddLine(slot.m_vPosition + line, slot.m_vPosition - line, DEBUG_PRECISION_LINE_COLOUR, ShapeFlags.DEPTH_DITHER);
				}
			}

			vector anchor = group.m_vAnchorOverride;
			int count = group.m_aSlots.Count();
			bool drawAnchor = anchor != vector.Zero || count > 1;

			if (anchor == vector.Zero && count > 1)
			{
				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					anchor += slot.m_vPosition;
				}

				anchor /= count;

				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					if (vector.Distance(anchor, slot.m_vPosition) < MIN_AVG_ANCHOR_DIST)
					{
						drawAnchor = false;
						break;
					}
				}
			}

			if (anchor != vector.Zero || count > 1)
			{
				if (drawAnchor)
					s_DebugShapeManager.AddSphere(anchor, DEBUG_ANCHOR_POS, DEBUG_ANCHOR_POS_COLOUR, ShapeFlags.DEPTH_DITHER | ShapeFlags.NOOUTLINE);

				// lines will be drawn
				foreach (SCR_PoleCableSlot slot : group.m_aSlots)
				{
					s_DebugShapeManager.AddLine(slot.m_vPosition, anchor, DEBUG_ANCHOR_LINE_COLOUR, ShapeFlags.DEPTH_DITHER);
				}
			}
		}
	}

#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	//! \param[in] src
	//! \param[in] parent
	void SCR_PowerPole(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH

		if (_WB_GetEditorAPI() && _WB_GetEditorAPI().IsPrefabEditMode())
		{
			if (!s_DebugShapeManager)
				s_DebugShapeManager = new SCR_DebugShapeManager();
		}

		SetEventMask(EntityEvent.INIT);
#endif // WORKBENCH
	}
}

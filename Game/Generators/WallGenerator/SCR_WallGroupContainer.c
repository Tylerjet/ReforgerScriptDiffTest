#ifdef WORKBENCH
//! Data container that holds the individual wall asset related information
class SCR_WallGroupContainer
{
	bool m_bGenerated;
	float m_fMiddleObjectLength;
	float m_fSmallestWall = float.MAX;

	// for quick access, wallgroup lengths are in a seperate array (indices corresponding with those in wallgroup array)
	protected ref array<float> m_aLengths = {};

	protected ref array<ref SCR_WallGroup> m_aWallGroups = {}; // sorted by ascending length
	protected WallGeneratorEntity m_WallGenerator;

	//------------------------------------------------------------------------------------------------
	bool IsEmpty()
	{
		return m_aWallGroups.IsEmpty();
	}

	//------------------------------------------------------------------------------------------------
	SCR_WallPair GetRandomWall(float biggestSmallerThan = -1)
	{
		for (int i = m_aLengths.Count() - 1; i >= 0; i--) // find the longest wall which is smaller than given amount
		{
			if (m_aLengths[i] < biggestSmallerThan)
				return m_aWallGroups[i].GetRandomWall();
		}

		return null;
	}

	//------------------------------------------------------------------------------------------------
	//! Goes through walls and prepares a data structure that's used during wall generation
	void PrepareWallGroups(notnull array<ref WallLengthGroup> groups, bool forward, string middleObj, WorldEditorAPI api)
	{
		m_aWallGroups.Clear();

		array<ref SCR_WallGroup> wallGroupsTmp = {};

		int forwardAxis = 2;
		if (forward)
			forwardAxis = 0;

		if (!middleObj.IsEmpty())
			m_fMiddleObjectLength = WallGeneratorEntity.MeasureEntity(middleObj, forwardAxis, api);

		SCR_WallGroup wallGroup;
		SCR_WallPair wallPair;
		float wallLength;
		foreach (WallLengthGroup group : groups)
		{
			wallGroup = new SCR_WallGroup();

			foreach (WallWeightPair pair : group.m_aWallPrefabs)
			{
				wallPair = new SCR_WallPair();

				wallPair.m_sWallAsset = pair.m_sWallAsset;
				wallPair.m_fPostPadding = pair.m_fPostPadding;
				wallPair.m_fPrePadding = pair.m_fPrePadding;
				wallGroup.m_aWeights.Insert(pair.m_fWeight);

				wallLength = WallGeneratorEntity.MeasureEntity(wallPair.m_sWallAsset, forwardAxis, api);
				// wallLength += wallPair.m_fPostPadding;
				wallPair.m_fWallLength = wallLength;

				if (wallLength > wallGroup.m_fWallLength)
					wallGroup.m_fWallLength = wallLength; // biggest length is the one being used for the whole group

				if (!wallPair.m_sWallAsset.IsEmpty())
					wallGroup.m_aWallPairs.Insert(wallPair);
			}

			if (!wallGroup.m_aWallPairs.IsEmpty())
			{
				wallGroupsTmp.Insert(wallGroup);
				m_bGenerated = true;
			}
		}

		float smallestSize;
		int smallestIndex;

		// order groups by length
		while (!wallGroupsTmp.IsEmpty())
		{
			smallestSize = float.MAX;
			smallestIndex = 0;
			foreach (int i, SCR_WallGroup g : wallGroupsTmp)
			{
				if (g.m_fWallLength < smallestSize)
				{
					smallestSize = g.m_fWallLength;
					if (smallestSize < m_fSmallestWall)
						m_fSmallestWall = smallestSize;

					smallestIndex = i;
				}
			}

			m_aWallGroups.Insert(wallGroupsTmp[smallestIndex]);
			m_aLengths.Insert(wallGroupsTmp[smallestIndex].m_fWallLength);
			wallGroupsTmp.Remove(smallestIndex);
		}

		if (m_WallGenerator && m_WallGenerator.m_bDebug)
		{
			foreach (SCR_WallGroup group : m_aWallGroups)
			{
				Print(group.m_fWallLength, LogLevel.NORMAL);
				foreach (SCR_WallPair pair : group.m_aWallPairs)
				{
					Print(pair.m_sWallAsset, LogLevel.NORMAL);
					Print(pair.m_fWallWeight, LogLevel.NORMAL);
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	void SCR_WallGroupContainer(WorldEditorAPI api, array<ref WallLengthGroup> items, bool forward, string middleObj, WallGeneratorEntity ent)
	{
		m_WallGenerator = ent;
		PrepareWallGroups(items, forward, middleObj, api);
	}
}
#endif // WORKBENCH

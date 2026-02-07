#ifdef WORKBENCH
class SCR_RandomisationEditorData
{
	bool m_bPlaceByBoundingBox; //!< false = place by origin
	bool m_bRandomYaw;
	bool m_bAlignToNormal;
	vector m_vRandomScale;		//!< the UI allows for negatives, and WorldEditorAPI too - only entity.SetScale bothers blocking negative scales
	vector m_vRandomVertOffset;	//!< [0] <= [1], always
	float m_fRandomPitchAngle;	//!< range 0..180
	float m_fRandomRollAngle;	//!< range 0..180

	//------------------------------------------------------------------------------------------------
	//! Randomise the provided entitySource with its Prefab Library values, assuming it is on the ground already\n
	//! (WorldEditorAPI.CreateEntity vs .CreateEntityExt)\n
	//! Does not use BeginEntityAction or any other kind of Begin..End action\n
	//! Place By Bounding Box is not supported (yet?)
	//! \param[in] entitySource
	//! \param[in] randomGenerator
	//! \return true on success, false on failure
	bool RandomiseExt(notnull WorldEditorAPI worldEditorAPI, notnull IEntitySource entitySource, RandomGenerator randomGenerator) // bool snapToGround // \param[in] snapToGround
	{
		BaseWorld world = worldEditorAPI.GetWorld();
		if (!world)
			return false;

		vector angles;
		if (!entitySource.Get("angleX", angles[0])
			|| !entitySource.Get("angleY", angles[1])
			|| !entitySource.Get("angleZ", angles[2]))
			return false;

		vector coords;
		if (!entitySource.Get("coords", coords))
			return false;

		IEntity entity = worldEditorAPI.SourceToEntity(entitySource);
		if (!entity)
			return false;

		vector worldPos = entity.GetOrigin();

		float ySurface = worldEditorAPI.GetWorld().GetSurfaceY(worldPos[0], worldPos[2]);
		vector surfacePos = { worldPos[0], ySurface, worldPos[2] };

		if (m_bRandomYaw)
			worldEditorAPI.SetVariableValue(entitySource, null, "angleY", Math.Repeat(randomGenerator.RandFloatXY(0, 360), 360).ToString());

		if (m_fRandomPitchAngle > 0)
			worldEditorAPI.SetVariableValue(entitySource, null, "angleX", Math.Repeat(randomGenerator.RandFloatXY(-m_fRandomPitchAngle, m_fRandomPitchAngle), 360).ToString());

		if (m_fRandomRollAngle > 0)
			worldEditorAPI.SetVariableValue(entitySource, null, "angleZ", Math.Repeat(randomGenerator.RandFloatXY(-m_fRandomRollAngle, m_fRandomRollAngle), 360).ToString());

		if (m_bAlignToNormal)
		{
			float angleY = angles[1];
			TraceParam traceParam = new TraceParam();
			traceParam.Flags = TraceFlags.WORLD;
			traceParam.Start = surfacePos + vector.Up;
			traceParam.End = surfacePos - vector.Up;

			if (!entity)
				entity = worldEditorAPI.SourceToEntity(entitySource);

			vector mat[4];
			entity.GetTransform(mat);
			entity.GetWorld().TraceMove(traceParam, null);

			vector newUp = traceParam.TraceNorm.Normalized();
			vector newRight = (newUp * mat[2]).Normalized();
			// vector newForward = (newRight * newUp).Normalized();

			mat[1] = traceParam.TraceNorm.Normalized();		// newUp
			mat[0] = (mat[1] * mat[2]).Normalized();		// newRight
			mat[2] = (mat[0] * mat[1]).Normalized();		// newForward

			angles = Math3D.MatrixToAngles(mat);
			worldEditorAPI.SetVariableValue(entitySource, null, "angleX", angles[1].ToString());
			worldEditorAPI.SetVariableValue(entitySource, null, "angleY", angles[0].ToString());
			worldEditorAPI.SetVariableValue(entitySource, null, "angleZ", angles[2].ToString());
		}

		if (m_vRandomScale[0] != m_vRandomScale[1])
			worldEditorAPI.SetVariableValue(entitySource, null, "scale", randomGenerator.RandFloatXY(m_vRandomScale[0], m_vRandomScale[1]).ToString());
		else
		if (m_vRandomScale[0] != 1)
			worldEditorAPI.SetVariableValue(entitySource, null, "scale", m_vRandomScale[0].ToString());

		float absoluteYOffset;
//		if (snapToGround)
//			absoluteYOffset += ySurface - worldPos[1];

		if (m_vRandomVertOffset[0] != 0 || m_vRandomVertOffset[1] != 0)
			absoluteYOffset += randomGenerator.RandFloatXY(m_vRandomVertOffset[0], m_vRandomVertOffset[1]);

		if (absoluteYOffset != 0)
		{
			if (!entity)
				entity = worldEditorAPI.SourceToEntity(entitySource);

			vector newLocalPos = coords + entity.CoordToLocal(worldPos + absoluteYOffset * vector.Up); // offset, not position - add coords
			worldEditorAPI.SetVariableValue(entitySource, null, "coords", string.Format("%1 %2 %3", newLocalPos[0], newLocalPos[1], newLocalPos[2]));
		}

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Get randomisation information from the provided entitySource
	//! (WorldEditorAPI.CreateEntity vs .CreateEntityExt)
	//! \param[in] entitySource the entity source from which to grab randomisation data
	//! \return randomisation data if correctly grabbed, null otherwise
	static SCR_RandomisationEditorData CreateFromEntitySource(notnull IEntitySource entitySource)
	{
		BaseContainerList editorData = entitySource.GetObjectArray("editorData");
		if (!editorData)
			return null;

		int editorDataCount = editorData.Count();
		if (editorDataCount < 1)
			return null;

		BaseContainer randomisationEditorData;
		for (int i; i < editorDataCount; ++i)
		{
			randomisationEditorData = editorData.Get(i);
			if (randomisationEditorData.GetClassName() == "RandomizationEditorData")
				break;

			randomisationEditorData = null;
		}

		if (!randomisationEditorData)
			return null;

		SCR_RandomisationEditorData result = new SCR_RandomisationEditorData();

		int placement;
		if (randomisationEditorData.Get("randomYaw", placement))
			result.m_bPlaceByBoundingBox = placement == 1; // "boundingBox"

		randomisationEditorData.Get("randomYaw", result.m_bRandomYaw);
		randomisationEditorData.Get("alignToNormal", result.m_bAlignToNormal);
		randomisationEditorData.Get("randomScale", result.m_vRandomScale);
		randomisationEditorData.Get("randomVertOffset", result.m_vRandomVertOffset);
		randomisationEditorData.Get("randomPitchAngle", result.m_fRandomPitchAngle);
		randomisationEditorData.Get("randomRollAngle", result.m_fRandomRollAngle);

		return result;
	}

	//------------------------------------------------------------------------------------------------
	// constructor
	protected void SCR_RandomisationEditorData();
}
#endif

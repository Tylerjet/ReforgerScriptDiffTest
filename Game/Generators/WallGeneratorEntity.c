[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "WallGeneratorEntity", dynamicBox: true, visible: false)]
class WallGeneratorEntityClass : SCR_GeneratorBaseEntityClass
{
}

class WallGeneratorEntity : SCR_GeneratorBaseEntity
{
	/*
		Middle Object
	*/

	[Attribute(defvalue: "1", desc: "Enable middle object", category: "Middle Object")]
	protected bool m_bEnableMiddleObject;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Middle Object Prefab", "et", category: "Middle Object")]
	protected ResourceName MiddleObject;

	[Attribute(defvalue: "1", desc: "Place middle prefab at vertex", category: "Middle Object")]
	protected bool PlaceMiddleAtVertex;

	[Attribute(defvalue: "0", desc: "Place middle prefab at vertex only", category: "Middle Object")]
	protected bool PlaceMiddleAtVertexOnly;

	[Attribute("0", UIWidgets.Slider, "Middle object pre-padding, essentially a gap between previous prefab and this", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectPrePadding;

	[Attribute("0", UIWidgets.Slider,"Middle object offset to the side", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Middle object post-padding, essentially a gap between this asset and the next one", params: "-5 5 0.01", category: "Middle Object")]
	protected float MiddleObjectPostPadding;

	[Attribute("0", UIWidgets.Slider, "Middle object offset up/down", params: "-10 10 0.01", category: "Middle Object")]
	protected float MiddleObjectOffsetUp;

	/*
		First Object
	*/

	[Attribute(defvalue: "1", desc: "Enable first object", category: "First Object")]
	protected bool m_bEnableFirstObject;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "First Object Prefab", "et", category: "First Object")]
	protected ResourceName FirstObject;

	[Attribute("0", UIWidgets.Slider, "First object pre-padding, essentially a gap between previous vertex first object",params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectPrePadding;

	[Attribute("0", UIWidgets.Slider, "First object post-padding, essentially a gap between first object and the following one", params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectPostPadding;

	[Attribute("0", UIWidgets.Slider, "First object offset to the side", params: "-5 5 0.01", category: "First Object")]
	protected float FirstObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "First object offset up/down", params: "-10 10 0.01", category: "First Object")]
	protected float FirstObjectOffsetUp;

	/*
		Last Object
	*/

	[Attribute(defvalue: "1", desc: "Enable last object", category: "Last Object")]
	protected bool m_bEnableLastObject;

	[Attribute("", UIWidgets.ResourcePickerThumbnail, "Last Object Prefab", "et", category: "Last Object")]
	protected ResourceName LastObject;

	[Attribute("0", UIWidgets.Slider, "Last object pre-padding, essentially a gap between previous vertex first object",params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectPrePadding;

	[Attribute("0", UIWidgets.Slider, "Last object post-padding, essentially a gap between first object and the following one", params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectPostPadding;

	[Attribute("0", UIWidgets.Slider,"Last object offset to the side", params: "-5 5 0.01", category: "Last Object")]
	protected float LastObjectOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Last object offset up/down", params: "-10 10 0.01", category: "Last Object")]
	protected float LastObjectOffsetUp;

	/*
		Global
	*/

	[Attribute("0", UIWidgets.Slider, "Global object pre-padding, essentially a gap between previous prefab and this", params: "-2 2 0.01", category: "Global")]
	protected float PrePadding;

	[Attribute("0", UIWidgets.Slider, "Allow pre-padding on first wall asset in each line segment", params: "-2 2 0.01", category: "Global")]
	protected float PostPadding;

	[Attribute("0.5", UIWidgets.Slider, "Allow overshooting the segment line by this amount when placing assets", params: "-5 5 0.01", category: "Global")]
	protected float m_fOvershoot;

	[Attribute("0", UIWidgets.Slider, "Objects offset to the side", params: "-5 5 0.01", category: "Global")]
	protected float m_fOffsetRight;

	[Attribute("0", UIWidgets.Slider, "Object offset up/down", params: "-5 5 0.01", category: "Global")]
	protected float m_fOffsetUp;

	[Attribute(defvalue: "", uiwidget: UIWidgets.Object, "Contains wall groups which group Wall/Weight pairs by length", category: "Global")]
	protected ref array<ref WallLengthGroup> m_aWallGroups;

	/*
		Other
	*/

	[Attribute(defvalue: "1", desc: "Allow pre-padding on first wall asset in each line segment", category: "Other")]
	protected bool PrePadFirst;

	[Attribute(defvalue: "0", desc: "Copy the polyline precisely while sacrificing wall assets contact", category: "Other")]
	protected bool ExactPlacement;

	[Attribute(defvalue: "0", desc: "Start the wall from the other end of the polyline", category: "Other")]
	protected bool m_bStartFromTheEnd;

	[Attribute(defvalue: "1", desc: "Rotate object so that its X axis is facing in the direction of the polyline segment", category: "Other")]
	protected bool UseXAsForward;

	[Attribute(defvalue: "0", desc: "Rotate object 180° around the Yaw axis", category: "Other")]
	protected bool Rotate180;

	[Attribute(defvalue: "0", desc: "If you want to generate objects smaller than 10 centimetres", category: "Other")]
	protected bool UseForVerySmallObjects;

	[Attribute(defvalue: "0", desc: "Draw developer debug", category: "Other")]
	bool m_bDebug;

	[Attribute(defvalue: "0", desc: "Whether or not walls should be snapped to the terrain", category: "Other")]
	protected bool m_bSnapToTerrain;

	[Attribute(uiwidget: UIWidgets.None)] // obsolete, kept for Prefabs and layers retrocompatibility
	protected ref array<ref ResourceName> WallPrefabs;

#ifdef WORKBENCH

	protected IEntitySource m_ParentSource;

	protected ref SCR_WallGroupContainer m_WallGroupContainer;

	protected ref array<ref SCR_WallGeneratorPoint> m_aPoints = {};
	protected static ref array<ref Shape> s_aDebugShapes = {};

	//------------------------------------------------------------------------------------------------
	//! \param entityName Prefab to measure
	//! \param measureAxis 0 for X measure, 2 for Z measure
	//! \param api required 
	//! \return entity side on provided axis, 1 on close-to-zero (< 0.00001m) measurement, float.MAX on failure
	static float MeasureEntity(ResourceName entityName, int measureAxis, WorldEditorAPI api)
	{
		float result = float.MAX;

		if (entityName.IsEmpty())
			return result;

		Resource resource = Resource.Load(entityName);
		if (!resource.IsValid())
			return result;

		GenericEntity wallEntity = GenericEntity.Cast(GetGame().SpawnEntityPrefab(resource, api.GetWorld()));
		if (!wallEntity)
			return result;

		vector minBB;
		vector maxBB;
		wallEntity.GetBounds(minBB, maxBB);
		delete wallEntity;

		result = (maxBB - minBB)[measureAxis];
		if (result < 0.000001)
		{
			Print("Wall asset " + entityName + " is too small, does it have a valid mesh?", LogLevel.ERROR);
			return 1; // generating is invalid, just avoid infinite loop
		}

		return result;
	}

	//------------------------------------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);

		if (key == "coords")
			return false;

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return false;

		IEntitySource thisSrc = api.EntityToSource(this);
		IEntitySource parentSrc = thisSrc.GetParent();

		BaseContainerTools.WriteToInstance(this, thisSrc);
		OnShapeChanged(parentSrc, ShapeEntity.Cast(parent), {}, {});

		return true;
	}

	//------------------------------------------------------------------------------------------------
	//! Offsets the points in the 'points' array, used with m_fOffsetRight
	protected void OffsetPoints(array<vector> points, float offset, bool debugAllowed = false)
	{
		array<vector> pointsTemp = {};
		pointsTemp.Copy(points);
		points.Clear();
		int lastIndex = pointsTemp.Count() - 1;

		vector matWrld[4];
		if (m_bDebug && debugAllowed)
			GetWorldTransform(matWrld);

		vector forwardPrev = vector.One;
		foreach (int i, vector pointTemp : pointsTemp)
		{
			vector forwardNext;

			if (i < lastIndex)
				forwardNext = pointsTemp[i + 1] - pointTemp;
			else
				forwardNext = -forwardPrev;

			if (i == 0)
				forwardPrev = -forwardNext;

			forwardNext.Normalize();
			forwardPrev.Normalize();

			float dotProductPrevNext = vector.Dot(forwardPrev, forwardNext);
			bool almostLine = dotProductPrevNext < -0.95;
			vector diagonal = forwardNext + forwardPrev;

			vector normalRight = -forwardPrev * vector.Up;
			normalRight.Normalize();
			float dotProductNormNext = vector.Dot(normalRight, forwardNext);
			bool isLeft = dotProductNormNext > 0;

			vector nextModified = dotProductPrevNext * forwardNext;
			float dist = vector.Distance(nextModified, forwardPrev);
			float diff;
			if (dist != 0)
				diff = offset / dist;

			diagonal *= diff;

			if (!isLeft)
				diagonal = diagonal * -1;

			if (almostLine)
			{
				vector vec = forwardNext - forwardPrev;
				vector right = vec * vector.Up;
				right.Normalize();
				diagonal = right * offset;
			}

			points.Insert(pointsTemp[i] + diagonal);
			forwardPrev = -forwardNext;

			// debug
			if (m_bDebug && debugAllowed)
			{
				pointTemp = pointTemp.Multiply4(matWrld); // variable reuse
				s_aDebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER, pointTemp, pointTemp + forwardNext));
				s_aDebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, pointTemp, pointTemp + forwardPrev));
				s_aDebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, pointTemp, pointTemp + diagonal));
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);

		// TODO: auto-trigger generation here as well
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (!api || api.UndoOrRedoIsRestoring())
			return;

		Preprocess(shapeEntitySrc);
		Generate(shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected void Preprocess(IEntitySource shapeEntitySrc)
	{
		m_aPoints.Clear();
		WorldEditorAPI api = _WB_GetEditorAPI();

		m_WallGroupContainer = new SCR_WallGroupContainer(api, m_aWallGroups, UseXAsForward, MiddleObject, this);
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (points != null && points.Count() >= 2 && m_WallGroupContainer.m_bGenerated != 0)
		{
			bool isShapeClosed = false;
			shapeEntitySrc.Get("IsClosed", isShapeClosed);
			array<vector> pointsVec = {};
			int pointCount = points.Count();
			bool addFirstAsLast = false;

			if (isShapeClosed && pointCount > 2) // no reason to "loop" for 2 points
				addFirstAsLast = true;

			// offset start
			BaseContainer point;
			vector pos;
			for (int i = 0; i < pointCount; i++)
			{
				point = points.Get(i);
				point.Get("Position", pos);
				pointsVec.Insert(pos);
			}

			if (addFirstAsLast)
				pointsVec.Insert(pointsVec[0]);

			if (m_fOffsetRight != 0)
				OffsetPoints(pointsVec,m_fOffsetRight, true);
			// offset end

			ResourceName customMesh;
			float prePadding, postPadding, offsetUp;
			bool generate, clipping, align;
			BaseContainerList dataArr;
			BaseContainer data;
			int dataCount, lastPointIndex;
			SCR_WallGeneratorPoint genPoint;

			for (int i = 0; i < pointCount; i++)
			{
				point = points.Get(i);
				pos = pointsVec[i];
				customMesh = string.Empty;
				prePadding = 0;
				postPadding = 0;
				generate = true;
				clipping = false;
				offsetUp = 0;
				align = false;
				dataArr = point.GetObjectArray("Data");
				dataCount = dataArr.Count();

				for (int j = 0; j < dataCount; ++j)
				{
					data = dataArr.Get(j);
					if (data.GetClassName() == "WallGeneratorPointData")
					{
						data.Get("MeshAtPoint", customMesh);
						data.Get("PrePadding", prePadding);
						data.Get("PostPadding", postPadding);
						data.Get("m_bGenerate", generate);
						data.Get("m_bAllowClipping", clipping);
						data.Get("m_fOffsetUp", offsetUp);
						data.Get("m_bAlignWithNext", align);
						break;
					}
				}

				genPoint = new SCR_WallGeneratorPoint();
				genPoint.m_vPos = pos;
				genPoint.m_sCustomMesh = customMesh;
				genPoint.m_fPrePadding = prePadding;
				genPoint.m_fPostPadding = postPadding;
				genPoint.m_bGenerate = generate;
				genPoint.m_bClip = clipping;
				genPoint.m_fOffsetUp = offsetUp;
				genPoint.m_bAlignNext = align;
				m_aPoints.Insert(genPoint);
			}

			if (addFirstAsLast)
			{
				m_aPoints.Insert(m_aPoints[0]);
				lastPointIndex = m_aPoints.Count() - 1;
				m_aPoints[lastPointIndex].m_vPos = pointsVec[lastPointIndex];
			}
		}
	}

	//------------------------------------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		if (!shapeEntitySrc || _WB_GetEditorAPI().UndoOrRedoIsRestoring())
			return;

		Preprocess(shapeEntitySrc);
		Generate(shapeEntity);
	}

	//------------------------------------------------------------------------------------------------
	protected IEntitySource PlacePrefab(
		bool generate,
		ResourceName name,
		out vector pos,
		vector dir,
		vector prevDir,
		float rotationAdjustment,
		bool isGeneratorVisible,
		float length,
		float prePadding,
		float postPadding,
		float offsetUp,
		bool alignNext,
		bool prepadNext,
		bool snapToGround = false,
		bool allowClipping = false,
		vector offsetRight = vector.Zero)
	{
		if (name.IsEmpty())
			return null;

		vector prepadDirection;
		vector orientation;

		if (alignNext)
			orientation = dir;
		else
			orientation = prevDir;

		if (prepadNext)
			prepadDirection = dir;
		else
			prepadDirection = prevDir;

		pos += prepadDirection * prePadding;
		IEntitySource ent;
		WorldEditorAPI api = _WB_GetEditorAPI();
		IEntitySource thisSource = api.EntityToSource(this);
		int layerID = api.GetCurrentEntityLayerId();

		if (generate)
		{
			vector rotMat[4];
			Math3D.DirectionAndUpMatrix(orientation, vector.Up, rotMat);
			vector rot = Math3D.MatrixToAngles(rotMat);
			rot[1] = rot[0] + rotationAdjustment;
			rot[0] = 0;
			rot[2] = 0;

			if (m_bSnapToTerrain)
			{
				vector world = CoordToParent(pos);
				pos[1] = api.GetTerrainSurfaceY(world[0], world[2]);
				if (m_ParentSource)
					pos[1] = pos[1] - api.SourceToEntity(m_ParentSource).GetOrigin()[1];
			}

			if (snapToGround)
			{
				vector matWrld[4];
				GetWorldTransform(matWrld);

				ent = api.CreateEntityExt(name, "", layerID, thisSource, (pos + offsetRight), rot, TraceFlags.ENTS);
				api.ParentEntity(thisSource, ent, true);
			}
			else
			{
				ent = api.CreateEntity(name, "", layerID, thisSource, pos + offsetRight, rot);
			}

			if (offsetUp != 0)
			{
				vector entPos;
				ent.Get("coords", entPos);
				entPos[1] = entPos[1] + offsetUp;
				string coords = entPos[0].ToString() + " " + entPos[1].ToString() + " " + entPos[2].ToString();
				api.SetVariableValue(ent, null, "coords", coords);
			}

			api.SetEntityVisible(ent, isGeneratorVisible, false);
		}

		if (allowClipping)
			length = 0;

		if (UseForVerySmallObjects)
			pos += dir * (length + postPadding);
		else
			// if objects are big, advancement lower than 0.1 (10cm) is probably a bug
			// and we may end up freezing by generating outrageous amout of entities
			pos += dir * Math.Max(0.1, length + postPadding);

		return ent;
	}

	//------------------------------------------------------------------------------------------------
	protected void Generate(ShapeEntity shapeEntity)
	{
		if (!m_WallGroupContainer || m_WallGroupContainer.IsEmpty())
			return;

		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api == null || m_WallGroupContainer.IsEmpty())
			return;

		IEntitySource entSrc = api.EntityToSource(this);
		m_ParentSource = entSrc.GetParent();
		int childCount = entSrc.GetNumChildren();

		for (int i = childCount - 1; i >= 0; --i)
		{
			api.DeleteEntity(entSrc.GetChild(i));
		}

		if (m_aPoints.Count() < 2)
			return;

		int forwardAxis = 2;
		if (UseXAsForward)
			forwardAxis = 0;

		float lastObjectLength;

		if (m_bEnableLastObject && LastObject)
			lastObjectLength = MeasureEntity(LastObject, forwardAxis, api);

		bool isGeneratorVisible = api.IsEntityVisible(entSrc);

		// copy points so we have them after the entity is reinitialised
		array<ref SCR_WallGeneratorPoint> localPoints = {};
		foreach (SCR_WallGeneratorPoint wgPoint : m_aPoints)
		{
			localPoints.Insert(wgPoint);
		}

		if (m_bStartFromTheEnd)
			SCR_ArrayHelperT<ref SCR_WallGeneratorPoint>.Reverse(localPoints);

		float rotationAdjustment = 0;

		if (UseXAsForward)
			rotationAdjustment = -90;

		if (Rotate180)
			rotationAdjustment += 180;

		if (m_bStartFromTheEnd)
			rotationAdjustment += 180;

		vector from = localPoints[0].m_vPos;
		vector to, dir, prevDir, rightVec;

		bool exhausted, lastPoint, lastSegment, generate, firstPass;
		ResourceName customMesh;
		float remaining;

		// while-loop variables
		string bestWall;
		float bestLen, prePaddingToUse, postPaddingToUse, offsetUp, lengthRequirement;
		bool allowClipping, alignNext, prepadNext, custom;
		bool firstPlaced, placeMiddle, placeLast, lastPlaced, lastInSegmentDoNotPlace, middleOfSegmentDoNotPlace;
		vector offsetRight;
		SCR_WallPair wall;

		for (int i, count = localPoints.Count(); i < count; i++)
		{
			exhausted = false;
			lastPoint = i == count - 1;
			lastSegment = i == count - 2;

			prevDir = dir;

			if (ExactPlacement)
			{
				if (i == 1)
					from = localPoints[i].m_vPos;
				else
					from = localPoints[i].m_vPos + (dir * PrePadding);
			}

			if (lastPoint)
			{
				to = localPoints[i].m_vPos;
				dir = (localPoints[i].m_vPos - localPoints[i - 1].m_vPos).Normalized();
			}
			else
			{
				to = localPoints[i + 1].m_vPos;
				dir = (to - from).Normalized();
				if (i == 0)
					prevDir = dir;
			}

			customMesh = localPoints[i].m_sCustomMesh;
			generate = localPoints[i].m_bGenerate;
			remaining = vector.Distance(from, to) + m_fOvershoot;

			firstPass = true; // first segment on the polyline

			rightVec = dir * vector.Up;

			// walking the polyline line segment and populating it with assets
			// as long as there is enough room for at least the smallest wall available
			while (!exhausted)
			{
				bestWall = string.Empty;
				bestLen = 0;
				prePaddingToUse = PrePadding;
				postPaddingToUse = PostPadding;
				allowClipping = false;
				alignNext = true;
				prepadNext = true;
				offsetUp = m_fOffsetUp;
				custom = false;
				offsetRight = vector.Zero;
				firstPlaced = false;
				placeMiddle = false;
				placeLast = false;
				lengthRequirement = 0;
				lastPlaced = false;
				lastInSegmentDoNotPlace = false;
				middleOfSegmentDoNotPlace = false;

				// first object
				if (i == 0 && firstPass && m_bEnableFirstObject && !FirstObject.IsEmpty())
				{
					bestWall = FirstObject;
					bestLen = MeasureEntity(FirstObject, forwardAxis, api);
					prePaddingToUse = FirstObjectPrePadding;
					postPaddingToUse = FirstObjectPostPadding;
					firstPlaced = true;
					offsetRight = rightVec * FirstObjectOffsetRight;
					offsetUp = FirstObjectOffsetUp;
				}
				// custom mesh from the vertex data
				else if (!customMesh.IsEmpty())
				{
					bestWall = customMesh;
					prePaddingToUse = localPoints[i].m_fPrePadding;
					postPaddingToUse = localPoints[i].m_fPostPadding;
					allowClipping = localPoints[i].m_bClip;
					offsetUp = localPoints[i].m_fOffsetUp;
					alignNext = localPoints[i].m_bAlignNext;
					prepadNext = false;
					bestLen = MeasureEntity(customMesh, forwardAxis, api);
					customMesh = string.Empty;
					custom = true;
				}
				else if (!lastPoint)
				{
					wall = m_WallGroupContainer.GetRandomWall(remaining);
					if (wall)
					{
						bestLen = wall.m_fWallLength;
						bestWall = wall.m_sWallAsset;
						prePaddingToUse += wall.m_fPrePadding;
						postPaddingToUse += wall.m_fPostPadding;
					}
				}
				else
				{
					// get here when the vertex has wall data with a mesh object and it's a last point on the polyline
					break;
				}

				if (bestLen + PrePadding <= 0)
				{
					prePaddingToUse = 0;
					Print("PrePadding is set too low, not using it in this instance", LogLevel.WARNING);
				}

				if (!PrePadFirst && firstPass) // do not prepad the first asset in the segment
					prePaddingToUse = 0;

				PlacePrefab(generate, bestWall, from, dir, prevDir, rotationAdjustment, isGeneratorVisible, bestLen, prePaddingToUse, postPaddingToUse, offsetUp, alignNext, prepadNext, false, allowClipping, offsetRight);

				remaining -= (bestLen * !allowClipping) + prePaddingToUse + postPaddingToUse; // TODO: fix bool multiplier

				placeMiddle = !custom && m_bEnableMiddleObject && !MiddleObject.IsEmpty();
				placeLast = lastSegment && m_bEnableLastObject && !LastObject.IsEmpty();

				lengthRequirement = m_WallGroupContainer.m_fSmallestWall; // the minimal space required to place the next wall asset

				if (placeMiddle)
					lengthRequirement += m_WallGroupContainer.m_fMiddleObjectLength;

				if (placeLast)
					lengthRequirement += lastObjectLength;

				if (remaining < lengthRequirement)
					exhausted = true;

				lastPlaced = false;

				// no more room for 'normal' assets + placing last object
				if (exhausted && placeLast)
				{
					bestWall = LastObject;
					bestLen = lastObjectLength;
					prePaddingToUse = LastObjectPrePadding;
					postPaddingToUse = LastObjectPostPadding;
					offsetRight = rightVec * LastObjectOffsetRight;
					offsetUp = LastObjectOffsetUp;

					PlacePrefab(generate, bestWall, from, dir, prevDir, rotationAdjustment, isGeneratorVisible, bestLen, prePaddingToUse, postPaddingToUse, offsetUp, alignNext, prepadNext, false, allowClipping, offsetRight);
					lastPlaced = true;
					remaining -= (bestLen * !allowClipping) + prePaddingToUse + postPaddingToUse;
				}

				// placing middle object
				if (placeMiddle)
				{
					lastInSegmentDoNotPlace = exhausted && !(PlaceMiddleAtVertex || PlaceMiddleAtVertexOnly);

					// do not place middle object after first/last assets and any other assets which do not close a line segment
					middleOfSegmentDoNotPlace = (!exhausted && PlaceMiddleAtVertexOnly) || (PlaceMiddleAtVertexOnly && (lastPlaced || firstPlaced));

					if (!lastInSegmentDoNotPlace && !middleOfSegmentDoNotPlace)
					{
						from += dir * MiddleObjectPrePadding;
						offsetRight = rightVec * MiddleObjectOffsetRight;
						PlacePrefab(generate, MiddleObject, from, dir, prevDir, rotationAdjustment, isGeneratorVisible, m_WallGroupContainer.m_fMiddleObjectLength, MiddleObjectPrePadding, MiddleObjectPostPadding, MiddleObjectOffsetUp,true, prepadNext, true, false, offsetRight);
						remaining -= m_WallGroupContainer.m_fMiddleObjectLength + MiddleObjectPrePadding + MiddleObjectPostPadding;
					}
				}

				firstPass = false;
			}
		}

		// get the array back to normal
		if (m_bStartFromTheEnd)
			SCR_ArrayHelperT<ref SCR_WallGeneratorPoint>.Reverse(localPoints);
	}
#endif // WORKBENCH

	//------------------------------------------------------------------------------------------------
	// constructor
	void WallGeneratorEntity(IEntitySource src, IEntity parent)
	{
#ifdef WORKBENCH
		SetEventMask(EntityEvent.INIT);
#endif // WORKBENCH
	}
}

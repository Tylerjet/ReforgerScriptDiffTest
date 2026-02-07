//-----------------------------------------------------------------------
class PrefabGeneratorPointData : ShapePointDataScriptBase
{
	[Attribute("", UIWidgets.CheckBox, "Generete Assets On This Segment", "")]
	bool m_bGenerate;
};
//-----------------------------------------------------------------------
class PrefabGeneratorPointMeta
{
	bool m_bGenerate;
	vector m_vPos;
};
//-----------------------------------------------------------------------
class PrefabGeneratorAssetPoint
{
	vector Pos;
	vector Forward;
	float m_fPerlinWeight;
	bool m_bDraw = true;
};
//-----------------------------------------------------------------------
[EntityEditorProps(category: "GameLib/Scripted/Generator", description: "PrefabGeneratorEntity", dynamicBox: true, visible: false)]
class PrefabGeneratorEntityClass: SCR_GeneratorBaseEntityClass
{
};

//-----------------------------------------------------------------------
class PrefabGeneratorEntity : SCR_GeneratorBaseEntity
{
	[Attribute("", UIWidgets.None, "Prefab list", "et")]//! OBSOLETE, LEGACY PARAM
	ResourceName m_PrefabName;
	
	[Attribute("", UIWidgets.ResourceNamePicker, "Prefab list", "et")]
	ref array<ResourceName> m_PrefabNames;

	[Attribute("", UIWidgets.EditBox, "Weights", "et")]
	ref array<float> m_Weights;
	
	[Attribute("0", UIWidgets.CheckBox, "If checked prefabs are placed only to vertices")]
	bool m_bOnlyToVertices;

	[Attribute("5", UIWidgets.EditBox, "Dinstance between spawned assets along the spline/polyline")]
	float m_fDistance;

	[Attribute("1", UIWidgets.CheckBox, "If checked prefabs are aligned with the shape")]
	bool m_bAlignWithShape;

	[Attribute("0", UIWidgets.CheckBox, "Only when aligning with shape")]
	bool m_bUseXAsForward;

	[Attribute("0", UIWidgets.CheckBox, "Flip Forward")]
	bool m_bFlipForward;
	
	[Attribute("0", UIWidgets.EditBox, "How much we offset from center to the side")]
	float m_fOffsetRight;
	
	[Attribute("0", UIWidgets.EditBox, "How much offset variability we want in meters when using offset from the center")]
	float m_fOffsetVariance;
	
	[Attribute("0", UIWidgets.EditBox, "Gap from the centerline")]
	float m_fGap;

	[Attribute("0", UIWidgets.EditBox, "Random Spacing Offset")]
	bool m_fRandomSpacing;

	[Attribute("0", UIWidgets.EditBox, "How much we offset in world up vector")]
	float m_fOffsetUp;
	
	[Attribute("0", UIWidgets.EditBox, "Forward offset")]
	float m_fOffsetForward;
	
	[Attribute("0", UIWidgets.CheckBox, "Prefab spawn density uses Perlin distribution")]
	bool m_bPerlinDens;
	
	[Attribute("0", UIWidgets.EditBox, "Spawns prefabs if perlin value is above this threshold", "-1 1")]
	float m_fPerlinThreshold;
	
	[Attribute("0", UIWidgets.CheckBox, "Prefab spawn type uses Perlin distribution. Prefabs in the prefab array are stacked up from lower to higher indicies on the Y axis")]
	bool m_bPerlinAssetDistribution;
	
	[Attribute("0", UIWidgets.CheckBox, "Prefab size uses Perlin")]
	bool m_bPerlinSize;
	
	[Attribute("40", UIWidgets.EditBox, "Perlin frequency, higher values mean smoother transitions")]
	float m_fPerlinFrequency;
	
	[Attribute("0", UIWidgets.EditBox, "Perlin seed", "0 1000")]
	float m_fPerlinSeed;
	
	[Attribute("1", UIWidgets.EditBox, "Perlin Amplitude")]
	float m_fPerlinAmplitude;
	
	[Attribute("0", UIWidgets.EditBox, "Perlin Phase Offset")]
	float m_fPerlinOffset;
	
	[Attribute("0", UIWidgets.CheckBox, "Disables asset generation bellow perlin threshold")]
	bool m_fPerlinThrowAway;
	
	[Attribute("0", UIWidgets.EditBox, "Draw developer debug")]
	bool m_bDrawDebug;
	

	
	
	private ref array<vector> m_PerlinCurveDebug = new array<vector>;
	private ref array<ref PrefabGeneratorAssetPoint> m_Points = new array<ref PrefabGeneratorAssetPoint>();
	private vector m_vPrevPoint;
	static ref array<ref Shape> m_DebugShapes = new array<ref Shape>;
	//bool m_bDraw;
	#ifdef WORKBENCH

	
	//-----------------------------------------------------------------------
	override bool _WB_OnKeyChanged(BaseContainer src, string key, BaseContainerList ownerContainers, IEntity parent)
	{
		super._WB_OnKeyChanged(src, key, ownerContainers, parent);
		
		if (key == "coords")
			return false;
		
		WorldEditorAPI api = _WB_GetEditorAPI();
		if (api == null || api.UndoOrRedoIsRestoring())
			return false;

		IEntitySource entSrc = src.ToEntitySource();

		IEntitySource parentSrc;

		IEntitySource thisSrc = api.EntityToSource(this);
		parentSrc = thisSrc.GetParent();
		BaseContainerTools.WriteToInstance(this, thisSrc);
		
	
		OnShapeChanged(parentSrc, ShapeEntity.Cast(parent), {}, {});
		return true;
	}

	//-----------------------------------------------------------------------
	protected vector GetPos(BaseContainerList points, int i)
	{
		BaseContainer point = points.Get(i);
		vector pos;
		point.Get("Position", pos);
		return pos;
	}

	//-----------------------------------------------------------------------
	protected override void OnShapeInitInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity)
	{
		super.OnShapeInitInternal(shapeEntitySrc, shapeEntity);
		
		if (!shapeEntity)
			shapeEntity = ShapeEntity.Cast(_WB_GetEditorAPI().SourceToEntity(shapeEntitySrc));
		
		OnShapeChanged(shapeEntitySrc, shapeEntity, {}, {});
	}

	//Offsets the points in the 'points' array
	void OffsetPointsMeta(array<ref PrefabGeneratorPointMeta> metas, float offset, bool debugAllowed = false)
	{
		ref auto metasTemp = new array<ref PrefabGeneratorPointMeta>;

		for (int i = 0; i < metas.Count();i++)
		{
			auto meta = new PrefabGeneratorPointMeta;
			meta.m_vPos = metas.Get(i).m_vPos;
			meta.m_bGenerate = metas.Get(i).m_bGenerate;
			metasTemp.Insert(meta);
		}
		vector forwardPrev = "1 1 1";
		for (int i = 0; i < metas.Count(); i++)
		{
			vector forwardNext;
			
			if ( i < metas.Count() - 1)
			{
				forwardNext = metas.Get(i + 1).m_vPos - metas.Get(i).m_vPos;
			}
			else
			{
				forwardNext = -forwardPrev;
			}
		
			if (i == 0)
			{
				forwardPrev = -forwardNext;
			}
		
			forwardNext.Normalize();
			forwardPrev.Normalize();
		
			float dotProductPrevNext = vector.Dot(forwardPrev, forwardNext);
			bool almostLine = dotProductPrevNext < -0.95;
			vector diagonal = forwardNext + forwardPrev;
		
			vector normalRight = -forwardPrev * "0 1 0";
			normalRight.Normalize();
			float dotProductNormNext = vector.Dot(normalRight, forwardNext);
			bool isLeft = dotProductNormNext > 0;

			vector nextModified = dotProductPrevNext * forwardNext;
			float dist = vector.Distance(nextModified, forwardPrev);
			float diff;
			if (dist !=0)
				diff = offset / dist;

			diagonal *= diff;
			
			if (!isLeft)
			{
				diagonal = diagonal * -1;
			}
			
			if (almostLine)
			{
				vector vec = forwardNext - forwardPrev;
				vector right = vec * "0 1 0";
				right.Normalize();
				diagonal = right * offset;
			}
			
			metas.Get(i).m_vPos = metasTemp.Get(i).m_vPos + diagonal;
			forwardPrev = -forwardNext;

		
			//debug
			if (m_bDrawDebug && debugAllowed)
			{
				vector matWrld[4];
				GetWorldTransform(matWrld);
			
				m_DebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 255, 255), ShapeFlags.NOZBUFFER, metas.Get(i).m_vPos.Multiply4(matWrld), metas.Get(i).m_vPos.Multiply4(matWrld) + forwardNext));
				m_DebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, metas.Get(i).m_vPos.Multiply4(matWrld), metas.Get(i).m_vPos.Multiply4(matWrld) + forwardPrev));
				m_DebugShapes.Insert(Shape.Create(ShapeType.LINE, ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, metasTemp.Get(i).m_vPos.Multiply4(matWrld), metas.Get(i).m_vPos.Multiply4(matWrld)));

			}
		}
	};

	//! Get a perlin value given specific parameters
	float SamplePerlin(float time, float frequency = 20, float seed = 1, float amplitude = 1, float phaseOffset = 0)
	{
		if( frequency == 0 )//division by zero protection
			return 0;

		float result = Math.PerlinNoise(time / frequency, seed * 100, seed * 1000);
		result *= amplitude;
		return result;
	}

	//! generates array of positions(+other data) where each asset is supposed to spawn
	void GenerateAssetPoints(array<ref PrefabGeneratorPointMeta> metas )
	{
		float stepDistance = m_fDistance;
	
		if (m_bPerlinDens)
		{
			stepDistance = 0.01;//TODO:make constant
		}
	
		float distanceWalked = 0;
		if(m_fOffsetForward)
			stepDistance = m_fOffsetForward;
		
		float space = 0;
		float moveDist = 0;
		vector lastPos = "0 0 0";
		bool firstPoint = true;
		bool draw = true;
		
		for (int i = 0; i < metas.Count() - 1; i++)
		{
			vector posThis = metas[i].m_vPos;
			//Print(posThis);
			lastPos = posThis;
			vector forward = metas[i+1].m_vPos - posThis;
			float dist = forward.Length();
			space += dist;

			// check for disabled generation on line segment
			if ( metas )
			{
				draw = metas.Get(i).m_bGenerate;
			}

			while (space >= stepDistance)
			{
				float leftOver = space - dist;
				if (leftOver < 0)
					leftOver = 0;
			
				if (firstPoint)
				{
					moveDist = 0;
					stepDistance = 0;
					firstPoint = false;
					if (m_fOffsetForward)
					{
						stepDistance = m_fOffsetForward;
						moveDist = dist - (space - stepDistance);
						
					}
				}
				else
				{
					stepDistance = m_fDistance;
				
					if (m_fRandomSpacing)
					{
						stepDistance = m_fDistance + Math.RandomFloat(-m_fDistance / 2, m_fDistance / 2);
						if( leftOver> stepDistance)
							leftOver = 0;
					}
				
					moveDist = stepDistance - leftOver;
				}
				
				space -= moveDist + leftOver;
				distanceWalked += stepDistance;
				vector newPos;
				float valuePerlin = SamplePerlin(distanceWalked + m_fPerlinOffset, m_fPerlinFrequency, m_fPerlinSeed, m_fPerlinAmplitude);

				newPos = lastPos + forward.Normalized() * moveDist;
				lastPos = newPos;
				auto genPoint = new PrefabGeneratorAssetPoint();
				genPoint.Pos = newPos;
				genPoint.Forward = forward;
				genPoint.m_fPerlinWeight = valuePerlin;

				if (m_bPerlinDens)
				{
					float invValue = Math.InverseLerp(m_fPerlinThreshold, 1, valuePerlin);
					invValue = Math.Clamp(invValue, 0,1);
					stepDistance = Math.Lerp(m_fDistance, 1, invValue);//TODO: expose values
				}
			
				bool throwAway = false;
				if (m_fPerlinThrowAway)
				{
					if (valuePerlin < m_fPerlinThreshold )
					{
						stepDistance = 0.01;
						throwAway = true;
					}
				}
				
				genPoint.m_bDraw = draw && !throwAway;
				m_Points.Insert(genPoint);

			}
		}
	}
	
	void GenerateMetaListLine(BaseContainerList points, array<ref PrefabGeneratorPointMeta> pointsMeta )
	{

		for(int i = 0; i < points.Count(); i++)
		{
			BaseContainer point = points.Get(i);
			BaseContainerList dataArr = point.GetObjectArray("Data");
			bool generate = true;
			vector pos;
			point.Get("Position", pos);
			
			if (dataArr)
			{
				int dataCount = dataArr.Count();
				for (int j = 0; j < dataCount; j++)
				{
					BaseContainer data = dataArr.Get(j);
					if (data.GetClassName() == "PrefabGeneratorPointData")
					{
						data.Get("m_bGenerate", generate);
						break;
					}
				}
			}
			
			PrefabGeneratorPointMeta meta = new PrefabGeneratorPointMeta;
			meta.m_bGenerate = generate;
			meta.m_vPos = pos;
			pointsMeta.Insert(meta);
		}
	}
	
	void GenerateMetaListSpline( array<ref PrefabGeneratorPointMeta> pointsMetaLine, array<vector> teselatedPoints, array<ref PrefabGeneratorPointMeta> pointsMetaSpline )
	{
		if( teselatedPoints )
		{
			bool generate = true;
			for(int i = 0; i < teselatedPoints.Count(); i++)
			{
				for (int j = 0; j < pointsMetaLine.Count(); j++)
				{
					PrefabGeneratorPointMeta meta = pointsMetaLine.Get(j);
				
					if( teselatedPoints.Get(i) == meta.m_vPos )//matching point on the spline with a point on the polyline to ascertain whether asset generation is disabled in point data or not for this segment
					{
						generate = meta.m_bGenerate;
						break;
					}
				}
			
			PrefabGeneratorPointMeta meta = new PrefabGeneratorPointMeta;
			meta.m_bGenerate = generate;
			meta.m_vPos = teselatedPoints.Get(i);
			pointsMetaSpline.Insert(meta);
			}
		}

	}

	//-----------------------------------------------------------------------
	protected override void OnShapeChangedInternal(IEntitySource shapeEntitySrc, ShapeEntity shapeEntity, array<vector> mins, array<vector> maxes)
	{
		m_DebugShapes.Clear();
		m_Points.Clear();
		if (!shapeEntitySrc)
			return;
		BaseContainerList points = shapeEntitySrc.GetObjectArray("Points");
		if (points != null)
		{
			auto pointsMetaLine = new array<ref PrefabGeneratorPointMeta>;		
			GenerateMetaListLine(points, pointsMetaLine);
			
			int pointCount = points.Count();
			if (pointCount == 0)
				return;
		
			bool isShapeClosed;
			shapeEntitySrc.Get("IsClosed", isShapeClosed);

			if (pointCount > 2 && isShapeClosed)
			{
				pointsMetaLine.Insert(pointsMetaLine[0]);
			}
		
			if (m_fOffsetRight != 0)
			{
				OffsetPointsMeta(pointsMetaLine, m_fOffsetRight, true);
			}

			if (!m_bOnlyToVertices && m_fDistance > 0 && shapeEntitySrc.GetClassName() == "PolylineShapeEntity")
			{
				GenerateAssetPoints(pointsMetaLine);
				if (m_bDrawDebug )
					DrawCurveDebug(pointsMetaLine);
			}

			else if (!m_bOnlyToVertices && m_fDistance > 0 && shapeEntitySrc.GetClassName() == "SplineShapeEntity")
			{
				array<vector> pointsCurve = new array<vector>();
				shapeEntity.GenerateTesselatedShape(pointsCurve);
			
				auto pointsMetaSpline = new array<ref PrefabGeneratorPointMeta>;	

				GenerateMetaListSpline(pointsMetaLine, pointsCurve, pointsMetaSpline);
	
				if (pointCount > 2 && isShapeClosed)
				{
					pointsMetaSpline.Insert(pointsMetaSpline[0]);
				}
			
				if (m_fOffsetRight != 0)
				{
					OffsetPointsMeta(pointsMetaSpline, m_fOffsetRight, true);
				}
			
				GenerateAssetPoints(pointsMetaSpline);
				if (m_bDrawDebug )
					DrawCurveDebug(pointsMetaSpline);
			}
			else if (m_bOnlyToVertices)
			{
				if (m_bDrawDebug )
					DrawCurveDebug(pointsMetaLine);
				// First point special care
				auto genPoint = new PrefabGeneratorAssetPoint();
				genPoint.Pos = pointsMetaLine[0].m_vPos;
			
				if ( pointsMetaLine )
				{
					genPoint.m_bDraw = pointsMetaLine.Get(0).m_bGenerate;
				}

				if (isShapeClosed && pointCount > 2)
					genPoint.Forward = pointsMetaLine[1].m_vPos - pointsMetaLine[pointCount - 1].m_vPos.Normalized();
				else if (pointCount > 1)
					genPoint.Forward = (pointsMetaLine[1].m_vPos - pointsMetaLine[0].m_vPos).Normalized();

				m_Points.Insert(genPoint);

				// Middle points
				for (int i = 1; i < pointsMetaLine.Count(); i++)
				{
					genPoint = new PrefabGeneratorAssetPoint();
					genPoint.Pos = pointsMetaLine[i % pointCount].m_vPos;
				
					if ( pointsMetaLine )
					{
						genPoint.m_bDraw = pointsMetaLine.Get(i).m_bGenerate;
					}
					if (i < pointsMetaLine.Count() - 1 || isShapeClosed)
						genPoint.Forward = (pointsMetaLine[(i + 1) % pointCount].m_vPos - pointsMetaLine[i - 1].m_vPos).Normalized();
					else if (i == pointsMetaLine.Count() - 1)
						genPoint.Forward = (pointsMetaLine[i % pointCount].m_vPos - pointsMetaLine[i - 1].m_vPos).Normalized();
					m_Points.Insert(genPoint);
				}
			}
			else
			{
				return;
			}
		}
		Generate();
	}

	//-----------------------------------------------------------------------
	//! Generate the assets
	void Generate()
	{
		WorldEditorAPI api = _WB_GetEditorAPI();
	
		array<string> prefabs = new array<string>();
	
		for (int i = 0; i < m_PrefabNames.Count(); i++)
		{
			if (m_PrefabNames[i] != "")
				prefabs.Insert(m_PrefabNames[i]);
		}

		if (api == null || prefabs.Count() == 0 || api.UndoOrRedoIsRestoring())
			return;

		IEntitySource entSrc = api.EntityToSource(this);
		int childCount = entSrc.GetNumChildren();
	
		for (int i = childCount - 1; i >= 0; --i)
		{
			IEntitySource childSrc = entSrc.GetChild(i);
			IEntity child = api.SourceToEntity(childSrc);
			api.DeleteEntity(child);
		}

		bool isGeneratorVisible = api.IsEntityVisible(this);
		vector worldMat[4];
		GetWorldTransform(worldMat);
		auto localPoints = m_Points;
		foreach (int i, PrefabGeneratorAssetPoint p: localPoints)
		{
			if (!p.m_bDraw)
				continue;
			vector worldPos = p.Pos.Multiply4(worldMat);
			
			vector rot = "0 0 0";
			vector mat[4];
			vector up = {0, 1, 0};
			Math3D.DirectionAndUpMatrix(p.Forward, up, mat);
			vector right = mat[0];
			vector angles = Math3D.MatrixToAngles(mat);

			if (m_fOffsetVariance != 0)
			{
				float offsetRandom = Math.RandomFloat(-m_fOffsetVariance/2 + m_fGap/2, m_fOffsetVariance/2 - m_fGap/2);
				if (offsetRandom < 0)
				{
					offsetRandom -= m_fGap/2;
				}
				else
				{
					offsetRandom += m_fGap/2;
				}
				
				worldPos = worldPos + offsetRandom * right;
			}
		
			if (m_bAlignWithShape)
			{

				if (m_bUseXAsForward)
					angles[0] = angles[0] - 90;
				if (m_bFlipForward)
					angles[0] = angles[0] + 180;

				rot = {angles[1], angles[0], angles[2]};
			}
		
			int index;
			index = Math.Clamp(SCR_Global.GetWeighedElement(m_Weights, Math.RandomFloat01()),0, m_PrefabNames.Count() - 1);
		
			if (m_bPerlinAssetDistribution)
			{
				float value = Math.Clamp(p.m_fPerlinWeight, m_fPerlinThreshold, 1);
				float invValue = Math.InverseLerp(m_fPerlinThreshold, 1, value);
				
				index = Math.Clamp(SCR_Global.GetWeighedElement(m_Weights, invValue),0, m_PrefabNames.Count() - 1);
			}
			
			string asset = m_PrefabNames[index];
		
		
			IEntity ent = api.CreateEntityExt(asset, "", 0, null, worldPos, rot, TraceFlags.WORLD);
			IEntitySource src = api.EntityToSource(ent);
		
			if (m_bPerlinSize)
			{
				BaseContainerList editorData = src.GetObjectArray("editorData");
			
                if (editorData && editorData.Count())
                {
					float valuePrln = Math.Clamp(p.m_fPerlinWeight, m_fPerlinThreshold, 1);
					//Print("raw perlin:" +p.m_fPerlinWeight);
					float invValue = Math.InverseLerp(m_fPerlinThreshold, 1, valuePrln);
					vector val;
                    editorData[0].Get("randomScale", val);
					
					string scale = (Math.Lerp(val[0], val[1], invValue )).ToString();
					api.ModifyEntityKey(ent, "scale", scale);
                }
			}
		
			if (m_fOffsetUp != 0)
			{
				vector entPos;
				src.Get("coords", entPos);
				entPos[1] = entPos[1] + m_fOffsetUp;
				string coords = entPos[0].ToString() + " " + entPos[1].ToString() + " " + entPos[2].ToString();
				api.ModifyEntityKey(ent, "coords", coords);
			}
			api.SetEntityVisible(ent, isGeneratorVisible, false);
			api.ParentEntity(this, ent, true);
		}
	}
	
	//! Draws debug curve
	//void DrawCurveDebug(array<vector> points)
	void DrawCurveDebug(array<ref PrefabGeneratorPointMeta> metas)
	{
		const int DEBUG_CURVE_LINE_SIZE = 8192;
		const float DEBUG_Y_MULTIPLIER = 5;
		static vector m_PerlinDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_ZeroDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_EdgeDebugLine[DEBUG_CURVE_LINE_SIZE];
		static vector m_ThresholdDebugLine[DEBUG_CURVE_LINE_SIZE];
		
		
		float stepDistance = 0.1;
		float distanceWalked = 0;
		vector lastPos = "0 0 0";
		float space = 0;
		float moveDist = 0;
		
		array<vector> line = new array<vector>;
		int itemsNum = 0;

		vector matWrld[4];
		GetWorldTransform(matWrld);
		
		int i = 0;
		for (; i < metas.Count() - 1; i++)
		{
			vector posThis = metas[i].m_vPos;
			lastPos = posThis;
			vector forward = metas[i+1].m_vPos - posThis;
			float dist = forward.Length();
			space += dist;
			
			m_ZeroDebugLine[i] = posThis.Multiply4(matWrld);
			
			if (i == metas.Count() - 2)
				m_ZeroDebugLine[i+1] = metas[i+1].m_vPos.Multiply4(matWrld);
			
			
			while (space >= stepDistance)
			{
				float leftOver = space - dist;
				if (leftOver < 0)
					leftOver = 0;
			
				moveDist = stepDistance - leftOver;

				space -= moveDist + leftOver;
				distanceWalked += stepDistance;
				vector stepPos;
				vector perlinPointPos;
				
				stepPos = lastPos + forward.Normalized() * moveDist;
				lastPos = stepPos;
				
				vector mat[4];
				vector up = {0, 1, 0};
				Math3D.DirectionAndUpMatrix(forward, up, mat);
				vector right = mat[0];
				
				float valuePerlin = SamplePerlin(distanceWalked + m_fPerlinOffset, m_fPerlinFrequency, m_fPerlinSeed, m_fPerlinAmplitude);
				//Print(distanceWalked);
				//Print(valuePerlin);
				perlinPointPos = stepPos + right * valuePerlin * DEBUG_Y_MULTIPLIER;
				
				if (itemsNum < DEBUG_CURVE_LINE_SIZE)
				{
					m_PerlinDebugLine[itemsNum] = perlinPointPos.Multiply4(matWrld);
					m_EdgeDebugLine[itemsNum] = (stepPos + right * DEBUG_Y_MULTIPLIER).Multiply4(matWrld);
					m_ThresholdDebugLine[itemsNum] = (stepPos + right * DEBUG_Y_MULTIPLIER * m_fPerlinThreshold).Multiply4(matWrld);
					itemsNum++;
				}
			}
		}
		
		m_DebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 255, 0), ShapeFlags.NOZBUFFER, m_PerlinDebugLine, itemsNum));
		m_DebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 255, 0), ShapeFlags.NOZBUFFER, m_ZeroDebugLine, i+1));
		m_DebugShapes.Insert(Shape.CreateLines(ARGB(255, 255, 0, 0), ShapeFlags.NOZBUFFER, m_EdgeDebugLine, itemsNum));
		m_DebugShapes.Insert(Shape.CreateLines(ARGB(255, 0, 0, 255), ShapeFlags.NOZBUFFER, m_ThresholdDebugLine, itemsNum));
	}
	
	// ! saves perlin in an image for debug purposes
	void PerlinDebug()
	{
			string filePath = "d:\\test.dds";
			ref array<int> data = new array<int>;
		
			const int WIDTH = 1024;
			const int HEIGHT = 1024;
		
			for (int y = 0; y < HEIGHT; y++) for (int x = 0; x < WIDTH; x++)
			{
				int count = x * y;
			
				float perlinVal = Math.PerlinNoise(x/m_fPerlinFrequency, y/m_fPerlinFrequency);
				//Print(perlinVal);
					
				int pixel = ARGB(255, perlinVal * 255, perlinVal * 255, perlinVal * 255 );
				data.Insert(pixel);
			}

			// save dds to file
			if (TexTools.SaveImageData(filePath, WIDTH, HEIGHT, data) == false)
			{
				//Print("Can't save image", LogLevel.ERROR);
				return;
			}
	}
	
	//-----------------------------------------------------------------------
	protected void PrefabGeneratorEntity(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
	
	//-----------------------------------------------------------------------
	protected void ~PrefabGeneratorEntity()
	{
		
	}
	
	
	#endif
	
};
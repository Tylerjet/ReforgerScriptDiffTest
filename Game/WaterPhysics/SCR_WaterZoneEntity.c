[EntityEditorProps(category: "GameScripted/Physics", description: "Water zone entity", dynamicBox: true, color: "0 128 255 255")]
class SCR_WaterZoneEntityClass: GenericEntityClass
{
};

class SCR_WaterZoneEntity: GenericEntity
{
	[Attribute("100", UIWidgets.Slider, "Width of water zone (in m)", "0 50000 0.1", category: "Water Zone")]
	float m_Width;
	[Attribute("100", UIWidgets.Slider, "Length of water zone (in m)", "0 50000 0.1", category: "Water Zone")]
	float m_Length;
	[Attribute("100", UIWidgets.Slider, "Depth of water zone (in m)", "0 5000 0.1", category: "Water Zone")]
	float m_Depth;
	[Attribute("1", UIWidgets.Slider, "Density of the liquid", "0 5 0.01", category: "Water Zone")]
	float m_Density;
	[Attribute("1", UIWidgets.Slider, "Scalar for how strong water currents are", "0 10 0.01", category: "Water Zone")]
	float m_CurrentStrength;
	[Attribute("30", UIWidgets.Slider, "Maximum depth below surface to apply water current forces", "0 1000 0.1", category: "Water Zone")]
	float m_MaxCurrentDepth;
	[Attribute("0", UIWidgets.CheckBox, "Whether this zone is for the ocean (this then uses the ocean wave heights etc)", category: "Water Zone")]
	bool m_IsOcean;
	[Attribute("{290DF77ED4B876EC}Common/Materials/Physics/default.physmat", UIWidgets.ResourceNamePicker, "Physics material to use for the water trigger collider", "physmat", category: "Water Zone")]
	ResourceName m_Physmat;
	
	#ifndef DISABLE_WATERPHYSICS
		protected ref array<IEntity> m_ContactEnts = {};
		
		//------------------------------------------------------------------------------------------------
		void GetWaterBounds(out vector mins, out vector maxs)
		{
			mins = Vector(-m_Width * 0.5, -m_Depth, -m_Length * 0.5);
			maxs = Vector(m_Width * 0.5, 0, m_Length * 0.5);
		}
		
		//------------------------------------------------------------------------------------------------
		vector GetWaterSurfaceAtPos(vector pos)
		{
			vector result = pos;
			result[1] = GetOrigin()[1];
			
			return result;
		}
		
		//------------------------------------------------------------------------------------------------
		float GetWaveHeightWorld(vector pos)
		{
			if (GetWorld().IsOcean())
				return GetWorld().GetOceanHeight(pos[0], pos[2]);
			
			return GetOrigin()[1];
		}
		
		//------------------------------------------------------------------------------------------------
		float GetWaveDepth(vector pos)
		{
			return GetOrigin()[1] - GetWaveHeightWorld(pos);
		}
		
		//------------------------------------------------------------------------------------------------
		vector GetWaterCurrentSpeed(IEntity ent, SCR_WaterPhysicsComponent waterPhys, float timeSlice)
		{
		/*
			if (GetWorld() && GetWorld().IsOcean())
			{
				vector pos = ent.GetOrigin();
				vector current = GetWorld().GetOceanHeightAndDisplace(pos[0], pos[2]) * (1 / timeSlice);
				float waveHeight = current[1];
				float waveheightDiff = waveHeight - waterPhys.m_fLastWaveHeight;
				if (waterPhys.m_fLastWaveHeight == 0)
					waveheightDiff = 0;
				waterPhys.m_fLastWaveHeight = waveHeight;
				if (waveheightDiff < 0)
					waveheightDiff = 0;
				current[1] = waveheightDiff;
				float currentScale = 0;
				if (m_MaxCurrentDepth > 0)
					currentScale = 1 - Math.Clamp((waveHeight - pos[1]) / m_MaxCurrentDepth, 0, 1); // No current deep below waves
				current *= current * Math.Pow(currentScale, 2) * 10;
				
				return current * m_CurrentStrength;
			}
		*/
			
			return vector.Zero;
		}
		
		//------------------------------------------------------------------------------------------------
		// Returns true if the position is within the zone's area
		bool TestArea(vector pos)
		{
			vector thisMins, thisMaxs;
			GetWaterBounds(thisMins, thisMaxs);
			thisMins += GetOrigin();
			thisMaxs += GetOrigin();
			return SCR_Global.IntersectBoxPoint(pos, thisMins, thisMaxs);
		}
		
		//------------------------------------------------------------------------------------------------
		// Returns true if the input box's mins and maxs are within the zone's area
		bool TestAreaMinsMaxs(vector mins, vector maxs)
		{
			vector thisMins, thisMaxs;
			GetWaterBounds(thisMins, thisMaxs);
			thisMins = thisMins + GetOrigin();
			thisMaxs = thisMaxs + GetOrigin();
			return Math3D.IntersectionBoxBox(thisMins, thisMaxs, mins, maxs);
		}
		
		//------------------------------------------------------------------------------------------------
		// Returns the depth at the input position
		float GetDepth(vector pos)
		{
			if (!TestArea(pos))
				return 0;
			
			return GetOrigin()[1] -pos[1];
		}
		
		//------------------------------------------------------------------------------------------------
		// Returns the depth at the input UP axis position
		float GetUpDepth(float myPosY, float posY)
		{
			if (posY > myPosY)
				return 0;
			if (posY < myPosY - m_Depth)
				return 0;
			
			return myPosY - posY;
		}
		
		//------------------------------------------------------------------------------------------------
		// Returns the depth of the input object's bound box
		float GetBoundBoxDepth(vector mins, vector maxs)
		{
			if (!TestAreaMinsMaxs(mins, maxs))
				return 0;
			
			float myside;
			float side;
			side = mins[1];
			myside = GetOrigin()[1];
			if (side < myside && side > -m_Depth + myside) // Below surface with bottom
				return Math.AbsFloat(side - myside);
			else
			{
				side = maxs[1];
				if (side < myside && side > -m_Depth + myside) // Above bottom with top
					return Math.AbsFloat(side - myside);
			}
			
			return 0;
		}
	
		//------------------------------------------------------------------------------------------------
		void UnregisterEntity(IEntity entity)
		{
			m_ContactEnts.RemoveItem(entity);
		}
		
		//------------------------------------------------------------------------------------------------
		override void EOnContact(IEntity owner, IEntity other, Contact contact) //!EntityEvent.CONTACT
		{
			if (!other)
				return;
			
			Physics otherPhysics = other.GetPhysics();
			if (!otherPhysics)
				return;
		
			if (!otherPhysics.IsDynamic())
				return;
			
			if (m_ContactEnts.Find(other) > -1)
				return;
			
			m_ContactEnts.Insert(other);
			SCR_WaterPhysicsComponent waterPhysics = SCR_WaterPhysicsComponent.Cast(other.FindComponent(SCR_WaterPhysicsComponent));
			if (waterPhysics)
				waterPhysics.SetWaterZone(this);
		}
		
		//------------------------------------------------------------------------------------------------
		override void EOnInit(IEntity owner) //!EntityEvent.INIT
		{
			Physics physics = GetPhysics();
			if (physics)
				physics.Destroy();
			
			ref PhysicsGeomDef geoms[1];
			PhysicsGeomDef geomDef = new PhysicsGeomDef("WaterZone", PhysicsGeom.CreateBox(Vector(m_Width, m_Depth, m_Length)), m_Physmat, EPhysicsLayerDefs.Water);
			
			geomDef.Frame[3] = Vector(0, -m_Depth * 0.5, 0);
			geoms[0] = geomDef;
			
			physics = Physics.CreateGhostEx(this, geoms);
			if (!physics)
			{
				Print("WATERZONE: Unable to create ghost collider", LogLevel.ERROR);
				return;
			}
		}
		
		#ifdef WORKBENCH
			//------------------------------------------------------------------------------------------------
			override void _WB_GetBoundBox(inout vector min, inout vector max, IEntitySource src)
			{
				GetWaterBounds(min, max);
			}
			
			//------------------------------------------------------------------------------------------------
			bool WB_GetSelected()
			{
				for (int i = 0; i < _WB_GetEditorAPI().GetSelectedEntitiesCount(); i++)
				{
					IEntity ent = _WB_GetEditorAPI().GetSelectedEntity(i);
					if (ent == this)
						return true;
				}
				
				return false;
			}
			
			//------------------------------------------------------------------------------------------------
			override void _WB_AfterWorldUpdate(float timeSlice)
			{
				vector mat[4];
				GetWorldTransform(mat);
				
				vector thisMins, thisMaxs;
				GetWaterBounds(thisMins, thisMaxs);
				Shape shp;
				if (WB_GetSelected())
				{
					shp = Shape.Create(ShapeType.BBOX, ARGB(64, 0, 128, 255), ShapeFlags.ONCE|ShapeFlags.TRANSP, thisMins, thisMaxs);
					shp.SetMatrix(mat);
					shp = Shape.Create(ShapeType.BBOX, ARGB(255, 0, 255, 255), ShapeFlags.ONCE|ShapeFlags.WIREFRAME|ShapeFlags.NOZBUFFER, thisMins, thisMaxs);
					shp.SetMatrix(mat);
				}
				mat[3] = mat[3] + vector.Up;
				DebugTextWorldSpace.CreateInWorld(GetWorld(), "WATER: " + GetName(), DebugTextFlags.CENTER | DebugTextFlags.ONCE | DebugTextFlags.FACE_CAMERA, mat, 1, ARGBF(1, 1, 1, 1), ARGBF(0, 0, 0, 0));
			}
		#endif
		
		//------------------------------------------------------------------------------------------------
		void SCR_WaterZoneEntity(IEntitySource src, IEntity parent)
		{
			SetFlags(EntityFlags.ACTIVE, false);
			SetEventMask(EntityEvent.INIT | EntityEvent.CONTACT);
		}
		
		//------------------------------------------------------------------------------------------------
		void ~SCR_WaterZoneEntity()
		{
			m_ContactEnts.Clear();
			m_ContactEnts = null;
			
			Physics physics = GetPhysics();
			if (physics)
				physics.Destroy();
		}
	#endif
};


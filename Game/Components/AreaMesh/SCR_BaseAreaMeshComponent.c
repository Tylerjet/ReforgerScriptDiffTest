[ComponentEditorProps(category: "GameScripted/Area Mesh", description: "")]
class SCR_BaseAreaMeshComponentClass: ScriptComponentClass
{
};
class SCR_BaseAreaMeshComponent: ScriptComponent
{
	[Attribute("10")]
	protected float m_fHeight;
	
	[Attribute("12", desc: "How many segments in the ellipse.", uiwidget: UIWidgets.Slider, params: "4 60 1", category: "Virtual Area")]
	protected int m_vResolution;
	
	[Attribute(desc: "True to let the border copy terrain, creating a 'wall'.", category: "Virtual Area")]
	protected bool m_bFollowTerrain;
	
	[Attribute(desc: "True to stretch the material along the whole circumference instead of mapping it on each segment.", category: "Virtual Area")]
	protected bool m_bStretchMaterial;
	
	[Attribute(desc: "Material mapped on outside and inside of the mesh. Inside mapping is mirrored.", uiwidget: UIWidgets.ResourcePickerThumbnail, params: "emat", category: "Virtual Area")]
	protected ResourceName m_Material;
	
	[Attribute("false", desc: "When enabled, the component will be active from init", category: "Virtual Area")]
	protected bool m_bActiveEveryFrameOnInit;
	
	protected vector m_vLastPos;
	protected vector m_vLastDir;
	
	//! Activate the area so it could be updated every frame
	void ActivateEveryFrame()
	{
		SetEventMask(GetOwner(), EntityEvent.FRAME);
	}
	
	//! Deactivate the area
	void DeactivateEveryFrame()
	{
		ClearEventMask(GetOwner(), EntityEvent.FRAME);
	}
	
	/*!
	Get radius of the area.
	To be overloaded by inherited classes.
	\return Radius
	*/
	float GetRadius()
	{
	}
	
	//~ Get the position of the AreaMesh. Can be overwritten to set custom position
	protected vector GetPosition(IEntity owner)
	{
		return owner.GetOrigin();
	}
	
	/*!
	Generate area mesh based on its settings.
	*/
	void GenerateAreaMesh()
	{
		IEntity owner = GetOwner();
		
		float radius = GetRadius();
		if (radius <= 0)
		{
			owner.SetObject(null, "");
			return;
		}
		
		//--- Reset orientation, as the mesh is created in local space
		vector transform[4];
		Math3D.MatrixIdentity3(transform);
		transform[3] = GetPosition(owner);
		owner.SetWorldTransform(transform);
		
		float dirStep = Math.PI2 / m_vResolution;
		array<vector> positions = {};
		
		//--- Get positions
		for (int v = 0; v < m_vResolution; v++)
		{
			float dir = dirStep * v;
			vector pos = Vector(Math.Sin(dir) * radius, -m_fHeight, Math.Cos(dir) * radius);
			positions.Insert(pos);
		}
		
		//--- Snap all positions to ground
		if (m_bFollowTerrain)
		{
			BaseWorld world = owner.GetWorld();
			vector worldPos;
			foreach (int i, vector pos: positions)
			{
				worldPos = owner.CoordToParent(pos);
				worldPos[1] = Math.Max(world.GetSurfaceY(worldPos[0], worldPos[2]), 0);
				positions[i] = owner.CoordToLocal(worldPos);
			}
		}
		
		MeshObject meshObject = SCR_Shape.CreateAreaMesh(positions, m_fHeight * 2, m_Material, m_bStretchMaterial);
		if (meshObject)
		{
			owner.SetObject(meshObject, "");
			
			m_vLastPos = GetPosition(owner);
			m_vLastDir = owner.GetAngles();
		}
		
	}
	
	override void EOnFrame(IEntity owner, float timeSlice)
	{
		if (vector.DistanceSq(m_vLastPos, GetPosition(owner)) > 0.1 || vector.DistanceSq(m_vLastDir, owner.GetAngles()) > 0.1)
			GenerateAreaMesh();
	}
	
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
		
		if (m_bActiveEveryFrameOnInit)
		{
			ActivateEveryFrame();
		}
	}
	
	#ifdef WORKBENCH
	//~ Makes sure mesh area is generated at the correct position in workbench
	override void _WB_SetTransform(IEntity owner, inout vector mat[4], IEntitySource src)
	{
		GenerateAreaMesh();
	}
	#endif
};
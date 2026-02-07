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
	
	/*!
	Get radius of the area.
	To be overloaded by inherited classes.
	\return Radius
	*/
	float GetRadius()
	{
	}
	/*!
	Generate area mesh based on its settings.
	*/
	void GenerateAreaMesh()
	{
		float radius = GetRadius();
		if (radius <= 0)
		{
			GetOwner().SetObject(null, "");
			return;
		}
		
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
			vector transform[4];
			float entitySurfaceY;
			GetOwner().GetTransform(transform);
			BaseWorld world = GetOwner().GetWorld();
			entitySurfaceY = Math.Max(world.GetSurfaceY(transform[3][0], transform[3][2]), 0);
			foreach (int i, vector pos: positions)
			{
				vector worldPos = transform[3] + transform[0] * pos[0] + transform[2] * pos[2];
				pos[1] = Math.Max(world.GetSurfaceY(worldPos[0], worldPos[2]), 0) - entitySurfaceY;
				positions[i] = pos;
			}
		}
		
		MeshObject meshObject = SCR_Shape.CreateAreaMesh(positions, m_fHeight * 2, m_Material, m_bStretchMaterial);
		if (meshObject)
			GetOwner().SetObject(meshObject, "");
	}
	override void OnPostInit(IEntity owner)
	{
		owner.SetFlags(EntityFlags.ACTIVE, false);
		SetEventMask(owner, EntityEvent.INIT);
	}

};
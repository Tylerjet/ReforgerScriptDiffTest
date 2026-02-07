//------------------------------------------------------------------------------------------------
class SCR_MoveInCircleComponentClass: ScriptComponentClass
{
};

class SCR_MoveInCircleComponent : ScriptComponent
{
	ref array<vector> m_CirclePoints = new array<vector>;
	vector m_vPrevPos;
	float m_fElapsedTime = 0;
	//------------------------------------------------------------------------------------------------
	override protected void EOnFrame(IEntity owner, float timeSlice)
	{
		m_fElapsedTime += timeSlice / 10;
		//Print(m_fElapsedTime);
		if(m_fElapsedTime > 1)
		{
			m_fElapsedTime = 0;
		}
		vector mat[4];
		Math3D.MatrixIdentity4(mat);
		
		mat[3] = Math3D.Curve(ECurveType.NaturalCubic, m_fElapsedTime, m_CirclePoints);
		vector rotMat[3];
		float rot =  Math.Lerp(0,360, m_fElapsedTime);
		vector rotV;
		rotV[0] = rot;
		Math3D.AnglesToMatrix(rotV, rotMat);
		mat[0] = rotMat[0];
		mat[1] = rotMat[1];
		mat[2] = rotMat[2];
		//Print(mat[3]);
		owner.SetTransform(mat);

	}
	
	//------------------------------------------------------------------------------------------------
	override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT | EntityEvent.FRAME);
		owner.SetFlags(EntityFlags.ACTIVE, true);
	}

	//------------------------------------------------------------------------------------------------
	override void EOnInit(IEntity owner)
	{
		vector entityPos = owner.GetOrigin();
		for (float i = 0; i < Math.PI2; )
		{
			float x = Math.Cos(i) * 10;
			float y = Math.Sin(i) * 10;
			i = i + 0.03;
			vector point;
			point[0] = entityPos[0] + x;
			point[1] = entityPos[1] + 0;
			point[2] = entityPos[2] + y;
			
			m_CirclePoints.Insert(point);
		}
		Print("done");
		
	}

	//------------------------------------------------------------------------------------------------
	void SCR_MoveInCircleComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}

	//------------------------------------------------------------------------------------------------
	void ~SCR_MoveInCircleComponent()
	{
	}

};

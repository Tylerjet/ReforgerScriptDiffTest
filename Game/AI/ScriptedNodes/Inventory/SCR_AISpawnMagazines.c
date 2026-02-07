class SCR_AISpawnMagazines : AITaskScripted
{
	static const string PORT_MAGAZINE_WELL = "MagazineWell";
	
	ref map<string, string> m_mTypenameToResourceName;
	
	[Attribute("", UIWidgets.EditBox, "Name of magazine well" )]
	protected string m_sMagazineWellType;
	
	[Attribute("4", UIWidgets.Slider, "Amount of magazines", "0 10 1" )]
	protected int m_iMagazineCount;

	private typename m_oMagazineWell;
	
	protected static ref TStringArray s_aVarsIn = {
		PORT_MAGAZINE_WELL
	};
	override TStringArray GetVariablesIn()
    {
        return s_aVarsIn;
    }
	
	override void OnInit(AIAgent owner)
	{
		m_mTypenameToResourceName = new map<string, string>;
		m_mTypenameToResourceName.Insert("MagazineWellAK545","{E5912E45754CD421}Prefabs/Weapons/Magazines/Magazine_545x39_AK_30rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellStanag556","{A9A385FE1F7BF4BD}Prefabs/Weapons/Magazines/Magazine_556x45_STANAG_30rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellPKM","{BEEA49E27174B224}Prefabs/Weapons/Magazines/Box_762x54_PK_100rnd_Tracer.et");
		m_mTypenameToResourceName.Insert("MagazineWellM60","{AD8AB93729348C3E}Prefabs/Weapons/Magazines/Box_762x51_M60_100rnd_Tracer.et");
	}	
	
	override ENodeResult EOnTaskSimulate(AIAgent owner, float dt)
	{
		if (!GetVariableIn(PORT_MAGAZINE_WELL,m_oMagazineWell))
			m_oMagazineWell = m_sMagazineWellType.ToType();
		
		ResourceName resourceName;
		if (m_mTypenameToResourceName.Find(m_oMagazineWell.ToString(),resourceName))
		{
			EntitySpawnParams params = EntitySpawnParams();
			params.TransformMode = ETransformMode.WORLD;
			params.Transform[3] = owner.GetControlledEntity().GetOrigin();	
			Resource res = Resource.Load(resourceName);
			
			for (int i = 0; i < m_iMagazineCount; i++)
				GetGame().SpawnEntityPrefab(res, null, params);
			return ENodeResult.SUCCESS;			
		}
		return ENodeResult.FAIL;			
	}	
		
	protected override bool VisibleInPalette()
	{
		return true;
	}
	
	protected override string GetOnHoverDescription()
	{
		return "AI task that picks up all magazines of provided MagazineWell type in the vicinity of its inventory.";
	}	
};
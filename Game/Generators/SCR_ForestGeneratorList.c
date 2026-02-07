[BaseContainerProps()]
class SCR_PrefabsList
{
	[Attribute()]
	ref array<ref SCR_PrefabListObject> m_Prefabs;

	ResourceName m_sConfigPath;

	//------------------------------------------------------------------------------------------------
	void SetConfigPath(ResourceName path)
	{
		m_sConfigPath = path;
	}
};

[BaseContainerProps(namingConvention: NamingConvention.NC_MUST_HAVE_NAME)]
class SCR_PrefabListObject : Managed
{
	[Attribute()]
	int m_iID;

	[Attribute()]
	ResourceName m_Prefab;
};

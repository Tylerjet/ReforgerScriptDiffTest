//Base class to be used for categories in radial menu

class BaseSelectionMenuCategory : ScriptedSelectionMenuEntry
{
	protected string m_sCategoryName;
	
	protected ref array<ref BaseSelectionMenuEntry> m_aCategoryContent = {};
	
	protected BaseSelectionMenuCategory m_ParentCategory;
	
	//------------------------------------------------------------------------------------------------
	string GetCategoryName()
	{
		return m_sCategoryName;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetCategoryName(string name)
	{
		m_sCategoryName = name;
	}
	
	//------------------------------------------------------------------------------------------------
	void AddElementToCategory(BaseSelectionMenuEntry element)
	{
		m_aCategoryContent.Insert(element);
	}
	
	//------------------------------------------------------------------------------------------------
	array<ref BaseSelectionMenuEntry> GetCategoryElements()
	{
		return m_aCategoryContent;
	}
	
	//------------------------------------------------------------------------------------------------
	BaseSelectionMenuCategory GetParentCategory()
	{
		return m_ParentCategory;
	}	
	
	//------------------------------------------------------------------------------------------------
	void SetParentCategory(BaseSelectionMenuCategory category)
	{
		m_ParentCategory = category;
	}
}
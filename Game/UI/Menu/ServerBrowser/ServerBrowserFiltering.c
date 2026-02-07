/*
File for server browser search filtering parameters class.
*/

//------------------------------------------------------------------------------------------------ 
//! Server search filtering
class FilteredServerParams : RoomFilterBase
{
	// Constant filter parameters 
	const int RECENT_SERVERS_SECONDS = 60 * 60 * 24 * 7;
	
	// Filter names 
	const string FILTER_FAVORITES = "favorites";
	const string FILTER_RECENT_SERVERS = "oldestJoinInSeconds";
	const string FILTER_MODDED = "modded";
	
	const string SORT_ASCENDENT = "ascendent";
	const string SORT_NAME = "SessionName";
	
	// Filters	
	protected ref SCR_FilterSet m_Filter;
	protected ref array<SCR_FilterEntryRoom> m_aFiltersUncategorized = {};
	protected ref array<ref SCR_FilterEntryRoom> m_aDefaultFilters = {};

	protected ref array<string> modIds = new ref array<string>;

	// Direct search
	protected string text = "";
	
	protected string hostAddress = "";
	protected string directJoinCode = "";
	
	// Scenario filter 
	protected string scenarioId = "";
	protected string hostedScenarioModId = "";
	
	// Sorting order
	protected string order = SORT_NAME;
	
	// Tabs filter
	protected bool m_bFavoriteFilterOn = false;
	protected bool m_bRecentlyPlayedOn = false;
	
	// Custom filter specific 
	protected int m_iSelectedTab = 0;
	protected int lockedBoth = 0;
	
	//--------------------------------------------
	// Overrided filter functions 
	//--------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	override void OnPack()
	{	
		// Setup filters 
		SCR_FilterEntryRoom favorite = FindFilterByInternalName(m_aDefaultFilters, FILTER_FAVORITES);
		favorite.SetSelected(m_bFavoriteFilterOn);
		
		SCR_FilterEntryRoom recentlyPlayed = FindFilterByInternalName(m_aDefaultFilters, FILTER_RECENT_SERVERS);
		recentlyPlayed.SetSelected(m_bRecentlyPlayedOn);
		
		m_bModdedFilterSelected = false;
		
		// Register filters 
		foreach (SCR_FilterEntryRoom filter : m_aFiltersUncategorized)
		{		
			// Store boolean of selected 
			if (filter.GetSelected())
			{			
				ActivateFilterValues(filter);
			}
		}
		
		// Set search by mod id 
		if (modIds.IsEmpty())
			UnregV("modIds");
		else
			RegV("modIds");
	}
	
	//--------------------------------------------
	// Public functions 
	//--------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	void AddModId(string sId)
	{
		modIds.Insert(sId);
	}
	
	//------------------------------------------------------------------------------------------------
	void ClearModIds()
	{
		modIds.Clear();
	}
	
	//------------------------------------------------------------------------------------------------
	void FilterByScenraioId(string id, bool allow)
	{
		scenarioId = id;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup host address param and clear join code 
	void SetHostAddress(string address) 
	{ 
		hostAddress = address; 
		directJoinCode = string.Empty;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup join code param and clear host address 
	void SetJoinCode(string joinCode) 
	{ 
		directJoinCode = joinCode;
		hostAddress = string.Empty;	 
	}
	
	//------------------------------------------------------------------------------------------------
	void FilteredServerParams()
	{	
		includePing = true;
		
		// Apply filter 
		//RegV("lockedBoth");
		RegV("directJoinCode");
		
		RegV("hostAddress");
		
		// Sort 
		RegV("order");
		RegV("text");
		
		// Room content 
		RegV("modIds");
		RegV("scenarioId");
		
		// Sort ascendence 
		SCR_FilterEntryRoom asc = new SCR_FilterEntryRoom;
		asc.m_sInternalName = SORT_ASCENDENT;
		asc.AddFilterValue(SORT_ASCENDENT, "1", EFilterType.TYPE_BOOL);
		asc.SetSelected(true);
		m_aDefaultFilters.Insert(asc);
		
		// Favorite 
		SCR_FilterEntryRoom filterFavorite = new SCR_FilterEntryRoom;
		filterFavorite.m_sInternalName = FILTER_FAVORITES;
		filterFavorite.AddFilterValue(FILTER_FAVORITES, "1", EFilterType.TYPE_BOOL);
		filterFavorite.SetSelected(true);
		m_aDefaultFilters.Insert(filterFavorite);
		
		// Recent servers 
		SCR_FilterEntryRoom filterRecent = new SCR_FilterEntryRoom;
		filterRecent.m_sInternalName = FILTER_RECENT_SERVERS;
		filterRecent.AddFilterValue(FILTER_RECENT_SERVERS, RECENT_SERVERS_SECONDS.ToString(), EFilterType.TYPE_INT);
		filterRecent.SetSelected(true);
		m_aDefaultFilters.Insert(filterRecent);
		
		// Add default filters 
		foreach (SCR_FilterEntryRoom filter : m_aDefaultFilters)
			m_aFiltersUncategorized.Insert(filter);
	}
	
	//--------------------------------------------
	// Protected functions 
	//--------------------------------------------

	//-----------------------------------------------------------------------------------------------------------
	//! Return filter with given internal name. Find filter within categories
	protected SCR_FilterEntryRoom FindFilterByInternalName(array<ref SCR_FilterEntryRoom> filters, string internalName)
	{
		if (!filters || filters.IsEmpty())
			return null;
		
		// Go throught filtres 
		foreach (SCR_FilterEntryRoom roomFilter : filters)
		{
			// check name and value 
			if (roomFilter.m_sInternalName == internalName) 
				return roomFilter;
		}
		
		return null;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Store all velues in filter array
	protected void ActivateFilterValues(SCR_FilterEntryRoom filter)
	{
		// Check filter 
		if (!filter)
			return;
		
		// Check values 
		array<ref SCR_FilterEntryRoomValue> values = filter.GetValues();
		
		if (!values || values.IsEmpty())
			return;
		
		// Go through values 
		foreach (SCR_FilterEntryRoomValue value : values)
		{
			// Values 
			string name = value.GetName();
			
			// Track modded filter 
			if (name == FILTER_MODDED && value.GetBoolValue() == true)
				m_bModdedFilterSelected = filter.GetSelected();
			
			// Check type 
			EFilterType filterType = value.GetType();
				
			switch(filterType)
			{
				// Booleans
				case EFilterType.TYPE_BOOL:
				StoreBoolean(name, value.GetBoolValue());
				break;
				
				// Integers 
				case EFilterType.TYPE_INT:
				StoreInteger(name, value.GetIntNumberValue());
				break;
				
				// Floating decimals 
				case EFilterType.TYPE_FLOAT:
				StoreFloat(name, value.GetFloatNumberValue());
				break;
				
				// Strings
				case EFilterType.TYPE_STRING:
				StoreString(name, value.GetStringValue());
				break;
			}
		}
	}
	
	//--------------------------------------------
	// Get & set
	//--------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	//! Go thorught filters in categories and create uncatergorized filter list 
	void SetFilters(SCR_FilterSet filterSet) 
	{ 
		// Filter list clear up
		if (!m_aFiltersUncategorized.IsEmpty())
			m_aFiltersUncategorized.Clear();
		
		// Check filter set 
		if (!filterSet)
			return; 
		
		m_Filter = filterSet;
		
		// Check categories 
		array<ref SCR_FilterCategory> categories = filterSet.GetFilterCategories();
		if (!categories || categories.IsEmpty())
			return;
		
		// Go thorugh filter set categories 
		foreach (SCR_FilterCategory category : categories)
		{
			// Add filters to uncategorized 
			foreach (SCR_FilterEntry filter : category.GetFilters())
			{
				SCR_FilterEntryRoom roomFilter = SCR_FilterEntryRoom.Cast(filter);
				if (roomFilter)
				{
					m_aFiltersUncategorized.Insert(roomFilter); 
				}
			}
		}
		
		// Add default filters 
		foreach (SCR_FilterEntryRoom filter : m_aDefaultFilters)
		{
			m_aFiltersUncategorized.Insert(filter);
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void SetFavoriteFilter(bool isAllowed)
	{
		m_bFavoriteFilterOn = isAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetFavoriteFilter()
	{
		return m_bFavoriteFilterOn;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetRecentlyPlayedFilter(bool isAllowed)
	{
		m_bRecentlyPlayedOn = isAllowed;
	}
	
	//------------------------------------------------------------------------------------------------
	bool GetRecentlyPlayedFilter()
	{
		return m_bRecentlyPlayedOn;
	}
	
	//------------------------------------------------------------------------------------------------
	void SetSorting(string mOrder, bool bAscendent)
	{
		order = mOrder;

		// find order filter 
		SCR_FilterEntryRoom orderFilter = FindFilterByInternalName(m_aDefaultFilters, SORT_ASCENDENT);

		// Set order 
		if (orderFilter)
		{
			orderFilter.GetValues()[0].SetValue("0");
			
			if (bAscendent)
				orderFilter.GetValues()[0].SetValue("1");
		}
	}
	
	//------------------------------------------------------------------------------------------------
	string GetSortOrder() { return order; }
	
	//------------------------------------------------------------------------------------------------
	void SetSearch(string sInput) { text = sInput; }
	
	//------------------------------------------------------------------------------------------------
	string GetSearchText() { return text; }
	
	//------------------------------------------------------------------------------------------------
	void SetSelectedTab(int tabId) { m_iSelectedTab = tabId; }
	
	//------------------------------------------------------------------------------------------------
	int GetSelectedTab() { return m_iSelectedTab; }
	
	// Scenario id 
	
	//------------------------------------------------------------------------------------------------
	string GetScenarioId() { return scenarioId; }
	
	//------------------------------------------------------------------------------------------------
	void SetScenarioId(string id) { scenarioId = id; }
	
	// Scenario mod id 
	
	//------------------------------------------------------------------------------------------------
	string GetHostedScenarioModId() { return hostedScenarioModId; }
	
	//------------------------------------------------------------------------------------------------
	void SetHostedScenarioModId(string id) 
	{ 
		hostedScenarioModId = id;
		
		// Registration
		if (!hostedScenarioModId.IsEmpty())
			RegV("hostedScenarioModId");
		else 
			UnregV("hostedScenarioModId");
	}
	
	protected bool m_bModdedFilterSelected;
	
	//------------------------------------------------------------------------------------------------
	bool IsModdedFilterSelected()
	{
		return m_bModdedFilterSelected;
	}

	
	
	//--------------------------------------------
	// Default filter setups 
	//--------------------------------------------
	
	//------------------------------------------------------------------------------------------------
	protected void DefaultFilter()
	{
		array<ref SCR_FilterEntryRoom> filters = {};
		
		// Clear filters 
		foreach (SCR_FilterEntryRoom filter : m_aFiltersUncategorized)
		{
			filters.Insert(filter);
			if (filter.GetSelected())
				Print("count: " + filter.m_sInternalName);
			
			filter.SetSelected(false);
		}
		
		// Clear search string 
		SetSearch(string.Empty);
		
		// Set ascendecy 
		SCR_FilterEntryRoom asc = FindFilterByInternalName(filters, SORT_ASCENDENT);
		if (asc)
			asc.SetSelected(true);
	}
	
	//protected array<SCR_FilterEntryRoom> m_aFiltersStored;
	
	//------------------------------------------------------------------------------------------------
	//! Setup default filters with favorites filtered only
	void DefaulFilterFavorite()
	{
		// Save filters 
		//m_aFiltersStored = m_aFiltersUncategorized;
		/*foreach (SCR_FilterEntryRoom filter : m_aFiltersUncategorized)
		{
			SCR_FilterEntryRoom filterToAdd = new SCR_FilterEntryRoom;
			filterToAdd = filter;
			
			m_aFiltersStored.Insert(filterToAdd);
			if (filter.GetSelected())
				Print("count: " + filter.m_sInternalName);
		}*/
		
		m_bFavoriteFilterOn = true;
		DefaultFilter();
		
		// Apply filter 
		SCR_FilterEntryRoom favorite = FindFilterByInternalName(m_aDefaultFilters, FILTER_FAVORITES);
		favorite.SetSelected(m_bFavoriteFilterOn);
	}
	
	//------------------------------------------------------------------------------------------------
	//! Setup default filters with recently played filtered only
	void DefaulFilterRecentlyPlayed()
	{
		m_bRecentlyPlayedOn = true;
		DefaultFilter();
		
		// Apply filters 
		SCR_FilterEntryRoom recentlyPlayed = FindFilterByInternalName(m_aDefaultFilters, FILTER_RECENT_SERVERS);
		recentlyPlayed.SetSelected(m_bRecentlyPlayedOn);
	}
	
	//------------------------------------------------------------------------------------------------
	void RestoreFilters()
	{
		/*m_aFiltersUncategorized.Clear();
		
		foreach (SCR_FilterEntryRoom filter : m_aFiltersStored)
		{
			m_aFiltersUncategorized.Insert(filter);
			if (filter.GetSelected())
				Print("count: " + filter.m_sInternalName);
		}*/
	}
};

//------------------------------------------------------------------------------------------------
class BoolFilter
{
	string m_sName = "";
	bool m_bValue = false;
	bool m_bIsUsed = false;
	
	void BoolFilter(string name, bool value, bool used = false)
	{
		m_sName = name;
		m_bValue = value;
		m_bIsUsed = used;
	}
};

//------------------------------------------------------------------------------------------------
// Filter entry class extended with value 
[BaseContainerProps(), SCR_FilterEntryTitle()]
class SCR_FilterEntryRoom : SCR_FilterEntry
{
	// Filter values array 
	[Attribute("", UIWidgets.Object)]
	protected ref array<ref SCR_FilterEntryRoomValue> m_aValues;
	
	//------------------------------------------------------------------------------------------------
	void AddFilterValue(string name, string value, EFilterType type)
	{
		// Check and initiate filter values
		if (!m_aValues)
			m_aValues = new array<ref SCR_FilterEntryRoomValue>;
		
		// Setup filter value
		SCR_FilterEntryRoomValue filterValue = new SCR_FilterEntryRoomValue;
		filterValue.SetName(name);
		filterValue.SetValue(value);
		filterValue.SetType(type);
		
		// Add value 
		m_aValues.Insert(filterValue);
	}

	//------------------------------------------------------------------------------------------------
	array<ref SCR_FilterEntryRoomValue> GetValues() { return m_aValues; }
};

//------------------------------------------------------------------------------------------------
[BaseContainerProps(), SCR_FilterEntryTitle()]
class SCR_FilterEntryRoomValue
{
	[Attribute("", UIWidgets.EditBox, "Name of used filter")]
	protected string m_sInternalName;
	
	[Attribute("", UIWidgets.EditBox, "Filter value for any type")]
	protected string m_sValue;
	
	[Attribute("0", UIWidgets.ComboBox, desc: "Filter variable type", category: "", ParamEnumArray.FromEnum(EFilterType))]
	protected EFilterType m_iType;
	
	//------------------------------------------------------------------------------------------------
	void SetName(string name) { m_sInternalName = name; }
	
	//------------------------------------------------------------------------------------------------
	string GetName() { return m_sInternalName; }
	
	//------------------------------------------------------------------------------------------------
	void SetType(EFilterType type) { m_iType = type; }
	
	//------------------------------------------------------------------------------------------------
	EFilterType GetType() { return m_iType; }
	
	// Setting and returning value 
	
	//------------------------------------------------------------------------------------------------
	void SetValue(string value) { m_sValue = value; }
	
	//------------------------------------------------------------------------------------------------
	string GetStringValue() { return m_sValue; }
	
	//------------------------------------------------------------------------------------------------
	bool GetBoolValue() { return m_sValue == "1"; }
	
	//------------------------------------------------------------------------------------------------
	float GetIntNumberValue() { return m_sValue.ToInt(); }
	
	//------------------------------------------------------------------------------------------------
	float GetFloatNumberValue() { return m_sValue.ToFloat(); } 
};

//------------------------------------------------------------------------------------------------
enum EFilterType
{
	TYPE_BOOL,
	TYPE_INT,
	TYPE_FLOAT,
	TYPE_STRING,
};
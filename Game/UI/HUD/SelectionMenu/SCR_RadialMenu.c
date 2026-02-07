//------------------------------------------------------------------------------------------------
class SCR_RadialMenu : SCR_BaseSelectionMenu
{
	#ifdef ENABLE_DIAG
	protected Widget m_wDiagRoot;
	protected Widget m_wDiagGizmo;
	#endif
	
	//! The minimum input magnitude for selection to be valid
	protected float m_fMinInputMagnitude = 0.25;
	
	//! Last selected element or null if none
	protected BaseSelectionMenuEntry m_pLastSelection;
	
	//! Filter used for filtering active/inactive actions
	protected ref SCR_RadialMenuFilter m_pFilter = new SCR_RadialMenuFilter();
	
	//! Thumbstick selection switch
	[Attribute("", UIWidgets.CheckBox, "Whick thumbstick is used for item selection \n false - Left stick \n true - Right stick")]
	protected bool m_bSelectionThumbstick;
	
	//------------------------------------------------------------------------------------------------
	// -1 for back, 0 for 1.st, elementCount for last
	// TODO: Move to global func
	protected int GetSelectedElementIndex(float angle, int elementCount)
	{
		if (elementCount == 0)
			return -1;

		float step = (2 * Math.PI) / elementCount;
		int idx = (int)Math.Round(angle/step);
		if (idx < 0)
			idx = elementCount - Math.AbsInt(idx);

		return (int)Math.Clamp(idx, 0, elementCount);
	}
	
	//------------------------------------------------------------------------------------------------
	//! TODO: Move to global func
	protected float GetClampedAngle(vector v1, vector v2, int elementCount)
	{
		if (elementCount == 0)
			return 0.0;

		float angle = GetAngle(v1, v2);
		float step = (2 * Math.PI) / (float)elementCount;
		float as = angle/step;
		float reg = Math.Round(as);

		return reg * step;
	}
	
	//------------------------------------------------------------------------------------------------
	protected float GetClampedAngle(float x, float y, int elementCount)
	{
		if (elementCount == 0)
			return 0.0;

		float angle = Math.Atan2(x,y);
		float step = (2 * Math.PI) / (float)elementCount;
		float as = angle/step;
		float reg = Math.Round(as);

		return reg * step;
	}
	
	//------------------------------------------------------------------------------------------------
	//! TODO: Move to global func
	static float GetAngle(vector v1, vector v2)
	{
		vector vec1 = v1.Normalized();
		vector vec2 = v2.Normalized();
		float angle = Math.Atan2(vec2[1], vec2[0]) - Math.Atan2(vec1[1], vec1[0]);

		return angle;
	}
	
	//------------------------------------------------------------------------------------------------
	//! Callback when open is requested
	protected override event void OnOpen(IEntity owner)
	{
		// Check if we have valid display and if not,
		// try to create a default one
		if (m_pDisplay == null)
		{
			m_pDisplay = new SCR_RadialMenuDisplay(owner);
			if (m_pDisplay)
				m_pDisplay.SetDefault();
		}
		
		if (m_pDisplay)
			m_pDisplay.SetOpen(owner, true);
		
		super.OnOpen(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when close is requested
	protected override event void OnClose(IEntity owner)
	{
		if (m_pDisplay)
			m_pDisplay.SetOpen(owner, false);
		
		// See if we have something to perform and we can perform it
		if (m_pLastSelection && m_pLastSelection.CanBePerformed(owner, this))
		{
			m_pLastSelection.Perform(owner, this);
		}
		
		super.OnClose(owner);
	}

	//------------------------------------------------------------------------------------------------
	//! Callback when menu update is requested
	protected override event void OnUpdate(IEntity owner, float timeSlice)
	{
		#ifdef ENABLE_DIAG
		if (IsRadialMenuDiagEnabled())
		{
			if (IsOpen())
			{			
				if (m_wDiagRoot == null)
				{
					m_wDiagRoot = GetGame().GetWorkspace().CreateWidgets("{908058418A98008E}UI/layouts/Debug/RadialDebugElement.layout");
					if (m_wDiagRoot)
					{
						m_wDiagGizmo = m_wDiagRoot.FindAnyWidget("Foreground");
						if (m_wDiagGizmo) 
						{
							m_wDiagGizmo.SetColorInt(ARGB(128,32,255,32));
							FrameSlot.SetAnchorMin(m_wDiagGizmo, 0.5, 0.5);
							FrameSlot.SetAnchorMax(m_wDiagGizmo, 0.5, 0.5);
							FrameSlot.SetAlignment(m_wDiagGizmo, 0.5, 0.5);
						}
					}
				}	
			}
			else if (m_wDiagRoot)
			{
				m_wDiagRoot.RemoveFromHierarchy();
				m_wDiagRoot = null;
			}
		}
		#endif
		
		
		// Do not update if menu is closed
		if (!IsOpen())
			return;
		
		// Activate input context and fetch input data
		InputManager inputManager = GetGame().GetInputManager();
		inputManager.ActivateContext("RadialMenuContext");
		float radialX, radialY;
		
		// thumbstick selection
		if (!m_bSelectionThumbstick)
		{
			radialX = inputManager.GetActionValue("RadialX");
			radialY = inputManager.GetActionValue("RadialY");
		}
		else
		{
			radialX = inputManager.GetActionValue("RadialX2");
			radialY = inputManager.GetActionValue("RadialY2");
		}
		
		#ifdef ENABLE_DIAG
		if (IsRadialMenuDiagEnabled())
		{
			if (m_wDiagRoot && m_wDiagGizmo)
			{
				const float gizmoScale = 300.0;
				vector input = Vector(radialX, -radialY, 0.0) * gizmoScale;
				
				FrameSlot.SetPos(m_wDiagGizmo, input[0], input[1]);
			}
		}
		#endif
		
		
		// Filter out entries based on their shown/disabled state
		if (m_pFilter)
			m_pFilter.DoFilter(owner, this);
		
		// Update input selection
		auto elementsCount = m_pFilter.m_aAllEntries.Count();
		float inputAngle = GetClampedAngle(radialX, radialY, elementsCount);
		
		int selection = -1;
		if ( Math.AbsFloat(radialX)+Math.AbsFloat(radialY) >= m_fMinInputMagnitude )
		{		
			selection = GetSelectedElementIndex(inputAngle, elementsCount);
		}			
		
		// Based on input select an element
		BaseSelectionMenuEntry selectedElement;		
		
		// If we have a valid selection, check it
		if (selection == -1)
			selectedElement = null;
		else if (selection < elementsCount)
			selectedElement = m_pFilter.m_aAllEntries[selection];
		
		// Entry we selected is disabled, clear selection
		if (m_pFilter.m_aDisabledEntries.Find(selectedElement) != -1)
			selectedElement = null;		
		
		// Update selection
		m_pLastSelection = selectedElement;
			
		// Pass in data to the radial menu
		if (m_pDisplay)
		{
			// Send in selected element
			m_pDisplay.SetSelection(m_pLastSelection, Vector(radialX, radialY, 0.0), inputAngle, m_fMinInputMagnitude);
			
			// Pass in data for display
			m_pDisplay.SetContent(m_pFilter.m_aAllEntries, m_pFilter.m_aDisabledEntries);
		}
		
		// TODO: Handle update of widgets
		// TODO: Handle input, selection and performing of the action
		super.OnUpdate(owner, timeSlice);
	}
	
	//------------------------------------------------------------------------------------------------
	void ~SCR_RadialMenu()
	{
		#ifdef ENABLE_DIAG
		if (m_wDiagRoot != null)
		{
			m_wDiagRoot.RemoveFromHierarchy();
			m_wDiagRoot = null;
			m_wDiagGizmo = null;
		}
		#endif
	}
};

//------------------------------------------------------------------------------------------------
//! Utility class for passing around lists of entries
/*class SCR_RadialMenuFilter
{	
	//! Contains all entries in order they were received
	//! Can contain items which are disabled too
	ref array<BaseSelectionMenuEntry> m_aAllEntries;
	
	//! Contains entries which cannot be performed (but can be shown)
	ref array<BaseSelectionMenuEntry> m_aDisabledEntries;
	
	//------------------------------------------------------------------------------------------------
	void DoFilter(IEntity owner, BaseSelectionMenu menu)
	{
		m_aAllEntries.Clear();
		m_aDisabledEntries.Clear();
		
		if (!owner || !menu)
			return;
		
		auto entries = new array<BaseSelectionMenuEntry>();
		auto entriesCount = menu.GetEntryList(entries);
		for (int i = 0; i < entriesCount; i++)
		{
			auto currentEntry = entries[i];
			if (!currentEntry)
				continue;			
			
			if (!currentEntry.CanBeShown(owner, menu))
				continue;
			
			m_aAllEntries.Insert(currentEntry);
			
			if (!currentEntry.CanBePerformed(owner, menu))
				m_aDisabledEntries.Insert(currentEntry);
		}
	}	
	
	//------------------------------------------------------------------------------------------------
	void SCR_RadialMenuFilter()
	{
		m_aAllEntries = new ref array<BaseSelectionMenuEntry>();
		m_aDisabledEntries = new ref array<BaseSelectionMenuEntry>();	
	}
}*/
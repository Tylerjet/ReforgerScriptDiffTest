//------------------------------------------------------------------------------------------------
class SCR_EditBoxSearchComponent : SCR_EditBoxComponent 
{
	const Color COLOR_DEFAULT = Color.White;
	const Color COLOR_FOCUSED = Color.White;
	const Color COLOR_SEARCHED = UIColors.CONTRAST_COLOR;
	
	protected bool m_bIsFilterActive;
	protected Widget m_wSeachIcon;
	
	//------------------------------------------------------------------------------------------------
	override protected void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wSeachIcon = w.FindAnyWidget("WriteIcon");
		if (m_wSeachIcon)
			m_wSeachIcon.SetColor(COLOR_DEFAULT);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnHandlerFocus()
	{
		super.OnHandlerFocus();
		
		if (!m_bIsFilterActive)
			WidgetAnimator.PlayAnimation(m_wSeachIcon, WidgetAnimationType.Color, COLOR_FOCUSED, m_fColorsAnimationTime);
	}
	
	//------------------------------------------------------------------------------------------------
	override void OnHandlerFocusLost()
	{
		super.OnHandlerFocusLost();
		if (!m_bIsFilterActive)
			WidgetAnimator.PlayAnimation(m_wSeachIcon, WidgetAnimationType.Color, COLOR_DEFAULT, m_fColorsAnimationTime);
	}

	//------------------------------------------------------------------------------------------------
	override protected void OnConfirm(Widget w)
	{
		super.OnConfirm(w);
		if (!m_bIsTyping)
			return;

		if (GetValue() == string.Empty)
		{
			m_bIsFilterActive = false;
			WidgetAnimator.PlayAnimation(m_wSeachIcon, WidgetAnimationType.Color, COLOR_FOCUSED, m_fColorsAnimationTime);
		}
		else
		{
			WidgetAnimator.PlayAnimation(m_wSeachIcon, WidgetAnimationType.Color, COLOR_SEARCHED, m_fColorsAnimationTime);
			m_bIsFilterActive = true;
		}
	}
	
	//------------------------------------------------------------------------------------------------
	//! Static method to easily find component by providing name and parent.
	//! Searching all children will go through whole hierarchy, instead of immediate chidren
	static SCR_EditBoxSearchComponent GetEditBoxSearchComponent(string name, Widget parent, bool searchAllChildren = true)
	{
		auto comp = SCR_EditBoxSearchComponent.Cast(
				SCR_WLibComponentBase.GetComponent(SCR_EditBoxSearchComponent, name, parent, searchAllChildren)
			);
		return comp;
	}
};
//------------------------------------------------------------------------------------------------
class WelcomeDialogUI: DialogUI
{
	protected string m_sHeaderName = "Text0";
	protected string m_sTextName = "Text";
	protected string m_sSelectionHintName = "SelectionHint";
	protected string m_sPreviousName = "Previous";
	protected string m_sNextName = "Next";
	protected string m_sLinksName = "Links";

	protected TextWidget m_wText;
	protected Widget m_wHeader;
	protected Widget m_wLinks;
	protected SCR_SelectionHintComponent m_SelectionHint;
	protected SCR_NavigationButtonComponent m_Previous;
	protected SCR_NavigationButtonComponent m_Next;

	protected int m_iCurrentIndex;
	protected int m_iMaxIndex;
	protected ref array<string> m_aPageTexts = 
	{
		"#ar-welcomescreen_text_01",
		"#ar-welcomescreen_text_02",
		"#ar-welcomescreen_text_03",
		"#ar-welcomescreen_text_04",
		"#ar-welcomescreen_text_05",
		"#ar-welcomescreen_text_06",
		"#ar-welcomescreen_text_07"
	};

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen()
	{
		super.OnMenuOpen();
		
		m_iMaxIndex = m_aPageTexts.Count() - 1;
		Widget w = GetRootWidget();
		m_wText = TextWidget.Cast(w.FindAnyWidget(m_sTextName));
		m_wHeader = w.FindAnyWidget(m_sHeaderName);
		m_wLinks = w.FindAnyWidget(m_sLinksName);

		// Make sure esc will close the dialog too
		GetGame().GetInputManager().AddActionListener("MenuBack", EActionTrigger.DOWN, OnConfirm);

		m_Previous = SCR_NavigationButtonComponent.GetNavigationButtonComponent(m_sPreviousName, w);
		if (m_Previous)
			m_Previous.m_OnActivated.Insert(OnPrevious);

		m_Next = SCR_NavigationButtonComponent.GetNavigationButtonComponent(m_sNextName, w);
		if (m_Next)
			m_Next.m_OnActivated.Insert(OnNext);

		Widget hint = w.FindAnyWidget(m_sSelectionHintName);
		if (hint)
		{
			m_SelectionHint = SCR_SelectionHintComponent.Cast(hint.FindHandler(SCR_SelectionHintComponent));
			if (m_SelectionHint)
			{
				m_SelectionHint.SetItemCount(m_aPageTexts.Count(), false);
				m_SelectionHint.SetCurrentItem(m_iCurrentIndex, false);
			}
		}

		ShowIndex(0);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnPrevious()
	{
		ShowIndex(m_iCurrentIndex - 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void OnNext()
	{
		ShowIndex(m_iCurrentIndex + 1);
	}

	//------------------------------------------------------------------------------------------------
	protected void ShowIndex(int index)
	{
		if (index < 0 || index > m_iMaxIndex)
			return;

		m_iCurrentIndex = index;
		
		if (m_wHeader)
			m_wHeader.SetVisible(index == 0);
		if (m_wLinks)
			m_wLinks.SetVisible(index == m_iMaxIndex);
		if (m_Previous)
			m_Previous.SetEnabled(index != 0);
		if (m_Next)
			m_Next.SetEnabled(index != m_iMaxIndex);
		if (m_wText)
			m_wText.SetTextFormat(m_aPageTexts[index]);
		if (m_SelectionHint)
			m_SelectionHint.SetCurrentItem(index);
	}
};



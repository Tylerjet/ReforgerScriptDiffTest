//------------------------------------------------------------------------------------------------
class SCR_NewsTileComponent : SCR_TileBaseComponent
{
	[Attribute("Title")]
	string m_sTitleName;
	
	[Attribute("Description")]
	string m_sDescriptionName;
	
	[Attribute("Image")]
	string m_sImageName;
	
	[Attribute("Author")]
	string m_sAuthorName;
	
	[Attribute("Date")]
	string m_sDateName;
	
	[Attribute("Footer")]
	string m_sFooterName;
	
	[Attribute("Unread")]
	string m_sUnreadImageName;
	
	[Attribute("Read")]
	string m_sReadButtonName;

	Widget m_wFooter;
	Widget m_wUnreadImage;
	TextWidget m_wTitle;
	TextWidget m_wAuthor;
	TextWidget m_wDate;
	TextWidget m_wDescription;
	ref SCR_NewsEntry m_Entry;
	
	SCR_NavigationButtonComponent m_Read;
	
	ref ScriptInvoker m_OnRead = new ScriptInvoker();

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_wUnreadImage = w.FindAnyWidget(m_sUnreadImageName);
		m_wTitle = TextWidget.Cast(w.FindAnyWidget(m_sTitleName));
		m_wAuthor = TextWidget.Cast(w.FindAnyWidget(m_sAuthorName));
		m_wDate = TextWidget.Cast(w.FindAnyWidget(m_sDateName));
		m_wDescription = TextWidget.Cast(w.FindAnyWidget(m_sDescriptionName));
		m_wFooter = w.FindAnyWidget(m_sFooterName);
		
		if (!m_sReadButtonName.IsEmpty())
			m_Read = SCR_NavigationButtonComponent.GetNavigationButtonComponent(m_sReadButtonName, w);
		if (m_Read)
			m_Read.m_OnActivated.Insert(OnRead);

		if (!m_wFooter)
		return;
		
		m_wFooter.SetOpacity(0);
		m_wFooter.SetEnabled(false);
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocus(Widget w, int x, int y)
	{
		super.OnFocus(w, x, y);
		if (m_wFooter)
		{
			m_wFooter.SetEnabled(true);
			WidgetAnimator.PlayAnimation(m_wFooter, WidgetAnimationType.Opacity, 1, m_fAnimationRate);
		}
		
		SetRead();

		m_OnFocused.Invoke(this);
		return false;
	}
	
	//------------------------------------------------------------------------------------------------
	override bool OnFocusLost(Widget w, int x, int y)
	{
		super.OnFocusLost(w, x, y);
		if (!m_wFooter)
			return false;
		
		WidgetAnimator.PlayAnimation(m_wFooter, WidgetAnimationType.Opacity, 0, m_fAnimationRate);
		m_wFooter.SetEnabled(false);
		return false;
	}

	//------------------------------------------------------------------------------------------------
	void SetRead()
	{
		if (!m_Entry || m_Entry.m_bRead)
			return;

		m_Entry.m_bRead = true;
		if (m_wUnreadImage)
			WidgetAnimator.PlayAnimation(m_wUnreadImage, WidgetAnimationType.Opacity, 0, WidgetAnimator.FADE_RATE_DEFAULT);
		
		array<SCR_AccountWidgetComponent> accounts = SCR_AccountWidgetComponent.GetInstances();
		foreach (SCR_AccountWidgetComponent account : accounts)
		{
			if (account)
				account.UpdateNotifications();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	void ShowTile(SCR_NewsEntry entry)
	{
		m_wRoot.SetOpacity(entry != null);
		m_wRoot.SetEnabled(entry != null);
		
		m_Entry = entry;
		if (!entry)
			return;

		if (m_wTitle)
			m_wTitle.SetText(entry.m_Item.Title());
		
		if (m_wDescription)
			m_wDescription.SetText(entry.m_Item.Excerpt());
		
		if (m_wAuthor)
			m_wAuthor.SetText(entry.m_Item.Slug());
		
		if (m_wDate)
			m_wDate.SetText(entry.m_Item.Date());

		
		string imagePreview; // TODO: Will be implemented later
		if (m_wImage && !imagePreview.IsEmpty())
			m_wImage.LoadImageTexture(0, imagePreview);
		
		if (m_wUnreadImage)
			m_wUnreadImage.SetOpacity(!entry.m_bRead);
	}

	//------------------------------------------------------------------------------------------------
	void OnRead()
	{
		SetRead();
		m_OnRead.Invoke(this);
	}
};
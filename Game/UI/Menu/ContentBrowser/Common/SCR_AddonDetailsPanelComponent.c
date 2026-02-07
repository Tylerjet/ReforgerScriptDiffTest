/*
Panel which shows state of addon.

!!! This component relies on SCR_WorkshopUiCommon to be initialized to work correctly.
*/

class SCR_AddonDetailsPanelComponent : SCR_ContentDetailsPanelBase
{
	protected const int MAX_ADDON_TYPE_IMAGES = 12; // Max amount of addon type images will be shown.
	
	protected ref SCR_AddonInfoWidgets m_InfoWidgets = new SCR_AddonInfoWidgets();
	protected ref SCR_WorkshopItem m_Item;
	protected SCR_WorkshopItemBackendImageComponent m_BackendImageComponent;
	
	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);

		m_InfoWidgets.Init(m_CommonWidgets.m_wAdditionalInfo.GetChildren());
		
		m_BackendImageComponent = SCR_WorkshopItemBackendImageComponent.Cast(m_CommonWidgets.m_wBackendImage.FindHandler(SCR_WorkshopItemBackendImageComponent));
		
		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	//! Updates properties which never change at run time (addon description, size, etc)
	//! This is called only when new addon is assigned or at start
	protected void UpdateAllWidgets()
	{
		// Don't mess up widgets in workbench
		if (SCR_Global.IsEditMode())
			return;

		ClearTypeDisplays();
		
		if (!m_Item)
		{
			m_CommonWidgets.m_WarningOverlayComponent.SetWarningVisible(false);
			m_CommonWidgets.m_wMainArea.SetVisible(false);
			m_BackendImageComponent.SetWorkshopItemAndImage(null, null);
			return;
		}
		
		m_CommonWidgets.m_wMainArea.SetVisible(true);

		// Rating
		int rating = Math.Ceil(m_Item.GetAverageRating() * 100.0);
		m_InfoWidgets.m_wModRating.SetText(WidgetManager.Translate("#AR-ValueUnit_Percentage", rating));
		
		// Name, description, author name
		m_CommonWidgets.m_wNameText.SetText(m_Item.GetName());
		m_CommonWidgets.m_wAuthorNameText.SetText(m_Item.GetAuthorName());
		m_CommonWidgets.m_wDescriptionText.SetText(m_Item.GetSummary());

		// Size
		float sizef = m_Item.GetSizeBytes();
		string sizeStr = SCR_ByteFormat.GetReadableSize(sizef);
		m_InfoWidgets.m_wModSize.SetText(sizeStr);

		// Version
		Revision revisionCurrent = m_Item.GetCurrentLocalRevision();
		Revision revisionLatest = m_Item.GetLatestRevision();

		bool needsUpdate = revisionCurrent && revisionLatest && !Revision.AreEqual(revisionCurrent, revisionLatest);
		
		m_InfoWidgets.m_wVersionUpdateHorizontalLayout.SetVisible(needsUpdate);
		
		// Version text
		if (needsUpdate || !m_Item.GetOffline())
			m_InfoWidgets.m_wCurrentVersion.SetColor(UIColors.NEUTRAL_ACTIVE_STANDBY);
		else
			m_InfoWidgets.m_wCurrentVersion.SetColor(UIColors.CONFIRM);

		string revision;
		if (m_Item.GetOffline() && revisionCurrent)
			revision = revisionCurrent.GetVersion();
		else if (revisionLatest)
			revision = revisionLatest.GetVersion();
		
		m_InfoWidgets.m_wCurrentVersion.SetText(SCR_WorkshopUiCommon.FormatVersion(revision));
		
		if (revisionLatest)
			m_InfoWidgets.m_wUpdateVersion.SetText(SCR_WorkshopUiCommon.FormatVersion(revisionLatest.GetVersion()));
		
		// Version Icon
		if (!m_Item.GetOffline())
		{
			m_InfoWidgets.m_wCurrentVersionIcon.SetColor(UIColors.NEUTRAL_ACTIVE_STANDBY);
			m_InfoWidgets.m_wCurrentVersionIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, "download");
		}
		else if (needsUpdate)
		{
			m_InfoWidgets.m_wCurrentVersionIcon.SetColor(UIColors.SLIGHT_WARNING);
			m_InfoWidgets.m_wCurrentVersionIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, "update");
		}
		else
		{
			m_InfoWidgets.m_wCurrentVersionIcon.SetColor(UIColors.CONFIRM);
			m_InfoWidgets.m_wCurrentVersionIcon.LoadImageFromSet(0, UIConstants.ICONS_IMAGE_SET, "check");
		}
		
		// Time since last downloaded
		int timeSinceDownload = m_Item.GetTimeSinceFirstDownload();
		m_InfoWidgets.m_wLastDownloadedHorizontalLayout.SetVisible(timeSinceDownload >= 0);
		
		if (timeSinceDownload >= 0)
			m_InfoWidgets.m_wLastDownloaded.SetText(SCR_FormatHelper.GetTimeSinceEventImprecise(timeSinceDownload));

		// Types
		array<WorkshopTag> tagObjects = {};	// Get tag objects from workshop item
		m_Item.GetWorkshopItem().GetTags(tagObjects);

		foreach (WorkshopTag tag : tagObjects)
		{
			string image = SCR_WorkshopUiCommon.GetTagImage(tag.Name());
			if (!image.IsEmpty())
				AddTypeDisplay(image, UIConstants.ICONS_IMAGE_SET, UIConstants.ICONS_GLOW_IMAGE_SET);
		}
		
		// Image
		m_BackendImageComponent.SetWorkshopItemAndImage(m_Item, m_Item.GetThumbnail());

		// Dependencies and dependent
		array<ref SCR_WorkshopItem> dependentAddons = SCR_AddonManager.SelectItemsBasic(m_Item.GetDependentAddons(), EWorkshopItemQuery.OFFLINE);
		array<ref SCR_WorkshopItem> dependencies = m_Item.GetLatestDependencies();
		
		m_InfoWidgets.m_wDependenciesHorizontalLayout.SetVisible(!dependencies.IsEmpty());
		m_InfoWidgets.m_wDependentHorizontalLayout.SetVisible(!dependentAddons.IsEmpty());
		
		if (!dependencies.IsEmpty())
		{
			if (dependencies.Count() == 1)
				m_InfoWidgets.m_wDependencies.SetText("#AR-Workshop_Details_ModDependencies_Short_One_LC");
			else
				m_InfoWidgets.m_wDependencies.SetTextFormat("#AR-Workshop_Details_ModDependencies_Short_LC", dependencies.Count());
		}

		if (!dependentAddons.IsEmpty())
		{
			if (dependentAddons.Count() == 1)
				m_InfoWidgets.m_wDependent.SetText("#AR-Workshop_Details_DependentMods_Short_One_LC");
			else
				m_InfoWidgets.m_wDependent.SetTextFormat("#AR-Workshop_Details_DependentMods_Short_LC", dependentAddons.Count());
		}
		
		// Error message
		WorkshopItem item = m_Item.GetWorkshopItem();
		ERevisionAvailability revAvailability = SCR_AddonManager.ItemAvailability(item);
		bool localModError = revAvailability != SCR_ERevisionAvailability.ERA_AVAILABLE && revAvailability != SCR_ERevisionAvailability.ERA_UNKNOWN_AVAILABILITY;
		float saturation = UIConstants.ENABLED_WIDGET_SATURATION;
		
		m_CommonWidgets.m_WarningOverlayComponent.SetWarningVisible(m_Item.GetRestricted() || localModError);
		
		if (m_Item.GetRestricted())
			m_CommonWidgets.m_WarningOverlayComponent.SetWarning("#AR-Workshop_Dialog_Error_ModIsBlocked", "reportedByMe");
		else
			m_CommonWidgets.m_WarningOverlayComponent.SetWarning(SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorMessageVerbose(item), SCR_WorkshopUiCommon.GetRevisionAvailabilityErrorTexture(item));
		
		if (localModError)
			saturation = UIConstants.DISABLED_WIDGET_SATURATION;
		
		m_BackendImageComponent.SetImageSaturation(saturation);
	}	

	//------------------------------------------------------------------------------------------------
	override void HandlerDeattached(Widget w)
	{
		super.HandlerDeattached(w);
		
		if (m_Item)
			m_Item.m_OnChanged.Remove(Callback_OnChanged);
	}

	//------------------------------------------------------------------------------------------------
	protected void Callback_OnChanged()
	{
		UpdateAllWidgets();
	}
	
	// --- Public ----
	//------------------------------------------------------------------------------------------------
	void SetWorkshopItem(SCR_WorkshopItem item)
	{
		if (m_Item)
			m_Item.m_OnChanged.Remove(Callback_OnChanged);

		m_Item = item;

		if (m_Item)
			m_Item.m_OnChanged.Insert(Callback_OnChanged);

		UpdateAllWidgets();
	}

	//------------------------------------------------------------------------------------------------
	SCR_WorkshopItem GetItem()
	{
		return m_Item;
	}
}

//! Dialog to cancel downloads during to server joining
class SCR_ServerJoinDownloadsConfirmationDialog : SCR_ConfigurableDialogUi
{
	protected ref array<ref SCR_WorkshopItemActionDownload> m_aActions = {};
	protected ref array<SCR_DownloadManager_AddonDownloadLine> m_aLineCompents = {};

	protected static const string TAG_ALL = 		"server_download_all_cancel";
	protected static const string TAG_REQUIRED = 	"server_download_required";
	protected static const string TAG_UNRELATED = 	"server_download_unrelated_cancel";

	protected const ResourceName DOWNLOAD_LINE_LAYOUT = "{1C5D2CC10D7A1BC3}UI/layouts/Menus/ContentBrowser/DownloadManager/DownloadManager_AddonDownloadLineNonInteractive.layout";

	//------------------------------------------------------------------------------------------------
	static SCR_ServerJoinDownloadsConfirmationDialog Create(array<ref SCR_WorkshopItemActionDownload> actions, SCR_EJoinDownloadsConfirmationDialogType type)
	{
		string tag;

		switch (type)
		{
			case SCR_EJoinDownloadsConfirmationDialogType.ALL:
				tag = TAG_ALL;
				break;

			case SCR_EJoinDownloadsConfirmationDialogType.REQUIRED:
				tag = TAG_REQUIRED;
				break;

			case SCR_EJoinDownloadsConfirmationDialogType.UNRELATED:
				tag = TAG_UNRELATED;
				break;
		}

		return new SCR_ServerJoinDownloadsConfirmationDialog(tag, actions);
	}

	//------------------------------------------------------------------------------------------------
	void SCR_ServerJoinDownloadsConfirmationDialog(string tag, array<ref SCR_WorkshopItemActionDownload> actions)
	{
		m_aActions = actions;
		SCR_ConfigurableDialogUi.CreateFromPreset(SCR_WorkshopDialogs.DIALOGS_CONFIG, tag, this);
	}

	//------------------------------------------------------------------------------------------------
	override void OnMenuOpen(SCR_ConfigurableDialogUiPreset preset)
	{
		VerticalLayoutWidget layout = VerticalLayoutWidget.Cast(GetContentLayoutRoot().FindAnyWidget(SCR_WorkshopDialogs.WIDGET_LIST));

		// Create widgets for downloads
		foreach (SCR_WorkshopItemActionDownload action : m_aActions)
		{
			Widget w = GetGame().GetWorkspace().CreateWidgets(DOWNLOAD_LINE_LAYOUT, layout);
			SCR_DownloadManager_AddonDownloadLine comp = SCR_DownloadManager_AddonDownloadLine.Cast(w.FindHandler(SCR_DownloadManager_AddonDownloadLine));
			comp.InitForServerDownloadAction(action);

			m_aLineCompents.Insert(comp);
		}

		super.OnMenuOpen(preset);
	}
}
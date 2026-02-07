class WorldSaveManifest
{
	ref array<string> m_aFileNames;	//list of files to upload
	ref array<ref JsonApiStruct> m_aFiles;	//list of files to upload - WIP
	ref array<string> m_aDependencyIds;
	ref array<string> m_aScreenshots;
	bool m_bUnlisted = false;
	string m_sName;				
	string m_sSummary;
	string m_sDescription;
	string m_sPreview;			//name of the file with preview image - its full path has to be among other files in m_aFiles
	string m_sScenarioId;
}

class WorldSaveItem extends WorkshopItem
{
	/**
	\brief Request delete of this item from the backend storage
	*/
	proto native void DeleteOnline(BackendCallback callback);

	proto native void FillManifest(out notnull WorldSaveManifest manifest);
	
	proto native int CreatedAt();	//time in DateTimeUtcAsInt format
	
	proto native void DeleteOffline();
	
	proto native void Save(notnull WorldSaveManifest manifest);
	
	proto native WorldSaveItem UploadWorldSave(notnull WorldSaveManifest manifest, BackendCallback callback);
	
	static proto native WorldSaveItem CreateLocalWorldSave(notnull WorldSaveManifest manifest);
}

class WorldSaveApi extends DownloadableCatalogue
{
	[Obsolete("use WorkshopApi.GetCurrentSave")] proto native WorldSaveItem GetCurrentSave();
	
	[Obsolete("use WorkshopApi.SetCurrentSave")] proto native void SetCurrentSave(WorldSaveItem item);
	
	[Obsolete("use WorldSaveItem.UploadWorldSave")] proto native WorldSaveItem UploadWorldSave(notnull WorldSaveManifest manifest, BackendCallback callback, WorldSaveItem itemToUpdate);

	[Obsolete("use WorkshopApi.GetOfflineSaves")] proto native int GetOfflineItems(out notnull array<WorldSaveItem> items);

	[Obsolete("use WorkshopApi.FindItem")] proto native WorldSaveItem FindItem(string id);
	
	[Obsolete("use WorldSaveItem.CreateLocalWorldSave")] proto native WorldSaveItem CreateLocalWorldSave(notnull WorldSaveManifest manifest);
	
	/*!
	Get page content, returns current count of items on active page.
	\param items Array of Workshop Items
	*/
	[Obsolete("use WorkshopApi.GetPageItems")] proto native int GetPageItems( out array<WorldSaveItem> items );
}



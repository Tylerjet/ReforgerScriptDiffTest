class WorldSaveManifest
{
	ref array<string> m_aFileNames;	//list of files to upload
	ref array<ref JsonApiStruct> m_aFiles;	//list of files to upload
	string m_sName;				
	string m_sSummary;
	string m_sPreview;			//name of the file with preview image - its full path has to be among other files in m_aFiles
}

class WorldSaveItem extends DownloadableItem
{
	/**
	\brief Request delete of this item from the backend storage
	*/
	proto native void DeleteFromBackend(BackendCallback callback);

	proto native void FillManifest(out notnull WorldSaveManifest manifest);
	
	proto native int CreatedAt();	//time in DateTimeUtcAsInt format
	
	proto native void Save(notnull WorldSaveManifest manifest);
	proto native void DeleteLocalRevision();
	proto native void DeleteDownloadedRevision();
}

class WorldSaveApi extends DownloadableCatalogue
{
	proto native WorldSaveItem GetCurrentSave();
	
	proto native WorldSaveItem UploadWorldSave(notnull WorldSaveManifest manifest, BackendCallback callback, WorldSaveItem itemToUpdate);

	proto native int GetOfflineItems(out notnull array<WorldSaveItem> items);

	proto native WorldSaveItem FindItem(string id);
	
	proto native WorldSaveItem CreateLocalWorldSave(WorldSaveManifest manifest);
	
	/*!
	Get page content, returns current count of items on active page.
	\param items Array of Workshop Items
	*/
	proto native int GetPageItems( out array<WorldSaveItem> items );
	
	/*
	\brief Test API. Evailable only for internal builds.
	*/
	proto void FindOrCreateTestItem(BackendCallback callback, out WorldSaveItem item);
}



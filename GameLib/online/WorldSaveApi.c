class WorldSaveManifest
{
	ref array<string> m_aFiles;	//list of files to upload
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

	proto native bool LoadData(out notnull JsonApiStruct data, string name);
}

class WorldSaveApi extends DownloadableCatalogue
{
	proto native WorldSaveItem UploadWorldSave(WorldSaveManifest manifest, BackendCallback callback, WorldSaveItem itemToUpdate);

	proto native int GetOfflineItems(out notnull array<WorldSaveItem> items);

	proto native WorldSaveItem FindItem(string id);
	
	/**
	\brief Get page content, returns current count of items on active page
	\param item items Array of Workshop Items
	*/
	proto native int GetPageItems( out array<WorldSaveItem> items );
}



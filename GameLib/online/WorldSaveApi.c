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
	
	proto native void Save(notnull WorldSaveManifest manifest);
	
	proto native WorldSaveItem UploadWorldSave(notnull WorldSaveManifest manifest, BackendCallback callback);
	
	static proto native WorldSaveItem CreateLocalWorldSave(notnull WorldSaveManifest manifest);
	
	/**
	\brief Creates JPEG data from PixelRawData based on given parameters
	\param data Raw data of the image
	\param quality Between 1-100, the higher quality results in a bigger image
	\param channels How many channels to use (4 usually as RGBA)
	\param preview True if the screenshot should be uploaded as preview, false if it should be added to gallery instead
	\return 0 on failure, non-0 on success
	*/
	proto native int SaveJPEG(PixelRawData data, int width, int height, int quality, int channels, bool preview);
}

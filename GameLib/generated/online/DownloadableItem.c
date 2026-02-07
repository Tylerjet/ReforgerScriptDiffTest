/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class DownloadableItem: BaseWorkshopItem
{
	private void DownloadableItem();
	void ~DownloadableItem();

	/*!
	Request verification of files integrity. Sets flags of revision and each file accordingly.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void VerifyIntegrity(BackendCallback pCallback);
	/*!
	Request repair of local files integrity in case of corruption. Corrupted files are deleted and downloaded as new.
	\param pCallback  - Is script callback where you will receive result/error when request finishes
	*/
	proto external void RepairIntegrity(BackendCallback pCallback);
	//! Returns true if verification is currently in process
	proto external bool IsVerificationRunning();
	//! Returns progress of current verification process in range of <0.0. ... 1.0>
	proto external int GetVerificationProgress();
	//! Returns Id of item.
	proto external string Id();
	//! Returns Summary text of this item.
	proto external string Summary();
	//! Returns thumbnail image of this item.
	proto external BackendImage Thumbnail();
	/*!
	Request details about this item from Workshop.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void AskDetail(notnull BackendCallback pCallback);
	/*!
	Request list of all dependencies for this item on specific Revision.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	\param pRevision - Is Revision from which we want to request dependency list
	*/
	[Obsolete("Use Revision::LoadDependencies() instead!")]
	proto external void LoadDependencies(BackendCallback pCallback, notnull Revision pRevision);
	/*!
	Request list of all scenarios for this item on specific Revision.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	\param pRevision - Is Revision from which we want to request scenario list
	*/
	[Obsolete("Use Revision::LoadScenarios() instead!")]
	proto external void LoadScenarios(BackendCallback pCallback, notnull Revision pRevision);
	/*!
	Request changelog for this item on specific Revision
	\param pCallback - Is script callback where you will receive result/error when request finishes
	\param pRevision - Is Revision from which we want to request changelog
	*/
	[Obsolete("Use Revision::LoadChangelog() instead!")]
	proto external void LoadChangelog(BackendCallback pCallback, notnull Revision pRevision);
	/*!
	Request download of this item.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	\param pRevision - Is Revision which will be requested to download
	*/
	proto external void Download(BackendCallback pCallback, Revision pRevision);
	/*!
	Will pause download of this item.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void PauseDownload(BackendCallback pCallback);
	/*!
	Will resume download of this item.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void ResumeDownload(BackendCallback pCallback);
	/*!
	Will cancel download of this item. Data will be cleared.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void Cancel(BackendCallback pCallback);
	//! Will delete locally downloaded item
	proto external void DeleteLocally();
	//! Will delete progress from downloading of this item. Download must be first canceled.
	proto external void DeleteDownloadProgress();
	/*!
	Request to delete this item from the Workshop. Client must be owner of item for successful delete.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void DeleteOnline(BackendCallback pCallback);
	//! Returns true if latest Revision is downloaded.
	proto external bool HasLatestVersion();
	//! Returns latest Revision of this item.
	proto external Revision GetLatestRevision();
	//! Returns currently downloaded Revision of item.
	proto external Revision GetActiveRevision();
	//! Returns currently pending/downloading Revision
	proto external Revision GetPendingDownload();
	//! Returns currently downloading Revision (will return null if it is paused)
	proto external Revision GetDownloadingRevision();
	//! Returns pseudo Revision for local non-workshop item.
	proto external Revision GetLocalRevision();
	/*!
	Provides array of Revision history of X latest revisions.
	Limit of how many revisions is provided is specified by Workshop so it may differ.
	\param[out] revisions - array containing X latest revisions for this item.
	\returns int - count of revisions
	*/
	proto external int GetRevisions(out notnull array<Revision> revisions);
	//! Returns progress of current download in range of <0.0. ... 1.0>. Returns 1 if it is already downloaded
	proto external float GetProgress();
	//! Return progress of current processing (delta patching) in range of <0.0. ... 1.0>. Returns 1 if it is already downloaded
	proto external float GetProcessingProgress();
	//! Returns true if addon is currently processing delta patches from already downloaded version.
	proto external bool IsProcessing();
	//! Returns true if item is currently processing with Workshop. Is Downloading or Uploading.
	proto external bool IsProcessed();
	//! Returns current state flags
	proto external int GetStateFlags();
	//! Returns current persistent flags defined and set by scripts
	proto external int GetScriptFlag();
	//! Set persistent flag defined by script
	proto external void SetScriptFlag(int iFlag);
	//! Clears specific persistent flag defined by script
	proto external void ClearScriptFlag(int iFlag);
}

/*!
\}
*/

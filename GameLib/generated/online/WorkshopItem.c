/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

//! Workshop Item instance
sealed class WorkshopItem: DownloadableItem
{
	private void WorkshopItem();
	void ~WorkshopItem();

	//! Returns type of this item.
	proto external EWorkshopItemType GetType();
	/*!
	Returns name of Backend Environment of Workshop from which item originates.
	Can be used to validate origin of item to confirm that source and current environment are same.
	*/
	proto external string GetBackendEnv();
	//! Returns true if item is currently loaded.
	proto external bool IsLoaded();
	/*!
	Returns true if this item can be loaded.
	When item or dependencies are not Offline and Revision is equal to selected.
	*/
	proto external bool IsReadyToRun();
	/*!
	Sets enable state for this item - if it should be loaded or not.
	Cannot be changed if item is already loaded.
	*/
	proto external void Enable(bool enable);
	/*!
	Returns true if addon is set as enabled and would be loaded.
	Will always return true if it is already loaded.
	*/
	proto external bool IsEnabled();
	//! Returns access state flag -> EWorkshopItemAccessState
	proto external int GetAccessState();
	//! Returns true when item have been Unlisted on the Workshop.
	proto external bool IsUnlisted();
	//! Returns time since last played game with this addon
	proto external int GetTimeSinceLastPlay();
	//! Time in seconds since we downloaded this addon first time. Further updates do not affect this.
	proto external int GetTimeSinceFirstDownload();
	//! Resets the GetTimeSinceLastPlay time
	proto external void NotifyPlay();
	//! Returns true if item is subscribed.
	proto external bool IsSubscribed();
	//! Request to subscribe this item.
	proto external void Subscribe(BackendCallback callback);
	//! Request to unsubscribe this item.
	proto external void Unsubscribe(BackendCallback callback);
	//! Returns true if item is set as favorite
	proto external bool IsFavorite();
	/*!
	Request to set favorite state for this item.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	\param isFavorite - which state is desired
	*/
	proto external void SetFavorite(BackendCallback pCallback, bool isFavorite);
	//! Returns true if the current user is author of this item
	proto external bool IsAuthor();
	//! Returns objects containing details about author of this item.
	proto external WorkshopAuthor Author();
	//! Returns true if the current user is contributor of this item
	proto external bool IsContributor();
	/*!
	Provides array of contributors of this item.
	\param[out] outContributors - array of WorkshopAuthor containing details about contributors
	\returns int - count of contributors
	*/
	proto external int Contributors(out notnull array<WorkshopAuthor> outContributors);
	//! Returns type of license for this item
	proto external string License();
	//! Returns additional text to license for this item
	proto external string LicenseText();
	/*!
	Request creation of report to this item.
	\param eReport - type of report
	\param sDesc - user description message attached to the report
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void Report(EWorkshopReportType eReport, string sDesc, BackendCallback pCallback);
	/*!
	Request load of already existing report to this item.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void LoadReport(BackendCallback pCallback);
	/*!
	Request to cancel/delete your report of this item.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void CancelReport(BackendCallback pCallback);
	//! Returns type of report that was already submitted to this item.
	proto external EWorkshopReportType GetReportType();
	//! Returns description message provided by user to the report.
	proto external string GetReportDescription();
	/*!
	Request up vote or down vote rating for this item.
	\param bUpvote - up or down vote
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void Rate(bool bUpvote, BackendCallback pCallback);
	/*!
	Request reset of user vote for rating.
	\param pCallback - Is script callback where you will receive result/error when request finishes
	*/
	proto external void ResetRating (BackendCallback pCallback);
	//! Returns true if user voted in rating for this item.
	proto external bool IsRatingSet();
	//! Returns what vote user chose for this item.
	proto external bool MyRating();
	//! Returns count of how many votes item has in rating.
	proto external int RatingCount();
	//! Returns average rating of this item.
	proto external float AverageRating();
	/*!
	Provides array of tags on this item.
	\param items - array of tags
	\return int - count of items
	*/
	proto external int GetTags(out notnull array<WorkshopTag> items);
	//! Returns true if any of tags passed via array is present on item.
	proto external bool HasAnyTag(notnull array<WorkshopTag> tags);
	//! Returns gallery of images for this item.
	proto external int Gallery(out notnull array<BackendImage> images);
	//! Set the preferred thumbnail size (width) that is automatically downloaded when page is requested.
	static proto void SetThumbnailGridScale(int iWidth);
	//! Returns true if there is enough space on local storage to fit this item.
	proto external bool EnoughLocalSpace();
}

/*!
\}
*/

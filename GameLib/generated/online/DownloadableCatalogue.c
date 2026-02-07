/*
===========================================
Do not modify, this script is generated
===========================================
*/

/*!
\addtogroup online
\{
*/

sealed class DownloadableCatalogue: WorkshopCatalogue
{
	private void DownloadableCatalogue();
	private void ~DownloadableCatalogue();

	/*!
	\brief Destroy items.
	*/
	proto external void Cleanup();
	/*!
	\brief Scans for both local addons and world saves
	*/
	proto external void ScanOfflineItems();
	/*!
	*/
	proto external int GetBannedItems(out notnull array<string> items);
	/*!
	*/
	proto external int GetDownloads(out notnull array<DownloadableItem> items);
}

/*!
\}
*/

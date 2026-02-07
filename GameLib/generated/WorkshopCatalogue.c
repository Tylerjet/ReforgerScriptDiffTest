/*
===========================================
Do not modify, this script is generated
===========================================
*/

class WorkshopCatalogue
{
	proto external void RequestPage(BackendCallback pCallback, notnull PageParams pParams, bool bClearCache);
	//! \brief Get total item count on all pages
	proto external int GetTotalItemCount();
	//! \brief Get page count
	proto external int GetPageCount();
	//! \brief Set number of items per page
	proto external void SetPageItems(int iCount);
	//! \brief Get current page number
	proto external int GetPage();
	//! \brief Get item count on actual page
	proto external int GetPageItemCount();
}

// When enabled, the component will visualize debug information
//#define BACKEND_IMAGE_DEBUG

//! This abstract component handles loading of backend images and also manages the loading overlay while image is loading.
class SCR_BackendImageComponentBase : SCR_ScriptedWidgetComponent
{	
	// Attributes
	[Attribute("", UIWidgets.ResourceNamePicker, desc: "Image used as fallback for addon image", params: "edds")]
	protected ResourceName m_sFallbackImage;
	
	// Other
	protected ref BackendImage m_BackendImage;
	protected ref SCR_WorkshopItemCallback_DownloadImage m_DownloadImageCallback;

	protected bool m_bIsWaitingForWidgetInit;
	protected int m_iPreferredWidth;	
	
	// Debugging
	protected int m_iScreenWidth;
	
	protected ref ScriptInvokerVoid Event_OnImageSelected;
	
	//------------------------------------------------------------------------------------------------
	protected void InvokeEventOnImageSelected()
	{
		if (Event_OnImageSelected)
			Event_OnImageSelected.Invoke();
	}

	//------------------------------------------------------------------------------------------------
	ScriptInvokerVoid GetEventOnImageSelected()
	{
		if (!Event_OnImageSelected)
			Event_OnImageSelected = new ScriptInvokerVoid();

		return Event_OnImageSelected;
	}
	
	//----------------------------------------------------------------------------------
	//! image can be null
	void SetImage(BackendImage image)
	{
		bool showDefaultImage = true;
		if (image)
		{
			if (image.GetScale(0))
				showDefaultImage = false;
		}
		
		if (showDefaultImage)
		{
			ShowDefaultImage();
			m_iPreferredWidth = 0;
			#ifdef BACKEND_IMAGE_DEBUG
			ShowDebugText("Image unavailable\nShowing default image");
			#endif
			return;
		}
	
		m_BackendImage = image;
			
		CheckWidgetInitialized();
	}
	
	//----------------------------------------------------------------------------------
	protected void CheckWidgetInitialized()
	{
		Widget wImageSize = GetImageSizeWidget();
		if (!wImageSize) // Makes no sense
		{
			#ifdef BACKEND_IMAGE_DEBUG
			ShowDebugText("SCR_BackendImageComponent - IsWidgetReady() - no image size widget!");
			#endif
			return;
		}
			
		// Get widget size instantly, related variables might be needed for debug text
		float fWidth, fHeight;
		wImageSize.GetScreenSize(fWidth, fHeight);
		
		m_iScreenWidth = Math.Round(fWidth); // Store for debugging
		
		// Wait untill the parent widget has been initialized
		int preferredWidth = Math.Round(fWidth);
		if (preferredWidth == 0)
		{
			#ifdef BACKEND_IMAGE_WIDGET_DEBUG
			Debug.Error(string.Format("SCR_BackendImageComponent - IsWidgetReady() - widget has no size: %1 | %2", m_wRoot, m_wRoot.GetName()));
			#endif
			
			if (!m_bIsWaitingForWidgetInit)
			{				
				ShowLoadingImage(string.Empty);
				GetGame().GetCallqueue().CallLater(CheckWidgetInitialized, 0, true);
				
				m_bIsWaitingForWidgetInit = true;
			}
			
			return;
		}
		
		GetGame().GetCallqueue().Remove(CheckWidgetInitialized);
		
		m_iPreferredWidth = preferredWidth;
		
		m_bIsWaitingForWidgetInit = false;
		
		SetImage_Internal();
	}
	
	//----------------------------------------------------------------------------------
	protected void SetImage_Internal()
	{	
		BackendApi backend = GetGame().GetBackendApi();
		bool connected = backend.IsActive() && backend.IsAuthenticated();		
		if (connected)
		{
			// We are connected, download image
			ImageScale imageScale = m_BackendImage.GetScale(m_iPreferredWidth);
			
			// Discard the old callback
			// We must unsubscribe from events of previous callback, because it still might get called
			// When previous image is downloaded
			if (m_DownloadImageCallback)
			{
				m_DownloadImageCallback.m_OnError.Remove(Callback_DownloadImage_OnTimeoutError);
				m_DownloadImageCallback.m_OnTimeout.Remove(Callback_DownloadImage_OnTimeoutError);
				m_DownloadImageCallback.m_OnSuccess.Remove(Callback_DownloadImage_OnSuccess);
			}
			
			m_DownloadImageCallback = new SCR_WorkshopItemCallback_DownloadImage();
			m_DownloadImageCallback.m_OnError.Insert(Callback_DownloadImage_OnTimeoutError);
			m_DownloadImageCallback.m_OnTimeout.Insert(Callback_DownloadImage_OnTimeoutError);
			m_DownloadImageCallback.m_OnSuccess.Insert(Callback_DownloadImage_OnSuccess);
			
			m_DownloadImageCallback.m_Scale = imageScale;
			bool downloadStarted = imageScale.Download(m_DownloadImageCallback);
			if (downloadStarted)
			{
				// While we are downloading, try to get a smaller local scale
				ImageScale localImageScale = m_BackendImage.GetLocalScale(m_iPreferredWidth);
				if (localImageScale)
				{
					if (!localImageScale.Path().IsEmpty())
						ShowLoadingImage(localImageScale.Path());
					else
						ShowLoadingImage(string.Empty);
				}
				else
					ShowLoadingImage(string.Empty);
				
				#ifdef BACKEND_IMAGE_DEBUG
				ShowDebugText("Waiting");
				#endif
			}
			else
			{
				// Most likely we have already downloaded this
				string imagePath = imageScale.Path();
				if (!imagePath.IsEmpty())
				{
					#ifdef BACKEND_IMAGE_DEBUG
					string dbgtxt = string.Format("Local Image: %1x%2", imageScale.Width(), imageScale.Height());
					ShowDebugText(dbgtxt);
					#endif
					
					ShowImage(imagePath);
				}
				else
					ShowDefaultImage();
			}
		}
		else
		{
			// No connection, try to use local image
			ImageScale localImageScale = m_BackendImage.GetLocalScale(m_iPreferredWidth);
			if (localImageScale)
			{
				string imagePath = localImageScale.Path();
				if (!imagePath.IsEmpty())
					ShowImage(localImageScale.Path());
				else
					ShowDefaultImage();
			}
			else
				ShowDefaultImage();
		}
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_DownloadImage_OnTimeoutError(SCR_WorkshopItemCallback_DownloadImage callback)
	{		
		ShowDefaultImage();
	}
	
	//------------------------------------------------------------------------------------------------
	protected void Callback_DownloadImage_OnSuccess(SCR_WorkshopItemCallback_DownloadImage callback)
	{		
		if (callback.m_Scale)
		{
			string imgPath = callback.m_Scale.Path();

			if (!imgPath.IsEmpty())
			{
				ShowImage(imgPath);
				
				#ifdef BACKEND_IMAGE_DEBUG
				string dbgtxt = string.Format("Received %1x%2", callback.m_Scale.Width(), callback.m_Scale.Height());
				ShowDebugText(dbgtxt);
				#endif
			}
			else
				ShowDefaultImage();
		}
		else
			ShowDefaultImage();
	}
	
	// Implement handling of widgets here
	protected void ShowLoadingImage(string fallbackImage);	// Must visualize loading of image. When !fallbackImage.IsEmpty(), there is a local different size of this image.
	protected void ShowDefaultImage();						// Must display something when image is not available
	protected void ShowImage(string imagePath);				// Must display provided image
	protected Widget GetImageSizeWidget();					// Must return widget which wraps image widget. It will be used for size measurement.
	protected void ShowDebugText(string txt);				// Should implement visualization of debug text
}

//! This component implements basic image and loading overlay handling
//! m_sImageWidgetName must be an Image Widget
//! m_sLoadingOverlayName must have an SCR_LoadingOverlay component attached
class SCR_BackendImageComponent : SCR_BackendImageComponentBase
{
	[Attribute()]
	protected string m_sImageWidgetName;
	
	[Attribute("", UIWidgets.EditBox, "Widget from which we will be reading size of image widget. It can be detached from image widget, since its reported size can be different when wrapped into a scale widget.")]
	protected string m_sImageSizeWidgetName;
	
	[Attribute("1")]
	protected bool m_bShowLoadingImage;
	
	[Attribute("")]
	protected string m_sLoadingOverlayName;
	
	[Attribute("")]
	protected string m_sDebugTextName;
	
	// Widgets
	protected ImageWidget m_wImage;
	protected Widget m_wImageSize;
	protected SCR_LoadingOverlay m_LoadingOverlay;
	protected TextWidget m_wDebugText;
	
	//----------------------------------------------------------------------------------
	override void ShowLoadingImage(string fallbackImage)
	{
		if (!m_wImage)
			return;
		
		if (fallbackImage.IsEmpty())
			m_wImage.SetVisible(false);
		else
			ShowImage(fallbackImage);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(true);
	}
	
	//----------------------------------------------------------------------------------
	//! By default it hides image widget
	override void ShowDefaultImage()
	{
		if (!m_wImage)
			return;
		
		m_wImage.SetVisible(false);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);
		
		InvokeEventOnImageSelected();
	}
	
	//----------------------------------------------------------------------------------
	override void ShowImage(string imagePath)
	{
		if (!m_wImage)
			return;
		
		if (imagePath.IsEmpty())
		{
			m_wImage.LoadImageTexture(0, m_sFallbackImage, false, true);
			return;
		}
		
		m_wImage.LoadImageTexture(0, imagePath, false, true);
		
		int sx, sy;
		m_wImage.GetImageSize(0, sx, sy);
		m_wImage.SetSize(sx, sy);
		
		m_wImage.SetVisible(true);
		
		if (m_LoadingOverlay)
			m_LoadingOverlay.SetShown(false);
		
		InvokeEventOnImageSelected();
	}
	
	//----------------------------------------------------------------------------------
	override Widget GetImageSizeWidget()
	{
		return m_wImageSize;
	}
	
	//----------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		
		if (!m_sImageWidgetName.IsEmpty())
			m_wImage = ImageWidget.Cast(w.FindAnyWidget(m_sImageWidgetName));
		
		if (!m_sImageSizeWidgetName.IsEmpty())
			m_wImageSize = w.FindAnyWidget(m_sImageSizeWidgetName);
		
		if (!m_sLoadingOverlayName.IsEmpty())
		{
			Widget wLoading = w.FindAnyWidget(m_sLoadingOverlayName);
			if (wLoading)
			{
				m_LoadingOverlay = SCR_LoadingOverlay.Cast(wLoading.FindHandler(SCR_LoadingOverlay));
			}
		}
		
		if (!m_sDebugTextName.IsEmpty())
		{
			m_wDebugText = TextWidget.Cast(w.FindAnyWidget(m_sDebugTextName));
			
			
			if (m_wDebugText)
			{
				#ifdef BACKEND_IMAGE_DEBUG
					m_wDebugText.SetVisible(true);
					ShowDebugText("SCR_BackendImageComponent\nInitialized");
				#else
					m_wDebugText.SetVisible(false);
				#endif
			}
		}	
	}
	
	//----------------------------------------------------------------------------------
	override void ShowDebugText(string txt)
	{
		if (!m_wDebugText)
			return;
		
		string commotDebugData;
		
		if (m_iPreferredWidth != 0)
			commotDebugData = string.Format("Widget Screen Width: %1\nPrefered Width: %2\nScales in Backend:\n%3",
				m_iScreenWidth, m_iPreferredWidth, FormatAvailableScales());
		
		m_wDebugText.SetText(commotDebugData + "\n" + txt);
	}
	
	//----------------------------------------------------------------------------------
	protected string FormatAvailableScales()
	{
		if (!m_BackendImage)
			return string.Empty;
		
		array<ImageScale> scales = {};
		m_BackendImage.GetScales(scales);
		
		string str;
		int j = 0;
		for (int i = 0; i < scales.Count(); i++)
		{
			string s;
			
			if (scales[i])
				s = string.Format("%1x%2 ", scales[i].Width(), scales[i].Height());
			else
				s = "null ";
			
			j = j+1;
			if (j == 3)
			{
				s = s + "\n";
				j = 0;
			}
			
			str = str + s;
		}
		
		return str;
	}
	
	//----------------------------------------------------------------------------------
	void SetImageSaturation(float saturation)
	{
		if (m_wImage)
			m_wImage.SetSaturation(saturation);
	}
}
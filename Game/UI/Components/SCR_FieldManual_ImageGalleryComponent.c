enum EFieldManual_ImageGalleryType
{
	ICONS_VERTICAL,
	ICONS_LIST,
	GALLERY_HORIZONTAL,
	GALLERY_VERTICAL,
};

class SCR_FieldManual_ImageGalleryComponent : ScriptedWidgetComponent
{
	[Attribute(defvalue: SCR_Enum.GetDefault(EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL), uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(EFieldManual_ImageGalleryType))]
	protected EFieldManual_ImageGalleryType m_eType;

	[Attribute()]
	protected ref array<ref SCR_FieldManual_ImageGalleryLayoutInfo> m_aEnumLayoutPairs;

	[Attribute()]
	protected ref array<ref SCR_FieldManual_ImageData> m_aImagesData;

	[Attribute()]
	protected string m_sText;

	protected Widget m_wRoot;
	protected Widget m_wGallery;
	protected static const string S_IMAGES_PARENT_WIDGET_NAME = "Images";
	protected static const string S_IMAGE_WIDGET_NAME = "Image";
	protected static const string S_CAPTION_WIDGET_NAME = "Caption";
	protected static const string S_TEXT_WIDGET_NAME = "Text";

	//------------------------------------------------------------------------------------------------
	void SCR_FieldManual_ImageGalleryComponent()
	{
		if (!m_aEnumLayoutPairs)
		{
			m_aEnumLayoutPairs = {};
		}
		AddMissingEnumLayoutPairs();

		if (!m_aImagesData)
		{
			m_aImagesData = {};
		}
	}

	//------------------------------------------------------------------------------------------------
	override void HandlerAttached(Widget w)
	{
		super.HandlerAttached(w);
		m_wRoot = w;
		Rebuild();
	}

	//------------------------------------------------------------------------------------------------
	void Init(EFieldManual_ImageGalleryType type, string text, array<ref SCR_FieldManual_ImageData> imagesData)
	{
		m_eType = type;
		m_sText = text;
		if (imagesData)
			m_aImagesData = imagesData;
		Rebuild();
	}

	//------------------------------------------------------------------------------------------------
	void ClearImages()
	{
		m_aImagesData.Clear();
		Rebuild();
	}

	//------------------------------------------------------------------------------------------------
	void SetImages(array<ref SCR_FieldManual_ImageData> imagesData)
	{
		if (!imagesData || imagesData.IsEmpty())
			return;
		m_aImagesData = imagesData;
		Rebuild();
	}

	//------------------------------------------------------------------------------------------------
	void SetText(string text)
	{
		m_sText = text;

		if (!m_wGallery)
			return;

		TextWidget textWidget = TextWidget.Cast(m_wGallery.FindAnyWidget(S_TEXT_WIDGET_NAME));
		if (textWidget)
			textWidget.SetText(m_sText);
	}

	//------------------------------------------------------------------------------------------------
	void SetType(EFieldManual_ImageGalleryType type)
	{
		m_eType = type;
		Rebuild();
	}

	//------------------------------------------------------------------------------------------------
	protected void Rebuild()
	{
		if (!m_wRoot || !m_aEnumLayoutPairs)
			return;

		SCR_FieldManual_ImageGalleryLayoutInfo galleryInfo;
		foreach (SCR_FieldManual_ImageGalleryLayoutInfo layoutInfo : m_aEnumLayoutPairs)
		{
			if (layoutInfo && layoutInfo.m_eType == m_eType)
			{
				galleryInfo = layoutInfo;
				break;
			}
		}

		if (!galleryInfo || galleryInfo.m_GalleryLayout.IsEmpty() || galleryInfo.m_ImageLayout.IsEmpty())
			return;

		SCR_WidgetHelper.RemoveAllChildren(m_wRoot);

		m_wGallery = GetGame().GetWorkspace().CreateWidgets(galleryInfo.m_GalleryLayout, m_wRoot);
		if (!m_wGallery)
			return;

		TextWidget textWidget = TextWidget.Cast(m_wGallery.FindAnyWidget(S_TEXT_WIDGET_NAME));
		if (textWidget)
			textWidget.SetText(m_sText);

		Widget imagesParent = m_wGallery.FindAnyWidget(S_IMAGES_PARENT_WIDGET_NAME);
		if (!imagesParent)
			return;

		Widget image;
		foreach (SCR_FieldManual_ImageData imageData : m_aImagesData)
		{
			image = GetGame().GetWorkspace().CreateWidgets(galleryInfo.m_ImageLayout, imagesParent);
			if (!image)
				continue;

			SetImageData(image, imageData);
		}
	}

	//------------------------------------------------------------------------------------------------
	protected void SetImageData(Widget widget, SCR_FieldManual_ImageData imageData)
	{
		ImageWidget imageWidget = ImageWidget.Cast(SCR_WidgetHelper.GetWidgetOrChild(widget, S_IMAGE_WIDGET_NAME));
		if (imageWidget && !imageData.m_Image.IsEmpty())
		{
			if (imageData.m_sImageName.IsEmpty())
			{
				imageWidget.LoadImageTexture(0, imageData.m_Image);
			}
			else
			{
				imageWidget.LoadImageFromSet(0, imageData.m_Image, imageData.m_sImageName);
			}
			SCR_WidgetHelper.ResizeToImage(imageWidget);
		}

		TextWidget captionWidget = TextWidget.Cast(widget.FindAnyWidget(S_CAPTION_WIDGET_NAME));
		if (captionWidget)
		 	captionWidget.SetText(imageData.m_sCaption);
	}

	//------------------------------------------------------------------------------------------------
	protected void AddMissingEnumLayoutPairs()
	{
		set<EFieldManual_ImageGalleryType> foundEnums = new set<EFieldManual_ImageGalleryType>();
		foreach (SCR_FieldManual_ImageGalleryLayoutInfo pair : m_aEnumLayoutPairs)
		{
			foundEnums.Insert(pair.m_eType);
		}

		if (!foundEnums.Contains(EFieldManual_ImageGalleryType.ICONS_VERTICAL))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = EFieldManual_ImageGalleryType.ICONS_VERTICAL;
			info.m_GalleryLayout = "{912B608D2650F8B9}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_VERTICAL_ImageGallery.layout";
			info.m_ImageLayout = "{BF790BE149822E0F}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_VERTICAL_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}

		if (!foundEnums.Contains(EFieldManual_ImageGalleryType.ICONS_LIST))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = EFieldManual_ImageGalleryType.ICONS_LIST;
			info.m_GalleryLayout = "{B4DF69D7D553CFBC}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_LIST_ImageGallery.layout";
			info.m_ImageLayout = "{3194AFE59F1B877E}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/ICONS_LIST_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}

		if (!foundEnums.Contains(EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL;
			info.m_GalleryLayout = "{D8E4A2905F167C53}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_HORIZONTAL_ImageGallery.layout";
			info.m_ImageLayout = "{D08B8D5E3209D733}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_HORIZONTAL_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}

		if (!foundEnums.Contains(EFieldManual_ImageGalleryType.GALLERY_VERTICAL))
		{
			SCR_FieldManual_ImageGalleryLayoutInfo info = new SCR_FieldManual_ImageGalleryLayoutInfo();
			info.m_eType = EFieldManual_ImageGalleryType.GALLERY_VERTICAL;
			info.m_GalleryLayout = "{856A53946CA73978}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_VERTICAL_ImageGallery.layout";
			info.m_ImageLayout = "{D077D7EA510D2FC5}UI/layouts/Menus/FieldManual/Pieces/ImageGallery/GALLERY_VERTICAL_Image.layout";
			m_aEnumLayoutPairs.Insert(info);
		}
	}
};

[BaseContainerProps(), SCR_BaseContainerCustomTitleEnum(enumType: EFieldManual_ImageGalleryType, propertyName: "m_eType", format: "Image Gallery %1")]
class SCR_FieldManual_ImageGalleryLayoutInfo
{
	[Attribute(defvalue: SCR_Enum.GetDefault(EFieldManual_ImageGalleryType.GALLERY_HORIZONTAL), uiwidget: UIWidgets.ComboBox, enums: SCR_Enum.GetList(EFieldManual_ImageGalleryType))]
	EFieldManual_ImageGalleryType m_eType;

	[Attribute()]
	ResourceName m_GalleryLayout;

	[Attribute()]
	ResourceName m_ImageLayout;
};

enum MetaEddsConversion
{
	None, // CHC_NONE
	DXTCompression, // CHC_DXT
	RedHQCompression, // CHC_BC4
	RedGreenHQCompression, // CHC_BC5
	RedGreen, // CHC_RG
	Alpha8, // CHC_A8
	Red, // CHC_R
	HDRCompression, // CHC_BC6H
	ColorHQCompression, // CHC_BC7
}

enum MetaEddsColorSpaceConversion
{
	None,
	ToSrgb,
	ToLinear,
}

class TextureTypeProperty
{
	string m_Name;
	typename m_ValType;
	int m_Val;
	ref array<int> m_OtherVariants;
}

class PlatformConfig
{
	string m_Platform;
	string m_ConfigFile;
}

class TextureType
{
	// Take a look at typename.EnumToString and store typename variables for enums.
	string m_PostFix;
	ref array<ref TextureTypeProperty> m_Properties = new array<ref TextureTypeProperty>;
	ref array<ref PlatformConfig> m_PlatformConfigs = new array<ref PlatformConfig>;

	void TextureType(array<ref TextureType> container, string name)
	{
		m_PostFix = name;
		m_PostFix.ToLower();
		container.Insert(this);
	}

	void Insert(string name, typename valType, int val, array<int> otherVariants = null)
	{
		auto prop = new TextureTypeProperty;
		prop.m_Name = name;
		prop.m_ValType = valType;
		prop.m_Val = val;
		prop.m_OtherVariants = otherVariants;
		m_Properties.Insert(prop);
	}

	void AddBaseConfig(string platform, string configFile)
	{
		PlatformConfig config = new PlatformConfig;
		config.m_Platform = platform;
		config.m_ConfigFile = configFile;
		m_PlatformConfigs.Insert(config);
	}

	string GetBaseConfig(string platform)
	{
		foreach (PlatformConfig config : m_PlatformConfigs)
		{
			if(config.m_Platform == platform)
				return config.m_ConfigFile;
		}

		return "";
	}

	void CopyFrom(TextureType parent)
	{
		foreach (TextureTypeProperty property : parent.m_Properties)
		{
			m_Properties.Insert(property);
		}

		foreach (PlatformConfig config : parent.m_PlatformConfigs)
		{
			m_PlatformConfigs.Insert(config);
		}
	}

	bool IsType(string path)
	{
		path.ToLower();
		return path.Contains(m_PostFix);
	}

	bool TestPostFix(string resource)
	{
		int resourceLength = resource.Length();
		int postFixLength = m_PostFix.Length();
		if (resourceLength < postFixLength)
			return false;

		resource.ToLower();
		int lastIndex = resource.LastIndexOf(m_PostFix);
		return lastIndex == (resourceLength - postFixLength);
	}

	// Property names
	static const string Conversion = "Conversion";
	static const string ColorSpace = "ColorSpace";
	static const string GenerateMips = "GenerateMips";
	static const string GenerateCubemap = "GenerateCubemap";

	static void RegisterTypes(array<ref TextureType> container)
	{
		//--------------------------------------------------------------------
		//color maps -> to sRGB
		ref TextureType COType = new TextureType(container, "_CO.", );
		COType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		COType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToSrgb);
		COType.AddBaseConfig("PC", "{EAB5DE3219F9CBA8}configs/ResourceTypes/PC/TextureColorMap.conf");
		COType.AddBaseConfig("XBOX_ONE", "{91D862F89991BFBE}configs/ResourceTypes/XBOX_ONE/TextureColorMap.conf");
		COType.AddBaseConfig("XBOX_SERIES", "{5FEAED1642ECE679}configs/ResourceTypes/XBOX_SERIES/TextureColorMap.conf");
		COType.AddBaseConfig("PS4", "{12273E1A0928F0C4}configs/ResourceTypes/PS4/TextureColorMap.conf");
		COType.AddBaseConfig("HEADLESS", "{BEAF5CD0C438676E}configs/ResourceTypes/HEADLESS/TextureColorMap.conf");

		ref TextureType BCType = new TextureType(container, "_BC.");
		BCType.CopyFrom(COType);

		ref TextureType BCRType = new TextureType(container, "_BCR.");
		BCRType.CopyFrom(COType);

		ref TextureType BCHType = new TextureType(container, "_BCH.");
		BCHType.CopyFrom(COType);

		ref TextureType CAType = new TextureType(container, "_CA.");
		CAType.CopyFrom(COType);

		ref TextureType BCAType = new TextureType(container, "_BCA.");
		BCAType.CopyFrom(COType);

		ref TextureType MCType = new TextureType(container, "_MC.");
		MCType.CopyFrom(COType);

		ref TextureType MCAType = new TextureType(container, "_MCA.");
		MCAType.CopyFrom(COType);

		ref TextureType MLODType = new TextureType(container, "_MLOD.");
		MLODType.CopyFrom(COType);

		//--------------------------------------------------------------------
		//EM maps
		ref TextureType EMType = new TextureType(container, "_EM.");
		EMType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression, {MetaEddsConversion.DXTCompression});
		EMType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToSrgb);
		EMType.AddBaseConfig("PC", "{EAB5DE3219F9CBA8}configs/ResourceTypes/PC/TextureColorMap.conf");
		EMType.AddBaseConfig("XBOX_ONE", "{91D862F89991BFBE}configs/ResourceTypes/XBOX_ONE/TextureColorMap.conf");
		EMType.AddBaseConfig("XBOX_SERIES", "{5FEAED1642ECE679}configs/ResourceTypes/XBOX_SERIES/TextureColorMap.conf");
		EMType.AddBaseConfig("PS4", "{12273E1A0928F0C4}configs/ResourceTypes/PS4/TextureColorMap.conf");
		EMType.AddBaseConfig("HEADLESS", "{BEAF5CD0C438676E}configs/ResourceTypes/HEADLESS/TextureColorMap.conf");

		//--------------------------------------------------------------------
		//MCR maps - rgb is linear overlay modificator of albedo
		ref TextureType MCRType = new TextureType(container, "_MCR.");
		MCRType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		MCRType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToLinear, {MetaEddsColorSpaceConversion.ToSrgb});
		MCRType.AddBaseConfig("PC", "{A6CC0A2F9DB86CBE}configs/ResourceTypes/PC/TextureMCRMap.conf");
		MCRType.AddBaseConfig("XBOX_ONE", "{224B78299A038E4A}configs/ResourceTypes/XBOX_ONE/TextureMCRMap.conf");
		MCRType.AddBaseConfig("XBOX_SERIES", "{B5FA7506CF384E79}configs/ResourceTypes/XBOX_SERIES/TextureMCRMap.conf");
		MCRType.AddBaseConfig("PS4", "{E7A6FCCE55593073}configs/ResourceTypes/PS4/TextureMCRMap.conf");
		MCRType.AddBaseConfig("HEADLESS", "{D49C34F7354B6F47}configs/ResourceTypes/HEADLESS/TextureMCRMap.conf");

		//--------------------------------------------------------------------
		//pure normal maps
		ref TextureType NOType = new TextureType(container, "_NO.");
		NOType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);
		NOType.AddBaseConfig("PC", "{CEA87F769FD618D2}configs/ResourceTypes/PC/TextureNormalMap.conf");
		NOType.AddBaseConfig("XBOX_ONE", "{5B84FAB5D39FB8AA}configs/ResourceTypes/XBOX_ONE/TextureNormalMap.conf");
		NOType.AddBaseConfig("XBOX_SERIES", "{1B409AA3788B720F}configs/ResourceTypes/XBOX_SERIES/TextureNormalMap.conf");
		NOType.AddBaseConfig("PS4", "{9533DFD9EDDCEDD2}configs/ResourceTypes/PS4/TextureNormalMap.conf");
		NOType.AddBaseConfig("HEADLESS", "{FC84ABE4B37D0DA0}configs/ResourceTypes/HEADLESS/TextureNormalMap.conf");

		ref TextureType NType = new TextureType(container, "_N.");
		NType.CopyFrom(NOType);

		ref TextureType NOHQType = new TextureType(container, "_NOHQ.");
		NOHQType.CopyFrom(NOType);

		// Terrain textures are caught by this and they would require swizzle to work.
		// ref TextureType normalType = new TextureType(container, "_normal.");
		// normalType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);

		//--------------------------------------------------------------------
		//N types and packing with other sources
		ref TextureType NMOType = new TextureType(container, "_NMO.");
		NMOType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		NMOType.AddBaseConfig("PC", "{A968DA7F9A1E3A3E}configs/ResourceTypes/PC/TextureNType.conf");
		NMOType.AddBaseConfig("XBOX_ONE", "{7F6A4D372443A88D}configs/ResourceTypes/XBOX_ONE/TextureNType.conf");
		NMOType.AddBaseConfig("XBOX_SERIES", "{065D289C7FF8B20D}configs/ResourceTypes/XBOX_SERIES/TextureNType.conf");
		NMOType.AddBaseConfig("PS4", "{29DF4A6CBBABE916}configs/ResourceTypes/PS4/TextureNType.conf");
		NMOType.AddBaseConfig("HEADLESS", "{AFD658E4D0EB5FBC}configs/ResourceTypes/HEADLESS/TextureNType.conf");

		ref TextureType NHOType = new TextureType(container, "_NHO.");
		NHOType.CopyFrom(NMOType);

		ref TextureType NTCType = new TextureType(container, "_NTC.");
		NTCType.CopyFrom(NMOType);

		ref TextureType NTOType = new TextureType(container, "_NTO.");
		NTOType.CopyFrom(NMOType);

		//--------------------------------------------------------------------
		ref TextureType VFXType = new TextureType(container, "_VFX.");
		VFXType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		VFXType.AddBaseConfig("PC", "{4EC6C20A754D338D}configs/ResourceTypes/PC/TextureEffect.conf");
		VFXType.AddBaseConfig("XBOX_ONE", "{CA41B00C72F6D179}configs/ResourceTypes/XBOX_ONE/TextureEffect.conf");
		VFXType.AddBaseConfig("XBOX_SERIES", "{5DF0BD2327CD114A}configs/ResourceTypes/XBOX_SERIES/TextureEffect.conf");
		VFXType.AddBaseConfig("PS4", "{0FAC34EBBDAC6F40}configs/ResourceTypes/PS4/TextureEffect.conf");
		VFXType.AddBaseConfig("HEADLESS", "{3C96FCD2DDBE3074}configs/ResourceTypes/HEADLESS/TextureEffect.conf");

		//--------------------------------------------------------------------
		//various masks
		ref TextureType MASKQType = new TextureType(container, "_MASK.");
		MASKQType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		MASKQType.AddBaseConfig("PC", "{2231C67ACC4BB0AC}configs/ResourceTypes/PC/TextureMask.conf");
		MASKQType.AddBaseConfig("XBOX_ONE", "{3B78A666261DD8B4}configs/ResourceTypes/XBOX_ONE/TextureMask.conf");
		MASKQType.AddBaseConfig("XBOX_SERIES", "{9A79E1F659B378E7}configs/ResourceTypes/XBOX_SERIES/TextureMask.conf");
		MASKQType.AddBaseConfig("PS4", "{5CB64ED6EEBDE368}configs/ResourceTypes/PS4/TextureMask.conf");
		MASKQType.AddBaseConfig("HEADLESS", "{0B025DFAF55852CD}configs/ResourceTypes/HEADLESS/TextureMask.conf");

		ref TextureType MASK1QType = new TextureType(container, "_MASK1.");
		MASK1QType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedHQCompression);
		MASK1QType.AddBaseConfig("PC", "{51097E750D3FCE37}configs/ResourceTypes/PC/TextureMask1.conf");
		MASK1QType.AddBaseConfig("XBOX_ONE", "{870BE93DB3625C84}configs/ResourceTypes/XBOX_ONE/TextureMask1.conf");
		MASK1QType.AddBaseConfig("XBOX_SERIES", "{FE3C8C96E8D94604}configs/ResourceTypes/XBOX_SERIES/TextureMask1.conf");
		MASK1QType.AddBaseConfig("PS4", "{D1BEEE662C8A1D1F}configs/ResourceTypes/PS4/TextureMask1.conf");
		MASK1QType.AddBaseConfig("HEADLESS", "{57B7FCEE47CAABB5}configs/ResourceTypes/HEADLESS/TextureMask1.conf");

		ref TextureType MASK2QType = new TextureType(container, "_MASK2.");
		MASK2QType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);
		MASK2QType.AddBaseConfig("PC", "{0FE15736B2B24038}configs/ResourceTypes/PC/TextureMask2.conf");
		MASK2QType.AddBaseConfig("XBOX_ONE", "{D9E3C07E0CEFD28B}configs/ResourceTypes/XBOX_ONE/TextureMask2.conf");
		MASK2QType.AddBaseConfig("XBOX_SERIES", "{A0D4A5D55754C80B}configs/ResourceTypes/XBOX_SERIES/TextureMask2.conf");
		MASK2QType.AddBaseConfig("PS4", "{8F56C72593079310}configs/ResourceTypes/PS4/TextureMask2.conf");
		MASK2QType.AddBaseConfig("HEADLESS", "{095FD5ADF84725BA}configs/ResourceTypes/HEADLESS/TextureMask2.conf");

		
		//camo mask
		ref TextureType CRMType = new TextureType(container, "_CRM.");
		CRMType.CopyFrom(MASKQType);
		
		//clutter mask
		ref TextureType CMASKType = new TextureType(container, "_CMASK.");
		CMASKType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedGreenHQCompression);
		CMASKType.Insert(GenerateMips, bool, false);
		CMASKType.AddBaseConfig("PC", "{6733F446131BB044}configs/ResourceTypes/PC/TextureCMask.conf");
		CMASKType.AddBaseConfig("XBOX_ONE", "{B131630EAD4622F8}configs/ResourceTypes/XBOX_ONE/TextureCMask.conf");
		CMASKType.AddBaseConfig("XBOX_SERIES", "{C80606A5F6FD3877}configs/ResourceTypes/XBOX_SERIES/TextureCMask.conf");
		CMASKType.AddBaseConfig("PS4", "{E784645532AE636D}configs/ResourceTypes/PS4/TextureCMask.conf");
		CMASKType.AddBaseConfig("HEADLESS", "{618D76DD59EED5C6}configs/ResourceTypes/HEADLESS/TextureCMask.conf");
	
		
		//--------------------------------------------------------------------
		//one channel
		ref TextureType AType = new TextureType(container, "_A.");
		AType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.RedHQCompression);
		AType.AddBaseConfig("PC", "{6B149ECE3A72EF45}configs/ResourceTypes/PC/TextureOneChannel.conf");
		AType.AddBaseConfig("XBOX_ONE", "{DC95471339A9BC2B}configs/ResourceTypes/XBOX_ONE/TextureOneChannel.conf");
		AType.AddBaseConfig("XBOX_SERIES", "{C2F053ADC70F3804}configs/ResourceTypes/XBOX_SERIES/TextureOneChannel.conf");
		AType.AddBaseConfig("PS4", "{19FBA5E0ACB1C98E}configs/ResourceTypes/PS4/TextureOneChannel.conf");
		AType.AddBaseConfig("HEADLESS", "{6866463573917E52}configs/ResourceTypes/HEADLESS/TextureOneChannel.conf");

		ref TextureType HType = new TextureType(container, "_H.");
		HType.CopyFrom(AType);

		ref TextureType OType = new TextureType(container, "_O.");
		OType.CopyFrom(AType);

		//--------------------------------------------------------------------
		ref TextureType layerType = new TextureType(container, "_layer.");
		layerType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.None);
		layerType.AddBaseConfig("PC", "{A59F335F96C4442F}configs/ResourceTypes/PC/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("XBOX_ONE", "{9843D16A29542D5A}configs/ResourceTypes/XBOX_ONE/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("XBOX_SERIES", "{2071F9D39E062268}configs/ResourceTypes/XBOX_SERIES/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("PS4", "{4C431036F57D4D58}configs/ResourceTypes/PS4/TextureTerrainLayer.conf");
		layerType.AddBaseConfig("HEADLESS", "{3B38AD285230D19F}configs/ResourceTypes/HEADLESS/TextureTerrainLayer.conf");

		ref TextureType supertextureType = new TextureType(container, "_supertexture.");
		supertextureType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.ColorHQCompression);
		supertextureType.Insert(ColorSpace, MetaEddsColorSpaceConversion, MetaEddsColorSpaceConversion.ToSrgb);
		supertextureType.AddBaseConfig("PC", "{CBA1266690ABD336}configs/ResourceTypes/PC/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("XBOX_ONE", "{F67DC4532F3BBA43}configs/ResourceTypes/XBOX_ONE/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("XBOX_SERIES", "{4E4FECEA9869B571}configs/ResourceTypes/XBOX_SERIES/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("PS4", "{227D050FF312DA41}configs/ResourceTypes/PS4/TextureTerrainSuper.conf");
		supertextureType.AddBaseConfig("HEADLESS", "{5506B811545F4686}configs/ResourceTypes/HEADLESS/TextureTerrainSuper.conf");

		ref TextureType normaltextureType = new TextureType(container, "_normal.");
		normaltextureType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.DXTCompression);
		normaltextureType.AddBaseConfig("PC", "{835E083C88C3D9C3}configs/ResourceTypes/PC/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("XBOX_ONE", "{6C5EFFA712A52100}configs/ResourceTypes/XBOX_ONE/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("XBOX_SERIES", "{90ADE65A903F3042}configs/ResourceTypes/XBOX_SERIES/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("PS4", "{6F6E5A1432381FD2}configs/ResourceTypes/PS4/TextureTerrainNormal.conf");
		normaltextureType.AddBaseConfig("HEADLESS", "{EAB38BDFD096C0A6}configs/ResourceTypes/HEADLESS/TextureTerrainNormal.conf");

		// UI texture atlas
		ref TextureType UITextureAtlasType = new TextureType(container, "_atlas.");
		UITextureAtlasType.Insert(GenerateMips, bool, false);
		UITextureAtlasType.AddBaseConfig("PC", "{1D692833852EC72A}configs/ResourceTypes/PC/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("XBOX_ONE", "{96767353535A4256}configs/ResourceTypes/XBOX_ONE/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("XBOX_SERIES", "{8A3A5BA26FD3DE70}configs/ResourceTypes/XBOX_SERIES/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("PS4", "{962DE8F3FC2848A3}configs/ResourceTypes/PS4/TextureUIAtlas.conf");
		UITextureAtlasType.AddBaseConfig("HEADLESS", "{15892FB1C3E30D2C}configs/ResourceTypes/HEADLESS/TextureUIAtlas.conf");

		// UI textures
		ref TextureType uiType = new TextureType(container, "_ui.");
		uiType.AddBaseConfig("PC", "{3549C665CAA9A8EA}configs/ResourceTypes/PC/TextureUI.conf");
		uiType.AddBaseConfig("XBOX_ONE", "{5E6D8E2B87D36B34}configs/ResourceTypes/XBOX_ONE/TextureUI.conf");
		uiType.AddBaseConfig("XBOX_SERIES", "{03194CD39583675A}configs/ResourceTypes/XBOX_SERIES/TextureUI.conf");
		uiType.AddBaseConfig("PS4", "{A752014F057BCE60}configs/ResourceTypes/PS4/TextureUI.conf");
		uiType.AddBaseConfig("HEADLESS", "{E604D15366D985DF}configs/ResourceTypes/HEADLESS/TextureUI.conf");
		
		// UI textures uncompressed
		ref TextureType uiTypeUncompressed = new TextureType(container, "_uiuc.");
		uiTypeUncompressed.AddBaseConfig("PC", "{D256EAFDC1226362}configs/ResourceTypes/PC/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("XBOX_ONE", "{712BC6681ADE741A}configs/ResourceTypes/XBOX_ONE/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("XBOX_SERIES", "{B613AECC9D415504}configs/ResourceTypes/XBOX_SERIES/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("PS4", "{9A726D592B629239}configs/ResourceTypes/PS4/TextureUIUncompressed.conf");
		uiTypeUncompressed.AddBaseConfig("HEADLESS", "{A677FEF8099B272E}configs/ResourceTypes/HEADLESS/TextureUIUncompressed.conf");
		
		// Env. map (Generate env map from panorama, HDR compression enabled by default)
		ref TextureType environmentMapType = new TextureType(container, "_env.");
		environmentMapType.AddBaseConfig("PC", "{009B945B41B128FC}configs/ResourceTypes/PC/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("XBOX_ONE", "{841CE65D460ACA08}configs/ResourceTypes/XBOX_ONE/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("XBOX_SERIES", "{13ADEB7213310A3B}configs/ResourceTypes/XBOX_SERIES/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("PS4", "{41F162BA89507431}configs/ResourceTypes/PS4/TextureEnvMap.conf");
		environmentMapType.AddBaseConfig("HEADLESS", "{72CBAA83E9422B05}configs/ResourceTypes/HEADLESS/TextureEnvMap.conf");

		//Lut texture
		ref TextureType lutType = new TextureType(container, "_lut.");
		lutType.AddBaseConfig("PC", "{FDA8724761A5E880}configs/ResourceTypes/PC/TextureLut.conf");
		lutType.AddBaseConfig("XBOX_ONE", "{9AE8215636729E7A}configs/ResourceTypes/XBOX_ONE/TextureLut.conf");
		lutType.AddBaseConfig("XBOX_SERIES", "{CBB7E88AA7DE5748}configs/ResourceTypes/XBOX_SERIES/TextureLut.conf");
		lutType.AddBaseConfig("PS4", "{F34906600DC4FE84}configs/ResourceTypes/PS4/TextureLut.conf");
		lutType.AddBaseConfig("HEADLESS", "{3D6CF9FDEC095638}configs/ResourceTypes/HEADLESS/TextureLut.conf");
		
		//this is special type which will be assigned when no one from normal types above matches
		ref TextureType unspecifiedType = new TextureType(container, "");
		unspecifiedType.Insert(Conversion, MetaEddsConversion, MetaEddsConversion.DXTCompression);
		unspecifiedType.AddBaseConfig("PC", "{DC555BD399D92412}configs/ResourceTypes/PC/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("XBOX_ONE", "{8F13AE697AE60784}configs/ResourceTypes/XBOX_ONE/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("XBOX_SERIES", "{D28E01700D90F52C}configs/ResourceTypes/XBOX_SERIES/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("PS4", "{C6CD3D8752652D2A}configs/ResourceTypes/PS4/TextureUnspecified.conf");
		unspecifiedType.AddBaseConfig("HEADLESS", "{699C82A6807668A7}configs/ResourceTypes/HEADLESS/TextureUnspecified.conf");

		// new TextureType(container, "_ads.");
		// new TextureType(container, "_OcclusionRoughnessMetallic.");
		// new TextureType(container, "_smdi.");
		// new TextureType(container, "_dtsmdi.");
		// new TextureType(container, "_nrm.");
		// new TextureType(container, "_as.");
	}
}

//----------------------------------------------------------------------------------------------
bool FixTextureMetaFile(MetaFile meta, string absFileName, array<ref TextureType> textureTypes)
{
	BaseContainerList configurations = meta.GetObjectArray("Configurations");
	if(!configurations)
		return false;

	bool anyChangeInMetaFile = false;

	for(int c = 0; c < configurations.Count(); c++)
	{
		BaseContainer cfg = configurations.Get(c);

		string cfgName = cfg.GetName();

		TextureType unspecifiedType = null;
		TextureType typeToAssign = null;

		foreach (TextureType texType : textureTypes)
		{
			if(texType.IsType(absFileName))
			{
				typeToAssign = texType;
				break;
			}
			else if(texType.m_PostFix == "")
			{
				unspecifiedType = texType
			}
		}

		if(!typeToAssign)
			typeToAssign = unspecifiedType;

		ResourceName ancestor = typeToAssign.GetBaseConfig(cfgName);
		if(ancestor != "")
		{
			cfg.SetAncestor(ancestor);
			anyChangeInMetaFile = true;
		}

//		cfg.ClearVariable(TextureType.Conversion);
//		cfg.ClearVariable(TextureType.ColorSpace);
	}

	return anyChangeInMetaFile;
}

[WorkbenchPluginAttribute("Texture Import", "Texture Import Helper", "", "", {"ResourceManager"},"",0xf574)]
class TextureImportPlugin: ResourceManagerPlugin
{
	[Attribute("true", UIWidgets.CheckBox)]
	bool Enabled;

	ref array<ref TextureType> m_TextureTypes = new array<ref TextureType>;

	//--------------------------------------------------------------------
	void TextureImportPlugin()
	{
		TextureType.RegisterTypes(m_TextureTypes);
	}

	//--------------------------------------------------------------------
	bool IsImage(string className)
	{
		return
			className == "PNGResourceClass" ||
			className == "DDSResourceClass" ||
			className == "TGAResourceClass" ||
			className == "TIFFResourceClass" ||
			className == "PNGResourceClass" ||
			className == "HDRResourceClass" ||
			className == "PAAResourceClass" ||
			className == "JPGResourceClass";
	}

	//--------------------------------------------------------------------
	override void OnRegisterResource(string absFileName, BaseContainer metaFile)
	{
		BaseContainer conf = metaFile.GetObjectArray("Configurations")[0];
		if (!Enabled || !IsImage(conf.GetClassName()))
			return;

		FixTextureMetaFile(metaFile, absFileName, m_TextureTypes);
/*
		foreach (TextureType texType : m_TextureTypes)
		{
			if (!texType.IsType(absFileName))
				continue;

			Print("TextureImportPlugin: registering '" + texType.m_PostFix + "' texture '" + absFileName +"', " + texType.m_Properties.Count() + " properties", LogLevel.VERBOSE);
			foreach (TextureTypeProperty prop : texType.m_Properties)
			{
				conf.Set(prop.m_Name, prop.m_Val);
			}
		}
*/
	}
	
	override void OnRenameResource(string absFileNameOld, string absFileNameNew, BaseContainer metaFile)
	{
		BaseContainer conf = metaFile.GetObjectArray("Configurations")[0];
		if (!Enabled || !IsImage(conf.GetClassName()))
			return;

		FixTextureMetaFile(metaFile, absFileNameNew, m_TextureTypes);
	}

	override void Configure()
	{
		Workbench.ScriptDialog("Configure Texture Import Plugin", "", this);
	}

	[ButtonAttribute("OK")]
	void OkButton() {}
}

[WorkbenchPluginAttribute("Batch texture processor", "Perform simple checks and fixes on many textures", "", "", {"ResourceManager"},"",0xf1c5)]
class BatchTextureProcessorPlugin: WorkbenchPlugin
{
	[Attribute("", UIWidgets.EditBox, "Check only textures whose path starts with given filter string.")]
	string PathStartsWith;

	[Attribute("", UIWidgets.EditBox, "Check only textures whose path contains given filter string.")]
	string PathContains;

	[Attribute("", UIWidgets.EditBox, "Check only textures whose path ends with given filter string.")]
	string PathEndsWith;

	[Attribute("false", UIWidgets.CheckBox, "Report unrecognized postfix")]
	bool ReportUnrecognizedPostfix;

	[Attribute("true", UIWidgets.CheckBox, "Report missing meta-file")]
	bool ReportMissingMetaFile;

	[Attribute("true", UIWidgets.CheckBox, "Report missing configurations")]
	bool ReportMissingConfigurations;

	[Attribute("true", UIWidgets.CheckBox, "Report wrong resource class")]
	bool ReportWrongResourceClass;

	[Attribute("true", UIWidgets.CheckBox, "Report wrong property values")]
	bool ReportWrongPropertyValues;

	[Attribute("100", UIWidgets.SpinBox, "Number of issues that will be reported. Set 0 to report all issues.", "0 10000 1")]
	int MaxReportCount;

	[Attribute("false", UIWidgets.CheckBox, "Fix import properties with wrong values and reimport corresponding textures.")]
	bool FixProperties;

	[Attribute("false", UIWidgets.CheckBox, "Fix inheritance of configurations")]
	bool FixMetaFile;

	[Attribute("10", UIWidgets.SpinBox, "Number of textures to fix and reimport. Can be used to shorten processing time. Set 0 to remove any restrictions.", "0 10000 1")]
	int MaxFixCount;

	ref array<ref TextureType> m_TextureTypes = new array<ref TextureType>;
	ref array<string> m_Resources = new array<string>;

	//----------------------------------------------------------------------------------------------
	void BatchTextureProcessorPlugin()
	{
		TextureType.RegisterTypes(m_TextureTypes);
	}

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Run", true)]
	bool OK()
	{
		return true;
	}

	//----------------------------------------------------------------------------------------------
	[ButtonAttribute("Cancel")]
	bool Cancel()
	{
		return false;
	}

	//----------------------------------------------------------------------------------------------
	bool TestAgainstFilter(string resource)
	{
		int resourceLength = resource.Length();

		int prefixLength = PathStartsWith.Length();
		if (resourceLength < prefixLength)
			return false;

		int suffixLength = PathEndsWith.Length();
		if (resourceLength < suffixLength)
			return false;

		int subLength = PathContains.Length();
		if (resourceLength < subLength)
			return false;

		resource.ToLower();
		if (!resource.StartsWith(PathStartsWith))
			return false;

		if (!resource.EndsWith(PathEndsWith))
			return false;

		if (!resource.Contains(PathContains))
			return false;

		return true;
	}

	//----------------------------------------------------------------------------------------------
	void Find(ResourceName resName, string filePath)
	{
		m_Resources.Insert(filePath);
	}

	//----------------------------------------------------------------------------------------------
	override void Run()
	{
		if (!Workbench.ScriptDialog("Configure batch texture processor", "", this))
			return;

		PathStartsWith.ToLower();
		PathContains.ToLower();
		PathEndsWith.ToLower();

		// TODO: Search resources in specific directory? Must filter myself.

		array<ref TextureType> textureTypes = new array<ref TextureType>;
		TextureType.RegisterTypes(textureTypes);
		foreach (TextureType texType : textureTypes)
		{
			texType.m_PostFix += "edds";
		}

		Workbench.SearchResources(Find, {"edds"});
		m_Resources.Sort();

		ResourceManager rb = Workbench.GetModule(ResourceManager);
		WBProgressDialog progress = new WBProgressDialog("Processing...", rb);

		bool noMetaFileReport = !ReportMissingMetaFile && !ReportMissingConfigurations && !ReportWrongPropertyValues;
		float count = m_Resources.Count();

		int reportCount = 0;

		int reimportCount = 0;
		int reimportCountMax;
		if (MaxFixCount > 0)
			reimportCountMax = MaxFixCount;
		else
			reimportCountMax = m_Resources.Count();

		array<string> toRebuild = new array<string>;

		foreach (int resourceIdx, string resource : m_Resources)
		{
			progress.SetProgress(resourceIdx / count);

			if (!TestAgainstFilter(resource))
				continue;

			TextureType matchingType = null;
			foreach (TextureType texType : textureTypes)
			{
				if (texType.TestPostFix(resource))
				{
					matchingType = texType;
					break;
				}
			}

			if (!matchingType)
			{
				if (ReportUnrecognizedPostfix)
				{
					ReportIssue(reportCount, resource, "postfix is not recognized");
				}
				continue;
			}

			// Optimization. Loading metafiles is rather expensive (file IO), so if we don't report
			// anything for these files, don't open them.
			if (noMetaFileReport && !FixProperties && !FixMetaFile)
				continue;

			MetaFile meta = rb.GetMetaFile(resource);
			if (!meta)
			{
				if (ReportMissingMetaFile)
				{
					ReportIssue(reportCount, resource, "meta-file is missing");
				}
				continue;
			}

			BaseContainerList configurations = meta.GetObjectArray("Configurations");
			if (!configurations)
			{
				if (ReportMissingConfigurations)
				{
					ReportIssue(reportCount, resource, "meta-file is missing 'Configurations' property");
				}
				meta.Release();
				continue;
			}

			BaseContainer conf = configurations.Get(0);
			if (ReportWrongResourceClass && conf.GetClassName() == "EDDSResourceClass")
			{
				ReportIssue(reportCount, resource, "Resource class is 'EDDSResourceClass'");
			}

			bool propertiesModified = false;
			if (ReportWrongPropertyValues || FixProperties)
			{
				foreach (TextureTypeProperty prop : matchingType.m_Properties)
				{
					int propVal;
					if (conf.Get(prop.m_Name, propVal))
					{
						int propValCorrect = prop.m_Val;
						bool isIncorrect = propVal != propValCorrect;
						if (isIncorrect && prop.m_OtherVariants)
						{
							foreach (int otherVariant : prop.m_OtherVariants)
							{
								if (otherVariant == propVal)
								{
									isIncorrect = false;
									break;
								}
							}
						}

						if (isIncorrect)
						{
							if (ReportWrongPropertyValues)
							{
								string valName, valNameCorrect;
								if (prop.m_ValType == bool)
								{
									if (propVal)        { valName        = "true"; } else { valName        = "false"; }
									if (propValCorrect) { valNameCorrect = "true"; } else { valNameCorrect = "false"; }
								}
								else
								{
									valName        = typename.EnumToString(prop.m_ValType, propVal);
									valNameCorrect = typename.EnumToString(prop.m_ValType, propValCorrect);
								}
								string issue = string.Format(
									"Property '%1' has wrong value ('%2' instead of '%3').",
									prop.m_Name, valName, valNameCorrect
								);
								ReportIssue(reportCount, resource, issue);
							}

							if (FixProperties && toRebuild.Count() < reimportCountMax)
							{
								propertiesModified = true;
								conf.Set(prop.m_Name, propValCorrect);
							}
						}
					}
					else
					{
						if (ReportWrongPropertyValues)
						{
							ReportIssue(reportCount, resource, string.Format("Property '%1' is not set", prop.m_Name));
						}

						if (FixProperties && toRebuild.Count() < reimportCountMax)
						{
							propertiesModified = true;
							conf.Set(prop.m_Name, prop.m_Val);
						}
					}
				}
			}

			bool anyChangeInMetaFile = false;

			if(FixMetaFile)
			{
				anyChangeInMetaFile = FixTextureMetaFile(meta, resource, m_TextureTypes);

				if(anyChangeInMetaFile)
					Print(resource);
			}

			if (propertiesModified || anyChangeInMetaFile )
			{
				meta.Save();

				if(!FixMetaFile)
					toRebuild.Insert(resource);
			}
			meta.Release();
		}

		if (MaxReportCount > 0 && reportCount > MaxReportCount)
			PrintFormat("... and %1 more issues.", reportCount);

		if (toRebuild.Count() > 0)
		{
			Print("Reimporting modified resources.");
			rb.RebuildResourceFiles(toRebuild, "PC");
		}

		m_Resources.Clear();
	}

	//----------------------------------------------------------------------------------------------
	void ReportIssue(inout int reportCount, string resource, string issue)
	{
		reportCount++;
		if (MaxReportCount > 0 && reportCount > MaxReportCount)
			return;

		PrintFormat("@\"%1\" : %2", resource, issue);
	}
}

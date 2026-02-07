class SCR_EditorSettings: ModuleGameSettings
{	
	[Attribute(defvalue: "0", desc: "When enabled the GM can enter layers, add entities to layers and remove them from layers. Some group functions are excluded.")]
	bool m_bLayerEditing;
	
	[Attribute(EEditorTransformVertical.TERRAIN.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Preview vertical mode, Will placed entities be alligned to the terrain or to the sea", enums: ParamEnumArray.FromEnum(EEditorTransformVertical))]
	EEditorTransformVertical m_PreviewVerticleMode;
	
	[Attribute(EEditorTransformSnap.TERRAIN.ToString(), uiwidget: UIWidgets.ComboBox, desc: "Preview vertical snap, When enabled, will snap entities to the terrain, if false it will be possible for entities to be placed inside the ground.", enums: ParamEnumArray.FromEnum(EEditorTransformVertical))]
	EEditorTransformSnap m_PreviewVerticalSnap;
};
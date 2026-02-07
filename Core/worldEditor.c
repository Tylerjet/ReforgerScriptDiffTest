class WorldEditorAPI
{
	proto native bool BeginTerrainAction(TerrainToolType toolType, string historyPointName = "", string historyPointIcon = "");
	proto native void EndTerrainAction(string historyPointName = "");
	proto native bool BeginEntityAction(string historyPointName = "", string historyPointIcon = "");	//!< Beginning of logical edit action.
	proto native bool EndEntityAction(string historyPointName = "");	//!< Ending of edit action.
	proto native bool BeginEditSequence(IEntitySource entSrc);	//!< Together with EndEditSequence() it can be used to define a block where entity will never be re-initialized. Useful for multiple edit actions upon single entity.
	proto native bool EndEditSequence(IEntitySource entSrc);	//!< Together with BeginEditSequence() it can be used to define a block where entity will never be re-initialized. Useful for multiple edit actions upon single entity.
	proto native bool IsEditSequenceActive(IEntitySource entSrc);	//!< Check whether we are between BeginEditSequence() and EndEditSequence().
	proto native bool IsDoingEditAction();	//!< Check whether we are between BeginEntityAction() and EndEntityAction().
	proto native bool UndoOrRedoIsRestoring();		//!< Check whether editor is restoring undo or redo state.
	proto native bool IsCreatingEntityInWindow(); //!< Check whether user created an entity in editor window. Eg. drag and drop from hierarchy, etc.
	proto native bool IsModifyingData();
	proto native external void AddTerrainFlatterEntity(IEntity entity, vector mins, vector maxs, int iPriority, float fFalloffStart, float fFalloff, bool bForceUpdate = true, array<vector> updateMins = null, array<vector> updateMaxes = null); // TODO
	proto native void RemoveTerrainFlatterEntity(IEntity entity, bool bUpdateTerrain = true); // TODO
	proto native IEntity SourceToEntity(IEntitySource entSrc);
	proto native IEntitySource EntityToSource(IEntity ent);
	proto native IEntitySource FindEntityByName(string name);
	proto native IEntitySource FindEntityByID(EntityID id);
	proto external string GetEntityNiceName(IEntitySource entSrc);
	
	//! Puts new (nowhere used) entity source instances as children of an entity in prefab
	proto native external bool AddPrefabMembers(IEntitySource prefabMember, notnull array<IEntitySource> members);
	proto native external bool RemovePrefabMembers(notnull array<IEntitySource> members);
	
	/*!
	Moves existing entity instances from map into a prefab.
	\param newParentInMap It's any entity of the prefab instance that exists in map (can be any child or sub-child). This it's only an optional parameter which ensures that transformations of the entitiesInMap after move to the prefab will be relative to the entity that exists in map. If this parameter is null then world transformations of the entitiesInMap will be stored into a prefab
	\param newParentInPrefab It's a further parent entity in the prefab. It can be root or any sub-child entity stored in a prefab.
	\param entitiesInMap Any entity instancies that currently exist in a map. These entities will be deleted from map and added into a prefab as children of newParentInPrefab.
	*/
	proto native external bool MoveEntitiesToPrefab(IEntitySource newParentInMap, IEntitySource newParentInPrefab, notnull array<IEntitySource> entitiesInMap);
	
	proto native void RegenerateFlowMaps(bool preview = true, bool save = true, bool asyncReturn = true, bool noWarning = false);

	//! Returns the number of entities in the editor.
	proto native int GetEditorEntityCount();

	//! Returns an entity in the editor on given index. To obtain count, use GetEditorEntityCount().
	proto native IEntitySource GetEditorEntity(int index);

	/*!
	Shows a selection dialog with provided items where user can select one or multiple items.
	\param title Title of the dialog window.
	\param message Message shown in the dialog window.
	\param width Width of the dialog window.
	\param height Height of the dialog window.
	\param items List of items that will be shown in the dialog MenuBase.
	\param selectedItems List of indices that will be filled up after selection in the dialog corresponding to selected items.
	\param currentItem Item selected by default.
	*/
	proto native external int ShowItemListDialog(string title, string message, int width, int height, notnull array<string> items, notnull out array<int> selectedItems, int currentItem);

	/*!
	Modifies the terrain height at specified position based on given brush.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param shape Shape of the brush.
	\param interpFunc Interpolation function used when creating the brush.
	*/
	proto native void ModifyHeightMap(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, FilterMorphShape shape, FilterMorphLerpFunc interpFunc);

	/*!
	Modifies the terrain height at specified position based on given map.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param userShape NxN array describing the modification brush. Values from 0.0 to 1.0 describing no to maximum strength.
	\param userShapeFilter Filtering to use when the tool size is not the same as the userShape array dimension.
	*/
	proto native void ModifyHeightMapUserShape(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, array<float> userShape, UserShapeFilter userShapeFilter);

	/*!
	Modifies the terrain layers at specified position based on given brush.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param shape Shape of the brush.
	\param interpFunc Interpolation function used when creating the brush.
	*/
	proto native void ModifyLayers(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, FilterMorphShape shape, FilterMorphLerpFunc interpFunc);

	/*!
	Modifies the terrain layers at specified position based on given map.
	\param xWorldPos X coordinate of the modification position.
	\param zWorldPos Z coordinate of the modification position.
	\param operation Operation to perform on the terrain.
	\param toolDesc Parameters of the operation.
	\param userShape NxN array describing the modification brush. Values from 0.0 to 1.0 describing no to maximum strength.
	\param userShapeFilter Filtering to use when the tool size is not the same as the userShape array dimension.
	*/
	proto native void ModifyLayersUserShape(float xWorldPos, float zWorldPos, FilterMorphOperation operation, notnull TerrainToolDesc toolDesc, array<float> userShape, UserShapeFilter userShapeFilter);

	proto native bool ParentEntity(notnull IEntity parent, notnull IEntity child, bool transformChildToParentSpace);
	proto native bool RemoveEntityFromParent(notnull IEntity child);

	proto native IEntityComponentSource CreateComponent(IEntitySource owner, string className);
	proto native bool DeleteComponent(IEntitySource owner, IEntityComponentSource component);

	proto native external void SetEntitySelection(notnull IEntity ent);
	proto native external void AddToEntitySelection(notnull IEntity ent);
	proto native void ClearEntitySelection();
	proto native void RemoveFromEntitySelection(notnull IEntity ent);
	proto native void SetPropertySelection(string id);

	proto native external bool ModifyEntityKey(notnull IEntity ent, string key, string value);
	proto native external bool ModifyEntityTemplateKey(IEntitySource tmpl, string key, string value);
	proto native external bool ModifyComponentKey(notnull IEntity ent, string componentName, string key, string value);

	proto native bool RenameEntity(notnull IEntity ent, string newName);

	proto native bool CreateEntityTemplate(IEntitySource entitySrc, string templateFileAbs);
  proto native bool SaveEntityTemplate(IEntitySource tmpl);
	proto native external IEntity CreateEntity(string className, string name, int layerId, IEntitySource parent, vector coords, vector angles);
	//! Extended version of base method that will use randomization/placement parameters set in Template Library (if any).
	proto native external IEntity CreateEntityExt(string className, string name, int layerId,  IEntitySource parent, vector coords, vector angles, int traceFlags);
	proto native external IEntity CreateEntityInWindow(RenderTargetWidget window, int winX, int winY, string className, string name, int layerID);

	proto native external IEntity CreateClonedEntity(notnull IEntity ent, string name, bool cloneChildren);
	proto native external bool DeleteEntity(notnull IEntity ent);
	proto native external bool DeleteEntities(notnull array<IEntity> ents);
	proto native IEntity GetEntityUnderCursor();
	//! Trace inside WorldEditor window, x and y coords are window coordinates.
	proto external bool TraceWorldPos(int x, int y, TraceFlags traceFlags, out vector traceStart, out vector traceEnd, out vector traceDir, out IEntity hitEntity = null);
	//! Returns terrain height (world space) in given point x,z (world space). Very fast method.
	proto native float GetTerrainSurfaceY(float x, float z);
	//! Fills `y` with GetTerrainSurfaceY() and returns true if on x, z position is terrain, returns false otherwise.
	proto external bool TryGetTerrainSurfaceY(float x, float z, out float y);
	//! Returns terrain Planar resolution (meters).
	proto native float GetTerrainUnitScale(int terrainIndex = 0);

	proto native int GetSelectedEntitiesCount();
	proto native IEntity GetSelectedEntity(int n = 0);
	proto native void UpdateSelectionGui();

	proto external void GetWorldPath(out string path);
	proto native external BaseWorld GetWorld();

	proto native bool IsEntityVisible(IEntity entity);
	proto native void SetEntityVisible(IEntity entity, bool isVisible, bool recursive);
	proto native bool IsEntitySelected(IEntity entity);
	proto native bool IsEntitySelectedAsMain(IEntity entity);

	proto native bool IsEditMode();
	proto native bool IsGameMode();

	proto native int GetCurrentSubScene();
	proto native int GetNumSubScenes();
	proto native external int GetSelectedEntityLayers(int subscene, notnull array<int> outLayerIDs);
	proto native int GetEntityLayerId(int subscene, string name);
	proto native int GetCurrentEntityLayerId();
	proto native void SetCurrentEntityLayerId(int subscene, int layerID);
	proto native bool IsEntityLayerVisible(int subscene, int layerID);
	proto native void DeleteEntityLayer(int subscene, int layerID);
	proto native external int CreateSubsceneLayer(int subscene, string name);

	proto native external owned string GetCurrentToolName();

	//! Returns width of active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	proto native int GetScreenWidth();

	//! Returns height of active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	proto native int GetScreenHeight();

	/*!
	Returns horizontal mouse cursor position in active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	\param clamped Whether returned values should be clamped in a range between zero and screen width (window width in fact).
	*/
	proto native int GetMousePosX(bool clamped);

	/*!
	Returns vertical mouse cursor position in active scene window (usually it's perspective window but it can be also one of Top, Back, Right view).
	\param clamped Whether returned values should be clamped in a range between zero and screen height (window height in fact).
	*/
	proto native int GetMousePosY(bool clamped);

	/*!
	Set value of potentially nested variable. The whole path must exist, this function is just to set
	the value. For creation, see CreateObjectArrayVariableMember().

	This function traverses containers from `topLevel` using `containerIdPath`. For each entry selects property with given index.
	If the property is an array index from entry is used to select object at given index. For array properties index must not be -1.

	Let's have following config:
	\code
		Shape { // <- top level container
			Points { // <- array of points
				ShapePoint { // <- first element
					Position 2 3 1
					Metadata {
						Name name1
					}
				}
				ShapePoint { // <- second element
					Position 0 1 1
					Metadata {
						Name name2
					}
				}
			}
		}
	\endcode

	To set a name of the second shape point to "secondPoint" we do:
	\code
		auto containerPath = new array<ref ContainerIdPathEntry>();

		auto entry1 = new ContainerIdPathEntry("Points", 1); // Take the first point
		containerPath.Insert(entry1);

		auto entry2 = new ContainerIdPathEntry("Metadata"); // Go to metadata of the first point
		containerPath.Insert(entry2);

		m_API.SetVariableValue(topLevelShapeContainer, containerPath, "Name", "secondPoint"); // Set Name to given value
	\endcode

	\param topLevel Container from which the search is started.
	\param containerIdPath Path to the variable we want to set.
	\param key The actual variable to set.
	\param value Value which will be set to variable.
	\return `true` if successful, `false` otherwise.
	*/
	proto native external bool SetVariableValue(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, string value);

	/*!
	Clear value of potentially nested variable. The whole path must exist, this function is just to clear the value.
	\param topLevel Container from which the search is started.
	\param containerIdPath Path to the variable we want to clear.
	\param key The actual variable to clear.
	\return `true` if successful, `false` otherwise.
	*/
	proto native external bool ClearVariableValue(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key);

	/*!
	Creates a new container in object property.
	\param topLevel Container from which the search is started
	\param containerIdPath Path to the variable we want to set
	\param key How is the object property in `container` called
	\param baseClassName What type of object should be created to the property
	\return `true` if successful, `false` otherwise.
	*/
	proto native external bool CreateObjectVariableMember(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, string baseClassName);

	/*!
	Creates a new element in array of objects property in `container`.
	\param topLevel Container from which the search is started
	\param containerIdPath Path to the variable we want to set
	\param key How is the array of object property in `container` called
	\param baseClassName What type of object should be created to the array
	\param memberIndex At which index should be the new element inserted
	\return `true` if successful, `false` otherwise.
	*/
	proto native external bool CreateObjectArrayVariableMember(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, string baseClassName, int memberIndex);

	/*!
	Removes an element from array of objects of property `key` in `container`.
	\param topLevel Container from which the search is started
	\param containerIdPath Path to the variable we want to set
	\param key How is the array of object property in `container` called
	\param memberIndex Which element to remove
	\return `true` if successful, `false` otherwise.
	*/
	proto native external bool RemoveObjectArrayVariableMember(notnull BaseContainer topLevel, array<ref ContainerIdPathEntry> containerIdPath, string key, int memberIndex);

	//! Set world editor perspective view camera position and look direction.
	proto native void SetCamera(vector pos, vector lookVec);

	//! Enable/disable calling of events on generators. Must be enabled for the VectorTool and generators to work properly.
	proto native void ToggleGeneratorEvents(bool isEnabled);
	proto native bool AreGeneratorEventsEnabled();

	private void WorldEditorAPI() {}
	private void ~WorldEditorAPI() {}
}

/*class EditorWindowWidget: RenderTargetWidget
{
}*/

class WorldEditorIngame
{
	proto native bool LoadWorld(string worldFilePath);
	proto native bool SaveWorld();

	proto native void SetMoveTool();
	proto native void SetRotateTool();
	proto native void SetScaleTool();

	proto native bool Undo();
	proto native bool Redo();

	proto native void Update(float tDelta);
	proto native void Init(notnull RenderTargetWidget mainWindow);
	proto native void Cleanup();

	proto native WorldEditorAPI GetAPI();
}

class ContainerIdPathEntry : Managed
{
	string PropertyName;
	int Index;

	void ContainerIdPathEntry(string propertyName, int index = -1)
	{
		PropertyName = propertyName;
		Index = index;
	}
}

/*!
Wrapper around GetEditorEntity and GetEditorEntityCount
Skips all entities that are not top-level.
Editing the underlying entity list invalidates the iterator.
*/
class EditorEntityIterator
{
	void EditorEntityIterator(notnull WorldEditorAPI api)
	{
		m_pApi = api;
		m_iCurrentIdx = 0;
		m_iCount = m_pApi.GetEditorEntityCount();
	}

	//! Returns all top level entities in order, null when all entities were iterated.
	IEntitySource GetNext()
	{
		while (m_iCurrentIdx < m_iCount && m_pApi.GetEditorEntity(m_iCurrentIdx).GetParent())
			m_iCurrentIdx++;

		if (m_iCurrentIdx < m_iCount)
			return m_pApi.GetEditorEntity(m_iCurrentIdx++);

		return null;
	}

	int GetCurrentIdx()
	{
		return m_iCurrentIdx;
	}

	private WorldEditorAPI m_pApi;
	private int m_iCurrentIdx;
	private int m_iCount;
}

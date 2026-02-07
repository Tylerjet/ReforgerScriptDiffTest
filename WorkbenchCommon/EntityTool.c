[WorkbenchPluginAttribute("Check Entities", "Check position of all entities in opened map and select invalid ones", "", "", {"WorldEditor"},"",0xf058)]
class EntityCheckPlugin: WorkbenchPlugin
{
	override void Run()
	{
		vector boundsMins = "-4000 -4000 -4000";
		vector boundsMaxs = "4000 4000 4000";
		
		WorldEditor module = Workbench.GetModule(WorldEditor);
		if (module)
		{
			WorldEditorAPI api = module.GetApi();
			api.ClearEntitySelection();
			IEntitySource worldSrc = api.FindEntityByName("world");
			if (worldSrc)
			{
				worldSrc.Get("boundMins", boundsMins);
				worldSrc.Get("boundMaxs", boundsMaxs);
			}
			
			WBProgressDialog progress = new WBProgressDialog("Entity Check", module);
			int c = module.GetNumContainers();
			
			Print("EntityCheckPlugin: checking " + c + " entities...");
			Print(boundsMins);
			Print(boundsMaxs);
			
			for (int i = 0; i < c; i++)
			{
				IEntitySource src = IEntitySource.Cast(module.GetContainer(i));
				if (src)
				{
					IEntity entity = api.SourceToEntity(src);
					vector pos = entity.GetOrigin();
					for (int j = 0; j < 3; j++)
					{
						if (pos[j] < boundsMins[j] || pos[j] > boundsMaxs[j])
						{
							api.AddToEntitySelection(entity);
							if (src.GetName())
								Print("Entity(" + entity + " ID:" + entity.GetID() + " Name: " + src.GetName() + ") has invalid position " + pos, LogLevel.ERROR);
							else
								Print("Entity(" + entity + " ID:" + entity.GetID() + ") has invalid position " + pos, LogLevel.ERROR);
							break;
						}
					}
				}
				
				if ((i % 1000) == 0)
					progress.SetProgress((float)i / (float)c);
			}
			
			api.UpdateSelectionGui();
			Print("EntityCheckPlugin: entities checking done.");
		}
	}
}

[WorkbenchPluginAttribute("Change absolute Y to relative", "Modifies entity Y position from absolute value to relative.", "", "", {"WorldEditor"},"",0xf548)]
class EntityRelativePlugin: WorkbenchPlugin
{
	override void Run()
	{
		WorldEditor we = Workbench.GetModule(WorldEditor);
		if (!we)
		{
			Print("World editor module is not available.", LogLevel.ERROR);
			return;
		}

		WorldEditorAPI api = we.GetApi();
		int selectedCount = api.GetSelectedEntitiesCount();
		if (selectedCount == 0)
		{
			Print("You need to select at least one entity.", LogLevel.ERROR);
			return;
		}

		WBProgressDialog progress = new WBProgressDialog("Processing entities", we);
		
		api.BeginEntityAction("Changing entity Y from absolute to relative");

		Print("EntityRelativePlugin: processing " + selectedCount + " entities...");
		for (int i = 0; i < selectedCount; i++)
		{
			progress.SetProgress(i / selectedCount);
			
			IEntity e = api.GetSelectedEntity(i);
			IEntitySource src = api.EntityToSource(e);
			
			vector pos;
			src.Get("coords", pos);
			
			float y;
			if (api.TryGetTerrainSurfaceY(pos[0], pos[2], y))
			{
				pos[1] = pos[1] - y;
				api.ModifyEntityKey(e, "coords", pos.ToString(false));
			}
			else
			{
				Print("Entity " + e.GetID() + " can't relatively positioned to the terrain.", LogLevel.WARNING);
			}
		}
		
		api.EndEntityAction();
	}

	override void Configure() { }
}
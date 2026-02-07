class SCR_CommentEditableEntityUIComponent: SCR_BaseEditableEntityUIComponent
{
	const string WIDGET_TEXT = "Text"; //--- ToDo: Don't hardcode
	const string WIDGET_ICON = "Icon"; //--- ToDo: Don't hardcode
	
	override void OnInit(SCR_EditableEntityComponent entity, SCR_UIInfo info, SCR_EditableEntityBaseSlotUIComponent slot)
	{
		Widget widget = GetWidget();
		if (!widget) return;
		
		GenericEntity owner = entity.GetOwner();
		if (!owner) return;
		
		SCR_EditableCommentComponent comment = SCR_EditableCommentComponent.Cast(entity);
		if (!comment) return;
		
		TextWidget textWidget = TextWidget.Cast(widget.FindAnyWidget(WIDGET_TEXT));
		if (textWidget)
			comment.ApplyTo(textWidget);
		
		ImageWidget iconWidget = ImageWidget.Cast(widget.FindAnyWidget(WIDGET_ICON));
		if (iconWidget)
			comment.ApplyTo(iconWidget);
	}
};
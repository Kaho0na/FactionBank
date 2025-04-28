modded class ActionConstructor
{
	// ------------------------------------------------------------
	// RegisterActions
	// ------------------------------------------------------------
	override void RegisterActions( TTypenameArray actions )
	{
		super.RegisterActions( actions );	

		actions.Insert(ExpansionActionOpenFactionMenu);

	}
}
package rocccplugin.perspectives;

import org.eclipse.ui.IFolderLayout;
import org.eclipse.ui.IPageLayout;
import org.eclipse.ui.IPerspectiveFactory;

import rocccplugin.views.DebugVariables;
import rocccplugin.views.ROCCC_IPCores;

public class Perspective implements IPerspectiveFactory 
{
	public void createInitialLayout(IPageLayout layout) 
	{			
		String editorArea = layout.getEditorArea();
		
		layout.addView(IPageLayout.ID_PROJECT_EXPLORER, IPageLayout.LEFT, 0.21f, editorArea);
		
		IFolderLayout bottom = layout.createFolder("bottom", IPageLayout.BOTTOM, 0.74f, editorArea);
		bottom.addView(ROCCC_IPCores.ID);
		
		// Comment this out if you would like the debug view turned off
		IFolderLayout bottomRight = layout.createFolder("bottomRight", IPageLayout.RIGHT, 0.74f, "bottom");
		bottomRight.addView(DebugVariables.ID);
	
		IFolderLayout right = layout.createFolder("right", IPageLayout.RIGHT, 0.68f, editorArea);
		right.addView("org.eclipse.ui.console.ConsoleView");
	}
}

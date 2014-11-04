package rocccplugin.handlers;
import java.io.File;

import org.eclipse.core.commands.ExecutionEvent;
import org.eclipse.core.commands.ExecutionException;
import org.eclipse.core.commands.IHandler;
import org.eclipse.core.commands.IHandlerListener;
import org.eclipse.core.resources.IFile;
import org.eclipse.jface.text.ITextSelection;
import org.eclipse.jface.viewers.ISelection;
import org.eclipse.jface.viewers.ISelectionProvider;
import org.eclipse.ui.IEditorPart;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.texteditor.ITextEditor;

import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.views.DebugVariables;


public class ToggleDebugVariable implements IHandler 
{
	public void addHandlerListener(IHandlerListener handlerListener) 
	{
		
	}

	public void dispose() 
	{
		
	}

	public Object execute(ExecutionEvent event) throws ExecutionException 
	{
		IEditorPart editor =  PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().getActiveEditor();
		if (editor instanceof ITextEditor) 
		{
			
			//editor.getEditorSite().getActionBars().getStatusLineManager().
			
			ISelectionProvider selectionProvider = ((ITextEditor)editor).getSelectionProvider();
			ISelection selection = selectionProvider.getSelection();
		 		  
			if (selection instanceof ITextSelection) 
			{
				IFile eclipseSourceFile = EclipseResourceUtils.getSelectedFileInEditor();
				
				File sourceFile = null;
				if(eclipseSourceFile != null)
					sourceFile = new File(eclipseSourceFile.getRawLocation().toOSString());
				
				ITextSelection textSelection = (ITextSelection)selection;
				DebugVariables.addVariable(sourceFile, textSelection.getText());
			}
		}
	
		return null;
	}

	public boolean isEnabled() 
	{
		return true;
	}

	public boolean isHandled() 
	{

		return true;
	}

	public void removeHandlerListener(IHandlerListener handlerListener) 
	{

	}

}

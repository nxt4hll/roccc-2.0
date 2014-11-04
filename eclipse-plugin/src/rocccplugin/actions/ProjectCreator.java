package rocccplugin.actions;

import org.eclipse.core.resources.IProject;
import org.eclipse.core.resources.IResource;
import org.eclipse.core.resources.ResourcesPlugin;

import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.StringUtils;

public class ProjectCreator 
{
	static public boolean createProject(String projectName)
	{
		try
		{
			String name = "";
			
			IProject project = null;//ResourcesPlugin.getWorkspace().getRoot().getProject(name);
				
			boolean invalidProjectName = false;
				
			try
			{
				project = ResourcesPlugin.getWorkspace().getRoot().getProject(projectName);
			}
			catch(Exception e)
			{
				invalidProjectName = true;
			}
			
			if(invalidProjectName || !StringUtils.isValidVariableName(projectName))
			{
				//if(MessageDialog.openQuestion(new Shell(), "Project Name Error", "The project name \"" + projectName + "\" is invalid. Would you like to try a different name?") == false) 
				{
					return false;
				}
			}
			else if(EclipseResourceUtils.projectExists(project.getName()))
			{
				//if(MessageDialog.openQuestion(new Shell(), "Existing Project", "The project \"" + project.getName() + "\" already exists. Would you like to try a different name?") == false) 
				{
					return false;
				}
			}
				
			project.create(null);
			project.open(null);
			
			project.refreshLocal(IResource.DEPTH_INFINITE, null);
			
			//MessageUtils.printlnConsoleSuccess("Project \"" + project.getName() + "\" created.\n");
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		return true;
	}
}

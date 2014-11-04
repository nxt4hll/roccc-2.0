package rocccplugin.views;

import java.io.File;
import java.util.Map;
import java.util.TreeMap;
import java.util.Vector;

import org.eclipse.core.resources.IFile;
import org.eclipse.jface.dialogs.InputDialog;
import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.SWT;
import org.eclipse.swt.events.KeyEvent;
import org.eclipse.swt.events.KeyListener;
import org.eclipse.swt.events.MouseEvent;
import org.eclipse.swt.events.MouseListener;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Table;
import org.eclipse.swt.widgets.TableColumn;
import org.eclipse.swt.widgets.TableItem;
import org.eclipse.ui.IEditorReference;
import org.eclipse.ui.IPageListener;
import org.eclipse.ui.IPartListener;
import org.eclipse.ui.IPartListener2;
import org.eclipse.ui.IWorkbench;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchPartReference;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.part.ViewPart;

import rocccplugin.Activator;
import rocccplugin.Activator.VersionStatus;
import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.CompositeUtilities;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;
import rocccplugin.utilities.GuiLockingUtils;
import rocccplugin.utilities.StringUtils;

/*
   The purpose of this class is to keep track of all the variables that have been marked as debug values.
     This should exist even if the view has not been opened.
 */
public class DebugVariables extends ViewPart 
{
	// This table is what appears in the view, but actually contains all of the data
	static Table debugVariables;
	
	// These are the columns that make up the debugVariables table
	static TableColumn variablesColumn ;
	static TableColumn watchpointColumn ; 	    // Added by Jason on 1/8/2013 in order to handle watchpoint generation
	static TableColumn watchpointValidColumn ;	// Added by Jason on 1/8/2013 in order to handle watchpoint generation

	// This map is a convinent way to keep track of all currently defined debug variables associated with a ROCCC file
	static Map<String, Vector<String>> variablesList;

	static boolean validROCCCFile = true;
	static String savedDebugValuesFile;
	static IPartListener partL = null;
	static IPartListener2 partListener = null;
	
	public static final String ID = "rocccplugin.views.debugVariables";

	// The constructor just creates the map of debug variables connected to each file
	public DebugVariables() 
	{
		validROCCCFile = false;
		if(variablesList == null)
		{
			variablesList = new TreeMap<String, Vector<String>>() ;
		}
	}

	public static String getDebugValuesFile(File file, boolean generate)
	{
		if(generate)
		{
			loadDebugVariables(file);
			saveDebugVariables(file);
		}
		else
		{
			return file.getAbsolutePath().replace(file.getName(), "") + ".ROCCC/" + file.getName().replace(".c", ".debug");
		}
		
		return savedDebugValuesFile;
	}
	
	boolean GuiCheck()
	{
		if(!GuiLockingUtils.lockGui())
		{
			return false;
		}
		if(!DatabaseInterface.openConnectionWithoutWarnings())
		{
			GuiLockingUtils.unlockGui();
			return false;
		}
		if(!Activator.areCompilerAndPluginSynced())
		{
			DatabaseInterface.closeConnection();
			GuiLockingUtils.unlockGui();
			return false;
		}
		return true;
	}
	
	private static void saveDebugVariables(File sourceFile)
	{
		try
		{
			StringBuffer buf = new StringBuffer();
			
			// Original code: just writes out the variables to the debug file that will be read by Hi-CIRRF
			//if(variablesList != null)
			//{
			//	for(int i = 0; i < variablesList.get(sourceFile.getAbsolutePath()).size(); ++i)
			//	{
			//		buf.append(variablesList.get(sourceFile.getAbsolutePath()).get(i) + "\n");
			//	}
			//}
			
			// Write out each element's name, watch point, and watch point valid
			if (variablesList != null)
			{
				for (int i = 0 ; i < variablesList.get(sourceFile.getAbsolutePath()).size(); ++i)
				{
					buf.append(debugVariables.getItem(i).getText(0) + " ") ;
					buf.append(debugVariables.getItem(i).getText(1) + " ");
					buf.append(debugVariables.getItem(i).getText(2) + "\n") ;
				}
			}

			String folderString = sourceFile.getAbsolutePath().replace(sourceFile.getName(), "");
			File folder = new File(folderString + ".ROCCC/");
			try 
			{
				if(!folder.exists())
					folder.mkdir();
			}
			catch (Exception e1) 
			{
				e1.printStackTrace();
			}
			
			File debugFile = new File(folderString + ".ROCCC/" + sourceFile.getName().replace(".c", ".debug"));
			
			//Actually create the source file.
			try
			{
				if(debugFile.exists())
					debugFile.delete();
				FileUtils.createFileFromBuffer(buf, debugFile);
			}
			catch(java.lang.Exception e)
			{
				e.printStackTrace();
				MessageDialog.openInformation(new Shell(), "Error", "There was an error creating your file.");
			}
			
			savedDebugValuesFile = debugFile.getAbsolutePath();
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	private static void loadDebugVariables(File sourceFile)
	{
		try
		{
			if(!validROCCCFile)
				return;
		
			if(!sourceFile.exists())
				return;
			
			StringBuffer buf = new StringBuffer();
			
			if(sourceFile == null)
			{
				if(EclipseResourceUtils.isViewOpen(ID))
					EclipseResourceUtils.closeView(DebugVariables.ID);
			}
			
			String folderString = sourceFile.getAbsolutePath().replace(sourceFile.getName(), "");
			File debugFile = new File(folderString + ".ROCCC/" + sourceFile.getName().replace(".c", ".debug"));
			
			if(!debugFile.exists())
				return;
			
			FileUtils.addFileContentsToBuffer(buf, debugFile.getAbsolutePath());
			
			if(variablesList != null)
			{
				if(!variablesList.containsKey(sourceFile.getAbsolutePath()))
					variablesList.put(sourceFile.getAbsolutePath(), new Vector<String>());
				
				variablesList.get(sourceFile.getAbsolutePath()).removeAllElements();
			}
			if(debugVariables != null)
			{
				debugVariables.removeAll();
			
				while(buf.length() > 0)
				{
					// Original code
					//String var = StringUtils.getNextStringValue(buf);
					//variablesList.get(sourceFile.getAbsolutePath()).add(var);
					//new TableItem(debugVariables, SWT.NONE).setText(var);

					// Modified code for temporary test of watchpoint functionality
					String[] var = new String[4] ;
					var[0] = StringUtils.getNextStringValue(buf) ;
					var[1] = StringUtils.getNextStringValue(buf) ;
					var[2] = StringUtils.getNextStringValue(buf) ;
					var[3] = "\0" ;
					// Add the variable name to the list of all debug variables associated with this file
					variablesList.get(sourceFile.getAbsolutePath()).add(var[0]) ;
					// Add the table row to the table
					new TableItem(debugVariables, SWT.NONE).setText(var) ;
				}
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
		
	// This should really be called "toggle" variable
	public static void addVariable(File sourceFile, String variable)
	{
		EclipseResourceUtils.openView(DebugVariables.ID);
		
		if(!validROCCCFile)
			return;
		
		if(!StringUtils.isValidVariableName(variable))
		{
			MessageDialog.openError(new Shell(), "Variable Error", "\"" + variable + "\" is not a valid variable name.");
			return;
		}
			
		if(StringUtils.isCPlusPlusReservedWord(variable))
		{
			MessageDialog.openError(new Shell(), "Variable Error", "\"" + variable + "\" is not a valid variable name.");
			return;
		}
			
		// This actually toggles what happens, it doesn't just add it
		if(variablesList.get(sourceFile.getAbsolutePath()).contains(variable))
		{
			debugVariables.remove(variablesList.get(sourceFile.getAbsolutePath()).indexOf(variable));
			variablesList.get(sourceFile.getAbsolutePath()).remove(variable);
		}
		else
		{	
			String[] tmpVars = new String[4] ;
			tmpVars[0] = variable ;
			tmpVars[1] = "0" ;
			tmpVars[2] = "0" ;
			tmpVars[3] = "\0" ;
			// Original code
			//new TableItem(debugVariables, SWT.NONE).setText(variable);
			// Modified code
			new TableItem(debugVariables, SWT.NONE).setText(tmpVars) ;
			variablesList.get(sourceFile.getAbsolutePath()).add(variable);
		}
		
		saveDebugVariables(sourceFile);
	}
	
	public boolean isValidROCCCFile(File sourceFile)
	{
		if(sourceFile == null)
		{
			//Activator.closeView(DebugVariables.ID);
			return false;
		}
		
		//Check that the file you are compiling is a c file.
		if(!sourceFile.getAbsolutePath().endsWith(".c"))
		{
			return false;
		}
		
		try
		{
			if(sourceFile.getAbsolutePath().contains("/src/modules/"))
			{
				
			}
			else if(sourceFile.getAbsolutePath().contains("/src/systems/"))
			{
				
			}
			else if(sourceFile.getAbsolutePath().contains("/src/intrinsics/"))
			{
				
			}
			else
			{
				return false;
			}
		}
		catch(Exception e)
		{
			e.printStackTrace();
			return false;
		}
		
		if(sourceFile.getName().equals("hi_cirrf.c"))
			return false;
		
		return true;
	}
	
	// When does this get called?
	public void createPartControl(Composite parent) 
	{
		try
		{
			if(!GuiLockingUtils.lockGui())
			{
				new Label(parent, SWT.NONE).setText("There is another ROCCC process running. Please wait for that to finish.");
				return;
			}
			
			if(!DatabaseInterface.openConnectionWithoutWarnings())
			{
				Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false, SWT.CENTER);
				
				debugVariables = new Table(top, SWT.FULL_SELECTION | SWT.HIDE_SELECTION | SWT.BORDER);
				debugVariables.setHeaderVisible(true);
				debugVariables.setLinesVisible(true);
				debugVariables.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
				
				variablesColumn = new TableColumn(debugVariables, SWT.NONE);
				variablesColumn.setText("Invalid ROCCC Distribution Directory");
				debugVariables.setEnabled(false);
				
				variablesColumn.setWidth(400);
				
				// Added by Jason on 1/8/2013 to handle watchpoints
				watchpointColumn = new TableColumn(debugVariables, SWT.NONE);
				watchpointColumn.setText("") ;
				watchpointColumn.setWidth(8);
				watchpointValidColumn = new TableColumn(debugVariables, SWT.NONE);
				watchpointValidColumn.setText("") ;
				watchpointColumn.setWidth(3) ;
				debugVariables.showColumn(variablesColumn) ;
				debugVariables.showColumn(watchpointColumn) ;
				debugVariables.showColumn(watchpointValidColumn) ;
								
				GuiLockingUtils.unlockGui();
				return;
			}
			
			if(Activator.checkCompilerAndPluginStatus() == VersionStatus.COMPILER_UPDATE_NEEDED ||
			   Activator.checkCompilerAndPluginStatus() == VersionStatus.PLUGIN_UPDATE_NEEDED)
			{
				new Label(parent, SWT.NONE).setText("The ROCCC Compiler and ROCCC GUI plugin are out of sync. An update must take place before using this view.");
				DatabaseInterface.closeConnection();
				GuiLockingUtils.unlockGui();
				return;
			}
			
			final Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false);
			
			IFile eclipseSourceFile = EclipseResourceUtils.getSelectedFileInEditor();
			
			File sourceFile = null;
			if(eclipseSourceFile != null)
				sourceFile = new File(eclipseSourceFile.getRawLocation().toOSString());
			
			validROCCCFile = isValidROCCCFile(sourceFile);
			
			debugVariables = new Table(top, SWT.FULL_SELECTION | SWT.HIDE_SELECTION | SWT.BORDER);
			debugVariables.setHeaderVisible(true);
			debugVariables.setLinesVisible(true);
			debugVariables.setLayoutData(new GridData(SWT.FILL, SWT.FILL, true, true));
			
			variablesColumn = new TableColumn(debugVariables, SWT.NONE);
			variablesColumn.setText("Debug Variables" );
			
			variablesColumn.setWidth(400);
			
			
			// Added by Jason on 1/8/2013 to handle watchpoints
			watchpointColumn = new TableColumn(debugVariables, SWT.NONE);
			watchpointColumn.setText("Watch point") ;
			watchpointColumn.setWidth(200);
			watchpointValidColumn = new TableColumn(debugVariables, SWT.NONE);
			watchpointValidColumn.setText("Watch point enabled") ;
			watchpointValidColumn.setWidth(200) ;
			debugVariables.showColumn(variablesColumn) ;
			debugVariables.showColumn(watchpointColumn) ;
			debugVariables.showColumn(watchpointValidColumn) ;

			//Make sure we have that vector added first!!!
			
			// If the element is selected and the user hits delete or backspace, remove the row from the table
			debugVariables.addKeyListener(new KeyListener()
			{
				public void keyPressed(KeyEvent e) 
				{
					if(!validROCCCFile)
					{
						return;
					}
					if(e.character == SWT.BS || e.character == SWT.DEL)
					{
						if(debugVariables.getSelectionIndex() != -1)
						{
							IFile eclipseSourceFile = EclipseResourceUtils.getSelectedFileInEditor();
							
							File sourceFile = null;
							if(eclipseSourceFile != null)
								sourceFile = new File(eclipseSourceFile.getRawLocation().toOSString());
							
							variablesList.get(sourceFile.getAbsolutePath()).remove(debugVariables.getSelectionIndex());
							debugVariables.remove(debugVariables.getSelectionIndex());
							saveDebugVariables(sourceFile);
						}
					}
				}
				
				public void keyReleased(KeyEvent e) 
				{	
				}
			});
			
			// If the element is selected and double-clicked, allow the user to turn on and turn off the watchpoint
			debugVariables.addMouseListener(new MouseListener()
			{

				public void mouseDoubleClick(MouseEvent e) {
					// Open up a box in order to get the new values for the watchpoint
					int selectedRow = debugVariables.getSelectionIndex() ;
					
					if (selectedRow == -1)
						return ;

					// First, get the value for the watch point.
					InputDialog watchpointDialog = new InputDialog(null, "Watchpoint", "Enter a watchpoint for " + debugVariables.getItem(selectedRow).getText(0),
															       debugVariables.getItem(selectedRow).getText(1), null) ;
					watchpointDialog.open();
					String watchpointValue = watchpointDialog.getValue() ;
					
					// If the string entered is not a valid number, set it to 0 ;
					if(!StringUtils.isAnInt(watchpointValue))
						watchpointValue = "0" ;
					// Now, get if this watchpoint should be enabled or not					
					boolean enableWatchpoint = MessageDialog.openQuestion(null, "Enable Watchpoint:" + watchpointValue + "?", "Enable Watchpoint?") ;
					if (enableWatchpoint == true)
					{
						debugVariables.getItem(selectedRow).setText(1, watchpointValue) ;
						debugVariables.getItem(selectedRow).setText(2, "1") ;
					}
					else
					{
						debugVariables.getItem(selectedRow).setText(1, watchpointValue) ;
						debugVariables.getItem(selectedRow).setText(2, "0") ;
					}
					// Save the data to the file
					IFile eclipseSourceFile = EclipseResourceUtils.getSelectedFileInEditor() ;
					File sourceFile = null ;
					if (eclipseSourceFile != null) 
					{
						sourceFile = new File(eclipseSourceFile.getRawLocation().toOSString()) ;
						saveDebugVariables(sourceFile);
					}
				}

				public void mouseDown(MouseEvent e) {
					
				}

				public void mouseUp(MouseEvent e) {
					
				}
			}) ;
						
			
			try
			{
				PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().toString();
			}
			catch(Exception e)
			{
				//Activator.unlockGui();
				//Activator.closeView(DebugVariables.ID);
				//return ;
			}
			
			partListener = new IPartListener2()
			{
				public void partActivated(IWorkbenchPartReference partRef) 
				{
					
					if(partRef instanceof IEditorReference)
					{
						IFile eclipseSourceFile = EclipseResourceUtils.getSelectedFileInEditor();
						
						File sourceFile = null;
						if(eclipseSourceFile != null)
							sourceFile = new File(eclipseSourceFile.getRawLocation().toOSString());
						
						validROCCCFile = isValidROCCCFile(sourceFile);
						
						if(!EclipseResourceUtils.isViewShown(ID))
							return;
						
						if(sourceFile == null)
						{
							variablesColumn.setText("Not a valid ROCCC file");
							debugVariables.removeAll();
							debugVariables.setEnabled(false);
							return;
						}
							
						debugVariables.removeAll();
						
						variablesColumn.setText(sourceFile.getName() + " is not a valid ROCCC file");
						
			 			debugVariables.setEnabled(false);
						if(!validROCCCFile)
							return;
						
						debugVariables.setEnabled(true);
						variablesColumn.setText("Debug Variables for " + sourceFile.getName());
						
						if(variablesList.containsKey(sourceFile.getAbsolutePath()))
						{
							for(int i = 0; i < variablesList.get(sourceFile.getAbsolutePath()).size(); ++i)
							{
								// Default should be 0, 0 for all watchpoints at this time
								
								new TableItem(debugVariables, SWT.NONE).setText(variablesList.get(sourceFile.getAbsolutePath()).get(i));
							}
						}
						else
						{
							variablesList.put(sourceFile.getAbsolutePath(), new Vector<String>());
							loadDebugVariables(sourceFile);
							saveDebugVariables(sourceFile);
						}
					}
					else
					{
						IFile eclipseSourceFile = EclipseResourceUtils.getSelectedFileInEditor();
						
						File sourceFile = null;
						if(eclipseSourceFile != null)
							sourceFile = new File(eclipseSourceFile.getRawLocation().toOSString());
						
						if(sourceFile == null)
						{
							variablesColumn.setText("Not a valid ROCCC file");
							debugVariables.removeAll();
							debugVariables.setEnabled(false);
							return;
						}
					}
				}
	
				public void partBroughtToTop(IWorkbenchPartReference partRef)
				{
				}
	
				public void partClosed(IWorkbenchPartReference partRef)
				{
				}
	
				public void partDeactivated(IWorkbenchPartReference partRef) 
				{
				}
	
				public void partHidden(IWorkbenchPartReference partRef) 
				{
				}
	
				public void partInputChanged(IWorkbenchPartReference partRef) 
				{
				}
	
				public void partOpened(IWorkbenchPartReference partRef) 
				{
				}
				
				public void partVisible(IWorkbenchPartReference partRef) 
				{
				}
			};
			
			try
			{
				PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().addPartListener(partListener);
			}
			catch(Exception e)
			{
				PlatformUI.getWorkbench().getActiveWorkbenchWindow().addPageListener(new IPageListener()
				{
					public void pageActivated(IWorkbenchPage page) 
					{
						PlatformUI.getWorkbench().getActiveWorkbenchWindow().getActivePage().addPartListener(partListener);
					}
	
					public void pageClosed(IWorkbenchPage page) 
					{	
					}
	
					public void pageOpened(IWorkbenchPage page) 
					{		
					}
				});
			}
			
			if(sourceFile == null)
			{
				//Activator.closeView(DebugVariables.ID);
				GuiLockingUtils.unlockGui();
				return;
			}
			
			loadDebugVariables(sourceFile);
			
			
			variablesColumn.setText("Debug Variables for " + sourceFile.getName());
			
			GuiLockingUtils.unlockGui();
		}
		catch(Exception e)
		{
			e.printStackTrace();
			GuiLockingUtils.unlockGui();
		}
		
	}
	
	@Override
	public void dispose()
	{
		try
		{
			if(debugVariables != null)
			{
				debugVariables.dispose();
				debugVariables = null;
			}
			if(variablesColumn != null)
			{
				variablesColumn.dispose();
				variablesColumn = null;
			}
			// Added on 1/8/2013 by Jason to handle watch points
			if (watchpointColumn != null)
			{
				watchpointColumn.dispose();
				watchpointColumn = null ;
			}
			if (watchpointValidColumn != null)
			{
				watchpointValidColumn.dispose() ;
				watchpointValidColumn = null ;
			}
			
			if(partListener != null)
			{
				IWorkbench wb = PlatformUI.getWorkbench();
				if(wb != null)
				{
					IWorkbenchWindow wbw = wb.getActiveWorkbenchWindow();
					if(wbw != null)
					{
						IWorkbenchPage page = wbw.getActivePage();
						if(page != null)
							page.removePartListener(partListener);
					}
				}
				partListener = null;
			}
			variablesList = null;
			
			super.dispose();
		
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}

	public void setFocus() 
	{
		
	}

}

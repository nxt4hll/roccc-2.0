package rocccplugin.utilities;

import org.eclipse.jface.dialogs.MessageDialog;
import org.eclipse.swt.graphics.Color;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.ui.IWorkbenchPage;
import org.eclipse.ui.IWorkbenchWindow;
import org.eclipse.ui.PartInitException;
import org.eclipse.ui.PlatformUI;
import org.eclipse.ui.console.ConsolePlugin;
import org.eclipse.ui.console.IConsole;
import org.eclipse.ui.console.IConsoleManager;
import org.eclipse.ui.console.MessageConsole;
import org.eclipse.ui.console.MessageConsoleStream;

public class MessageUtils 
{
	static MessageConsole myConsole = null;
	static MessageConsoleStream out = null;
	static MessageConsoleStream success = null; 
	static MessageConsoleStream error = null;
	
	public static void initialize()
	{
		myConsole = findConsole("ROCCC Toolchain");
		out = myConsole.newMessageStream();
		success = myConsole.newMessageStream();
		error = myConsole.newMessageStream();
		
		success.setColor(new Color(null, 0, 0, 255));
		error.setColor(new Color(null, 255, 0, 0));
	}
	
	//Finds the console with the given name.
	private static MessageConsole findConsole(String name)
	{
		ConsolePlugin plugin = ConsolePlugin.getDefault();
		IConsoleManager conMan = plugin.getConsoleManager();
		IConsole[] existing = conMan.getConsoles();
		for (int i = 0; i < existing.length; i++)
			if (name.equals(existing[i].getName()))
				return (MessageConsole) existing[i];
		//no console found, so create a new one
		MessageConsole myConsole = new MessageConsole(name, null);
		conMan.addConsoles(new IConsole[]{myConsole});
		return myConsole;
	}
	
	public static void openConsoleView()
	{
		IWorkbenchWindow window = PlatformUI.getWorkbench().getActiveWorkbenchWindow();
		if(window == null)
			return;
		//Get the active page on the window.
		IWorkbenchPage page = window.getActivePage();
		if(page == null)
			return;
		
		for(int i = 0; i < page.getViewReferences().length; ++i)
		{
			if(page.getViewReferences()[i].getId().equals("org.eclipse.ui.console.ConsoleView"))
				return;
		}
		
		try 
		{
			page.showView("org.eclipse.ui.console.ConsoleView");
		} 
		catch (PartInitException e) 
		{
			e.printStackTrace();
		}	
	}
	
	//These functions display text in the console.
	
	public static void printConsoleMessage(String message)
	{
		out.print(message);
	}
	
	public static void printConsoleSuccess(String message)
	{
		success.print(message);
	}
	
	public static void printConsoleError(String message)
	{
		error.print(message);
	}
	
	public static void printlnConsoleMessage(String message)
	{
		printConsoleMessage(message + "\n");
	}
	
	public static void printlnConsoleSuccess(String message)
	{
		printConsoleSuccess(message + "\n");
	}
	
	public static void printlnConsoleError(String message)
	{
		printConsoleError(message + "\n");
	}
	
	
	//These functions open popup windows displaying messages.
	
	public static void openMessageWindow(String title, String message)
	{
		MessageDialog.openInformation(new Shell(), title, message);
	}
	
	public static void openErrorWindow(String title, String message)
	{
		MessageDialog.openError(new Shell(), title, message);
	}
	
	public static boolean openQuestionWindow(String title, String message)
	{
		return MessageDialog.openQuestion(new Shell(), title, message);
	}
}

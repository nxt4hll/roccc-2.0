package rocccplugin.composites;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Display;
import org.eclipse.swt.widgets.Group;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Shell;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;
import rocccplugin.utilities.CompositeUtilities;

public class GetTextMessageBox
{
	public Text userText;
	public String userString;
	String title;
	String description;
	boolean canceled = false;
	
	public static boolean CANCELED = false;
	
	Shell shell;
	
	public GetTextMessageBox(String title, String description) 
	{
		shell = new Shell(Display.getDefault());
		
		this.title = title;
		this.description = description;
		shell.setLayout(new GridLayout(1, false));
		shell.setLayoutData(CompositeUtilities.createNewGD(0));
		shell.setText(title);
		createControl(shell);
	}

	public boolean open()
	{
		shell.open();
		while (!shell.isDisposed()) 
		{
			if (!Display.getDefault().readAndDispatch()) 
			{
				Display.getDefault().sleep();
		    }
		}
		
		return !canceled;
	}
	
	public void createControl(Composite parent) 
	{
		Composite top = CompositeUtilities.createDefaultComposite(parent, 1, false);
		
		Group group1 = new Group(top, SWT.SHADOW_ETCHED_IN);
		group1.setText(description);
		group1.setLayout(new GridLayout());
		group1.setLayoutData(CompositeUtilities.createNewGD(0));
		
		Composite inside = CompositeUtilities.createDefaultComposite(group1, 2, false);
		inside.setLayoutData(CompositeUtilities.createNewGD(0));
		
		new Label(inside, SWT.NONE).setText(title);
		
		userText = new Text(inside, SWT.SINGLE | SWT.BORDER);
		userText.setLayoutData(CompositeUtilities.createNewGD(0));
		
		Composite buttons = CompositeUtilities.createDefaultComposite(top, 2, false);
		buttons.setLayoutData(CompositeUtilities.createNewGD(SWT.RIGHT, false, false, SWT.BOTTOM));
		
		Button ok = new Button(buttons, SWT.PUSH);
		ok.setLayoutData(CompositeUtilities.createNewGD(0));
		ok.setText("Ok");
		
		Button cancel = new Button(buttons, SWT.PUSH);
		cancel.setLayoutData(CompositeUtilities.createNewGD(0));
		cancel.setText("Cancel");

		ok.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{	
			}

			public void widgetSelected(SelectionEvent e) 
			{
				canceled = false;
				userString = userText.getText();
				shell.close();
			}
		});
		
		cancel.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
								
			}

			public void widgetSelected(SelectionEvent e) 
			{
				canceled = true;
				shell.close();
			}
		});
		
		
		CompositeUtilities.setCompositeSize(parent, 400, 150);
		
	}

}

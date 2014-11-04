package rocccplugin.helpers;

import java.io.File;

import org.eclipse.swt.SWT;
import org.eclipse.swt.events.SelectionEvent;
import org.eclipse.swt.events.SelectionListener;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.widgets.Button;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.FileDialog;
import org.eclipse.swt.widgets.Label;
import org.eclipse.swt.widgets.Text;
import org.eclipse.ui.PlatformUI;

import rocccplugin.Activator;
import rocccplugin.utilities.CompositeUtilities;

public class FileSelector 
{
	Label label;
	Text textField;
	Button browse;
	
	/*public FileSelector(String labelText, String initialText, Composite parent)
	{
		label = new Label(parent, SWT.NONE);
		textField = new Text(parent, SWT.BORDER);
		browse = new Button(parent, SWT.PUSH);
		
		label.setText(labelText);
		textField.setText(initialText);
		browse.setText("Browse");
		
		textField.setSize(300, (int) (textField.getLineHeight() * 0.85));
        Rectangle trim = textField.computeTrim(0, 0, 300, (int) (textField.getLineHeight() * 0.85));
		GridData d = Activator.createNewGD(0, true, false, SWT.CENTER);
		d.heightHint = trim.height;
		d.widthHint = trim.width;
		textField.setLayoutData(d);
		
		browse.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				
			}

			public void widgetSelected(SelectionEvent e) 
			{
				FileDialog fd = new FileDialog(PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell());
				File f = new File(textField.getText().toString());
				if(f.exists())
					fd.setFilterPath(f.getAbsolutePath().replace(f.getName(), ""));
				else
					fd.setFilterPath(startingFolder);
				if(fd.open() != null)
					textField.setText(fd.getFilterPath() + "/" + fd.getFileName());
			}
		});
	}*/
	
	public FileSelector(String labelText, String initialText, final String startingFolder, Composite parent)
	{
		label = new Label(parent, SWT.NONE);
		textField = new Text(parent, SWT.BORDER);
		browse = new Button(parent, SWT.PUSH);
		
		label.setText(labelText);
		textField.setText(initialText);
		browse.setText("Browse");
		
		
		textField.setSize(300, (int) (textField.getLineHeight() * 0.85));
        Rectangle trim = textField.computeTrim(0, 0, 300, (int) (textField.getLineHeight() * 0.85));
		GridData d = CompositeUtilities.createNewGD(0, true, false, SWT.CENTER);
		d.heightHint = trim.height;
		d.widthHint = trim.width;
		textField.setLayoutData(d);
		
		browse.addSelectionListener(new SelectionListener()
		{
			public void widgetDefaultSelected(SelectionEvent e) 
			{
				
			}

			public void widgetSelected(SelectionEvent e) 
			{
				FileDialog fd = new FileDialog(PlatformUI.getWorkbench().getActiveWorkbenchWindow().getShell());
				File f = new File(textField.getText().toString());
				if(f.exists())
					fd.setFilterPath(f.getAbsolutePath().replace(f.getName(), ""));
				else
					fd.setFilterPath(startingFolder);
				
				if(fd.open() != null)
					textField.setText(fd.getFilterPath() + "/" + fd.getFileName());
			}
		});
	}
	
	public void hide()
	{
		label.setVisible(false);
		textField.setVisible(false);
		browse.setVisible(false);
	}
	
	public void show()
	{
		label.setVisible(true);
		textField.setVisible(true);
		browse.setVisible(true);
	}
	
	public String getText()
	{
		return textField.getText();
	}
	
	public Text getTextBox()
	{
		return textField;
	}
	
	public void setText(String text)
	{
		textField.setText(text);
	}
	
	public void setLabel(String text)
	{
		label.setText(text);
	}
	
}

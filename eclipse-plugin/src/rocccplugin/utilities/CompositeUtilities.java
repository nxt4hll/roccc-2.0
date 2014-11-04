package rocccplugin.utilities;

import org.eclipse.swt.SWT;
import org.eclipse.swt.graphics.Rectangle;
import org.eclipse.swt.layout.GridData;
import org.eclipse.swt.layout.GridLayout;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.swt.widgets.Event;
import org.eclipse.swt.widgets.Listener;
import org.eclipse.swt.widgets.Text;

import rocccplugin.Activator;

public class CompositeUtilities 
{
	public static Text createTextBox(Composite parent, String initialText)
	{
		Text textField = new Text(parent, SWT.BORDER);
		textField.setText(initialText);
		
		textField.setSize(300, (int) (textField.getLineHeight() * 0.85));
        Rectangle trim = textField.computeTrim(0, 0, 300, (int) (textField.getLineHeight() * 0.85));
		GridData d = CompositeUtilities.createNewGD(0, true, false, SWT.CENTER);
		d.heightHint = trim.height;
		d.widthHint = trim.width;
		textField.setLayoutData(d);
		
		return textField;
	}
	
	public static void setCompositeSize(final Composite comp, final int width, final int height)
	{
		comp.addListener(SWT.Resize, new Listener() 
		{
		    int oldWidth = -1;
		    public void handleEvent(Event e) 
		    {
		      int newWidth = comp.getSize().x;
		      if (newWidth != oldWidth) 
		      {
				comp.setSize(width, height);
		      }
		    }
		  });
		
		comp.notifyListeners(SWT.Resize, null);
	}
	
	public static void setCompositeSize(final Composite comp, final Composite comp2, final int size1, final int size2)
	{
		comp.addListener(SWT.Resize, new Listener() 
		{
		    int width = -1;
		    public void handleEvent(Event e) 
		    {
		      int newWidth = comp.getSize().x;
		      if (newWidth != width) 
		      {
		    	comp2.setSize(SWT.DEFAULT, SWT.DEFAULT);
		        Rectangle trim = comp2.computeTrim(0, 0, 0, size1);
				GridData d = createNewGD(0);
				d.heightHint = trim.height;
				comp2.setLayoutData(d);
				comp.setSize(comp.getSize().x, size2);
		      }
		    }
		  });
		
		comp.notifyListeners(SWT.Resize, null);
	}
	
	public static void setCompositeSize(final Composite comp, final Composite comp2, final int width1, final int height1, final int width2, final int height2)
	{
		comp.addListener(SWT.Resize, new Listener() 
		{
		    int width = -1;
		    public void handleEvent(Event e) 
		    {
		      int newWidth = comp.getSize().x;
		      if (newWidth != width) 
		      {
		    	comp2.setSize(SWT.DEFAULT, SWT.DEFAULT);
		        Rectangle trim = comp2.computeTrim(0, 0, width1, height1);
				GridData d = createNewGD(0);
				d.heightHint = trim.height;
				d.widthHint = trim.width;
				comp2.setLayoutData(d);
				comp.setSize(width2, height2);
		      }
		    }
		  });
		
		comp.notifyListeners(SWT.Resize, null);
	}
	
	public static Text createTextField(Composite parent, int width)
	{
		Text text = new Text(parent, SWT.BORDER);
	
		text.setSize(width, (int) (text.getLineHeight() * 0.85));
        Rectangle trim = text.computeTrim(0, 0, width, (int) (text.getLineHeight() * 0.85));
		GridData d = CompositeUtilities.createNewGD(0, false, false, SWT.CENTER);
		d.heightHint = trim.height;
		d.widthHint = trim.width;
		text.setLayoutData(d);
		
		return text;
	}
	
	public static Composite createDefaultComposite(Composite parent, int numCols, boolean equal, int horizAlign)
	{
		Composite composite = new Composite(parent, SWT.LEFT);
		GridLayout layout = new GridLayout();
		layout.numColumns = numCols;
		layout.marginHeight = 2;
		layout.makeColumnsEqualWidth = equal;
		composite.setLayout(layout);

		GridData gd = new GridData(SWT.LEFT);
		gd.verticalAlignment = SWT.FILL;
		gd.horizontalAlignment = horizAlign;
		gd.grabExcessHorizontalSpace = false;
		
		composite.setLayoutData(gd);
		return composite;
	}
	
	public static Composite createDefaultComposite(Composite parent, int numCols, boolean equal) 
	{
		Composite composite = new Composite(parent, SWT.LEFT);
		GridLayout layout = new GridLayout();
		layout.numColumns = numCols;
		layout.marginHeight = 2;
		layout.makeColumnsEqualWidth = equal;
		composite.setLayout(layout);
		
		composite.setLayoutData(createNewGD(0));
		return composite;
	} 
	
	public static GridData createNewGD(int a)
	{
		GridData gd = new GridData(a);
		gd.verticalAlignment = SWT.FILL;
		gd.horizontalAlignment = SWT.FILL;
		gd.grabExcessHorizontalSpace = true;
		return gd;
	}
	
	public static GridData createNewGD(int a, boolean fillHoriz, boolean fillVert, int vertAlign)
	{
		GridData gd = new GridData(a);
		gd.verticalAlignment = fillVert? SWT.FILL : SWT.NONE;
		gd.horizontalAlignment = fillHoriz? SWT.FILL : SWT.NONE;
		gd.grabExcessHorizontalSpace = true;
		gd.verticalAlignment = vertAlign;
		return gd;
	}
}

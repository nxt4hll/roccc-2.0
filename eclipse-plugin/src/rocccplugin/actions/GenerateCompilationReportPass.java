package rocccplugin.actions;

import java.io.File;
import java.util.Map;
import java.util.Set;

import rocccplugin.database.DatabaseInterface;
import rocccplugin.utilities.EclipseResourceUtils;
import rocccplugin.utilities.FileUtils;

public class GenerateCompilationReportPass 
{
	static String tab = "\t";
	
	static public void run(File fileToGenerateReportFor)
	{
		try
		{
			StringBuffer fileBuffer = new StringBuffer();
			String componentName = DatabaseInterface.getComponentFromSourceFile(fileToGenerateReportFor);
		
			generateHeadInformation(fileBuffer, componentName);
			generateStyleSheetInfo(fileBuffer);
			generateHeader(fileBuffer, componentName);
			generateMainMiniInfo(fileBuffer, componentName);
			
			generateMainDetailedInfo(fileBuffer, componentName);
			generateCompilationOptionsInfo(fileBuffer, componentName);
			generateFooter(fileBuffer, componentName);
			genereateFootInformation(fileBuffer);
			
			String srcFileFolder = FileUtils.getFolderOfFile(fileToGenerateReportFor);
			FileUtils.createFileFromBuffer(fileBuffer, new File(srcFileFolder + componentName + "_Report.html"));
		}
		catch(Exception e)
		{
			e.printStackTrace();
		}
	}
	
	static private void generateHeadInformation(StringBuffer fileBuffer, String componentName)
	{
		fileBuffer.append("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD XHTML 1.0 Transitional//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd\">\n");
		fileBuffer.append("<head>\n");
		fileBuffer.append("<title>" + componentName + " Report</title>\n");
		fileBuffer.append("</head>\n\n");
	}
	
	static private void generateStyleSheetInfo(StringBuffer fileBuffer)
	{
		fileBuffer.append("<style type=\"text/css\">\n\n");
		fileBuffer.append("body {\n\n");
		
		fileBuffer.append("background-image: -webkit-gradient(\n");
		fileBuffer.append(tab + "linear,\n");
		fileBuffer.append(tab + "left bottom,\n");
		fileBuffer.append(tab + "left top,\n");
		fileBuffer.append(tab + "color-stop(0.92, rgb(255, 255, 255)),\n");
		fileBuffer.append(tab + "color-stop(1, rgb(219, 238, 255))\n");
		fileBuffer.append(");\n");
		
		fileBuffer.append("background-image: -moz-linear-gradient(\n");
		fileBuffer.append(tab + "center bottom,\n");
		fileBuffer.append(tab + "rgb(255, 255, 255) 92%,\n");
		fileBuffer.append(tab + "rgb(219, 238, 255) 100%\n");
		fileBuffer.append(");\n\n");
		
		fileBuffer.append("background-repeat: repeat-x;\n");
		fileBuffer.append("background-color: #FFFFFF;\n");
		fileBuffer.append("font-size: 10pt;\n");
		fileBuffer.append("font-family: Arial;\n");
		fileBuffer.append("margin: 0;\n");
		fileBuffer.append("padding: 0;\n");
		fileBuffer.append("color: #333333;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#page {\n");
		fileBuffer.append("width: 900px;\n");
		//fileBuffer.append("margin: auto;\n");
		fileBuffer.append("margin-left: auto;\n");
		fileBuffer.append("margin-right: auto;\n");
		fileBuffer.append("padding-left: 5px;\n");
		fileBuffer.append("padding-right:0 5px;\n");
		fileBuffer.append("horizontal-align: middle;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#header {\n");
		fileBuffer.append("clear: both;\n");
		fileBuffer.append("width: 885px;\n");
		fileBuffer.append("height: 89px;\n");
		fileBuffer.append("padding-top: 27px;\n");
		//fileBuffer.append("padding-left: 40px;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#headerleft {\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("height: 80px;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#quickinfo span {\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("background-image: none;\n");
		fileBuffer.append("display: block;\n");
		fileBuffer.append("color: #333333;\n");
		fileBuffer.append("text-decoration: none;\n");
		fileBuffer.append("font-size: 14px;\n");
		fileBuffer.append("font-weight: bold;\n");
		fileBuffer.append("padding-right: 30px;\n");
		fileBuffer.append("padding-top: 10px;\n");
		fileBuffer.append("}\n\n");

		fileBuffer.append("#mainarea {\n");
		fileBuffer.append("width: 910px;\n");
		//fileBuffer.append("padding-left: 40px;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#leftcontentarea {\n");
		fileBuffer.append("width: 600px;\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("line-height: 14pt;\n");
		fileBuffer.append("margin-right: 10px;\n");
		fileBuffer.append("margin-bottom: 10px;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#contentarea {\n");
		fileBuffer.append("border-radius: 10px;\n");
		fileBuffer.append("-moz-border-radius: 10px;\n");
		fileBuffer.append("margin-bottom: 10px;\n");
		fileBuffer.append("width: 570px;\n");
		fileBuffer.append("padding-left: 15px;\n");
		fileBuffer.append("padding-right: 15px;\n");
		fileBuffer.append("padding-bottom: 10px;\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("background-color: #EEF5F7;\n");
		fileBuffer.append("-moz-box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("-webkit-box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#contentareasplit {\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#contentarealeft {\n");
		fileBuffer.append("width: 275px;\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("line-height: 14pt;\n");
		fileBuffer.append("word-wrap: break-word;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#contentarearight {\n");
		fileBuffer.append("width: 275px;\n");
		fileBuffer.append("margin-left: 15px;\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("line-height: 14pt;\n");
		fileBuffer.append("word-wrap: break-word;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#sidebar {\n");
		fileBuffer.append("border-radius: 10px;\n");
		fileBuffer.append("-moz-border-radius: 10px;\n");
		fileBuffer.append("float: left;\n");
		fileBuffer.append("width: 265px;\n");
		fileBuffer.append("margin-left: 5px;\n");
		fileBuffer.append("margin-bottom: 10px;\n");
		fileBuffer.append("background-color: #D6E9Ef;\n");
		fileBuffer.append("line-height: 14pt;\n");
		fileBuffer.append("padding-left: 15px;\n");
		fileBuffer.append("padding-right: 5px;\n");
		fileBuffer.append("-moz-box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("-webkit-box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("padding-bottom: 10px;\n");
		fileBuffer.append("word-wrap: break-word;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("#footer {\n");
		fileBuffer.append("border-radius: 10px;\n");
		fileBuffer.append("-moz-border-radius: 10px;\n");
		fileBuffer.append("width: 900px;\n");
		fileBuffer.append("clear: both;\n");
		//fileBuffer.append("margin-left: 40px;\n");
		fileBuffer.append("padding-top: 15px;\n");
		fileBuffer.append("padding-bottom: 15px;\n");
		fileBuffer.append("text-align: center;\n");
		fileBuffer.append("line-height: 14pt;\n");
		fileBuffer.append("background-color: #F2F8F9;\n");
		fileBuffer.append("-moz-box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("-webkit-box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("box-shadow: 2px 2px 2px #CCD;\n");
		fileBuffer.append("margin-bottom: 10px;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("h1 {\n");
		fileBuffer.append("font-size: 25pt;\n");
		fileBuffer.append("color: #333333;\n");
		fileBuffer.append("margin: 0;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("h2 {\n");
		fileBuffer.append("font-size: 15pt;\n");
		fileBuffer.append("color: #333333;\n");
		fileBuffer.append("padding-top: 6px");
		fileBuffer.append("}\n\n");

		fileBuffer.append(".description {\n");
		fileBuffer.append("font-size: 14pt;\n");
		fileBuffer.append("color: #555555;\n");
		fileBuffer.append("margin: 0;\n");
		fileBuffer.append("}\n\n");
		
		fileBuffer.append("text-shadow: 1px 1px 1px #666;\n");
		fileBuffer.append("</style>\n\n");	
	}
	
	static private void generateHeader(StringBuffer fileBuffer, String componentName)
	{
		fileBuffer.append("<html>\n");
		fileBuffer.append("<body>\n\n");
		fileBuffer.append("<div id=\"page\">\n");
		fileBuffer.append(tab + "<div id=\"header\">\n");
		fileBuffer.append(tab + tab + "<div id=\"headerleft\">\n");
		fileBuffer.append(tab + tab + tab + "<h1>" + componentName + "</h1>\n");
		fileBuffer.append(tab + tab + tab + "<div class=\"description\">ROCCC 2.0 Compilation Report</div>\n");
		fileBuffer.append(tab + tab + "</div>\n");
		fileBuffer.append(tab + "</div>\n");
	
	}
	
	static private void generateMainMiniInfo(StringBuffer fileBuffer, String componentName)
	{
		int latency = DatabaseInterface.getDelay(componentName);
		String type = DatabaseInterface.getComponentType(componentName);
		
		fileBuffer.append(tab + "<div id=\"mainarea\">\n");
		fileBuffer.append(tab + tab + "<div id=\"leftcontentarea\">\n");
		fileBuffer.append(tab + tab + tab + "<div id=\"contentarea\">\n");
		fileBuffer.append(tab + tab + tab + tab + "<div id=\"quickinfo\">\n");
		fileBuffer.append(tab + tab + tab + tab + "<span>Component Type: <em>" + type + "</em></span>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "<span>Latency: <em>" + latency + " Cycles</em></span>\n");
		//fileBuffer.append(tab + tab + tab + tab + tab + "<span>Area Est: <em>10234 LUTS</em></span>\n");
		//fileBuffer.append(tab + tab + tab + tab + tab + "<span>Frequency Est: <em>" + CalculateFrequencyEstimationPass.run(componentName) + " MHZ</em></span>\n");
		
		
		fileBuffer.append(tab + tab + tab + tab + "</div>\n");
		fileBuffer.append(tab + tab + tab + "</div>\n");
	}
	
	static private void generateMainDetailedInfo(StringBuffer fileBuffer, String componentName)
	{
		fileBuffer.append(tab + tab + tab + "<div id=\"contentarea\">\n");
		fileBuffer.append(tab + tab + tab + tab + "<div id=\"contentareasplit\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarealeft\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<h2>Resources Used</h2>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<ul>\n");
		
		Map<String, Integer> resourcesUsed = DatabaseInterface.getResourcesUsed(componentName);
		
		String[] resourceNames = new String[0];
		if(resourcesUsed != null)
			resourceNames = resourcesUsed.keySet().toArray(resourceNames);
		
		for(int i = 0; resourcesUsed != null && i < resourcesUsed.size(); ++i)
		{
			String resourceName = resourceNames[i];
			Integer amountUsed = resourcesUsed.get(resourceName);
			fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li><strong>" + resourceName + ":</strong> " + amountUsed + " uses</li>\n");
		}
		
		if(resourcesUsed != null && resourcesUsed.size() == 0)
		{
			fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>None</li>\n");
		}
		
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "</ul>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
		
		fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarearight\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<h2>Feedback Scalars</h2>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<ul>\n");
		
		String[] inputScalars = DatabaseInterface.getInputPorts(componentName);
		int numFeedbacks = 0;
		for(int i = 0; i < inputScalars.length; ++i)
		{
			if(inputScalars[i].endsWith("_init"))
			{
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>" + inputScalars[i].substring(0, inputScalars[i].length() - 5) + "</li>\n");
				++numFeedbacks;
			}
		}
		
		if(numFeedbacks == 0)
			fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>None Found</li>\n");
		
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "</ul>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
		fileBuffer.append(tab + tab + tab + tab + "</div>\n");
		
		fileBuffer.append(tab + tab + tab + tab + "<div id=\"contentareasplit\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarealeft\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<h2>Ports Generated</h2>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarearight\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
		fileBuffer.append(tab + tab + tab + tab + "</div>\n");
		
		fileBuffer.append(tab + tab + tab + tab + "<div id=\"contentareasplit\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarealeft\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<strong><em>Input Scalars</em></strong>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<ul>\n");
		
		for(int i = 0; i < inputScalars.length; ++i)
		{
			fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>" + inputScalars[i] + "</li>\n"); 
		}
		
		if(inputScalars.length == 0)
			fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>None Found</li>\n");
		
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "</ul>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarearight\">\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<strong><em>Output Scalars</strong></em>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "<ul>\n");
		
		String[] outputScalars = DatabaseInterface.getOutputPorts(componentName);
		for(int i = 0; i < outputScalars.length; ++i)
		{
			fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>" + outputScalars[i] + "</li>\n"); 
		}
		
		if(outputScalars.length == 0)
			fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>None Found</li>\n");		
		
		fileBuffer.append(tab + tab + tab + tab + tab + tab + "</ul>\n");
		fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
		fileBuffer.append(tab + tab + tab + tab + "</div>\n");
		
		if(DatabaseInterface.getComponentType(componentName).equals("SYSTEM"))
		{	
			fileBuffer.append(tab + tab + tab + tab + "<div id=\"contentareasplit\">\n");
			fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarealeft\">\n");
			fileBuffer.append(tab + tab + tab + tab + tab + tab + "<strong><em>Input Stream Ports</strong></em>\n");
			fileBuffer.append(tab + tab + tab + tab + tab + tab + "<ul>\n");
			
			String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
			for(int i = 0; i < inputStreams.length; ++i)
			{
				String[] inputStreamPorts = DatabaseInterface.getStreamPorts(componentName, inputStreams[i]);
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>Stream " + inputStreams[i] + "\n");	
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<ul>\n");	
				for(int j = 0; j < inputStreamPorts.length; ++j)
				{
					fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + tab + "<li>" + inputStreamPorts[j] + "</li>\n");
				}
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "</ul>\n");	
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "</li>\n");	
			}
			
			fileBuffer.append(tab + tab + tab + tab + tab + tab + "</ul>\n");
			fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
			
			fileBuffer.append(tab + tab + tab + tab + tab + "<div id=\"contentarearight\">\n");
			fileBuffer.append(tab + tab + tab + tab + tab + tab + "<strong><em>Output Stream Ports</strong></em>\n");
			fileBuffer.append(tab + tab + tab + tab + tab + tab + "<ul>\n");
			
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			for(int i = 0; i < outputStreams.length; ++i)
			{
				String[] outputStreamPorts = DatabaseInterface.getStreamPorts(componentName, outputStreams[i]);
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<li>Stream " + outputStreams[i] + "\n");	
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "<ul>\n");	
				for(int j = 0; j < outputStreamPorts.length; ++j)
				{
					fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + tab + "<li>" + outputStreamPorts[j] + "</li>\n");
				}
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "</ul>\n");	
				fileBuffer.append(tab + tab + tab + tab + tab + tab + tab + "</li>\n");	
			}
			
			fileBuffer.append(tab + tab + tab + tab + tab + tab + "</ul>\n");
			fileBuffer.append(tab + tab + tab + tab + tab + "</div>\n");
			fileBuffer.append(tab + tab + tab + tab + "</div>\n");
		}
		
		fileBuffer.append(tab + tab + tab + "</div>\n");
		fileBuffer.append(tab + tab + "</div>\n");
	}
	
	static private void generateCompilationOptionsInfo(StringBuffer fileBuffer, String componentName)
	{
		fileBuffer.append(tab + tab + "<div id=\"sidebar\">\n");
		fileBuffer.append(tab + tab + tab + "<h2>Optimizations Used</h2>\n");
		fileBuffer.append(tab + tab + tab + "<p><strong>High-Level</strong></p>\n");
		fileBuffer.append(tab + tab + tab + "<ul>\n");
		
		String[] highLevelOpts = DatabaseInterface.getHighLevelOptsUsed(componentName);
		
		boolean isModule = false;
		
		if(highLevelOpts.length >= 2 && highLevelOpts[highLevelOpts.length - 2].equals("Export") && highLevelOpts[highLevelOpts.length - 1].equals("FullyUnroll"))
			isModule = true;
		
		for(int i = 0; i < highLevelOpts.length; ++i)
		{
			if(isModule && i >= highLevelOpts.length - 2)
				break;
				
			fileBuffer.append(tab + tab + tab + tab + "<li>" + highLevelOpts[i] + "</li>\n");
		}
		
		fileBuffer.append(tab + tab + tab + "</ul>\n");
		fileBuffer.append(tab + tab + tab + "<p><strong>Low-Level</strong></p>\n");
		fileBuffer.append(tab + tab + tab + "<ul>\n");
		
		String[] lowLevelOpts = DatabaseInterface.getLowLevelOptsUsed(componentName);
		
		for(int i = 0; i < lowLevelOpts.length; ++i)
		{
			fileBuffer.append(tab + tab + tab + tab + "<li>" + lowLevelOpts[i] + "</li>\n");
		}
		
		fileBuffer.append(tab + tab + tab + "</ul>\n");
		fileBuffer.append(tab + tab + tab + "<p><strong>Pipeline Retiming Values</strong></p>\n");
		fileBuffer.append(tab + tab + tab + "<ul>\n");
		
		String opsPerPipelineStage = DatabaseInterface.getOpsPerPipelineStageUsed(componentName);
		
		if(opsPerPipelineStage.equals("50000.0"))
			opsPerPipelineStage = "All in one";
		
		fileBuffer.append(tab + tab + tab + tab + "<li>Operations per stage attempted: " + opsPerPipelineStage + "</li>\n");
		fileBuffer.append(tab + tab + tab + "</ul>\n");
		
		if(DatabaseInterface.getComponentType(componentName).equals("SYSTEM"))
		{
			fileBuffer.append(tab + tab + tab + "<p><strong>Stream Management</strong></p>\n");
			fileBuffer.append(tab + tab + tab + "<ul>\n");
			
			String[] inputStreams = DatabaseInterface.getInputStreams(componentName);
			
			for(int i = 0; i < inputStreams.length; ++i)
			{
				fileBuffer.append(tab + tab + tab + tab + "<li>Stream " + inputStreams[i] + "\n");
				fileBuffer.append(tab + tab + tab + tab + "<ul>\n");
				fileBuffer.append(tab + tab + tab + tab + tab + "<li>" + DatabaseInterface.getNumStreamChannels(componentName, inputStreams[i]) + " channels</li>\n");
				//fileBuffer.append(tab + tab + tab + tab + tab + "<li>" +
				fileBuffer.append(tab + tab + tab + tab + "</ul>\n");
				fileBuffer.append(tab + tab + tab + tab + "</li>\n");
			}
			
			String[] outputStreams = DatabaseInterface.getOutputStreams(componentName);
			
			for(int i = 0; i < outputStreams.length; ++i)
			{
				fileBuffer.append(tab + tab + tab + tab + "<li>Stream " + outputStreams[i] + "\n");
				fileBuffer.append(tab + tab + tab + tab + "<ul>\n");
				fileBuffer.append(tab + tab + tab + tab + tab + "<li>" + DatabaseInterface.getNumStreamChannels(componentName, outputStreams[i]) + " channels</li>\n");
				fileBuffer.append(tab + tab + tab + tab + "</ul>\n");
				fileBuffer.append(tab + tab + tab + tab + "</li>\n");
			}
			
			fileBuffer.append(tab + tab + tab + "</ul>\n");
		}
		
		fileBuffer.append(tab + tab + tab + "<h2>Files Generated</h2>\n");
		fileBuffer.append(tab + tab + tab + "<ul>\n");
		
		String[] vhdlFilesGenerated = DatabaseInterface.getVHDLFilesGenerated(componentName);
		
		for(int i = 0; i < vhdlFilesGenerated.length; ++i)
		{
			fileBuffer.append(tab + tab + tab + tab + "<li>" + vhdlFilesGenerated[i] + "\n");
		}
		
		fileBuffer.append(tab + tab + tab + "</ul>\n");
		fileBuffer.append(tab + tab + "</div>\n");
		fileBuffer.append(tab + "</div>\n");
	}
	
	static private void generateFooter(StringBuffer fileBuffer, String componentName)
	{
		fileBuffer.append(tab + "<div id=\"footer\">\n");
		fileBuffer.append(tab + tab + "Compiled on <strong><em>" + DatabaseInterface.getDateCompiled(componentName) + "</em></strong> using compiler version <strong><em>" + DatabaseInterface.versionCompiledOn(componentName) + "</strong></em>\n");
		fileBuffer.append(tab + "</div>\n");
	}
	
	static private void genereateFootInformation(StringBuffer fileBuffer)
	{
		fileBuffer.append("</div>\n\n");
		
		fileBuffer.append("</body>\n");
		fileBuffer.append("</html>");
	}
}

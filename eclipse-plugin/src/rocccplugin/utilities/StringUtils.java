package rocccplugin.utilities;

import java.math.BigInteger;

public class StringUtils 
{
	public static String getNextStringValue(StringBuffer buf)
	{
		//Get the string version of the buffer
		String bufferString = buf.toString();
		bufferString = bufferString.trim();
		
		//Split the string into an array of strings by removing all the whitespaces.
		//Then empty the passed in buffer.
		String[] strings = null;
		strings = bufferString.split("\\s+");
		buf.delete(0, buf.length());
		buf.trimToSize();
		
		//If there is nothing in the strings array, return an empty string.
		if(strings == null || strings.length == 0)
			return "";
		
		//Refill the buffer with everything minus the string we just stripped off.
		String restOfBuffer = bufferString.replaceFirst("\\Q" + strings[0], "").replaceFirst("\\s+", "");
		buf.append(restOfBuffer);
		
		return strings[0];
	}	
	
	public static String getNextLine(StringBuffer buf)
	{
		//Get the string version of the buffer
		String bufferString = buf.toString();
		
		//Split the string into an array of strings by removing all the newlines.
		//Then empty the passed in buffer.
		String[] strings = bufferString.split("\\n+");
		buf.delete(0, buf.length());
		buf.trimToSize();
		
		//If there is nothing in the strings array, return an empty string.
		if(strings.length == 0)
			return "";
		
		//Refill the buffer with everything minus the string we just stripped off.
		String restOfBuffer = bufferString.replaceFirst("\\Q" + strings[0], "").replaceFirst("\\n+", "");
		buf.append(restOfBuffer);
		return strings[0];
	}	
	
	public static boolean isValidVariableName(String s)
	{
		if(s.length() == 0)
			return false;
		
		if(!Character.isLetter(s.charAt(0)) && s.charAt(0) != '_')
			return false;

		for(int i = 0; i < s.length(); ++i)
		{
			if(!Character.isLetter(s.charAt(i)) && !Character.isDigit(s.charAt(i)) && s.charAt(i) != '_')
				return false;
		}
		
		return true;
	}
	
	public static boolean isPositiveInt(String s)
	{
		boolean allZeros = true;
		
		if(s.length() == 0)
			return false;
		
		for(int i = 0; i < s.length(); ++i)
		{
			if(!Character.isDigit(s.charAt(i)))
				return false;
			if(s.charAt(i) != '0')
				allZeros = false;
		}
		
		return !allZeros;
	}
	
	public static boolean isANumber(String s)
	{
		int numDecimals = 0;
		int numMinus = 0;
		for(int i = 0; i < s.length(); ++i)
		{
			if(i == 0)
			{
				if(s.charAt(i) != '-' && s.charAt(i) != '.' && !Character.isDigit(s.charAt(i)))
					return false;
			}
			else
			{
				if(s.charAt(i) != '.' && !Character.isDigit(s.charAt(i)))
					return false;
			}
			
			if(s.charAt(i) == '-')
				++numMinus;
			else if(s.charAt(i) == '.')
				++numDecimals;
		}
		
		if(numMinus > 1 || numDecimals > 1)
			return false;
		
		return true;
	}
	
	public static boolean isAnInt(String s)
	{
		int numDecimals = 0;
		int numMinus = 0;
		for(int i = 0; i < s.length(); ++i)
		{
			if(i == 0)
			{
				if(s.charAt(i) != '-' && s.charAt(i) != '.' && !Character.isDigit(s.charAt(i)))
					return false;
			}
			else
			{
				if(s.charAt(i) != '.' && !Character.isDigit(s.charAt(i)))
					return false;
			}
			
			if(s.charAt(i) == '-')
				++numMinus;
			else if(s.charAt(i) == '.')
				++numDecimals;
		}
		
		if(numMinus > 1 || numDecimals > 0)
			return false;
		
		return true;
	}
	
	public static boolean isAHexValue(String s)
	{	
		//It must have at least 2 characters to be hex.
		if(s.length() < 2)
		{
			return false;
		}
		boolean firstCharIsX =  s.charAt(0) == 'X' || s.charAt(0) == 'x';
		boolean secondCharIsX = s.charAt(1) == 'X' || s.charAt(1) == 'x';
		
		//There can only be one X character for it to be hex.
		if(!(firstCharIsX ^ secondCharIsX))
		{
			return false;
		}
	
		//If the first character is not zero, the second value cannot be an X.
		if(s.charAt(0) != '0' && (secondCharIsX))
		{
			return false;
		}
		
		//The first character must be a zero or x.
		if(s.charAt(0) != '0' && !firstCharIsX)
		{
			return false;
		}
		
		for(int i = 1; i < s.length(); ++i)
		{
			if(i == 1 && secondCharIsX)
				continue;
			if(!Character.isDigit(s.charAt(i)) && (Character.toLowerCase(s.charAt(i)) < 'a' || Character.toLowerCase(s.charAt(i)) > 'f')) 
			{
				return false;
			}
		}
		return true;
	}
	
	public static String numberToBinary(String s, int bitsize, String type)
	{
		String newString = s;
		if(type.contains("float"))
		{
			if(isAnInt(newString))
			{
				newString = newString + ".0";
			}
		}
		else
		{
			if(!isAnInt(newString) && !isAHexValue(newString))
			{
				newString = newString.substring(0, newString.indexOf("."));
				if(newString.length() == 0)
					newString = "0";
				if(newString.equals("-"))
					newString = "0";
			}
		}
		
		return numberToBinary(newString, bitsize);
	}
	
	public static String numberToBinary(String s, int bitSize)
	{
		String binary;
		//If the value is a float value, convert it to the integer representation of the float bits.
		if(s.contains("."))
		{
			if(bitSize == 32)
				binary = Integer.toBinaryString(Float.floatToRawIntBits(Float.parseFloat(s)));
			else if(bitSize == 64)
				binary = Long.toBinaryString(Double.doubleToRawLongBits(Double.parseDouble(s)));
			else
			{
				int f = Float.floatToRawIntBits(Float.parseFloat(s));
				short half = (short) (((f>>16)&0x8000)|((((f&0x7f800000)-0x38000000)>>13)&0x7c00)|((f>>13)&0x03ff));
				int intHalf = (int)half & 0x0000FFFF;
				binary = Integer.toBinaryString(intHalf);
			}
		}
		else
		{	
			BigInteger bigInt;
			//If it is an int value
			if(isANumber(s))
				bigInt = new BigInteger(s);
			//If it is a hex value
			else if(isAHexValue(s))
				bigInt = new BigInteger(s.toUpperCase().split("X")[1], 16);
			else
				return "";
			
			binary = bigInt.toString(2);
		}
		
		//If the binary leads with a negative and s is an integer
		if(binary.charAt(0) == '-' && isPositiveInt(s.substring(1)))
		{
			//Flip the 0 and 1 bits
			binary = binary.substring(1);
			binary = binary.replace('0', '2');
			binary = binary.replace('1', '0');
			binary = binary.replace('2', '1');
			
			for(int i = binary.length() - 1; i >= 0; --i)
			{
				if(binary.charAt(i) == '0')
				{
					char[] string = binary.toCharArray();
					string[i] = '1';
					binary = new String(string);
					break;
				}
				else
				{
					char[] string = binary.toCharArray();
					string[i] = '0';
					binary = new String(string);
				}
			}
			
			binary = "1" + binary;
		}
		else if(isPositiveInt(s)) 
		{
			binary = "0" + binary;
		}
		
		//If the value passed in is equal to zero, just make is zero and the the padding to the rest.
		if(!isAHexValue(s) && Float.parseFloat(s) == 0)
		{
			binary = "0";
		}
		
		//If the binary string is shorter than the desired bitSize, pad the string.
		while(binary.length() < bitSize)
		{
			if(isAnInt(s))
				binary = binary.charAt(0) + binary;
			else
				binary = "0" + binary;
		}
		
		//If the binary string is longer than the desired bitSize, cut the string.
		while(binary.length() > bitSize)
			binary = binary.substring(1);
		
		//Change -0 in float to begin with a 1 in binary.
		if(!isAHexValue(s) && !isAnInt(s) && Float.parseFloat(s) == 0 && s.startsWith("-"))
		{
			binary = binary.replaceFirst("0", "1");
		}
		
		return binary;
	}
	
	public static String addEscapeCharactersForSpaces(String s)
	{
		String outputString = "";
		
		for(int i = 0; i < s.length(); ++i)
		{
			if(s.charAt(i) == ' ')
				outputString = outputString.concat("\\");
			outputString = outputString.concat(s.substring(i, i + 1));
		}
		
		return outputString;
	}
	
	static public boolean isCPlusPlusReservedWord(String s)
	{
		return s.equals("or") 		   || s.equals("and")      || s.equals("static") || 
			   s.equals("char") 	   || s.equals("float")    || s.equals("int") ||
			   s.equals("auto") 	   || s.equals("const")    || s.equals("double") ||
			   s.equals("short")       || s.equals("struct")   || s.equals("unsigned") ||
			   s.equals("break") 	   || s.equals("continue") || s.equals("else") ||
			   s.equals("for") 		   || s.equals("long")     || s.equals("signed") || 
			   s.equals("switch") 	   || s.equals("void")     || s.equals("case") || 
			   s.equals("default")     || s.equals("enum")     || s.equals("goto") || 
			   s.equals("register")    || s.equals("sizeof")   || s.equals("typedef") ||
			   s.equals("volatile")    || s.equals("do") 	   || s.equals("while") ||
			   s.equals("extern") 	   || s.equals("if") 	   || s.equals("return") || 
			   s.equals("union") 	   || s.equals("asm") 	   || s.equals("dynamic_cast") ||
			   s.equals("namespace")   || s.equals("try") 	   || s.equals("reinterpret_cast") || 
			   s.equals("catch") 	   || s.equals("bool")     || s.equals("class") || 
			   s.equals("const_cast")  || s.equals("delete")   || s.equals("new") ||
			   s.equals("explicit")    || s.equals("false")    || s.equals("friend") ||
			   s.equals("inline")      || s.equals("mutable")  || s.equals("operator") ||
			   s.equals("private")     || s.equals("public")   || s.equals("protected") ||
			   s.equals("static_cast") || s.equals("template") || s.equals("this") ||
			   s.equals("throw") 	   || s.equals("true") 	   || s.equals("typeid") ||
			   s.equals("typename")    || s.equals("using")    || s.equals("virtual") ||
			   s.equals("wchar_t") 	   || s.equals("bitand")   || s.equals("compl") ||
			   s.equals("not_eq") 	   || s.equals("or_eq")    || s.equals("xor_eq") ||
			   s.equals("and_eq") 	   || s.equals("bitor")    || s.equals("not") || 
			   s.equals("xor") 		   || s.equals("cin") 	   || s.equals("endl") || 
			   s.equals("null")        || s.equals("INT_MIN")  || s.equals("INT_MAX") || 
			   s.equals("iomanip")     || s.equals("main")     || s.equals("npos") ||
			   s.equals("std") 		   || s.equals("cout")     || s.equals("include") || 
			   s.equals("iostream")    || s.equals("MAX_RAND") || s.equals("string") ||
			   s.equals("atexit");
	}
	
	static public boolean isROCCCReservedWord(String s)
	{
		return s.startsWith("ROCCC") || s.startsWith("suifTmp") || s.equals("StreamHandler") ||
			   s.equals("roccc-library") || s.equals("ROCCCHelper");
	}
	
	static public String convertSpacesToURLForm(String s)
	{
		return s.replace(" ", "%20");
	}
}

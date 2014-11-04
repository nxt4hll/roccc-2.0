#include "common/system_specific.h"
/**
  * A .suif file contains the serialization of two objects:
  * an object factory then a FileSetBlock.
  */

#include "iokernel/aggregate_meta_class.h"
#include "iokernel/pointer_meta_class.h"
#include "iokernel/metaclass_visitor.h"
#include "iokernel/clone_stream.h"
#include "iokernel/binary_streams.h"
#include "iokernel/object_factory.h"
#include "iokernel/synchronizer.h"

#include "suif_clone_stream.h"
#include "io_subsystem_default_impl.h"
#include "suif_env.h"
#include "suif_object.h"
#include "suif_exception.h"

#include <fstream>
using namespace std;

/**
 * \class SuifCompatibleHeaders
 *
 * This class helps to waste some space at the beginning
 * of an file written out to disk in order to allow
 * suif1 passes to recognize that this file format is
 * not supported.
 *
 * In addition, it checks to see that the file is
 * a suif version 2 file and gives nice error messages
 * if the user tries to read a SUIF1 file.
 *
 */

class SuifCompatibleHeaders {
public:

  static String get_SUIF_magic_word() {
    const char SUIF_magic_word[] = {0x70, 0x69, 0x67, 0x73, 0x0};
    String magic(SUIF_magic_word);
    return(magic);
  }
  static String get_SUIF_version() {
    return("2.0.0");
  }
  static String get_SUIF_system() {
    return("2.0.0.0");
  }


  static void write_string(ostream &outputFile, const String &str)
  {
    // This is the format of a suif1 string. write the number of characters
    // we use "big" endian 32 bit form
    size_t length = str.length() + 1; // count the '\0'
    size_t padded_length = get_padded_size(length);
    assert(length < 256);
    outputFile.put('\0');
    outputFile.put('\0');
    outputFile.put('\0');
    outputFile.put((char)length);
    outputFile.write(str.c_str(), length);
    for (size_t i = length; i < padded_length; i++)
      {
	outputFile.put('\0');
      }
  }

  static void write_chunk(ostream &outputFile, String chunk)
  {
    for (int i = 0; i< chunk.size(); i++)
      outputFile.put(chunk.c_str()[i]);
  }

  static String read_chunk(istream &inputFile, size_t len)
  {
    String chunk;
    for (size_t i = 0; i< len; i++)
      {
	suif_assert_message(!inputFile.eof(),
			    ("Early end of file"));
	char val = inputFile.get();
	chunk.push(val);
	//fprintf(stderr, "magic=%s\n", chunk.c_str());
      }
    return(chunk);
  }

  static void write_header(ostream &outputFile)
  {
    String magic_word = get_SUIF_magic_word();
    String version = get_SUIF_version();
    String system = get_SUIF_system();
    
    // This header format is used so the SUIF1 files will recognize the
    // incompatible format.
    write_chunk(outputFile, magic_word);
    write_string(outputFile, version);
    write_string(outputFile, system);
  }
  static size_t get_padded_size(size_t size)
  {
    return (((size + 3) / 4) * 4);
  }

  static String read_string(istream &inputFile)
  {
    String str;
    size_t size = 0;
    for (int i=0; i<4; i++)
      {
	suif_assert_message(!inputFile.eof(), ("Early end of file"));
	char val = inputFile.get();
	//	fprintf(stderr, "val=%d\n", val);
	size = size * 256 + val;
      }
    //    fprintf(stderr, "size=%s\n", size.to_String().c_str());
    //    fflush(stderr);
    size_t padded_size = get_padded_size(size);
    size_t bytes_read;
    for (bytes_read = 0; bytes_read < size-1; bytes_read++)
      {
	suif_assert_message(!inputFile.eof(), ("Early end of file"));
	char val = inputFile.get();
	str.push(val);
      }
    for (; bytes_read < padded_size; bytes_read++)
      {
	suif_assert_message(!inputFile.eof(), ("Early end of file"));
	char val = inputFile.get();
	suif_assert_message(val == 0, ("Expected 0 padding"));
      }
    return(str);
  }


  static bool read_header(istream &inputFile)
  {
    String SUIF_magic_word = get_SUIF_magic_word();

    {
      // This is all just a silly compatibility mode
      // The alpha releases of the SUIF2 file format
      // had no 
      char peek_char = inputFile.peek();

      switch(peek_char) {
      case 0x50:
      case 0x73:
      case 0x70:
      case 0x66:
	break;
      default:
	{
	  suif_warning("\nThis file does not appear to be a SUIF file.\n"
		       "Attempting to read it as a SUIF version 2 alpha file\n\n");
	  return(true);
	}
      }
    }
    String chunk = read_chunk(inputFile, 4);
    //fprintf(stderr, "magic=%s vs %s\n", chunk.c_str(), SUIF_magic_word.c_str());
    //fflush(stderr);
    if (chunk != SUIF_magic_word)
      {
	char SUIF_magic_word[] = {0x50, 0x69, 0x67, 0x73, 0x0};
	char SUIF_reverse_magic_word[] = {0x73, 0x67, 0x69, 0x50, 0x0};
	char SUIF_32_magic_word[] = {0x70, 0x69, 0x67, 0x73, 0x0};
	char SUIF_32_reverse_magic_word[] = {0x73, 0x67, 0x69, 0x70, 0x0};
	char oldsuif_magic_word[] = {0x73, 0x75, 0x69, 0x66, 0x0};
	char oldsuif_reverse_magic_word[] = {0x66, 0x69, 0x75, 0x73, 0x0};
	if (chunk == String(SUIF_magic_word)
	    || chunk == String(SUIF_reverse_magic_word)
	    || chunk == String(SUIF_32_magic_word)
	    || chunk == String(SUIF_32_reverse_magic_word))
	  {
	    suif_assert_message(0, 
				("\nThis file is in SUIF version 1 file format.\n"
				 "This pass uses the SUIF2 file format.\n"
				 "The file must go through ``convertsuif1to2'' "
				 "before this pass may be used\n"));
	  }
	if (chunk == String(oldsuif_magic_word)
	    || chunk == String(oldsuif_reverse_magic_word))
	  {
	    suif_assert_message(0,
				("\nThis file is in the oldsuif file format.\n"
				 "this pass uses the SUIF2 file format.\n"
				 "The file must go through ``newsuif'' and "
				 "``convertsuif1to2'' "
				 "before this pass may be used\n"));
	  }
	suif_assert_message(0,
			    ("This file is not a valid SUIF file."));
      }

    String version = read_string(inputFile);
    if (version != get_SUIF_version())
      {
	suif_assert_message(0,
			    ("\nThis file is in SUIF version %s file format.\n"
			     "this pass requires the SUIF version %s file "
			     "format.\n"
			     "The file must go through ``convertsuif1to2'' "
			     "before this pass may be used\n",
			     version.c_str(),
			     get_SUIF_version().c_str()
			     ));
      }

    String system = read_string(inputFile);
    return(true);
  }
  
};




InputSubSystemDefaultImplementation::InputSubSystemDefaultImplementation( SuifEnv* suif_env ) :
  InputSubSystem( suif_env ) {
}


FileSetBlock *InputSubSystemDefaultImplementation::read( const String& inputFileName ) {
  FileSetBlock* file_set_block = 0;
  ObjectFactory* fileOF = 0 ;
  ObjectFactory* _object_factory = _suif_env->get_object_factory();

  // create the InputStream
  //#if defined(PGI_BUILD) || defined(MSVC)
  //  ifstream inputFile( inputFileName.c_str(), ios::in |ios::binary | ios::nocreate );
//#else
  ifstream inputFile( inputFileName.c_str(), ios::in | ios::binary );
  //#endif
  if (inputFile.fail()) {
    SUIF_THROW(SuifException(String("Could not read file ") + inputFileName + "\n"));
  }

  if (SuifCompatibleHeaders::read_header(inputFile) != true)
    {
      SUIF_THROW(SuifException(String("Invalid format for file ") + inputFileName + "\n"));
    }

  InputStream* inputStream = new BinaryInputStream( _object_factory, inputFile );

  // add the MetaClasses that are always known (they are not written out)
  _object_factory->get_rudimentary_address_map()->add_to_stream( inputStream );


  // read in all the other MetaClasses
  MetaClass* metaClassObjectFactoryMC = _object_factory->lookupMetaClass( "ObjectFactory" );
  fileOF = (ObjectFactory*)inputStream->read_object( metaClassObjectFactoryMC );
  inputStream->read_close();
  // synchronize the MetaClasses just read in with the ones already in the system
  Synchronizer sync(_object_factory);
  sync.synchronize( _object_factory, fileOF, inputStream );

  // read in the intermediate representation
  MetaClass* objectMC = _object_factory->lookupMetaClass( Object::get_class_name() );
  PointerMetaClass* objectMCPO =  _object_factory->get_pointer_meta_class( objectMC , true );
  inputStream->read_object( objectMCPO, (Address)&file_set_block );
  inputStream->read_close();

  delete inputStream;
  delete fileOF;
  return file_set_block;
}


OutputSubSystemDefaultImplementation::OutputSubSystemDefaultImplementation( SuifEnv* suif_env ) :
  OutputSubSystem( suif_env ) {
}



void OutputSubSystemDefaultImplementation::write( const String& outputFileName ) {
  ObjectFactory* _object_factory = _suif_env->get_object_factory();
  FileSetBlock* rootNode = _suif_env->get_file_set_block();


  //#if defined(PGI_BUILD) || defined(MSVC)
  //ofstream outputFile( outputFileName.c_str(), ios::out  |ios::binary);
  //#else
  ofstream outputFile( outputFileName.c_str(), ios::out | ios::binary );
  //#endif
  if (outputFile.bad()) {
    SUIF_THROW(SuifException(String("Could not open ") + outputFileName + "\n"));
  }
  OutputStream* outputStream = new BinaryOutputStream( outputFile );

  SuifCompatibleHeaders::write_header(outputFile);

  _object_factory->get_rudimentary_address_map() -> add_to_stream( outputStream );
  outputStream->write_object( ObjectWrapper(_object_factory, 
					    _object_factory->get_meta_class()) );

  MetaClass* objectMC =
       _object_factory->lookupMetaClass( Object::get_class_name() );
  PointerMetaClass* rootNodeMetaClass =
       _object_factory->get_pointer_meta_class( objectMC, true );
  outputStream->write_object( ObjectWrapper(&rootNode, 
					    rootNodeMetaClass) );

  outputStream->write_close();
  delete outputStream;
  outputFile.close();

}


CloneSubSystemDefaultImplementation::CloneSubSystemDefaultImplementation( SuifEnv* suif_env ) :
  CloneSubSystem( suif_env ),
  _deep_stream(0),
  _shallow_stream(0) {
}

CloneSubSystemDefaultImplementation::~CloneSubSystemDefaultImplementation()
{
  delete _deep_stream;
  delete _shallow_stream;
}


CloneStream* CloneSubSystemDefaultImplementation::get_deep_clone_stream() {
    return _deep_stream;
    }


CloneStream* CloneSubSystemDefaultImplementation::get_shallow_clone_stream() {
    return _shallow_stream;
    }

void CloneSubSystemDefaultImplementation::set_deep_clone_stream(CloneStream *str) {
    _deep_stream = str;
    }


void CloneSubSystemDefaultImplementation::set_shallow_clone_stream(CloneStream *str) {
    _shallow_stream = str;
    }

































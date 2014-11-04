#ifndef SUIFKERNEL__FORWARDERS_H
#define SUIFKERNEL__FORWARDERS_H

#include "iokernel/iokernel_forwarders.h"

#define sf_owned

// ------ suifkernel --------

typedef unsigned int s_count_t;
typedef suif_vector<bool> bit_vector;

class SuifEnv;
class SuifObject;
class RealObjectFactory;
class Module;
class VisitorMap;
class WalkingMaps;

class InputSubSystem;
class OutputSubSystem;
class CloneSubSystem;
class ModuleSubSystem;
class ErrorSubSystem;
class DLLSubSystem;
class PrintSubSystem;
class TokenStream;
class IStreamTokenStream;
class WalkingMapsSubsystem;

struct Token;
class Option;
class OptionString;
class OptionInt;
class OptionLiteral;
class OptionSelection;
class OptionList;
class OptionLoop;
class OptionStream;

class ValueClass;
class StructureValueClass;
class StringValueClass;
class IntValueClass;
class StreamValueClass;
class SuifCloneStream;
struct OptionDescription;


class TypeBuilder;
#ifdef SUIF_NAMESPACE
namespace SUIF_NAMESPACE {
#endif
class SymbolTable;
class FileSetBlock;
class FileBlock;
class ProcedureDefinition;
class VariableDefinition;
class DefinitionBlock;
#ifdef SUIF_NAMESPACE
}
#ifndef NO_IMPLICIT_USING
using namespace SUIF_NAMESPACE;
#endif
#endif

class FormattedText;
template < class T > class CascadingMap;

#include "suifkernel_messages.h"

#endif






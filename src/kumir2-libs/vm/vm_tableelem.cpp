#include "vm_tableelem.hpp"
#include <sstream>


namespace Bytecode
{

static void stdStringToDataStream(std::list<char> &stream, const std::string &str)
{
	uint16_t size = uint16_t(str.length());
	valueToDataStream(stream, size);
	for (int i = 0; i < (int)str.length(); i++) {
		stream.push_back(str[i]);
	}
}

static void stringToDataStream(std::list<char> &stream, const String &str)
{
	Kumir::EncodingError encodingError;
	const std::string utf = Kumir::Coder::encode(Kumir::UTF8, str, encodingError);
	stdStringToDataStream(stream, utf);
}

static void stdStringFromDataStream(std::list<char> &stream, std::string &str)
{
	uint16_t u16size;
	valueFromDataStream(stream, u16size);
	size_t size = size_t(u16size);
	str.resize(size);
	for (size_t i = 0; i < size; i++) {
		str[i] = stream.front();
		stream.pop_front();
	}
}

static void stringFromDataStream(std::list<char> &stream, String &str)
{
	std::string utf;
	stdStringFromDataStream(stream, utf);
	Kumir::EncodingError encodingError;
	str = Kumir::Coder::decode(Kumir::UTF8, utf, encodingError);
}

static void scalarConstantToDataStream(std::list<char> &stream, ValueType type, const VM::AnyValue &val)
{
	switch (type) {
	case VT_int: {
		const int32_t ival = val.toInt();
		valueToDataStream(stream, ival);
		break;
	}
	case VT_real: {
		const double rval = val.toReal();
		valueToDataStream(stream, rval);
		break;
	}
	case VT_bool: {
		const uint8_t bval = val.toBool() ? 1 : 0;
		valueToDataStream(stream, bval);
		break;
	}
	case VT_char: {
		const String &cval = val.toString();
		stringToDataStream(stream, cval);
		break;
	}
	case VT_string: {
		const String &sval = val.toString();
		stringToDataStream(stream, sval);
		break;
	}
	default:
		break;
	}
}

static void scalarConstantToDataStream(
	std::list<char> &stream,
	const std::list<ValueType> &type,
	const VM::Variable &val
) {
	if (type.front() != VT_record) {
		scalarConstantToDataStream(stream, type.front(), val.value());
	} else {
		const VM::Record value = val.value().toRecord();
		for (size_t i = 0; i < value.fields.size(); i++) {
			const VM::AnyValue &field = value.fields[i];
			scalarConstantToDataStream(stream, field.type(), field);
		}
	}
}

static void constantToDataStream(
	std::list<char> &stream,
	const std::list<ValueType> &baseType,
	const VM::Variable &val,
	uint8_t dimension
) {
	if (dimension == 0) {
		scalarConstantToDataStream(stream, baseType, val);
	} else {
		int bounds[7];
		val.getBounds(bounds);
		for (int i = 0; i < 7; i++) {
			int32_t bound32 = static_cast<int32_t>(bounds[i]);
			valueToDataStream(stream, bound32);
		}
		int32_t rawSize = val.rawSize();
		valueToDataStream(stream, rawSize);
		for (size_t i = 0; i < val.rawSize(); i++) {
			VM::AnyValue v = val[i];
			uint8_t defined = v.isValid() ? 1 : 0;
			valueToDataStream(stream, defined);
			if (defined == 1) {
				scalarConstantToDataStream(stream, baseType, VM::Variable(v));
			}
		}
	}
}

static void vtypeToDataStream(std::list<char> &ds, const std::list<ValueType> &vtype)
{
	valueToDataStream(ds, uint8_t(vtype.front()));
	if (vtype.front() == VT_record) {
		valueToDataStream(ds, uint32_t(vtype.size() - 1));
		std::list<ValueType>::const_iterator it = vtype.begin();
		it ++;
		for (; it != vtype.end(); ++it) {
			valueToDataStream(ds, uint8_t(*it));
		}
	}
}

static void vtypeFromDataStream(std::list<char> &ds, std::list<ValueType> &vtype)
{
	uint8_t u8;
	valueFromDataStream(ds, u8);
	ValueType first = ValueType(u8);
	vtype.clear();
	vtype.push_back(first);
	if (first == VT_record) {
		uint32_t sz;
		valueFromDataStream(ds, sz);
		for (uint32_t i = 0; i < sz; i++) {
			valueFromDataStream(ds, u8);
			first = ValueType(u8);
			vtype.push_back(first);
		}
	}
}

void tableElemToBinaryStream(std::list<char> &ds, const TableElem &e)
{
	valueToDataStream(ds, uint8_t(e.type));
	vtypeToDataStream(ds, e.vtype);
	valueToDataStream(ds, uint8_t(e.dimension));
	valueToDataStream(ds, uint8_t(e.refvalue));
	valueToDataStream(ds, uint8_t(e.module));
	if (e.type == EL_EXTERN) {
		String ma = Kumir::Core::fromAscii(e.moduleAsciiName);
		stringToDataStream(ds, ma);
		stringToDataStream(ds, e.moduleLocalizedName);
		stringToDataStream(ds, e.fileName);
		stringToDataStream(ds, e.signature);
	}
	if (e.type == EL_EXTERN_INIT) {
		String ma = Kumir::Core::fromAscii(e.moduleAsciiName);
		stringToDataStream(ds, ma);
		stringToDataStream(ds, e.moduleLocalizedName);
		stringToDataStream(ds, e.fileName);
	}
	if (e.type == EL_FUNCTION || e.type == EL_MAIN) {
		stringToDataStream(ds, e.signature);
	}
	valueToDataStream(ds, uint16_t(e.algId));
	valueToDataStream(ds, uint16_t(e.id));
	stringToDataStream(ds, e.name);
	Kumir::EncodingError encodingError;
	String mods = Kumir::Coder::decode(Kumir::ASCII, e.moduleAsciiName, encodingError);
	stringToDataStream(ds, mods);
	stringToDataStream(ds, e.moduleLocalizedName);
	if (e.type == EL_GLOBAL || e.type == EL_LOCAL || e.type == EL_CONST) {
		String ms = Kumir::Coder::decode(Kumir::ASCII, e.recordModuleAsciiName, encodingError);
		stringToDataStream(ds, ms);
		stringToDataStream(ds, e.recordModuleLocalizedName);
		String us = Kumir::Coder::decode(Kumir::ASCII, e.recordClassAsciiName, encodingError);
		stringToDataStream(ds, us);
		stringToDataStream(ds, e.recordClassLocalizedName);

	}
	if (e.type == EL_CONST) {
		constantToDataStream(ds, e.vtype, e.initialValue, e.dimension);
	} else if (e.type == EL_FUNCTION || e.type == EL_MAIN || e.type == EL_TESTING || e.type == EL_BELOWMAIN || e.type == EL_INIT) {
		valueToDataStream(ds, uint16_t(e.instructions.size()));
		for (int i = 0; i < (int)e.instructions.size(); i++) {
			valueToDataStream(ds, toUint32(e.instructions[i]));
		}
	}
}

static void scalarConstantFromDataStream(
	std::list<char> &stream,
	ValueType type, VM::AnyValue &val
) {
	switch (type) {
	case VT_int: {
		int32_t ival;
		valueFromDataStream(stream, ival);
		val = VM::AnyValue(ival);
		break;
	}
	case VT_real: {
		double rval;
		valueFromDataStream(stream, rval);
		val = VM::AnyValue(rval);
		break;
	}
	case VT_bool: {
		uint8_t bval;
		valueFromDataStream(stream, bval);
		val = VM::AnyValue(bool(bval > 0 ? true : false));
		break;
	}
	case VT_char: {
		String cval;
		stringFromDataStream(stream, cval);
		val = VM::AnyValue(Kumir::Char(cval.at(0)));
		break;
	}
	case VT_string: {
		Kumir::String sval;
		stringFromDataStream(stream, sval);
		val = VM::AnyValue(sval);
		break;
	}
	default:
		break;
	}
}

static void scalarConstantFromDataStream(
	std::list<char> &stream,
	const std::list<ValueType> &type,
	VM::AnyValue &val
) {
	if (type.front() != VT_record) {
		scalarConstantFromDataStream(stream, type.front(), val);
	} else {
		VM::Record record;
		std::list<ValueType>::const_iterator it = type.begin();
		it ++;
		for (; it != type.end(); ++it) {
			VM::AnyValue field;
			scalarConstantFromDataStream(stream, *it, field);
			record.fields.push_back(field);
		}
		val = VM::AnyValue(record);
	}
}

static void constantFromDataStream(
	std::list<char> &stream,
	const std::list<ValueType> &baseType,
	VM::Variable &val,
	uint8_t dimension
) {
	if (dimension == 0) {
		VM::AnyValue value;
		scalarConstantFromDataStream(stream, baseType, value);
		val.setBaseType(baseType.size() == 1 ? baseType.front() : VT_record);
		val.setValue(value);
	} else {
		val.setDimension(dimension);
		int bounds[7];
		for (int i = 0; i < 7; i++) {
			int32_t bounds32;
			valueFromDataStream(stream, bounds32);
			bounds[i] = static_cast<int>(bounds32);
		}
		val.setBounds(bounds);
		val.init();
		int32_t rawSize32;
		valueFromDataStream(stream, rawSize32);
		size_t rawSize = static_cast<size_t>(rawSize32);
		for (size_t i = 0; i < rawSize; i++) {
			uint8_t defined;
			valueFromDataStream(stream, defined);
			if (defined == 1) {
				VM::AnyValue element;
				scalarConstantFromDataStream(stream, baseType, element);
				val[i] = element;
			}
		}
	}
}

void tableElemFromBinaryStream(std::list<char> &ds, TableElem &e)
{
	uint8_t t;
	uint8_t d;
	uint8_t r;
	uint8_t m;
	uint16_t a;
	uint16_t id;
	String s;
	valueFromDataStream(ds, t);
	e.type = ElemType(t);
	vtypeFromDataStream(ds, e.vtype);
	valueFromDataStream(ds, d);
	e.dimension = d;
	valueFromDataStream(ds, r);
	e.refvalue = ValueKind(r);
	valueFromDataStream(ds, m);
	e.module = m;
	Kumir::EncodingError encodingError;
	if (e.type == EL_EXTERN) {
		String ma;
		stringFromDataStream(ds, ma);
		e.moduleAsciiName = Kumir::Coder::encode(Kumir::ASCII, ma, encodingError);
		stringFromDataStream(ds, e.moduleLocalizedName);
		stringFromDataStream(ds, e.fileName);
		stringFromDataStream(ds, e.signature);
	}
	if (e.type == EL_EXTERN_INIT) {
		String ma;
		stringFromDataStream(ds, ma);
		e.moduleAsciiName = Kumir::Coder::encode(Kumir::ASCII, ma, encodingError);
		stringFromDataStream(ds, e.moduleLocalizedName);
		stringFromDataStream(ds, e.fileName);
	}
	if (e.type == EL_FUNCTION || e.type == EL_MAIN) {
		stringFromDataStream(ds, e.signature);
	}
	valueFromDataStream(ds, a);
	e.algId = a;
	valueFromDataStream(ds, id);
	e.id = id;
	stringFromDataStream(ds, s);
	e.name = s;
	stringFromDataStream(ds, s);
	e.moduleAsciiName = Kumir::Coder::encode(Kumir::ASCII, s, encodingError);
	stringFromDataStream(ds, s);
	e.moduleLocalizedName = s;
	if (e.type == EL_GLOBAL || e.type == EL_LOCAL || e.type == EL_CONST) {
		String ms;
		stringFromDataStream(ds, ms);
		e.recordModuleAsciiName = Kumir::Coder::encode(Kumir::ASCII, ms, encodingError);
		stringFromDataStream(ds, e.recordModuleLocalizedName);
		String us;
		stringFromDataStream(ds, us);
		e.recordClassAsciiName = Kumir::Coder::encode(Kumir::ASCII, us, encodingError);
		stringFromDataStream(ds, e.recordClassLocalizedName);
	}
	if (e.type == EL_CONST) {
		constantFromDataStream(ds, e.vtype, e.initialValue, e.dimension);
	} else if (e.type == EL_FUNCTION || e.type == EL_MAIN || e.type == EL_TESTING || e.type == EL_BELOWMAIN || e.type == EL_INIT) {
		uint16_t u16sz;
		valueFromDataStream(ds, u16sz);
		size_t sz = size_t(u16sz);
		e.instructions.resize(sz);
		for (size_t i = 0; i < sz; i++) {
			uint32_t instr;
			valueFromDataStream(ds, instr);
			e.instructions[i] = fromUint32(instr);
		}
	}
}

static std::string elemTypeToString(ElemType t)
{
	if (t == EL_LOCAL) {
		return ".local";
	} else if (t == EL_GLOBAL) {
		return ".global";
	} else if (t == EL_CONST) {
		return ".constant";
	} else if (t == EL_FUNCTION) {
		return ".function";
	} else if (t == EL_EXTERN) {
		return ".extern";
	} else if (t == EL_INIT) {
		return ".init";
	} else if (t == EL_MAIN) {
		return ".main";
	} else if (t == EL_BELOWMAIN) {
		return ".belowmain";
	} else if (t == EL_TESTING) {
		return ".testing";
	} else  {
		return "";
	}
}

extern inline ElemType elemTypeFromString(const std::string &ss)
{
	const std::string s = Kumir::Core::toLowerCase(ss);
	if (s == ".local") {
		return EL_LOCAL;
	} else if (s == ".global") {
		return EL_GLOBAL;
	} else if (s == ".constant") {
		return EL_CONST;
	} else if (s == ".function") {
		return EL_FUNCTION;
	} else if (s == ".extern") {
		return EL_EXTERN;
	} else if (s == ".init") {
		return EL_INIT;
	} else if (s == ".main") {
		return EL_MAIN;
	} else if (s == ".belowmain") {
		return EL_BELOWMAIN;
	} else if (s == ".testing") {
		return EL_TESTING;
	} else {
		return ElemType(0);
	}
}


static std::string vtypeToString(const std::list<ValueType> &type, uint8_t dim)
{
	std::string result;
	ValueType t = type.front();
	if (t == VT_int) {
		result = "int";
	} else if (t == VT_real) {
		result = "real";
	} else if (t == VT_char) {
		result = "char";
	} else if (t == VT_string) {
		result = "string";
	} else if (t == VT_bool) {
		result = "bool";
	} else if (t == VT_record) {
		result = "record{";
		std::list<ValueType>::const_iterator it = type.begin();
		it ++;
		std::list<ValueType>::const_iterator itt;
		for (; it != type.end(); ++it) {
			t = *it;
			if (t == VT_int) {
				result += "int";
			} else if (t == VT_real) {
				result += "real";
			} else if (t == VT_char) {
				result += "char";
			} else if (t == VT_string) {
				result += "string";
			} else if (t == VT_bool) {
				result += "bool";
			}
			itt = it;
			itt++;
			if (itt != type.end()) {
				result += ",";
			}
		}
		result += "}";
	} else {
		result = "";
	}
	if (result.length() > 0) {
		for (uint8_t i = 0; i < dim; i++) {
			result += "[]";
		}
	}
	return result;
}

extern inline  void vtypeFromString(
	const std::string &ss,
	std::list<ValueType> &type,
	uint8_t &dim
) {
	size_t brackPos = ss.find_first_of('[');
	std::string s = Kumir::Core::toLowerCase(ss.substr(0, brackPos));
	Kumir::StringUtils::trim<std::string, char>(s);
	ValueType t;
	if (s == "int") {
		t = VT_int;
	} else if (s == "real") {
		t = VT_real;
	} else if (s == "char") {
		t = VT_char;
	} else if (s == "string") {
		t = VT_string;
	} else if (s == "bool") {
		t = VT_bool;
	} else if (s.length() >= std::string("record").length()) {
		t = VT_record;
	} else {
		t = VT_void;
	}
	dim = 0;
	if (brackPos != std::string::npos) {
		std::string brackets = s.substr(brackPos);
		for (size_t i = 0; i < brackets.length(); i++) {
			if (brackets[i] == '[') {
				dim++;
			}
		}
	}
	type.clear();
	type.push_back(t);
	if (t == VT_record) {
		// TODO implement me!
	}
}


static std::string kindToString(ValueKind k)
{
	if (k == VK_Plain) {
		return "var";
	} else if (k == VK_In) {
		return "in";
	} else if (k == VK_InOut) {
		return "inout";
	} else if (k == VK_Out) {
		return "out";
	} else  {
		return "unknown";
	}
}

static void replaceAll(String &str, const String &from, const String &to)
{
	size_t fromSize = from.length();
	size_t toSize = to.size();
	size_t startPos = 0;
	while ((startPos = str.find(from, startPos)) != String::npos) {
		str.replace(startPos, fromSize, to);
		startPos += toSize;
	}
}

static String screenString(String s)
{
	replaceAll(s, Kumir::Core::fromAscii("\\"), Kumir::Core::fromAscii("\\\\"));
	replaceAll(s, Kumir::Core::fromAscii("\n"), Kumir::Core::fromAscii("\\n"));
	replaceAll(s, Kumir::Core::fromAscii("\""), Kumir::Core::fromAscii("\\\""));
	replaceAll(s, Kumir::Core::fromAscii(" "), Kumir::Core::fromAscii("\\s"));
	replaceAll(s, Kumir::Core::fromAscii("\t"), Kumir::Core::fromAscii("\\t"));
	return s;
}

static String unscreenString(String s)
{
	replaceAll(s, Kumir::Core::fromAscii("\\n"), Kumir::Core::fromAscii("\n"));
	replaceAll(s, Kumir::Core::fromAscii("\\\""), Kumir::Core::fromAscii("\""));
	replaceAll(s, Kumir::Core::fromAscii("\\s"), Kumir::Core::fromAscii(" "));
	replaceAll(s, Kumir::Core::fromAscii("\\t"), Kumir::Core::fromAscii("\t"));
	replaceAll(s, Kumir::Core::fromAscii("\\\\"), Kumir::Core::fromAscii("\\"));
	return s;
}

static std::string constantToTextStream(const TableElem &e)
{
	std::ostringstream os;
	os.setf(std::ios::hex, std::ios::basefield);
	os.setf(std::ios::showbase);
	os << ".constant id=" << e.id << " type=" << vtypeToString(e.vtype, e.dimension) << " value=";
	os.unsetf(std::ios::basefield);
	os.unsetf(std::ios::showbase);
	if (e.vtype.front() == VT_int) {
		const int32_t val = e.initialValue.toInt();
		os << val;
	} else if (e.vtype.front() == VT_real) {
		const double val = e.initialValue.toDouble();
		os << val;
	} else if (e.vtype.front() == VT_bool) {
		const bool val = e.initialValue.toBool();
		os << (val ? "true" : "false");
	} else {
		const Kumir::String stringConstant = e.initialValue.toString();
		const Kumir::String screenedValue = screenString(stringConstant);
		Kumir::EncodingError encodingError;
		const std::string utf8Value = Kumir::Coder::encode(Kumir::UTF8, screenedValue, encodingError);
		os << "\"";
		os << utf8Value;
		os << "\"";
	}
	return os.str();
}

static std::string localToTextStream(const TableElem &e)
{
	std::ostringstream os;
	os.setf(std::ios::hex, std::ios::basefield);
	os.setf(std::ios::showbase);
	os << ".local kind=" << kindToString(e.refvalue) << " type=" << vtypeToString(e.vtype, e.dimension) << " ";
	os << "module=" << int(e.module) << " algorithm=" << e.algId << " id=" << e.id;
	if (e.name.length() > 0) {
		Kumir::EncodingError encodingError;
		os << " name=\"" << Kumir::Coder::encode(Kumir::UTF8, screenString(e.name), encodingError) << "\"";
	}
	return os.str();
}

static std::string globalToTextStream(const TableElem &e)
{
	std::ostringstream os;
	os.setf(std::ios::hex, std::ios::basefield);
	os.setf(std::ios::showbase);
	os << ".global type=" << vtypeToString(e.vtype, e.dimension) << " ";
	os << "module=" << int(e.module) << " id=" << e.id;
	if (e.name.length() > 0) {
		Kumir::EncodingError encodingError;
		os << " name=\"" << Kumir::Coder::encode(Kumir::UTF8, screenString(e.name), encodingError) << "\"";
	}
	return os.str();
}

static std::string functionToTextStream(
	const TableElem &e,
	const AS_Helpers &helpers
) {
	std::ostringstream os;
	os.setf(std::ios::hex, std::ios::basefield);
	os.setf(std::ios::showbase);
	os << elemTypeToString(e.type) << " ";
	os << "module=" << int(e.module) << " id=" << e.id << " size=" << e.instructions.size();
	if (e.name.length() > 0) {
		Kumir::EncodingError encodingError;
		os << " name=\"" << Kumir::Coder::encode(Kumir::UTF8, screenString(e.name), encodingError) << "\"";
	}
	os << "\n";
	os.unsetf(std::ios::basefield);
	os.unsetf(std::ios::showbase);
	for (size_t i = 0; i < e.instructions.size(); i++) {
		os << i << ":\t";
		os << instructionToString(e.instructions[i], helpers, e.module, e.algId);
		os << "\n";
	}
	return os.str();
}

static std::string externToTextStream(const TableElem &e)
{
	std::ostringstream os;
	os.setf(std::ios::hex, std::ios::basefield);
	os.setf(std::ios::showbase);
	Kumir::EncodingError encodingError;
	os << ".extern ";
	os << "module=";
	os << "\"" << Kumir::Coder::encode(Kumir::UTF8, screenString(e.moduleLocalizedName), encodingError) << "\"";
	os << " function=";
	os << "\"" << Kumir::Coder::encode(Kumir::UTF8, screenString(e.name), encodingError) << "\"";
	return os.str();
}

void tableElemToTextStream(
	std::ostream &ts,
	const TableElem &e,
	const AS_Helpers &helpers
) {
	if (e.type == EL_CONST) {
		ts << constantToTextStream(e);
	} else if (e.type == EL_EXTERN) {
		ts << externToTextStream(e);
	} else if (e.type == EL_LOCAL) {
		ts << localToTextStream(e);
	} else if (e.type == EL_GLOBAL) {
		ts << globalToTextStream(e);
	} else {
		ts << functionToTextStream(e, helpers);
	}
	ts << "\n";
}

} // namespace Bytecode


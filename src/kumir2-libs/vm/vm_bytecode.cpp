#include "vm_bytecode.hpp"
#include <string.h>

namespace Bytecode
{

bool isValidSignature(const std::list<char> &ds)
{
	static const size_t MaxLineSize = 255;
	static const char *Signature1 = "#!/usr/bin/env kumir2-run";
	static const char *Signature2 = "#!/usr/bin/env kumir2-xrun";

	char first[MaxLineSize];

	typedef std::list<char>::const_iterator CIt;
	size_t index = 0u;

	for (CIt it = ds.begin(); it != ds.end() && index < MaxLineSize; ++it, index++) {
		char ch = *it;
		if (ch == '\n' || ch == '\0') {
			break;
		}
		first[index] = ch;
	}

	bool firstMatch = strncmp(Signature1, first, index) == 0;
	bool secondMatch = strncmp(Signature2, first, index) == 0;
	return firstMatch || secondMatch;
}

void bytecodeToDataStream(std::list<char> &ds, const Data &data)
{
	static const char *header = "#!/usr/bin/env kumir2-run\n";
	for (size_t i = 0; i < strlen(header); i++) {
		ds.push_back(header[i]);
	}
	valueToDataStream(ds, data.versionMaj);
	valueToDataStream(ds, data.versionMin);
	valueToDataStream(ds, data.versionRel);
	uint32_t u32size = uint32_t(data.d.size());
	valueToDataStream(ds, u32size);
	for (size_t i = 0; i < data.d.size(); i++) {
		tableElemToBinaryStream(ds, data.d.at(i));
	}
}

void bytecodeToDataStream(std::ostream &ds, const Data &data)
{
	std::list<char> bytes;
	bytecodeToDataStream(bytes, data);
	char *buffer = reinterpret_cast<char *>(calloc(bytes.size(), sizeof(char)));
	size_t i = 0;
	for (std::list<char>::const_iterator it = bytes.begin(); it != bytes.end(); ++it) {
		buffer[i] = *it;
		i++;
	}
	ds.write(buffer, bytes.size()*sizeof(char));
	free(buffer);
}

void bytecodeFromDataStream(std::list<char> &ds, Data &data)
{
	if (ds.size() > 0 && ds.front() == '#') {
		char cur;
		while (1) {
			cur = ds.front();
			ds.pop_front();
			if (cur == '\n') {
				break;
			}
		}
	}
	if (ds.size() > 0) {
		valueFromDataStream(ds, data.versionMaj);
	}
	if (ds.size() > 0) {
		valueFromDataStream(ds, data.versionMin);
	}
	if (ds.size() > 0) {
		valueFromDataStream(ds, data.versionRel);
	}
	uint32_t u32_size = 0;
	if (ds.size() >= 4) {
		valueFromDataStream(ds, u32_size);
	}
	size_t size = size_t(u32_size);
	data.d.resize(size);
	for (size_t i = 0; i < size; i++) {
		tableElemFromBinaryStream(ds, data.d.at(i));
	}
}

void bytecodeFromDataStream(std::istream &is, Data &data)
{
	std::list<char> bytes;
	while (!is.eof()) {
		char buffer;
		is.read(&buffer, 1);
		bytes.push_back(buffer);
	}
	bytecodeFromDataStream(bytes, data);
}

static void makeHelpersForTextRepresentation(
	const Data &data,
	AS_Helpers &helpers
) {
	Kumir::EncodingError encodingError;
	for (size_t i = 0; i < data.d.size(); i++) {
		const TableElem &e = data.d.at(i);
		if (e.type == EL_LOCAL) {
			AS_Key akey((e.module << 16) | e.algId, e.id);
			if (helpers.locals.count(akey) == 0) {
				const std::string value = Kumir::Coder::encode(Kumir::UTF8, e.name, encodingError);
				helpers.locals.insert(std::pair<AS_Key, std::string>(akey, value));
			}
		}
		if (e.type == EL_GLOBAL || e.type == EL_EXTERN || e.type == EL_FUNCTION || e.type == EL_MAIN) {
			AS_Key akey(e.module << 16, e.type == EL_GLOBAL ? e.id : e.algId);
			AS_Values *vals = e.type == EL_GLOBAL ? &(helpers.globals) : &(helpers.algorithms);
			if (vals->count(akey) == 0) {
				const std::string value = Kumir::Coder::encode(Kumir::UTF8, e.name, encodingError);
				vals->insert(std::pair<AS_Key, std::string>(akey, value));
			}
		}
		if (e.type == EL_CONST) {
			AS_Key akey(0, e.id);
			if (helpers.constants.count(akey) == 0) {
				const std::string value = Kumir::Coder::encode(Kumir::UTF8, e.initialValue.toString(), encodingError);
				helpers.constants.insert(std::pair<AS_Key, std::string>(akey, value));
			}
		}
	}
}

void bytecodeToTextStream(std::ostream &ts, const Data &data)
{
	ts << "#!/usr/bin/env kumir2-run\n";
	ts << "#version " << int(data.versionMaj) << " " << int(data.versionMin) << " " << int(data.versionRel) << "\n\n";
	AS_Helpers helpers;
	for (size_t i = 0; i < data.d.size(); i++) {
		tableElemToTextStream(ts, data.d.at(i), helpers);
		makeHelpersForTextRepresentation(data, helpers);
		ts << "\n";
	}
}


} // namespace Bytecode


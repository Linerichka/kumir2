#include "kumircompilertoolplugin.h"
#include <kumir2-libs/extensionsystem/pluginmanager.h>

#include "plugins/kumiranalizer/analizer.h"
#include "plugins/kumiranalizer/kumiranalizerplugin.h"
#include <kumir2/generatorinterface.h>
#include <kumir2/analizer_sourcefileinterface.h>
#include <kumir2-libs/dataformats/kumfile.h>
#include <kumir2-libs/stdlib/kumirstdlib.hpp>
#include <kumir2-libs/vm/variant.hpp>
#include <kumir2-libs/vm/vm_bytecode.hpp>

using namespace KumirCompilerTool;
using namespace KumirAnalizer;

typedef Shared::GeneratorInterface::DebugLevel DebugLevel;

KumirCompilerToolPlugin::KumirCompilerToolPlugin()
	: KPlugin()
	, analizer_(nullptr)
	, generator_(nullptr)
	, useAnsiWindowsOutput_(true)
{
}

QList<ExtensionSystem::CommandLineParameter>
KumirCompilerToolPlugin::acceptableCommandLineParameters() const
{
	using ExtensionSystem::CommandLineParameter;
	QList<CommandLineParameter> result;
//    result << CommandLineParameter(
//                  false,
//                  'a', "ansi",
//                  tr("Show error messages in console using CP1251 encoding instead of CP866 (Windows only)")
//                  );
	result << CommandLineParameter(
			false,
			'e', "encoding",
			tr("Explicitly set source file text encoding"),
			QVariant::String, false
		);
	result << CommandLineParameter(
			false,
			'o', "out",
			tr("Explicitly set output file name"),
			QVariant::String, false
		);

	// Startup parameters

	result << CommandLineParameter(
			false,
			tr("PROGRAM.kum"),
			tr("Source file name"),
			QVariant::String,
			true
		);
	return result;
}


QString KumirCompilerToolPlugin::initialize(
	const QStringList & /*configurationArguments*/,
	const ExtensionSystem::CommandLine &runtimeArguments
)
{
	using namespace Shared;
	using namespace ExtensionSystem;
	PluginManager *manager = PluginManager::instance();
	analizer_ = manager->findPlugin<AnalizerInterface>();
	generator_ = manager->findPlugin<GeneratorInterface>();

	for (int i = 1; i < qApp->arguments().size(); i++) {
		const QString arg = qApp->arguments()[i];
		if (!arg.startsWith("-") && !arg.startsWith("[") && arg.endsWith(".kum")) {
			sourceFileName_ = arg;
		}
	}

	if (sourceFileName_.isEmpty()) {
		return tr("Error: source file name not specified.\nRun with --help parameter for more details");
	}

	useAnsiWindowsOutput_ = runtimeArguments.hasFlag('a');
	sourceFileEncoding_ = runtimeArguments.value('e').toString();
	outFileName_ = runtimeArguments.value('o').toString();

	return QString();
}

#include <iostream>


void KumirCompilerToolPlugin::start()
{

	if (sourceFileName_.length() == 0) {
		return;
	}
	const QString filename = QFileInfo(sourceFileName_).absoluteFilePath();
	QFile f(filename);
	if (f.open(QIODevice::ReadOnly)) {

		QByteArray fileData = f.readAll();
		f.close();

		Shared::Analizer::SFData kumFile;
		kumFile = analizer_->sourceFileHandler()->fromBytes(fileData, sourceFileEncoding_);
		kumFile.sourceUrl = QUrl::fromLocalFile(sourceFileName_);

		Shared::Analizer::InstanceInterface *analizer =
			analizer_->createInstance();

		QString dirname = QFileInfo(filename).absoluteDir().absolutePath();
		analizer->setSourceDirName(dirname);
		analizer->setSourceText(kumFile.visibleText + "\n" + kumFile.hiddenText);
		QList<Shared::Analizer::Error> errors = analizer->errors();
		AST::DataPtr ast = analizer->compiler()->abstractSyntaxTree();
		foreach (AST::ModulePtr mod, ast->modules) {
			if (mod->header.type != AST::ModTypeCached &&
				mod->header.type != AST::ModTypeExternal) {
				mod->header.sourceFileName = QFileInfo(sourceFileName_).fileName();
			}
		}

		const QString baseName = QFileInfo(filename).completeBaseName();

		for (int i = 0; i < errors.size(); i++) {
			Shared::Analizer::Error e = errors[i];
			QString errorMessage = tr("Error: ") +
				QFileInfo(filename).fileName() +
				":" + QString::number(e.line + 1) +
				":" + QString::number(e.start + 1) + "-" + QString::number(e.start + e.len) +
				": " + e.message;
#ifdef Q_OS_WIN32
			QTextCodec *cp866 = QTextCodec::codecForName("CP866");
			fprintf(stderr, "%s\n", cp866->fromUnicode(errorMessage).constData());
#else
			std::cerr << errorMessage.toLocal8Bit().data();
#endif
			std::cerr << std::endl;
		}

		QString suffix;
		QString mimeType;
		QByteArray outData;
		generator_->generateExecutable(ast, outData, mimeType, suffix);

		if (qApp->property("returnCode").toInt() != 0) {
			return;
		}

		QString outFileName = QFileInfo(filename).dir().absoluteFilePath(baseName + suffix);

		if (outFileName_.length() > 0) {
			outFileName = outFileName_;
		}


		if (!outFileName.endsWith(suffix)) {
			outFileName += suffix;
		}

		QFile binOut(outFileName);

		binOut.open(QIODevice::WriteOnly);
		binOut.write(outData);
		binOut.close();
		if (mimeType.startsWith("executable") && QFile::exists(outFileName)) {
			QFile::Permissions ps = binOut.permissions();
			ps |= QFile::ExeGroup | QFile::ExeOwner | QFile::ExeOther;
			QFile::setPermissions(outFileName, ps);
		}
		qApp->setProperty("returnCode", errors.isEmpty() ? 0 : 1);
	} else {
		const QString errorMessage = tr("Can't open file %1").arg(QDir::toNativeSeparators(filename));
#ifdef Q_OS_WIN32
		QTextCodec *cp866 = QTextCodec::codecForName("CP866");
		fprintf(stderr, "%s\n", cp866->fromUnicode(errorMessage).constData());
#else
		std::cerr << errorMessage.toLocal8Bit().data();
#endif
		std::cerr << std::endl;
		qApp->setProperty("returnCode", 2);
	}
}

void KumirCompilerToolPlugin::stop()
{

}

void KumirCompilerToolPlugin::createPluginSpec()
{
	_pluginSpec.name = "KumirCompilerTool";
	_pluginSpec.gui = false;
	_pluginSpec.dependencies.append("Analizer");
	_pluginSpec.dependencies.append("Generator");
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN(KumirCompilerToolPlugin)
#endif

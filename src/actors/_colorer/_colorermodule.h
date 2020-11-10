/*
This file is generated, but you can safely change it
until you run "gen_actor_source.py" with "--project" flag.

Generated file is just a skeleton for module contents.
You should change it corresponding to functionality.
*/

#ifndef _COLORERMODULE_H
#define _COLORERMODULE_H

// Base class include
#include "_colorermodulebase.h"

// Kumir includes
#include <kumir2-libs/extensionsystem/kplugin.h>

namespace Actor_Colorer
{

class _ColorerModule : public _ColorerModuleBase
{
	Q_OBJECT
public /* methods */:
	_ColorerModule(ExtensionSystem::KPlugin *parent);
	static QList<ExtensionSystem::CommandLineParameter> acceptableCommandLineParameters();
	static const QStringList &standardRussianColorNames();

public slots:
	void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
	void loadActorData(QIODevice *source);
	void reloadSettings(ExtensionSystem::SettingsPtr settings, const QStringList &keys);
	void reset();
	void terminateEvaluation() {}
	Color runOperatorINPUT(const QString &x, bool &ok);
	QString runOperatorOUTPUT(const Color &x);
	bool runOperatorEQUAL(const Color &x, const Color &y);
	bool runOperatorNOTEQUAL(const Color &x, const Color &y);
};


} // namespace Actor_Colorer

#endif // _COLORERMODULE_H

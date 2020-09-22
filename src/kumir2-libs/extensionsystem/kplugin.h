#ifndef KPLUGIN_H
#define KPLUGIN_H

#include "pluginspec.h"
#include "settings.h"
#include "commandlineparameter.h"
#include <kumir2/plugininterface.h>

#ifdef EXTENSIONSYSTEM_LIBRARY
#define EXTENSIONSYSTEM_EXPORT Q_DECL_EXPORT
#else
#define EXTENSIONSYSTEM_EXPORT Q_DECL_IMPORT
#endif

#include <QDir>
class QWidget;
class QIcon;

namespace ExtensionSystem
{

typedef Shared::PluginInterface::GlobalState GlobalState;

class EXTENSIONSYSTEM_EXPORT KPlugin :
	public QObject,
	public Shared::PluginInterface
{
	friend class PluginManager;
	friend struct PluginManagerImpl;
	Q_OBJECT
	Q_INTERFACES(Shared::PluginInterface)

public:
	enum State {
		Disabled = 0x00,
		Loaded = 0x01,
		Initialized = 0x02,
		Started = 0x03,
		Stopped = 0x04
	};

	KPlugin();
	virtual ~KPlugin();

	virtual QByteArray pluginName() const;

	const PluginSpec &pluginSpec() const;

	State state() const;

	virtual bool isGuiRequired() const
	{
		return pluginSpec().gui;    // Can be overridden in special cases
	}

	/** DEPRECATED
	 * Use
	 *   QList<QWidget*> settingsEditorPages()
	 * instead
	 */
	virtual QWidget *settingsEditorPage()
	{
		return 0;
	}

	virtual QList<QWidget *> settingsEditorPages()
	{
		return QList<QWidget *>();
	}

	SettingsPtr mySettings() const;

	QDir myResourcesDir() const;

	virtual void saveSession() const { }
	virtual void restoreSession() { }

	virtual void changeCurrentDirectory(const QString &path)
	{
		Q_UNUSED(path);
	}

	virtual void changeGlobalState(GlobalState old, GlobalState current)
	{
		Q_UNUSED(old);
		Q_UNUSED(current);
	}

	virtual QList<CommandLineParameter> acceptableCommandLineParameters() const
	{
		return QList<CommandLineParameter>();
	}

	void initialize(const QString &pluginResourcesRootPath);

	virtual QString initialize(
		const QStringList & /*configurationArguments*/,
		const CommandLine & /*runtimeArguments*/
	) = 0;

	virtual bool isSafeToQuit()
	{
		return true;
	}

	virtual void start() {}
	virtual void stop() {}
	virtual void updateSettings(const QStringList &keys) = 0;
	KPlugin *myDependency(const QByteArray &name) const;

	QList<KPlugin *> loadedPlugins(const QByteArray &pattern = "*");
	QList<const KPlugin *> loadedPlugins(const QByteArray &pattern = "*") const;

protected:
	virtual void createPluginSpec() = 0;
	PluginSpec _pluginSpec;

private:
	SettingsPtr _settings;
	QString _resourcesDir;
	State _state;
};

} // namespace ExtensionSystem

#endif // KPLUGIN_H

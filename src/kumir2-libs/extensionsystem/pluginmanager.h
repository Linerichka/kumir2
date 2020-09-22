#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "kplugin.h"

#ifdef EXTENSIONSYSTEM_LIBRARY
#define EXTENSIONSYSTEM_EXPORT Q_DECL_EXPORT
#else
#define EXTENSIONSYSTEM_EXPORT Q_DECL_IMPORT
#endif

namespace  ExtensionSystem
{

class EXTENSIONSYSTEM_EXPORT PluginManager : public QObject
{
	Q_OBJECT
public:
	~PluginManager();
	static PluginManager *instance();

	/** Set path to search plugins */
	void setPluginPath(const QString &path);

	/** Set path to search settings etc. */
	void setSharePath(const QString &path);

	QString sharePath() const;

	void switchGlobalState(GlobalState state);

	GlobalState currentGlobalState() const;

	SettingsPtr globalSettings() const;

	/** Loads plugins by given template in form:
	  *
	  *     Plugin1(),Plugin2,*Plugin3,Plugin4(argument1,argument2)
	  *
	  * where *-marked plugin is entry point,
	  * plugin arguments are in brackets (optional)
	  *
	  * @param templ template
	  * @return error message or empty string on success */
	QString loadPluginsByTemplate(const QByteArray &templ);

	/** Loads extra module (example: actor file on demand) by given canonical
	 *  file name @param canonicalFileName.
	 *
	 *  @returns error message or empty string on success */
	QString loadExtraModule(const std::string &canonicalFileName);
	QString loadExtraModule(const QByteArray &canonicalFileName)
	{
		return loadExtraModule(std::string(canonicalFileName.constData()));
	}

	/** Returns true if at least one module requires gui event processing */
	bool isGuiRequired() const;

	/** Initializes plugings by parsing commandline arguments. Returns error message */
	QString initializePlugins();

	/** Starts entry point plugin. Returns error message */
	QString start();

	/** Returns plugin spec */
	PluginSpec specByObject(const KPlugin *p) const;

	/** Returns plugin state */
	KPlugin::State stateByObject(const KPlugin *p) const;

	/** Returns plugin settings */
	SettingsPtr settingsByObject(const KPlugin *p) const;

	/** Returns plugin dependency by name */
	KPlugin *dependentPlugin(const QByteArray &name, const class KPlugin *p) const;

	/** Return all plugins matched by regexp */
	QList<KPlugin *> loadedPlugins(const QByteArray &pattern = "*");
	QList<const KPlugin *> loadedConstPlugins(const QByteArray &pattern = "*") const;

	/** Return plugin by specified name */
	KPlugin *loadedPlugin(const QByteArray &name);

	/** Module to start */
	KPlugin *startupModule();

	QString commandLineHelp() const;

	QFont initialApplicationFont() const;

	QDir findSystemResourcesDir(const QString &subdirShouldExists) const;

	template <class PluginInterface>
	PluginInterface *findPlugin()
	{
		QList<KPlugin *> plugins = loadedPlugins("");
		PluginInterface *result = nullptr;
		for (int i = 0; i < plugins.size(); i++) {
			KPlugin *plugin = plugins[i];
			result = qobject_cast<PluginInterface *>(plugin);
			if (result) {
				break;
			}
		}
		return result;
	}

	template <class PluginInterface>
	PluginInterface *findPlugin(const QByteArray &name)
	{
		QList<KPlugin *> plugins = loadedPlugins("");
		PluginInterface *result = nullptr;
		for (int i = 0; i < plugins.size(); i++) {
			KPlugin *plugin = plugins[i];
			if (plugin->pluginName() == name) {
				result = qobject_cast<PluginInterface *>(plugin);
			}
			if (result) {
				break;
			}
		}
		return result;
	}

	template <class PluginInterface>
	KPlugin *findKPlugin()
	{
		QList<KPlugin *> plugins = loadedPlugins();
		KPlugin *result = nullptr;
		for (int i = 0; i < plugins.size(); i++) {
			KPlugin *plugin = plugins[i];
			PluginInterface *test = qobject_cast<PluginInterface *>(plugin);
			if (test) {
				result = plugin;
				break;
			}
		}
		return result;
	}

	template <class PluginInterface>
	QList<PluginInterface *> findPlugins()
	{
		QList<KPlugin *> plugins = loadedPlugins();
		QList<PluginInterface *> result;
		for (int i = 0; i < plugins.size(); i++) {
			KPlugin *plugin = plugins[i];
			PluginInterface *implementation =
				qobject_cast<PluginInterface *>(plugin);
			if (implementation) {
				result.push_back(implementation);
			}
		}
		return result;
	}

	bool isPluginLoaded(const QByteArray &name) const;
	bool isPluginLoaded(const QString &name) const
	{
		return isPluginLoaded(name.toLatin1());
	}
	bool isPluginLoaded(const char *name) const
	{
		return isPluginLoaded(QByteArray(name));
	}

	void updateAllSettings();

	QString workspacePath() const;
	void switchToDefaultWorkspace(bool workDirOnly);
	void switchToWorkspace(const QString &path, bool workDirOnly);

	/** Stops all plugins in reverse-order of creation
	 * @returns true if there is not module blocking exit, false elsewhere
	 */
	bool shutdown();

	static void destroy();

public /*constants*/:

	static const QString WorkspacesListKey;
	static const QString CurrentWorkspaceKey;
	static const QString SkipChooseWorkspaceKey;
private:
	explicit PluginManager();
	QScopedPointer<struct PluginManagerImpl> pImpl_;
	void setupAdditionalPluginPaths();

};

} // namespace ExtensionSystem

#endif // PLUGINMANAGER_H

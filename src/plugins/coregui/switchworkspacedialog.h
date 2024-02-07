#ifndef COREGUI_SWITCHWORKSPACEDIALOG_H
#define COREGUI_SWITCHWORKSPACEDIALOG_H

#include <kumir2-libs/extensionsystem/settings.h>

#include <QDialog>

namespace CoreGUI
{

namespace Ui
{
class SwitchWorkspaceDialog;
}

class SwitchWorkspaceDialog : public QDialog
{
	Q_OBJECT

public:
	enum Message { MSG_ChangeWorkspace, MSG_ChangeWorkingDirectory };
	explicit SwitchWorkspaceDialog(ExtensionSystem::SettingsPtr settings);
	~SwitchWorkspaceDialog();
	void setCurrentWorkspace(const QString &path);
	void setMessage(Message message);
	void setUseAlwaysHidden(bool v);
	QString currentWorkspace() const;

private slots:
	void handleBrowseClicked();
	void handleAccepted();

private:
	Ui::SwitchWorkspaceDialog *ui;
	ExtensionSystem::SettingsPtr settings_;
};


} // namespace CoreGUI
#endif // COREGUI_SWITCHWORKSPACEDIALOG_H

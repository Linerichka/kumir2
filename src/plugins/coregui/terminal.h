#ifndef TERMINAL_TERMINAL_H
#define TERMINAL_TERMINAL_H

#include <QWidget>
class QScrollBar;
class QGridLayout;

#include <kumir2-libs/extensionsystem/kplugin.h>

namespace Terminal
{

class Term : public QWidget
{
	friend class Plane;
	Q_OBJECT
public:
	explicit Term(QWidget *parent = 0);
	bool isActiveComponent() const;
	QSize sizeHint() const;
	QSize minimumSizeHint() const;
	bool isEmpty() const;
	ExtensionSystem::SettingsPtr settings() const;
	void setSettings(const ExtensionSystem::SettingsPtr &settings);

signals:
	void openTextEditor(const QString &suggestedFileName, const QString &text);
	void message(const QString &);
	void inputFinished(const QVariantList &);
	void showWindowRequest();
public slots:
	void clear();
	void start(const QString &fileName);
	void finish();
	void terminate();
	void output(const QString &text);
	void outputErrorStream(const QString &text);
	void error(const QString &message);
	void input(const QString &format);
	void saveLast();
	void saveAll();
	void copyLast();
	void copyAll();
	void editLast();
	void setTerminalFont(const QFont &font);
	void changeGlobalState(ExtensionSystem::GlobalState old, ExtensionSystem::GlobalState current);
	QAction *actionSaveAll() const
	{
		return a_saveAll;
	}
	QAction *actionSaveLast() const
	{
		return a_saveLast;
	}
	QAction *actionCopyAll() const
	{
		return a_copyAll;
	}
	QAction *actionCopyLast() const
	{
		return a_copyLast;
	}
	QAction *actionEditLast() const
	{
		return a_editLast;
	}
	QAction *actionClear() const
	{
		return a_clear;
	}

protected:
	void focusInEvent(QFocusEvent *);
	void focusOutEvent(QFocusEvent *);
	void resizeEvent(QResizeEvent *);
	void paintEvent(QPaintEvent *);
	bool eventFilter(QObject *, QEvent *);
	void saveText(const QString &suggestedFileName, const QString &text);

protected slots:
	void handleInputTextChanged(const QString &text);
	void handleInputCursorPositionChanged(quint16 pos);
	void handleInputFinishRequested();
	void handleInputDone(const QVariantList &values);

private:
	QList<class OneSession *> sessions_;
	QScrollBar *sb_vertical;
	QScrollBar *sb_horizontal;
	class Plane *plane_;
	QAction *a_saveAll;
	QAction *a_saveLast;
	QAction *a_copyAll;
	QAction *a_copyLast;
	QAction *a_editLast;
	QAction *a_clear;
//    QToolBar * m_toolBar;
	QGridLayout *m_layout;
	QStringList inputFormats_;
	QVariantList inputValues_;
	ExtensionSystem::SettingsPtr settings_;


};

} // namespace Terminal

#endif // TERMINAL_TERMINAL_H

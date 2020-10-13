/****************************************************************************
**
** Copyright (C) 2004-2010 NIISI RAS. All rights reserved.
**
** This file is part of the KuMir.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "../course_model.h"
#include "../coursemanager_plugin.h"
#include "editdialog.h"
#include "newkursdialog.h"

#include <QMainWindow>
#include <QMenu>
class QTextBrowser;

#ifdef interface
#undef interface // used name 'interface' conflicts with Windows SDK
#endif

namespace Shared
{
namespace Browser
{
class InstanceInterface;
}
}

class KumZadanie
{
public:
	KumZadanie()
	{
		isps.clear();
	}

	QString Isp(int no) const
	{
		if (isps.count() <= no) {
			return "";
		}
		return isps[no];
	}

	QString field(QString ispName, int fieldNo)
	{
		QList<QString> ispFields = fields.values(ispName);
		if (fieldNo >= ispFields.count()) {
			return "";
		}
		return ispFields.at(fieldNo);
	}

	int fieldsCount(QString ispName)
	{
		QList<QString> ispFields = fields.values(ispName);

		return ispFields.count();
	}

	int minFieldCount()
	{
		QList<QString> isps = fields.keys();
		int min_fields = 999;
		if (isps.count() == 0) {
			return 0;
		}
		for (int i = 0; i < isps.count(); i++) {
			int fcount = fieldsCount(isps.at(i));
			if (fcount < min_fields) {
				min_fields = fcount;
			}
		}
		return min_fields;
	}

	QString name;

	//исполнители используемые в задании
	QStringList isps;

	//Обстановки для каждого исполнителя fields[исп,обст]
	QMap<QString, QString> fields;
};


namespace Ui
{
class MainWindowTask;
}

struct courseChanges
{
	courseChanges() {};
	void cleanChanges()
	{
		marksChanged.clear();
	}
	void setMark(int testId, int newMark)
	{
		marksChanged.insert(testId, newMark);
	}
	QMap<int, int> marksChanged;
};

class MainWindowTask : public QMainWindow
{
	Q_OBJECT
public:
	MainWindowTask(QWidget *parent = 0);

	~MainWindowTask();
	void setInterface(CourseManager::Plugin *csInterface)
	{
		interface = csInterface;
	};
	void setCS(QString cs)
	{
		CS = cs;
	};
	KumZadanie task;
	void setup(const QDir &resourcesRoot, ExtensionSystem::SettingsPtr sett);
	void updateSettings(const QStringList &keys, ExtensionSystem::SettingsPtr sett)
	{
		settings = sett;
	};
	QString getFileName(QString fileName);
	void setTeacher(bool mode);
	QList<QAction *> getActions();
	QString baseCourseFile()
	{
		return baseKursFile.absoluteFilePath();
	}
Q_SIGNALS:
	void activateRequest();

public slots:
	bool safeToQuit();
	void aboutToQuit();
	void openRescent();
	void loadCourse();
	void loadCourseFromFile(const QString &file);
	void returnTested();
	void saveCourse();
	void saveCourseFile();
	void saveBaseKurs();
	void showText(const QModelIndex &index);
	void loadHtml(QString fileName);
	void startTask();
	void checkTask();
	void lockCheck();
	void setMark(int mark);
	void resetTask();
	void Close();
	void customContextMenuRequested(QPoint  pos);
	void addTask();
	void addDeepTask();
	void deleteTask();
	void saveKurs();//TEACHER
	void saveKursAs();//TEACHER
	void editTask();//Teacher
	void setEditTaskEnabled(bool flag);
	void moveUp();
	void moveDown();
	void newKurs();
	void endRootEdit();
	void cancelRootEdit();
	void lockControls();
	void unlockControls();
	void updateLastFiles(const QString newFile);

	void nextTask();

protected:
	void changeEvent(QEvent *e);
	void closeEvent(QCloseEvent *event);
	void showEvent(QShowEvent *event);
	void hideEvent(QHideEvent *event);
private:
	void setupWebView();
	void setTaskViewHtml(const QString &data);
	void setTaskViewUrl(const QUrl &url);
	void markProgChange();
	void createMoveMenu();
	void setUpDown(QModelIndex index);
	QString loadScript(QString file_name);
	QString loadTestAlg(QString file_name);
	void loadCourseData(const QString filename);
	void loadMarks(const QString fileName);

	QString curDir;
	courseModel *course;
	QModelIndex curTaskIdx;
	CourseManager::Plugin *interface;
	QString CS;
	bool onTask;
	courseChanges changes;

	QString cursFile;
	QList<int> progChange;
	QFile cursWorkFile;
	QMenu customMenu;
	bool isTeacher;
	//EditDialog* editDialog;
	//newKursDialog* newDialog;
	ExtensionSystem::SettingsPtr settings;
	QLineEdit *editRoot;
	QFileInfo baseKursFile; //4 mode
	Ui::MainWindowTask *ui;
	bool isReadOnly;
	Shared::Browser::InstanceInterface *browserPluginInstance_;
	QTextBrowser *simpleBrowserWidget_;
};

#endif // MAINWINDOW_H

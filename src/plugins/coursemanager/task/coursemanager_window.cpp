#include "coursemanager_window.h"
#include "ui_coursemanager_window.h"

#include <kumir2/browserinterface.h>
#include <kumir2/browser_instanceinterface.h>

#include <QLineEdit>
#include <QMessageBox>
#include <QTextBrowser>
#include <QAbstractButton>
#include <QCloseEvent>

MainWindowTask::MainWindowTask(QWidget *parent) :
	QMainWindow(parent),
	ui(new Ui::MainWindowTask)
{

	cursFile = "";
	course = NULL;
	curDir = "";
	progChange.clear();
	connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(aboutToQuit()));

}
void MainWindowTask::setup(const QDir &resourcesRoot, ExtensionSystem::SettingsPtr sett)
{
	course = NULL;
	ui->setupUi(this);
	isReadOnly = false;
	ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
	ui->treeView->setIconSize(QSize(25, 25));
	ui->treeView->setStyleSheet("icon-size: 25px;font-size: 14px;");
	settings = sett;
	customMenu.hide();
	connect(ui->loadCurs, SIGNAL(triggered()), this, SLOT(loadCourse()));
	connect(ui->actionSave, SIGNAL(triggered()), this, SLOT(saveCourse()));
	connect(ui->treeView, SIGNAL(clicked(QModelIndex)), this, SLOT(showText(QModelIndex)));
	connect(ui->do_task, SIGNAL(triggered()), this, SLOT(startTask()));
	qDebug() << "Check Connect tttttttttttttttttt";
	connect(ui->checkTask, SIGNAL(triggered()), this, SLOT(checkTask()));
	connect(ui->actionReset, SIGNAL(triggered()), this, SLOT(resetTask()));
	connect(ui->actionClose, SIGNAL(triggered()), this, SLOT(Close()));
	connect(ui->actionTested, SIGNAL(triggered()), this, SLOT(returnTested()));
	connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), this, SLOT(customContextMenuRequested(QPoint)));

	customMenu.addAction(ui->actionAdd);
	customMenu.addAction(ui->actionRemove);
	customMenu.addAction(ui->actionEdit);
	customMenu.addSeparator();
	customMenu.addAction(ui->actionup);
	customMenu.addAction(ui->actionDown);
	customMenu.addAction(ui->addDeep);
	connect(ui->actionup, SIGNAL(triggered()), this, SLOT(moveUp()));
	connect(ui->actionDown, SIGNAL(triggered()), this, SLOT(moveDown()));
	connect(ui->actionAdd, SIGNAL(triggered()), this, SLOT(addTask()));
	connect(ui->addDeep, SIGNAL(triggered()), this, SLOT(addDeepTask()));
	connect(ui->actionSaveK, SIGNAL(triggered()), this, SLOT(saveKurs()));
	connect(ui->actionSaveKas, SIGNAL(triggered()), this, SLOT(saveKursAs()));
	connect(ui->actionRemove, SIGNAL(triggered()), this, SLOT(deleteTask()));
	connect(ui->actionNext, SIGNAL(triggered()), this, SLOT(nextTask()));
	//  newDialog=new newKursDialog();
	// connect(ui->actionNewK,SIGNAL(triggered()),this,SLOT(newKurs()));
	//  editDialog = new EditDialog(this);
	// connect(ui->actionEdit,SIGNAL(triggered()),this,SLOT(editTask()));
	// ui->menuKurs->menuAction()->setEnabled(false);
	setEditTaskEnabled(false);
	ui->treeView->setSelectionMode(QAbstractItemView::SingleSelection);
	editRoot = new QLineEdit(ui->treeView);
	editRoot->hide();
	connect(editRoot, SIGNAL(editingFinished()), this, SLOT(endRootEdit()));
	ui->treeView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	isTeacher = false;
	onTask = false;
	cursFile = "";
	setWindowIcon(QIcon(resourcesRoot.absoluteFilePath("10.png")));
	setupWebView();
	//ui->textBrowser->setVisible(false);
};
void MainWindowTask::nextTask()
{
	if (ui->treeView->indexBelow(curTaskIdx).isValid()) {
		ui->treeView->setCurrentIndex(ui->treeView->indexBelow(curTaskIdx));
		// curTaskIdx=ui->treeView->currentIndex();
		showText(ui->treeView->currentIndex());
	}
}

void MainWindowTask::setupWebView()
{
	using namespace ExtensionSystem;
	using namespace Shared;

	BrowserInterface *browserPlugin
		= PluginManager::instance()->findPlugin<BrowserInterface>();

	QWidget *webViewComponent = 0;
	simpleBrowserWidget_ = 0;
	browserPluginInstance_ = 0;

	if (browserPlugin) {
		browserPluginInstance_ = browserPlugin->createBrowser();
		webViewComponent = browserPluginInstance_->widget();
	} else {
		simpleBrowserWidget_ = new QTextBrowser();
		webViewComponent = simpleBrowserWidget_;
	}

	webViewComponent->setParent(ui->webView);
	webViewComponent->setMinimumWidth(200);
	QVBoxLayout *l = new QVBoxLayout;
	l->setContentsMargins(0, 0, 0, 0);
	ui->webView->setLayout(l);
	l->addWidget(webViewComponent);
}

void MainWindowTask::setTaskViewHtml(const QString &data)
{
	if (simpleBrowserWidget_) {
		simpleBrowserWidget_->setHtml(data);
	} else if (browserPluginInstance_) {
		browserPluginInstance_->setContent(data);
	}
}

void MainWindowTask::setTaskViewUrl(const QUrl &url)
{
	if (simpleBrowserWidget_) {
		simpleBrowserWidget_->setSource(url);
	} else if (browserPluginInstance_) {
		browserPluginInstance_->go(url);
	}
}

QList<QAction *> MainWindowTask::getActions()
{
	QList<QAction *> toRet;
	toRet.append(ui->loadCurs);
	toRet.append(ui->checkTask);
	toRet.append(ui->actionSave);
	toRet.append(ui->actionReset);
	toRet.append(ui->actionTested);
	return toRet;
}

MainWindowTask::~MainWindowTask()
{
	delete ui;
}

void MainWindowTask::changeEvent(QEvent *e)
{
	QMainWindow::changeEvent(e);
	switch (e->type()) {
	case QEvent::LanguageChange:
		ui->retranslateUi(this);
		break;
	default:
		break;
	}
}
void MainWindowTask::updateLastFiles(const QString newFile)
{
	QStringList lastFiles = settings->value("Courses/LastFiles", "").toString().split(";");
	qDebug() << lastFiles;
	qDebug() << settings->locationDirectory();
	if (lastFiles.indexOf(newFile) < 0) {
		lastFiles.prepend(newFile);
	}
	int max_fid = std::min(lastFiles.count(), 10);
	QString sett = "";
	for (int i = 0; i < max_fid; i++) {
		sett += lastFiles.at(i) + ";";
	}
	settings->setValue("Courses/LastFiles", sett);
	interface->rebuildRescentMenu();
};
void MainWindowTask::loadCourseData(const QString fileName)
{

	course = new courseModel();
	connect(course, SIGNAL(dataChanged(QModelIndex, QModelIndex)), ui->treeView, SLOT(dataChanged(QModelIndex, QModelIndex)));
	int tasks = course->loadCourse(fileName);
	course->setTeacher(isTeacher);
	if (tasks == -1) {
		QMessageBox::information(0, "", trUtf8("Ошибка открытия файла: ") + fileName, 0, 0, 0);
		return;
	};
	ui->treeView->setModel(course);
	curTaskIdx = QModelIndex();
	onTask = false;
	ui->actionSave->setEnabled(true);

	changes.cleanChanges();
	cursFile = fileName;
};
void MainWindowTask::loadMarks(const QString fileName)
{
	QDomDocument workXml;
	QFile f(fileName);

	if (!f.open(QIODevice::ReadOnly)) {
		QMessageBox::information(0, "", trUtf8("Ошибка открытия файла: ") + fileName, 0, 0, 0);
		return;

	};
	QFileInfo fi(f);
	if (!fi.isWritable()) {
		QMessageBox::information(0, "", trUtf8("Файл загружен только для чтения, изменение не будут сохранены: ") + fileName, 0, 0, 0);
		isReadOnly = true;
	} else {
		isReadOnly = false;
	}
	if (f.atEnd()) {
		QMessageBox::information(0, "", trUtf8("Ошибка открытия файла ,файл пуст: ") + fileName, 0, 0, 0);
		return;
	};
	cursWorkFile.setFileName(f.fileName());
	QString error;
	int str, pos;
	workXml.setContent(f.readAll(), true, &error, &str, &pos);
	qDebug() << "File parce:" << error << "str" << str << " pos" << pos;

	QDomElement root = workXml.documentElement();
	if (root.tagName() != "COURSE") {
		QMessageBox::information(0, "", trUtf8("Ошибка загрузки файла: ") + fileName, 0, 0, 0);
		return;
	};
	QDomElement fileEl = root.firstChildElement("FILE");
	QString krsFile = fileEl.attribute("fileName");
	loadCourseData(krsFile);//Gruzim kurs

	if (cursFile != krsFile) { //Esli ne udalos po puti - ishem v toyje direktorii
		QFileInfo finf(fileEl.attribute("fileName"));
		QFileInfo cfi(cursWorkFile);
		qDebug() << "PATH" << cfi.dir().canonicalPath() + "/" + finf.fileName();
		krsFile = cfi.dir().canonicalPath() + "/" + finf.fileName();
		curDir = cfi.dir().canonicalPath();
		QFileInfo krsFi(krsFile);
		if (krsFi.isReadable()) {
			loadCourseData(krsFile);
		}

	}

	QString fileN = fileEl.attribute("fileName");
//qDebug()<<"KURS ZAGRUZILI";
	if (cursFile != krsFile) {
		QMessageBox::information(0, "", trUtf8("Не найден файл курса:") + fileEl.attribute("fileName"), 0, 0, 0);
		fileN = getFileName(krsFile);
		loadCourseData(fileN);
		if (cursFile != fileN) {
			return;
		}
	}
	QFileInfo fi_kurs = QFileInfo(krsFile);
	curDir = fi_kurs.absolutePath();
	QDomNodeList marksElList = root.elementsByTagName("MARK"); //Оценки
//qDebug()<<"Loading marks "<<marksElList.count();
	for (int i = 0; i < marksElList.count(); i++) {
		int taskId = marksElList.at(i).toElement().attribute("testId").toInt();
		int mark = marksElList.at(i).toElement().attribute("mark").toInt();
		qDebug() << "task:" << taskId << " mark:" << mark;
		course->setMark(taskId, mark);
		changes.setMark(taskId, mark);
	}

//qDebug()<<"Loading user prgs...";
	QDomNodeList prgElList = root.elementsByTagName("USER_PRG"); //Программы
	for (int i = 0; i < prgElList.count(); i++) {
		int taskId = prgElList.at(i).toElement().attribute("testId").toInt();
		qDebug() << "Tassk id" << taskId;
		QString prg = prgElList.at(i).toElement().attribute("prg");

		if (progChange.indexOf(taskId) == -1) {

			progChange.append(taskId);
		}
		course->setUserText(taskId, prg);
	}

	QDomNodeList prgElListT = root.elementsByTagName("TESTED_PRG"); //Программы тестированные
	for (int i = 0; i < prgElListT.count(); i++) {
		int taskId = prgElListT.at(i).toElement().attribute("testId").toInt();
		QString prg = prgElListT.at(i).toElement().attribute("prg");

		course->setUserTestedText(taskId, prg);
	}
}

void MainWindowTask::loadCourseFromFile(const QString &file)
{
	QFileInfo fi(file);
	if (!fi.exists()) {

		return;
	}

	baseKursFile = fi;
	curDir = fi.absolutePath();
	settings->setValue("Directories/Kurs", curDir);
	qDebug() << "curDir" << curDir;
	QString fileName = file;
	progChange.clear();
	if (fileName.right(9) == ".work.xml") { //Загрузка оценок и программ
		loadMarks(fileName);
		this->show();
		return;
	} else {
		cursWorkFile.setFileName("");
	}
	loadCourseData(fileName);
	QString cText = course->courseDescr();


	if (cText.right(4) == ".htm" || cText.right(5) == ".html") {
		loadHtml(cText);
	} else {
		setTaskViewHtml(cText);
	}
	// if(isTeacher)ui->actionEdit->setEnabled(true);
	setWindowTitle(course->name() + trUtf8(" - Практикум"));
	updateLastFiles(fileName);
	interface->lockContrls();
	interface->setPreProgram(QVariant(""));
	ui->checkTask->setEnabled(false);
	this->show();
};

void MainWindowTask::loadCourse()
{
	editRoot->hide();
	ui->splitter->setEnabled(true);
	QString dir = settings->value("Directories/Kurs", "").toString();
	qDebug() << "Dir " << dir;
	QDir chD(dir);
	QDir resDir = interface->myResourcesDir();
	resDir.cdUp();
	resDir.cd("courses");
	if (0 == dir.length() || !chD.exists()) {
		dir = resDir.canonicalPath();
	}
//    QFileDialog dialog(this,trUtf8("Открыть файл"),dir, "(*.kurs.xml *.work.xml)");
//     dialog.setAcceptMode(QFileDialog::AcceptOpen);
//     if(!dialog.exec())return;


	QString    File = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл"), dir, "Xml (*.xml)");
	QFileInfo fi(File);
	if (!fi.exists()) {

		return;
	};
	this->showNormal();

	baseKursFile = fi;
	curDir = fi.absolutePath();
	settings->setValue("Directories/Kurs", curDir);
	qDebug() << "curDir" << curDir;
	QString fileName = File;
	progChange.clear();
	if (fileName.right(9) == ".work.xml") { //Загрузка оценок и программ
		isReadOnly = false;
		loadMarks(fileName);
		Q_EMIT activateRequest();
//         this->show();
		return;
	}
	bool createDefaultWorkFile = true;
	QMessageBox msgBoxCreateWorkbook(
		QMessageBox::Question,
		trUtf8("Практикум"),
		trUtf8("Вы хотите создать тетрадь?"),
		QMessageBox::Yes | QMessageBox::No,
		this
	);
	msgBoxCreateWorkbook.button(QMessageBox::Yes)->setText(trUtf8("Создать"));
	msgBoxCreateWorkbook.button(QMessageBox::No)->setText(trUtf8("Не создавать"));
	int ans = msgBoxCreateWorkbook.exec();
//            ans = QMessageBox::question(this, trUtf8("Практикум"), trUtf8("Вы хотите создать тетрадь?"),
//                    QMessageBox::Yes | QMessageBox::No , QMessageBox::Yes);
	if (ans == QMessageBox::Yes) {
		createDefaultWorkFile = false;

	};

	cursWorkFile.setFileName("");
	loadCourseData(fileName);
	isReadOnly = false;
	interface->setPreProgram(QVariant(""));
	QString cText = course->courseDescr();


	if (cText.right(4) == ".htm" || cText.right(5) == ".html") {
		loadHtml(cText);
	} else {
		setTaskViewHtml(cText);
	}
// if(isTeacher)ui->actionEdit->setEnabled(true);
	setWindowTitle(course->name() + trUtf8(" - Практикум"));
	updateLastFiles(fileName);
	interface->lockContrls();
	ui->checkTask->setEnabled(false);
	Q_EMIT activateRequest();
//    this->show();
	if (createDefaultWorkFile) {
		markProgChange();
		//curDir=QDir::currentPath();
		qDebug() << curDir;

		cursWorkFile.setFileName(QDir::tempPath() + "/default.work.xml");

		saveCourseFile();
	} else {
		saveCourse();
	};

};

void MainWindowTask::openRescent()
{



	QAction *s = qobject_cast<QAction *>(sender());

	loadCourseFromFile(s->property("fullName").toString());
	//  if( LoadFromFile(RobotFile)!=0)QMessageBox::information( mainWidget(), "", QString::fromUtf8("Ошибка открытия файла! ")+RobotFile, 0,0,0);
	Q_EMIT activateRequest();
};

void MainWindowTask::setUpDown(QModelIndex index)
{
	if (!isTeacher) {
		ui->actionup->setEnabled(false);
		ui->actionDown->setEnabled(false);
	}
	return;
	if (course->hasUpSib(index)) {
		ui->actionup->setEnabled(true);
	} else {
		ui->actionup->setEnabled(false);
	}
	if (course->hasDownSib(index)) {
		ui->actionDown->setEnabled(true);
	} else {
		ui->actionDown->setEnabled(false);
	}
};


void MainWindowTask::moveUp()

{
	curTaskIdx = course->moveUp(curTaskIdx);
	QModelIndex par = curTaskIdx.parent();
	ui->treeView->setCurrentIndex(curTaskIdx);
	setUpDown(curTaskIdx);


	ui->treeView->collapse(par);
	ui->treeView->expand(par);
	saveBaseKurs();
};
void MainWindowTask:: moveDown()
{
	ui->treeView->setCurrentIndex(curTaskIdx);

	curTaskIdx = course->moveDown(curTaskIdx);
	ui->treeView->setCurrentIndex(curTaskIdx);
	setUpDown(curTaskIdx);
	//ui->treeView->dataChanged(curTaskIdx,ui->treeView->indexAbove(curTaskIdx));
	//showText(curTaskIdx);
	// ui->treeView->update(curTaskIdx);

	ui->treeView->collapse(curTaskIdx.parent());
	ui->treeView->expand(curTaskIdx.parent());
};


void MainWindowTask::showText(const QModelIndex &index)
{
	editRoot->hide();
	qDebug() << "TASK IDX:" << curTaskIdx.internalId();
	if (index.internalId() > 0) {
		setEditTaskEnabled(true);
	} else {
		setEditTaskEnabled(false);
	}
	setUpDown(index);
	if (index == curTaskIdx) {
		return;
	}
//    if(onTask)
//    {
//        QMessageBox::StandardButton ans;
//        ans = QMessageBox::question(this, trUtf8("Задание"), trUtf8("Вы хотите сменить задание?"),
//                                                                                                                        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel, QMessageBox::Yes);
//        if ((ans == QMessageBox::Cancel)||(ans == QMessageBox::No))
//        {
//            qDebug()<<"Cancel";
//        if(curTaskIdx.internalId()>0)ui->treeView->setCurrentIndex(curTaskIdx);
//            return;};

//    }
	if (curTaskIdx.internalId() > 0) {
		markProgChange();
	}
	onTask = false;
	ui->checkTask->setEnabled(false);
	QString taskText = course->getTaskText(index);
//ui->textBrowser->setText(taskText);



	if (taskText.right(4) == ".htm" || taskText.right(5) == ".html") {
		loadHtml(taskText);
	} else {
		setTaskViewHtml(taskText);
	}
	qDebug() << "TaskText:" << course->getTaskText(index);
	curTaskIdx = index;

	if (course->isTask(curTaskIdx.internalId()) && curTaskIdx.internalId() > 0 && course->taskAvailable(curTaskIdx.internalId())) {
		ui->do_task->setEnabled(true);
		startTask();
		ui->do_task->setEnabled(false);
		ui->actionReset->setEnabled(true);
		QString testedText = course->getUserTestedText(curTaskIdx.internalId());
		qDebug() << "TESTED TEXT" << testedText;
		if (testedText != "") {
			ui->actionTested->setEnabled(true);
		}
	} else {
		ui->do_task->setEnabled(false);
		ui->actionReset->setEnabled(false);
		ui->actionTested->setEnabled(false);
	};
};

void MainWindowTask::loadHtml(QString fileName)
{
	const QString absolutePath = QDir(curDir).absoluteFilePath(fileName);
	const QUrl url = QUrl::fromLocalFile(absolutePath);
	setTaskViewUrl(url);
//    qDebug()<<"LoadHtml"<<fileName;
//    if(fileName.isEmpty())return;
//    QFile inp(curDir+'/'+fileName);
//    if  (!inp.open(QIODevice::ReadOnly))
//    {
//    QMessageBox::information( 0, "", trUtf8("Ошибка чтения: ") + fileName, 0,0,0);
//    return;
//    };
//    QString htmlData=QString::fromUtf8(inp.readAll());
//    //ui->textBrowser->setHtml(htmlData);

//    ui->webView->setHtml(htmlData,QUrl("file://"+curDir+'/'+fileName));
//    inp.close();

};
void MainWindowTask::startTask()
{
	editRoot->hide();
	qDebug() << "StartTask";
	if (curTaskIdx.internalId() <= 0) {
		QMessageBox::about(NULL, trUtf8("Не выбрано задание"), trUtf8("Необходимо выбрать задание"));
		return;
	}
	if (course->csName(curTaskIdx.internalId()).toLower() != CS) {
		QMessageBox::about(NULL, trUtf8("Неправильное окружение"), trUtf8("Необходим ") + course->csName(curTaskIdx.internalId()));
		return;
	}

	QString progFile = course->progFile(curTaskIdx.internalId());
//interface->setTesting(loadTestAlg(course->getTaskCheck(curTaskIdx)));
	QFileInfo ioDir(curDir + "/" + course->progFile(curTaskIdx.internalId()));
	qDebug() << "PRG FILE" << course->progFile(curTaskIdx.internalId());
	if (ioDir.isFile()) {
		interface->setParam("input dir", ioDir.absoluteFilePath());
	};
	task.isps = course->Modules(curTaskIdx.internalId());
	task.name = course->getTitle(curTaskIdx.internalId());
	qDebug() << "ISPS" << task.isps << "task.name" << task.name;
	task.fields.clear();
	for (int i = 0; i < task.isps.count(); i++) {
		// task.Scripts.append(loadScript(course->Script(curTaskIdx.internalId(),task.isps[i])));
		QStringList t_fields = course->Fields(curTaskIdx.internalId(), task.isps[i]);
		qDebug() << "fields" << t_fields;
		task.fields.clear();
		for (int j = 0; j < t_fields.count(); j++) {
			qDebug() << "Cur Dir" << curDir;
			task.fields.insertMulti(task.isps[i], curDir + '/' + t_fields[j]);
			qDebug() << curDir + '/' + t_fields[j];

		};
		qDebug() << "Fields!!!!" << task.fields;
	}
	qDebug() << "MODULES:" << course->Modules(curTaskIdx.internalId());
	if (!interface->startNewTask(course->Modules(curTaskIdx.internalId()), &task)) {
		QMessageBox::about(NULL, trUtf8("Невозможно выполнить задание"), trUtf8("Нет необходимых исполнителей"));
	}
	if (course->getUserText(curTaskIdx.internalId()) != "") {
		interface->setPreProgram(QVariant(course->getUserText(curTaskIdx.internalId())));
		ui->actionReset->setEnabled(true);
	} else if (!progFile.isEmpty()) {
		interface->setPreProgram(QVariant(curDir + '/' + progFile));
	}



//qDebug()<<"Scripts "<<task.Scripts;
	ui->do_task->setEnabled(false);
	ui->checkTask->setEnabled(true);
	onTask = true;

	QModelIndex nextT = ui->treeView->indexBelow(curTaskIdx);
	if (nextT.isValid() && course->isTask(nextT.internalId()) && nextT.internalId() > 0 && course->taskAvailable(nextT.internalId())) {
		ui->actionNext->setEnabled(true);
	} else {
		ui->actionNext->setEnabled(false);
	}
//ui->loadCurs->setEnabled(false);
	qDebug() << "end load task";
	if (progChange.indexOf(curTaskIdx.internalId()) == -1) {
		progChange.append(curTaskIdx.internalId());
	}
};





void MainWindowTask::checkTask()
{
	qDebug() << "CheckTASK";
	if (!onTask) {
		qDebug() << "!onTASK";
		return;
	};


	markProgChange();
	if (!cursWorkFile.exists()) {

		QMessageBox::information(0, "", trUtf8("Нужно завести файл рабочей тетради "), 0, 0, 0);
		saveCourse();
	};
	course->setMark(curTaskIdx.internalId(), 0);

	qDebug() << "task" << task.isps;
	interface->startProgram(QVariant("TODO LOAD SCRIPT"), &task);
	//ui->loadCurs->setEnabled(false);

};

void MainWindowTask::lockControls()
{
	ui->splitter->setEnabled(false);
	ui->checkTask->setEnabled(false);
	ui->loadCurs->setEnabled(false);
	ui->actionNext->setEnabled(false);
};

void MainWindowTask::unlockControls()
{
	ui->splitter->setEnabled(true);
	ui->checkTask->setEnabled(true);
	ui->loadCurs->setEnabled(true);

	QModelIndex nextT = ui->treeView->indexBelow(curTaskIdx);
	if (nextT.isValid() && course->isTask(nextT.internalId()) && nextT.internalId() > 0 && course->taskAvailable(nextT.internalId())) {
		ui->actionNext->setEnabled(true);
	} else {
		ui->actionNext->setEnabled(false);
	}

};

void  MainWindowTask::setMark(int mark)
{
	ui->loadCurs->setEnabled(true);
	if (!onTask) {
		return;
	}
	ui->loadCurs->setEnabled(true);
	qDebug() << "ui->cource enabled!";
	ui->splitter->setEnabled(true);
	ui->actionTested->setEnabled(true);
	qDebug() << "ui->treeView enabled!";
	ui->checkTask->setEnabled(true);
	if ((course->taskMark(curTaskIdx.internalId()) < mark) && (course->taskMark(curTaskIdx.internalId()) > 0)) {
		return;
	}

	course->setUserTestedText(curTaskIdx.internalId(), interface->getText());
	qDebug() << "Mark:" << mark;
	course->setMark(curTaskIdx.internalId(), mark);
	changes.setMark(curTaskIdx.internalId(), mark);
	ui->treeView->dataChanged(curTaskIdx, curTaskIdx);
	//if(mark==10)onTask=false;else onTask=true;

};
QString MainWindowTask::loadScript(QString file_name)
{
	qDebug() << "Script file name:" << file_name;
	if (file_name.isEmpty()) {
		return "";
	}
	QFile file(curDir + "/" + file_name);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
		QMessageBox::about(NULL, trUtf8("Ошибка"), trUtf8("Невозможно открыть ") + curDir + "/" + file_name);
		return "";
	};
	return file.readAll();
};


void MainWindowTask::saveCourse()
{
	editRoot->hide();
	markProgChange();
//    QFileDialog dialog(this,trUtf8("Сохранить изменения"),curDir+"/", "Work files(*.work.xml);;All files (*)");
//    dialog.setDefaultSuffix("work.xml");
//    dialog.setAcceptMode(QFileDialog::AcceptSave);
//    if(!dialog.exec())return;
//    QFileInfo fi(dialog.selectedFiles().first());
//    //curDir=fi.absolutePath ();
//    qDebug()<<"curDir"<<curDir;
//    QString fileName=dialog.selectedFiles().first();
	QString open = curDir;
	QFileInfo fi = QFileInfo(curDir);
	if (!fi.isWritable()) {
		open = QDir::currentPath();
	}


	QString fileName = QFileDialog::getSaveFileName(
			this,
			trUtf8("Сохранить изменения"),
			open,
			trUtf8("Тетради(*.work.xml);;Все файлы (*)")
		);
	QString type = fileName.right(9);
	if (type != ".work.xml") {
		fileName += ".work.xml";
	}
	cursWorkFile.setFileName(fileName);
	updateLastFiles(fileName);
	saveCourseFile();

};
void MainWindowTask::saveCourseFile()
{
	if (isReadOnly) {
		return;
	}
	qDebug() << "Save cource file";
	QDomDocument saveXml;

	QDomElement crsEl = saveXml.createElement("COURSE");
	QDomElement fileEl = saveXml.createElement("FILE");
	QDomAttr crsAtt = saveXml.createAttribute("fileName");
	crsAtt.setValue(cursFile);
	fileEl.setAttributeNode(crsAtt);

	crsEl.appendChild(fileEl);
	saveXml.appendChild(crsEl);
	QDomElement mrksEl = saveXml.createElement("MARKS");

	//USER PROGRAMS n TESTED PROGRAMS
	for (int i = 0; i < progChange.count(); i++) {
		QDomElement prgEl = saveXml.createElement("USER_PRG");
		QDomAttr testIdprg = saveXml.createAttribute("testId");
		testIdprg.setValue(QString::number(progChange[i]));
		QDomAttr userPrg = saveXml.createAttribute("prg");
		userPrg.setValue(course->getUserText(progChange[i]));
		prgEl.setAttributeNode(testIdprg);
		prgEl.setAttributeNode(userPrg);
		crsEl.appendChild(prgEl);

		QDomElement prgElT = saveXml.createElement("TESTED_PRG");
		QDomAttr testIdprgT = saveXml.createAttribute("testId");
		testIdprgT.setValue(QString::number(progChange[i]));
		QDomAttr userPrgT = saveXml.createAttribute("prg");
		userPrgT.setValue(course->getUserTestedText(progChange[i]));
		prgElT.setAttributeNode(testIdprg);
		prgElT.setAttributeNode(userPrgT);
		crsEl.appendChild(prgElT);



	}
	//END USER PROGRAMS




	QMapIterator<int, int> i(changes.marksChanged);
	while (i.hasNext()) {
		i.next();
		QDomElement mrk = saveXml.createElement("MARK");
		QDomAttr testId = saveXml.createAttribute("testId");
		testId.setValue(QString::number(i.key()));
		QDomAttr mvalue = saveXml.createAttribute("mark");
		mvalue.setValue(QString::number(i.value()));
		mrk.setAttributeNode(testId);
		mrk.setAttributeNode(mvalue);
		mrksEl.appendChild(mrk);
	}
	crsEl.appendChild(mrksEl);





	if (!cursWorkFile.open(QIODevice::WriteOnly)) {
		QMessageBox::information(0, "", trUtf8("Ошибка записи: ") + cursWorkFile.fileName(), 0, 0, 0);
		return;
	};
	cursWorkFile.write("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
	cursWorkFile.write(saveXml.toByteArray());
	cursWorkFile.close();

};

void MainWindowTask::markProgChange()
{

	course->setUserText(curTaskIdx, interface->getText());
	if (progChange.indexOf(curTaskIdx.internalId()) == -1) {
		progChange.append(curTaskIdx.internalId());
	}
	if (!cursWorkFile.exists()) {
		qDebug() << "!cursWorkFile";
	} else {
		saveCourseFile();
	}

}
QString MainWindowTask::getFileName(QString fileName)
{
	QFileInfo finf(fileName);
	qDebug() << "GET FILE!";
//    QFileDialog dialog(this,trUtf8("Открыть файл"),curDir, finf.fileName()+" *.kurs.xml");
//    dialog.setAcceptMode(QFileDialog::AcceptOpen);
//    if(!dialog.exec())return "";
//    QFileInfo fi(dialog.selectedFiles().first());



	QString File = QFileDialog::getOpenFileName(this, QString::fromUtf8("Открыть файл"), curDir,  finf.fileName() + " *.kurs.xml");
	QFileInfo fi(File);


	return File;
};

void MainWindowTask::resetTask()
{
	QString progFile = course->progFile(curTaskIdx.internalId());


	if (!progFile.isEmpty()) {
		interface->setPreProgram(QVariant(curDir + '/' + progFile));
	}
	course->setUserText(curTaskIdx, "");


};
void MainWindowTask::Close()
{
	qDebug() << cursFile;
	if (cursFile != "") {
		markProgChange();
	}
	saveBaseKurs();
	close();
};
void MainWindowTask::showEvent(QShowEvent *event)
{
	ui->splitter->restoreState(settings->value("Window/SpliterState")
		.toByteArray());
	QByteArray settlist = settings->value("Window/SpliterPos").toByteArray();
	qDebug() << settlist;
	ui->splitter->restoreGeometry(settlist);
};

void MainWindowTask::hideEvent(QHideEvent *event)
{
	settings->setValue("Window/SpliterPos", ui->splitter->saveGeometry());
	settings->setValue("Window/SpliterState", ui->splitter->saveState());
	settings->flush();
};

void MainWindowTask::closeEvent(QCloseEvent *event)
{

	if (settings) {
		settings->setValue("Window/SpliterPos", ui->splitter->saveGeometry());
		settings->setValue("Window/SpliterState", ui->splitter->saveState());
		settings->flush();
	}

	if (!course) {
		return;
	}
	qDebug() << "START CLOSE TASK WINDOW";

	markProgChange();
	qDebug() << "CLOSE TASK WINDOW";
	event->accept();
	close();
};

bool MainWindowTask::safeToQuit()
{
	if (!course) {
		return true;
	}
	if (!cursWorkFile.exists()) {
		QMessageBox msgBoxCreateWorkbook(
			QMessageBox::Question,
			trUtf8("Практикум"),
			trUtf8("Вы хотите сохранить работу?"),
			QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
			this
		);
		msgBoxCreateWorkbook.button(QMessageBox::Yes)->setText(trUtf8("Да"));
		msgBoxCreateWorkbook.button(QMessageBox::No)->setText(trUtf8("Нет"));
		msgBoxCreateWorkbook.button(QMessageBox::Cancel)->setText(trUtf8("Отмена"));
		int ans = msgBoxCreateWorkbook.exec();
		//            ans = QMessageBox::question(this, trUtf8("Практикум"), trUtf8("Вы хотите создать тетрадь?"),
		//                    QMessageBox::Yes | QMessageBox::No , QMessageBox::Yes);
		if (ans == QMessageBox::Yes) {

			saveCourse();
			return true;
		};
		if (ans == QMessageBox::No) {
			return true;
		}
		if (ans == QMessageBox::Cancel) {
			return false;
		}
	}
	return true;
};
void MainWindowTask::returnTested()
{
	interface->setPreProgram(QVariant(course->getUserTestedText(curTaskIdx.internalId())));
};
QString MainWindowTask::loadTestAlg(QString file_name)
{
	if (file_name.isEmpty()) {
		return "";
	}
	QFile file(curDir + "/" + file_name);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {

		QMessageBox::about(NULL, trUtf8("Ошибка"), trUtf8("Невозможно открыть ") + curDir + "/" + file_name);
		return "";
	};
	QTextStream ts(&file);
	ts.setCodec("UTF-16LE");
	QString testalg = ts.readAll();
	file.close();

	qDebug() << "Test alg" << testalg;
	return testalg;
}
void MainWindowTask::aboutToQuit()
{
	close();
};
void MainWindowTask:: customContextMenuRequested(QPoint  pos)
{
	editRoot->hide();
	if (!isTeacher) {
		return;
	}

	curTaskIdx = ui->treeView->currentIndex();
	if (curTaskIdx.internalId() == 0) {
		setEditTaskEnabled(false);
		ui->actionEdit->setEnabled(true);
		ui->addDeep->setEnabled(true);
	} else {
		setEditTaskEnabled(true);
	}
	if (!baseKursFile.isReadable()) {
		return;
	};
	pos.setY(pos.y() + ui->mainToolBar->height());
	customMenu.move(pos + this->pos() + ui->centralWidget->pos());
	customMenu.show();
	qDebug() << "Menu Request!!!";
};
void MainWindowTask::addTask()
{
	qDebug() << "Add task";
	QModelIndex par = curTaskIdx.parent();
	course->addSiblingTask(curTaskIdx.internalId());
	ui->treeView->collapse(par);
	ui->treeView->expand(par);
};
void MainWindowTask::addDeepTask()
{
	qDebug() << "Add deep task";
	QModelIndex par = curTaskIdx.parent();
	course->addDeepTask(curTaskIdx.internalId());
	ui->treeView->collapse(par);
	ui->treeView->expand(par);
	saveBaseKurs();
	if (curTaskIdx.internalId() == 0) {
		loadCourseData(baseKursFile.absoluteFilePath());
	}
};
void MainWindowTask::deleteTask()
{
	QModelIndex par = curTaskIdx.parent();
	course->removeNode(curTaskIdx.internalId());
	ui->treeView->collapse(par);
	ui->treeView->expand(par);
};

void MainWindowTask::saveKurs()
{
	qDebug() << "Save Kurs teacher";
	QFile cursKursFile("../../test.kurs.xml");
	if (!cursKursFile.open(QIODevice::WriteOnly)) {
		QMessageBox::information(0, "", trUtf8("Ошибка записи: ") + cursKursFile.fileName(), 0, 0, 0);
		return;
	};

	cursKursFile.write(course->document()->toByteArray());
	cursKursFile.close();
	ui->actionEdit->setEnabled(true);
};
void MainWindowTask::saveBaseKurs()
{
	QFile cursKursFile(baseKursFile.absoluteFilePath());
	if (!cursKursFile.open(QIODevice::WriteOnly)) {
		QMessageBox::information(0, "", trUtf8("Ошибка записи: ") + cursKursFile.fileName(), 0, 0, 0);
		return;
	};

	cursKursFile.write(course->document()->toByteArray());
	cursKursFile.close();
}

void MainWindowTask::saveKursAs()
{
	QString dir = curDir;
	QDir chD(curDir);
	if (!chD.exists()) {
		dir = QDir::homePath();
	}
	QFileDialog dialog(this, trUtf8("Сохранить файл курса"), dir, "(*.kurs.xml )");
	dialog.setAcceptMode(QFileDialog::AcceptSave);

	if (!dialog.exec()) {
		return;
	}
	QFileInfo fi(dialog.selectedFiles().first());
	baseKursFile = fi;






	saveBaseKurs();

	;
};
void MainWindowTask:: setTeacher(bool mode)
{
	ui->actionup->setVisible(false);
	ui->actionDown->setVisible(false);
	isTeacher = mode;
};
void MainWindowTask::editTask()
{
//if(curTaskIdx.internalId()==0)//ROOT
//    {
//
//    QRect rect=ui->treeView->visualRect(curTaskIdx);
//    editRoot->resize(rect.width(),rect.height());
//     editRoot->setText(course->rootText());
//    editRoot->move(rect.topLeft());
//    editRoot->show();
//    return;
//     };
//    QModelIndex par=curTaskIdx.parent();
//
//editDialog->setTitle(course->getTitle(curTaskIdx.internalId()));
//editDialog->setDesc(course->getTaskText(curTaskIdx));
//editDialog->setProgram(course->progFile(curTaskIdx.internalId()));
//QStringList isps=course->Modules(curTaskIdx.internalId());
//
//if(isps.count()>0)editDialog->setUseIsps(isps.first());
//   else editDialog->setUseIsps("");
//
//if(course->Modules(curTaskIdx.internalId()).count()>0)editDialog->setEnvs(course->Fields(curTaskIdx.internalId(),isps.first()));
// else editDialog->setEnvs(QStringList());
//editDialog->setCurDir(curDir);
// if(editDialog->exec ())
// {
//  course->setUserText(curTaskIdx.internalId(),"");
//  course->setTitle(curTaskIdx.internalId(),editDialog->getTitle());
//  course->setDesc(curTaskIdx.internalId(),editDialog->getDesc());
//  qDebug()<<"Desc"<<editDialog->getDesc();
//  course->setProgram(curTaskIdx.internalId(),editDialog->getProgram());
//  qDebug()<<"EDIT ISPS"<<editDialog->getUseIsps();
//
//
//  course->setIsps(curTaskIdx,editDialog->getUseIsps());
//  qDebug()<<"PRG"<<editDialog->getProgram();
//  if(course->Modules(curTaskIdx.internalId()).count()>0)course->setIspEnvs(curTaskIdx,course->Modules(curTaskIdx.internalId()).first(),editDialog->getEnvs());
// showText(curTaskIdx);
//  ui->treeView->collapse(par);
//  ui->treeView->expand(par);
//  qDebug()<<"Set task isps:"<<course->Modules(curTaskIdx.internalId());
//  qDebug()<<"EDIT DIALOG EXEC OK";
//  saveBaseKurs();
//  resetTask();
// };
//
};

void MainWindowTask::setEditTaskEnabled(bool flag)
{
	// ui->actionEdit->setEnabled(flag);
	ui->actionup->setEnabled(flag);
	ui->actionDown->setEnabled(flag);

	ui->actionSaveKas->setEnabled(flag);
	ui->actionAdd->setEnabled(flag);
	ui->actionRemove->setEnabled(flag);
	ui->addDeep->setEnabled(flag);
};

void MainWindowTask::newKurs()
{

// if(newDialog->exec())
// {
//    QFile newKurs(newDialog->fileName());
//
//    if  (!newKurs.open(QIODevice::WriteOnly))
//    {
//    QMessageBox::information( 0, "", trUtf8("Ошибка записи: ") + newKurs.fileName(), 0,0,0);
//    return;
//    };
//    QString toWr="<?xml version='1.0' encoding='UTF-8'?>\n";
//    newKurs.write(toWr.toUtf8());
//    toWr="<KURS xml:id=\"0\" xml:name=\""+newDialog->name()+"\">\n";
//    newKurs.write(toWr.toUtf8());
//
//    toWr=QString::fromUtf8("<T xml:id=\"1\" xml:name=\"Новое задание\">\n<DESC>Нет Описания</DESC>\n<CS>Кумир</CS>\n <ISP xml:ispname=\"Robot\">\n</ISP>\n<READY>false</READY>\n</T>\n");
//    newKurs.write(toWr.toUtf8());
//
//    toWr="</KURS>\n";
//    newKurs.write(toWr.toUtf8());
//    newKurs.close();
//    loadCourseData(newKurs.fileName());
//    baseKursFile=QFileInfo(newKurs);
//    curDir=baseKursFile.absolutePath();
//    ui->actionEdit->setEnabled(true);
// }

};
void  MainWindowTask::createMoveMenu()
{

}
void MainWindowTask::endRootEdit()
{
	course->setRootText(editRoot->text());
	editRoot->hide();
}
void MainWindowTask::cancelRootEdit()
{
	editRoot->hide();
};
void MainWindowTask::lockCheck()
{
	ui->checkTask->setDisabled(true);
}

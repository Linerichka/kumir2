#include "course_model.h"
#include <kumir2/coursesinterface.h>

#include <QSize>
#include <QColor>
#include <QBrush>
#include <QIcon>
#include <QMessageBox>

static const int MARK_BLOCK  = 12;

static QString fixWindowPath(QString fileName)
{
	// Some great course authors edit xml files by hands and
	// use f*cking operating system that uses '\' directory
	// separator instead of '/'. The code below is to fix
	// these entries.

	// RegExp: \\(\S)     '\' then any non-space symbol
	//                    The only reason to use \ in good systems
	//                    is to screen space symbols, so take it
	//                    in care. If there is non-space symbol
	//                    after '\' then this is Windows path
	static QRegExp rxManuallyEditedWindowsPath("\\\\(\\S)");
	int symbolPos = 0;
	while (-1 != (symbolPos = rxManuallyEditedWindowsPath.indexIn(fileName))) {
		const QString symbolAfterBackSlash = rxManuallyEditedWindowsPath.cap(1);
		fileName.replace(symbolPos, 2, "/" + symbolAfterBackSlash);
	}
	return fileName;
}

courseModel::courseModel() : QAbstractItemModel()
{
	isTeacher = false;
	itemFont = QFont("Helvetica [Cronyx]");
//#ifdef Q_OS_WIN32
//           markIcons.append(QIcon(":/out_stand.png"));
//           markIcons.append(QIcon(":/1.png"));
//           markIcons.append(QIcon(":/2.png"));
//           markIcons.append(QIcon(":/3.png"));
//           markIcons.append(QIcon(":/4.png"));
//           markIcons.append(QIcon(":/5.png"));
//           markIcons.append(QIcon(":/6.png"));
//           markIcons.append(QIcon(":/7.png"));
//           markIcons.append(QIcon(":/8.png"));
//           markIcons.append(QIcon(":/9.png"));
//           markIcons.append(QIcon(":/10.png"));
//           markIcons.append(QIcon(":/m.png"));
//
//           markIcons.append(QIcon(":/folder_close.png"));
//           markIcons.append(QIcon(":/folder_1.png"));
//           markIcons.append(QIcon(":/folder_2.png"));
//           markIcons.append(QIcon(":/folder_3.png"));
//           markIcons.append(QIcon(":/folder_4.png"));
//           markIcons.append(QIcon(":/folder_5.png"));
//           markIcons.append(QIcon(":/folder_6.png"));
//           markIcons.append(QIcon(":/folder_7.png"));
//           markIcons.append(QIcon(":/folder_8.png"));
//           markIcons.append(QIcon(":/folder_9.png"));
//           markIcons.append(QIcon(":/folder_10.png"));
//           markIcons.append(QIcon(":/folder_open.png"));
//           return;
//#endif
	const ExtensionSystem::KPlugin *csmanager =
		ExtensionSystem::PluginManager::instance()
		->findKPlugin<Shared::CoursesInterface>();
	Q_ASSERT(csmanager);
	QDir resourcesRoot = csmanager->myResourcesDir();
	Q_ASSERT(resourcesRoot.exists());
//         QUrl::fromLocalFile(qApp->property("sharePath").toString()+
//                                            "/coursemanager/out_stand.svg"
//                                            );     // WTF?
	bool hasX11 = true;
#ifdef Q_OS_LINUX
	hasX11 = 0 != getenv("DISPLAY");
#endif
	if (hasX11) {
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("out_stand.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("1.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("2.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("3.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("4.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("5.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("6.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("7.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("8.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("9.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("10.png")));
		markIcons.append(QIcon(":/m.png"));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_close.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_1.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_2.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_3.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_4.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_5.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_6.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_7.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_8.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_9.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_10.png")));
		markIcons.append(QIcon(resourcesRoot.absoluteFilePath("folder_open.png")));
	}
}

int courseModel::loadCourse(QString file, bool cmode)
{
	courseFileName = file;
	qDebug() << "courseModel::Load Course " << file;
	QFile f(courseFileName);
	if (!f.open(QIODevice::ReadOnly)) {
		qDebug() << "cant open" << courseFileName;
		return -1;
	}

	if (f.atEnd()) {
		return -1;
	}

	QString error = "";
	int eline, ecol;
	QString taskData;
	QTextStream ts(&f);
	ts.setCodec("UTF-8");
	taskData.append(ts.readAll());
	qDebug() << "Cource len in symbols:", taskData.length();
	if (!courceXml.setContent(taskData, true, &error, &eline, &ecol)) {
		qDebug() << "ERROR LOADING CB:" << error << " File" << file << " LINE" << eline << "COL" << ecol;
		if (cmode) {
			return -1;
		}
		QMessageBox::information(0, "", "Error:" + error + " File:" + file, 0, 0, 0);

	}
	f.close();

	qDebug() << "File parce:" << error;

	int count = 0;
	taskCount = count;
	root = courceXml.documentElement();
	insertRow(0);
	insertColumn(0);
	setData(createIndex(0, 0), QVariant());
	buildCash();

	return count;
}


int courseModel::rowCount(const QModelIndex &parent) const
{
	// qDebug()<<"RowCount"<< parent;
	if (!parent.isValid()) {
		// qDebug()<<"NOT VALID"<<" count"<<root.childNodes().length();
		return 1;
	}
	QDomNode node = nodeById(parent.internalId(), root);

	return subTasks(node);
}


QIcon courseModel::iconByMark(int mark, bool isFolder) const
{
	if (isFolder) {
		mark = mark + MARK_BLOCK;
	}
	// qDebug()<<"Mark:"<<mark;
	if ((mark > -1) && (mark < 2 * MARK_BLOCK)) {
		return markIcons[mark];
	}

	return QIcon(":/x.png");
}


QVariant courseModel::data(const QModelIndex &index, int role) const
{
	//qDebug()<<"Get data"<<index<<" role"<<role;
	if (!index.isValid()) {
		return QVariant();
	}
	QDomNode node = nodeById(index.internalId(), root);

	if (role == Qt::DisplayRole) {

		QString title = node.toElement().attribute("name", "");

		return QVariant(title);
	}

	if (role == Qt::SizeHintRole) {
		QDomNode nodeM = nodeById(index.internalId(), root);
		if (nodeM.toElement().attribute("root") == "true") {
			return QVariant(QSize(30, 30));
		}
		return QVariant(QSize(30, 30));
	}

	if (role == Qt::FontRole) {
		return QVariant();
	}

	if (role == Qt::TextAlignmentRole) {
		return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
	}
	//     if(role==Qt::ForegroundRole)
	//     {
	//         if(isTeacher)QVariant(QBrush(QColor(0,0,0)));
	//        if(!taskAvailable(node))
	//            return QVariant(QBrush(QColor(150,150,150)));
	//       return QVariant(QBrush(QColor(0,0,0)));
	//     }
	if (role == Qt::CheckStateRole) {
		return QVariant();
	}
	if (role == Qt::DecorationRole) {
		QDomNode nodeM = nodeById(index.internalId(), root);
		// qDebug()<<"Draw Mark id"<<index.internalId();
		if (nodeM.toElement().attribute("root") == "true") {
			//  qDebug()<<"Folder";
		}
		return iconByMark(taskMark(index.internalId()), nodeM.toElement().attribute("root") == "true");
		//NUZHNO IKONKI ISPOLNITELEY
	}
	//     if(role==Qt::BackgroundRole)
	//     {
	//        return QBrush(QColor(255,255,255));
	//     }
	//   qDebug()<<"No" << role<< "role";
	return QVariant();
}

QVariant courseModel::headerData(int section, Qt::Orientation orientation,
	int role) const
{
	//qDebug()<<"Get Header";
	return QVariant(courseFileName);
}

QModelIndex courseModel::index(int row, int column, const QModelIndex &parent) const
{
	//  qDebug()<<"index"<<" ROW:"<<row<<" column:"<<column;
	if (!hasIndex(row, column, parent)) {
		return QModelIndex();
	}
	//  qDebug()<<root.attribute("id","");
	return createMyIndex(row, column, parent);
}

int courseModel::domRow(QDomNode &child) const //TODO Check
{
	QDomNode par = child.parentNode();
	QDomNodeList list = par.childNodes();
	for (int i = 0; i < list.count(); i++) {
		if (child == list.at(i)) {
			return i;
		}
	}
	return 0;
}

QModelIndex courseModel::parent(const QModelIndex &child) const
{

	if (!child.isValid()) {
		return QModelIndex();
	}
	if (child.internalId() == 0) {
		return QModelIndex();
	}
	QDomNode child_n = nodeById(child.internalId(), root);
	// if(child_n.isNull())return QModelIndex();
	QDomNode par = child_n.parentNode();
	if (par.toElement().attribute("id").toInt() == 0) {
		return createIndex(0, 0);
	}
	return createIndex(domRow(par), 0, idByNode(par));
}

int courseModel::columnCount(const QModelIndex &parent) const
{
	return 1;
}


QDomNode courseModel::nodeByRowColumn(int row, int column, QDomNode *parent) const
{
	if (!parent) {
		return root;
	}
	// qDebug()<<"nodeByRowColumn";
	return parent->childNodes().at(row);

	return root;
}

QDomNode courseModel::nodeBySIdI(const QString &sid, const QDomNode &parent) const
{
	if (!parent.hasChildNodes()) {
		return QDomNode();
	}

	QDomNodeList childs = parent.childNodes();

	for (int i = 0; i < childs.size(); i++) {
		if (childs.at(i).toElement().attribute("id") == sid) {
			return childs.at(i);
		}
	}

	for (int i = 0; i < childs.size(); i++) {
		if (childs.at(i).hasChildNodes()) {
			QDomNode ret = nodeBySIdI(sid, childs.at(i));
			if (!ret.isNull()) {
				return ret;
			}
		}
	}

	return QDomNode();
}


QDomNode courseModel::nodeById(int id, const QDomNode &parent) const
{
	QString sid = QString::number(id);

	if (parent.toElement().attribute("id") == sid) {
		return parent;
	}

	QDomNode val = cash.value(id);
	if (!val.isNull() && val.toElement().attribute("id") == sid) {
		return val;
	}

	val = nodeBySIdI(sid, parent);
#if 0 // correct work of hashing is to be tested hard
	if (!val.isNull()) {
		cash.insert(id, val);
	}
#endif
	return val;
}


Qt::ItemFlags courseModel::flags(const QModelIndex &index) const
{
	if (!index.isValid()) {
		return 0;
	}
	// qDebug()<<"IS teacher:"<<isTeacher;
	if (isTeacher) {
		return  Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled;
	}

	if (!taskAvailable(index.internalId())) {
		return Qt::NoItemFlags;
	}
	// qDebug()<<"Flags:"<<index.internalId();
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QModelIndex courseModel::createMyIndex(int row, int column, QModelIndex parent) const
{
	if (!parent.isValid()) {
		return createIndex(0, 0);
	}
	int id = parent.internalId();
	if (id < 0) {
		return QModelIndex();
	}
	QDomNode par = nodeById(id, root);
	if (par.isNull()) {
		return QModelIndex();
	}
	QDomNodeList childs = par.childNodes();
	if (childs.count() <= row) {
		return QModelIndex();
	}
	bool ok;
	int new_id = childs.at(row).toElement().attribute("id", "").toInt(&ok);
	if (!ok) {
		//   qDebug()<<"Bad ID";
		return createIndex(-10, -10, -10);

	}
	return createIndex(row, column, new_id);
}

QString courseModel::getTaskText(QModelIndex index)
{
	if (!index.isValid()) {
		return "INDEX NOT VALID";
	}
	QDomNode node = nodeById(index.internalId(), root);
	QDomElement titleEl = node.firstChildElement("DESC");
	if (titleEl.isNull()) {
		return "";
	}

	return titleEl.text();
}

QString courseModel::getTaskCheck(QModelIndex index)
{
	if (!index.isValid()) {
		return "INDEX NOT VALID";
	}
	QDomNode node = nodeById(index.internalId(), root);
	QDomElement titleEl = node.firstChildElement("CHECK");
	if (titleEl.isNull()) {
		return "";
	}

	return titleEl.text();
}

QString courseModel::csName(int index)
{

	QDomNode node = nodeById(index, root);
	QDomElement csEl = node.firstChildElement("CS");
	if (csEl.isNull()) {
		return "NO CS";
	}
	return csEl.text();
}

QString courseModel::progFile(int index)
{

	QDomNode node = nodeById(index, root);
	QDomElement csEl = node.firstChildElement("PROGRAM");
	if (csEl.isNull()) {
		return "";
	}
	return fixWindowPath(csEl.text());
}

QStringList courseModel::Modules(int index)
{

	QDomNode node = nodeById(index, root);

	QDomElement csEl = node.firstChildElement("ISP");
	// qDebug()<<"csEl isNull:"<<csEl.isNull();

	QStringList modules;
	while (!csEl.isNull()) {
		modules << csEl.attribute("ispname");
		csEl = csEl.nextSiblingElement("ISP");
	}
	return modules;
}

void courseModel::setIsps(QModelIndex index, QStringList isp)
{
	QDomNode node = nodeById(index.internalId(), root);
	QDomElement csEl = node.firstChildElement("ISP");
	while (!csEl.isNull()) {
		node.removeChild(csEl);
		csEl = node.firstChildElement("ISP");
	}

	for (int i = 0; i < isp.count(); i++) {
		if (isp.at(i) == "") {
			continue;
		}
		QDomText text = courceXml.createTextNode(isp.at(i));
		qDebug() << "Append ISP" << isp.at(i);
		QDomElement ispEl = courceXml.createElement("ISP");
		ispEl.setAttribute("xml:ispname", isp.at(i));
		node.toElement().appendChild(ispEl);
	}
}

void courseModel::setIspEnvs(QModelIndex index, QString isp, QStringList Envs)
{
	QDomNode node = nodeById(index.internalId(), root);
	QDomElement csEl = node.firstChildElement("ISP");
	while (!csEl.isNull()) {
		//  qDebug()<<"ISP NAME"<<csEl.attribute("ispname")<<" isp:"<<isp;
		//modules<<csEl.attribute(ispName);
		if (csEl.attribute("ispname") == isp) {
			QDomNodeList childList = csEl.childNodes();
			for (int j = 0; j < childList.count(); j++) {
				if (childList.at(j).nodeName() == "ENV") {
					csEl.removeChild(childList.at(j));
				}
			}

			for (int i = 0; i < Envs.count(); i++) {
				QDomText text = courceXml.createTextNode(Envs.at(i));
				qDebug() << "Append Env" << Envs.at(i);
				QDomElement envEl = courceXml.createElement("ENV");
				csEl.toElement().appendChild(envEl);
				envEl.appendChild(text);
			}

		}
		csEl = csEl.nextSiblingElement("ISP");
	}
}

QStringList courseModel::Fields(int index, QString isp)
{
	QDomNode node = nodeById(index, root);
	QDomElement csEl = node.firstChildElement("ISP");

	QStringList fields;

	while (!csEl.isNull()) {
		// qDebug()<<"ISP NAME"<<csEl.attribute("ispname")<<" isp:"<<isp;
		//modules<<csEl.attribute(ispName);
		if (csEl.attribute("ispname") == isp) {
			QDomElement fieldEl = csEl.firstChildElement("ENV");
			while (!fieldEl.isNull()) {
				//qDebug()<<"fiield:"<<fieldEl.text();
				QString fieldFileName = fixWindowPath(fieldEl.text());
				fields.append(fieldFileName);
				fieldEl = fieldEl.nextSiblingElement("ENV");
			}
			//  qDebug()<<"Return fiield:"<<fields;
			return fields;
		}
		csEl = csEl.nextSiblingElement("ISP");
	};
	return fields;
}

QString courseModel::Script(int index, QString isp)
{
	QDomNode node = nodeById(index, root);
	QDomElement csEl = node.firstChildElement("ISP");

	while (!csEl.isNull()) {

		if (csEl.attribute("ispname", "") == isp) {
			QDomElement fieldEl = csEl.firstChildElement("CONTROL");
			qDebug() << "Script name from XML:" << fieldEl.text();
			return fieldEl.text();
		}
		csEl = csEl.nextSiblingElement("ISP");
	}
	return "";
}


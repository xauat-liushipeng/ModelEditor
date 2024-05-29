#include "modeleditor.h"

// MyEarth��Ĺ��캯�����̳���QWidget
ModelEditor::ModelEditor(QWidget* parent) : QMainWindow(parent) {
	// ����һ����ɫ͸������
	m_material = new osg::Material();
	m_material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
	m_material->setAlpha(osg::Material::FRONT_AND_BACK, 0.5);
	m_material->setName("yellow");

	osgEarth::initialize();
	initWindowContent();

	// ����osg::Group���ڵ㣬����osg�����ĸ��ڵ�
	m_root = new osg::Group;

	// ����osgEarth�ĵ��������
	m_em = new osgEarth::Util::EarthManipulator;

	// ���ļ��ж�ȡ����ģ�ͽڵ�
	m_earth = osgDB::readNodeFile("simple.earth");
	earth_mnode = osgEarth::MapNode::findMapNode(m_earth);

	// ������ģ�ͽڵ���ӵ����ڵ���
	m_root->addChild(earth_mnode);

	//����osgQOpenGLWidget���󣬲�������Ϊ�ӿؼ���ӵ���ǰ����
	m_osgqopenglWidget = new osgQOpenGLWidget(this);

	// ����osgQOpenGLWidget�ĳ�ʼ���źŵ�MyEarth��initWindow�ۺ���
	connect(m_osgqopenglWidget, SIGNAL(initialized()), this, SLOT(initWindow()));

	// ����osgQOpenGLWidget����ʾ�ߴ�Ϊ��ǰ���ڵĳߴ�
	m_osgqopenglWidget->setFixedSize(1500, 1000);
	this->setCentralWidget(m_osgqopenglWidget);

 	m_GUIEventHandler = new MyGUIEventHandler(m_em);
	connect(m_GUIEventHandler, &MyGUIEventHandler::selected, this, [=]() {
		// ��һ�μ�¼��node������գ�ָ�븳ֵnullptr
		if (m_lastSelectNode)
		{
			m_lastSelectNode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::MATERIAL);
			m_lastSelectNode = nullptr;
		}
		// ��ȡ��nodeΪ�գ�����
		if (!m_GUIEventHandler->getLastSelectedNode())
		{
			return;
		}
		// group���������ӣ������û�ɫ���ʸ���
		m_lastSelectNode = m_GUIEventHandler->getLastSelectedNode();
		qDebug() << "node: " << QString::fromStdString(m_lastSelectNode->getName());
		m_lastSelectNode->getOrCreateStateSet()->setAttribute(m_material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		});

	//loadCameraImage("C:\\Users\\ShipengWork\\Documents\\data\\Ӱ��\\��ɽ_��ͼ_Level_19_4490.tif");
	//loadTerrain("C:\\Users\\ShipengWork\\Documents\\data\\Ӱ��\\��ɽDEM_4490.tif");
	//loadObj("C:\\Users\\ShipengWork\\Documents\\data\\2.obj");
}

void ModelEditor::initWindowContent() {
	setWindowTitle("OSGEarth Visualization");
	setFixedSize(1600, 1040);

	// �����˵��������ҽ���һ��
	QMenuBar* menu_bar = menuBar();
	// �˵������봰����
	setMenuBar(menu_bar);
	// ��Ӳ˵�����
	QMenu* file_menu = menu_bar->addMenu("�ļ�");
	QAction* act_save = file_menu->addAction("����");
	QAction* act_saveAs = file_menu->addAction("���Ϊ");
	QAction* act_openCImage = file_menu->addAction("����Ӱ��");
	QAction* act_openTerrain = file_menu->addAction("���ص���");
	QAction* act_openObj = file_menu->addAction("������Ĥ");
	connect(act_save, &QAction::triggered, this, [=]() {
		if (m_layers.contains("object")) {
			this->save(m_fileNames["object"]);
		}});
	connect(act_saveAs, &QAction::triggered, this, [=]() {
		if (m_layers.contains("object")) {
			this->saveAs();
		}});
	connect(act_openCImage, &QAction::triggered, this, [=]() {
		QString str = QFileDialog::getOpenFileName(this, "���ļ�",
			".", "(*.tif)");
		loadCameraImage(str);
		});
	connect(act_openTerrain, &QAction::triggered, this, [=]() {
		QString str = QFileDialog::getOpenFileName(this, "���ļ�",
			".", "(*.tif)");
		loadTerrain(str);
		});
	connect(act_openObj, &QAction::triggered, this, [=]() {
		QString str = QFileDialog::getOpenFileName(this, "���ļ�",
			".", "(*.obj)");
		loadObj(str);
		});

	QMenu* edit_menu = menu_bar->addMenu("ģ���Ƴ�");
	QAction* act_removeCam = edit_menu->addAction("�Ƴ�Ӱ��");
	QAction* act_removeTer = edit_menu->addAction("�Ƴ�����");
	QAction* act_removeObj = edit_menu->addAction("�Ƴ���Ĥ");
	connect(act_removeCam, &QAction::triggered, this, [=]() {
		if (!m_layers.contains("camera_image")) {
			QMessageBox::information(this, "��ʾ", "��ǰû�м���Ӱ��");
		}
		else {
			earth_mnode->getMap()->removeLayer((osgEarth::GDALImageLayer*)m_layers["camera_image"]);
			m_layers["camera_image"] = nullptr;
			m_layers.remove("camera_image");
		}});
	connect(act_removeTer, &QAction::triggered, this, [=]() {
		if (!m_layers.contains("terrain")) {
			QMessageBox::information(this, "��ʾ", "��ǰû�м��ص���");
		}
		else {
			earth_mnode->getMap()->removeLayer((osgEarth::GDALElevationLayer*)m_layers["terrain"]);
			m_layers["terrain"] = nullptr;
			m_layers.remove("terrain");
		}});
	connect(act_removeObj, &QAction::triggered, this, [=]() {
		if (!m_layers.contains("object")) {
			QMessageBox::information(this, "��ʾ", "��ǰû�м��ذ�Ĥ");
		}
		else {
			earth_mnode->getMap()->removeLayer((osgEarth::ModelLayer*)m_layers["object"]);
			m_layers["object"] = nullptr;
			m_layers.remove("object");
		}});

	QMenu* view_menu = menu_bar->addMenu("�ӵ�");
	QAction* act_defaultView = view_menu->addAction("Ĭ��");
	QAction* act_ChinaView = view_menu->addAction("�й�");
	QAction* act_huashanView = view_menu->addAction("��ɽ");
	connect(act_defaultView, &QAction::triggered, this, [=]() {
		m_em->setViewpoint(osgEarth::Viewpoint("Ĭ��",
			-90.000000000000000, 0.0000000000000000, -9.3132257461547852e-10,
			-0.00023781259862716095, -89.000000000000099, 19134411.000000000), 1.5);
		});
	connect(act_ChinaView, &QAction::triggered, this, [=]() {
		m_em->setViewpoint(osgEarth::Viewpoint("�й�",
			108.05017287467311, 33.927468708848174, -1324.4244584860280,
			-0.00023781259862716095, -88.953905245834946, 19142359.855145693), 1.5);
		});
	connect(act_huashanView, &QAction::triggered, this, [=]() {
		m_em->setViewpoint(osgEarth::Viewpoint("��ɽ",
			110.055998715991, 34.5245844139754, 652.88835630845279,
			1.1812922265385689e-06, -88.999583188869821, 18851.538335830206), 3);
		});


	// ����ģ�͵Ĺ����������
	QToolBar* tool_bar = new QToolBar(this);
	// ���������봰���У�ͣ��λ��Ĭ�������棬�����Ϊ���
	addToolBar(Qt::LeftToolBarArea, tool_bar);
	// ���ý�ֹ�ƶ���������ͣ��ʧЧ��
	tool_bar->setMovable(false);
	// ��ӷָ���
	tool_bar->addAction(act_openCImage);
	tool_bar->addSeparator();
	tool_bar->addAction(act_openTerrain);
	tool_bar->addSeparator();
	tool_bar->addAction(act_openObj);
}

void ModelEditor::keyPressEvent(QKeyEvent* event) {
	//����
	if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S) {
		if (m_layers.contains("object")) {
			this->save(m_fileNames["object"]);
		}
	}
	
	if (m_lastSelectNode) {
		// ѡ��node��ctrl C ��ϼ�����
		if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
			//m_lastSelectNode��Ϊnode��selection����Ҫ���ѡ��node��dragger����m_lastSelectNode���¸�ֵ
			m_GUIEventHandler->lastNodeDraggerClear();

			// ��ȡԭʼ�ڵ�ĸ��ڵ�
			osg::Group* parent = m_lastSelectNode->getParent(0);
			if (parent) {
				// ��¡ԭʼ�ڵ㣬����ԭʼ�ڵ㣨x+10,y+10����λ��
				osg::Node* clonedNode = dynamic_cast<osg::Node*>(m_lastSelectNode->clone(osg::CopyOp::DEEP_COPY_ALL));
				tmpvis tv(20, 20, 0);
				clonedNode->accept(tv);
				// ԭʼ�ڵ��ɫ����ɾ�� 
				m_lastSelectNode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::MATERIAL);
				// ԭʼ�ڵ�ָ���ÿգ�ָ���¡�ڵ㣬��Ӳ���
				m_lastSelectNode = nullptr;
				m_lastSelectNode = clonedNode;
				m_lastSelectNode->getOrCreateStateSet()->setAttribute(m_material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
				// ����¡�ڵ���ӵ�ԭʼ�ڵ�ĸ��ڵ���
				parent->addChild(m_lastSelectNode);
			}
		}

		// ѡ��node��Tab���л�dragger
		if (event->key() == Qt::Key_Tab) {
			m_GUIEventHandler->switchDraggerType();
		}

		// ѡ��node��delete��ɾ��node
		if (event->key() == Qt::Key_Delete) {
			//m_lastSelectNode��Ϊnode��selection����Ҫ���ѡ��node��dragger
			m_GUIEventHandler->lastNodeDraggerClear();
			
			osg::Group* parent = m_lastSelectNode->getParent(0);
			if (parent) {
				parent->removeChild(m_lastSelectNode);
				m_lastSelectNode = nullptr;
			}
		}
	}
}

void ModelEditor::save(QString saveName) {
	if (m_lastSelectNode) {
		//���dragger
		m_GUIEventHandler->lastNodeDraggerClear();
		//�������
		m_lastSelectNode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::MATERIAL);
	}

	if (saveName.size() != 0) {
		osgEarth::ModelLayer* ml = static_cast<osgEarth::ModelLayer*> (m_layers["object"]);
		osg::Node* node = ml->getNode();

		// ���
		osg::Node* nodeClone = static_cast<osg::Node*> (node->clone(osg::CopyOp::DEEP_COPY_ALL));

		// ��ȡ�任����������
		osg::MatrixTransform* matrixTranform = dynamic_cast<osg::MatrixTransform*> (nodeClone->asGroup()->getChild(0));
		osg::Matrix matrix = matrixTranform->getMatrix();
		osg::Matrix matrix_invert;
		matrix_invert.invert(matrix);

		// visitor���껹ԭ
		tmpvis nodeVisitor(matrix_invert);
		nodeClone->accept(nodeVisitor);

		osgDB::writeNodeFile(*nodeClone, saveName.toStdString(), new osgDB::Options("noRotation"));

		nodeClone = nullptr;
	}
}

void ModelEditor::saveAs() {
	QString saveName = QFileDialog::getSaveFileName(this, "���Ϊ",".", "(*.obj)");
	this->save(saveName);
}

void ModelEditor::loadCameraImage(QString fileFullName) {
	osgEarth::GDALImageLayer* ly = new osgEarth::GDALImageLayer();
	ly->setURL(fileFullName.toLocal8Bit().data());
	ly->setName("camera");
	earth_mnode->getMap()->addLayer(ly);
	auto a =  ly->getExtent();
	
	m_layers["camera_image"] = ly;
	m_fileNames["camera_image"] = fileFullName;
}

void ModelEditor::loadTerrain(QString fileFullName) {
	osgEarth::GDALElevationLayer* ly = new osgEarth::GDALElevationLayer();
	ly->setURL(fileFullName.toLocal8Bit().data());
	ly->setName("terrain");
	earth_mnode->getMap()->addLayer(ly);

	m_layers["terrain"] = ly;
	m_fileNames["terrain"] = fileFullName;
}

void ModelEditor::loadObj(QString fileFullName, double x, double y, double z) {
	//��������ϵ
	osgEarth::SpatialReference* wgs84 = osgEarth::SpatialReference::get("wgs84");
	osgEarth::GeoPoint position(wgs84, x, y, z);
	osgEarth::ModelLayer::Options opt;
	opt.osgOptionString() = "noRotation";
	//��ȡobj�ļ�
	osg::ref_ptr<osgEarth::ModelLayer> modelLayer = new osgEarth::ModelLayer(opt);
	modelLayer->setURL(fileFullName.toLocal8Bit().data());
	modelLayer->setLocation(position);
	modelLayer->setName("arches");
	//�ӵ�����ڵ�
	earth_mnode->getMap()->addLayer(modelLayer);

	m_layers["object"] = modelLayer;
	m_fileNames["object"] = fileFullName;
}

// ��ʼ�����ڵĲۺ���
void ModelEditor::initWindow() {
	// ��ȡosgQOpenGLWidget�е�osgViewer����
	m_viewer = m_osgqopenglWidget->getOsgViewer();

	// ����osgViewer�����������Ϊ���������
	m_viewer->setCameraManipulator(m_em);

	// ����osgViewer�ĳ�������Ϊ���ڵ�
	m_viewer->setSceneData(m_root.get());

	//����¼�������
	m_viewer->addEventHandler(m_GUIEventHandler);

	//�������ͶӰ����Ϊ͸��ͶӰ������ֹ���ڱ��
	m_viewer->getCamera()->setProjectionMatrixAsPerspective(
		30.0f, // ��Ұ�Ƕ�
		1.5f, // ��߱�
		1.0f,  // ���ü������
		10000.0f // Զ�ü������
	);
}

// MyEarth�����������
ModelEditor::~ModelEditor() {
	// ���m_osgqopenglWidget��ΪNULL����ɾ����������ΪNULL
	if (m_osgqopenglWidget != NULL) {
		delete m_osgqopenglWidget;
		m_osgqopenglWidget = NULL;
	}
}
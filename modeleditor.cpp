#include "modeleditor.h"

// MyEarth类的构造函数，继承自QWidget
ModelEditor::ModelEditor(QWidget* parent) : QMainWindow(parent) {
	// 创建一个黄色透明材质
	m_material = new osg::Material();
	m_material->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 0.0f, 1.0f));
	m_material->setAlpha(osg::Material::FRONT_AND_BACK, 0.5);
	m_material->setName("yellow");

	osgEarth::initialize();
	initWindowContent();

	// 创建osg::Group根节点，用于osg场景的根节点
	m_root = new osg::Group;

	// 创建osgEarth的地球操作器
	m_em = new osgEarth::Util::EarthManipulator;

	// 从文件中读取地球模型节点
	m_earth = osgDB::readNodeFile("simple.earth");
	earth_mnode = osgEarth::MapNode::findMapNode(m_earth);

	// 将地球模型节点添加到根节点中
	m_root->addChild(earth_mnode);

	//创建osgQOpenGLWidget对象，并将其作为子控件添加到当前窗口
	m_osgqopenglWidget = new osgQOpenGLWidget(this);

	// 连接osgQOpenGLWidget的初始化信号到MyEarth的initWindow槽函数
	connect(m_osgqopenglWidget, SIGNAL(initialized()), this, SLOT(initWindow()));

	// 设置osgQOpenGLWidget的显示尺寸为当前窗口的尺寸
	m_osgqopenglWidget->setFixedSize(1500, 1000);
	this->setCentralWidget(m_osgqopenglWidget);

 	m_GUIEventHandler = new MyGUIEventHandler(m_em);
	connect(m_GUIEventHandler, &MyGUIEventHandler::selected, this, [=]() {
		// 上一次记录的node材质清空，指针赋值nullptr
		if (m_lastSelectNode)
		{
			m_lastSelectNode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::MATERIAL);
			m_lastSelectNode = nullptr;
		}
		// 获取的node为空，跳出
		if (!m_GUIEventHandler->getLastSelectedNode())
		{
			return;
		}
		// group（单个房子），设置黄色材质高亮
		m_lastSelectNode = m_GUIEventHandler->getLastSelectedNode();
		qDebug() << "node: " << QString::fromStdString(m_lastSelectNode->getName());
		m_lastSelectNode->getOrCreateStateSet()->setAttribute(m_material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
		});

	//loadCameraImage("C:\\Users\\ShipengWork\\Documents\\data\\影像\\华山_卫图_Level_19_4490.tif");
	//loadTerrain("C:\\Users\\ShipengWork\\Documents\\data\\影像\\华山DEM_4490.tif");
	//loadObj("C:\\Users\\ShipengWork\\Documents\\data\\2.obj");
}

void ModelEditor::initWindowContent() {
	setWindowTitle("OSGEarth Visualization");
	setFixedSize(1600, 1040);

	// 创建菜单栏，有且仅有一个
	QMenuBar* menu_bar = menuBar();
	// 菜单栏放入窗口中
	setMenuBar(menu_bar);
	// 添加菜单内容
	QMenu* file_menu = menu_bar->addMenu("文件");
	QAction* act_save = file_menu->addAction("保存");
	QAction* act_saveAs = file_menu->addAction("另存为");
	QAction* act_openCImage = file_menu->addAction("加载影像");
	QAction* act_openTerrain = file_menu->addAction("加载地形");
	QAction* act_openObj = file_menu->addAction("建筑白膜");
	connect(act_save, &QAction::triggered, this, [=]() {
		if (m_layers.contains("object")) {
			this->save(m_fileNames["object"]);
		}});
	connect(act_saveAs, &QAction::triggered, this, [=]() {
		if (m_layers.contains("object")) {
			this->saveAs();
		}});
	connect(act_openCImage, &QAction::triggered, this, [=]() {
		QString str = QFileDialog::getOpenFileName(this, "打开文件",
			".", "(*.tif)");
		loadCameraImage(str);
		});
	connect(act_openTerrain, &QAction::triggered, this, [=]() {
		QString str = QFileDialog::getOpenFileName(this, "打开文件",
			".", "(*.tif)");
		loadTerrain(str);
		});
	connect(act_openObj, &QAction::triggered, this, [=]() {
		QString str = QFileDialog::getOpenFileName(this, "打开文件",
			".", "(*.obj)");
		loadObj(str);
		});

	QMenu* edit_menu = menu_bar->addMenu("模型移除");
	QAction* act_removeCam = edit_menu->addAction("移除影像");
	QAction* act_removeTer = edit_menu->addAction("移除地形");
	QAction* act_removeObj = edit_menu->addAction("移除白膜");
	connect(act_removeCam, &QAction::triggered, this, [=]() {
		if (!m_layers.contains("camera_image")) {
			QMessageBox::information(this, "提示", "当前没有加载影像");
		}
		else {
			earth_mnode->getMap()->removeLayer((osgEarth::GDALImageLayer*)m_layers["camera_image"]);
			m_layers["camera_image"] = nullptr;
			m_layers.remove("camera_image");
		}});
	connect(act_removeTer, &QAction::triggered, this, [=]() {
		if (!m_layers.contains("terrain")) {
			QMessageBox::information(this, "提示", "当前没有加载地形");
		}
		else {
			earth_mnode->getMap()->removeLayer((osgEarth::GDALElevationLayer*)m_layers["terrain"]);
			m_layers["terrain"] = nullptr;
			m_layers.remove("terrain");
		}});
	connect(act_removeObj, &QAction::triggered, this, [=]() {
		if (!m_layers.contains("object")) {
			QMessageBox::information(this, "提示", "当前没有加载白膜");
		}
		else {
			earth_mnode->getMap()->removeLayer((osgEarth::ModelLayer*)m_layers["object"]);
			m_layers["object"] = nullptr;
			m_layers.remove("object");
		}});

	QMenu* view_menu = menu_bar->addMenu("视点");
	QAction* act_defaultView = view_menu->addAction("默认");
	QAction* act_ChinaView = view_menu->addAction("中国");
	QAction* act_huashanView = view_menu->addAction("华山");
	connect(act_defaultView, &QAction::triggered, this, [=]() {
		m_em->setViewpoint(osgEarth::Viewpoint("默认",
			-90.000000000000000, 0.0000000000000000, -9.3132257461547852e-10,
			-0.00023781259862716095, -89.000000000000099, 19134411.000000000), 1.5);
		});
	connect(act_ChinaView, &QAction::triggered, this, [=]() {
		m_em->setViewpoint(osgEarth::Viewpoint("中国",
			108.05017287467311, 33.927468708848174, -1324.4244584860280,
			-0.00023781259862716095, -88.953905245834946, 19142359.855145693), 1.5);
		});
	connect(act_huashanView, &QAction::triggered, this, [=]() {
		m_em->setViewpoint(osgEarth::Viewpoint("华山",
			110.055998715991, 34.5245844139754, 652.88835630845279,
			1.1812922265385689e-06, -88.999583188869821, 18851.538335830206), 3);
		});


	// 加载模型的工具栏，左侧
	QToolBar* tool_bar = new QToolBar(this);
	// 工具栏放入窗口中，停靠位置默认在上面，这里改为左侧
	addToolBar(Qt::LeftToolBarArea, tool_bar);
	// 设置禁止移动（浮动、停靠失效）
	tool_bar->setMovable(false);
	// 添加分割线
	tool_bar->addAction(act_openCImage);
	tool_bar->addSeparator();
	tool_bar->addAction(act_openTerrain);
	tool_bar->addSeparator();
	tool_bar->addAction(act_openObj);
}

void ModelEditor::keyPressEvent(QKeyEvent* event) {
	//保存
	if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_S) {
		if (m_layers.contains("object")) {
			this->save(m_fileNames["object"]);
		}
	}
	
	if (m_lastSelectNode) {
		// 选中node：ctrl C 组合键复制
		if (event->modifiers() == Qt::ControlModifier && event->key() == Qt::Key_C) {
			//m_lastSelectNode变为node的selection，需要清除选中node的dragger，给m_lastSelectNode重新赋值
			m_GUIEventHandler->lastNodeDraggerClear();

			// 获取原始节点的父节点
			osg::Group* parent = m_lastSelectNode->getParent(0);
			if (parent) {
				// 克隆原始节点，放在原始节点（x+10,y+10）的位置
				osg::Node* clonedNode = dynamic_cast<osg::Node*>(m_lastSelectNode->clone(osg::CopyOp::DEEP_COPY_ALL));
				tmpvis tv(20, 20, 0);
				clonedNode->accept(tv);
				// 原始节点黄色材质删除 
				m_lastSelectNode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::MATERIAL);
				// 原始节点指针置空，指向克隆节点，添加材质
				m_lastSelectNode = nullptr;
				m_lastSelectNode = clonedNode;
				m_lastSelectNode->getOrCreateStateSet()->setAttribute(m_material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
				// 将克隆节点添加到原始节点的父节点下
				parent->addChild(m_lastSelectNode);
			}
		}

		// 选中node，Tab键切换dragger
		if (event->key() == Qt::Key_Tab) {
			m_GUIEventHandler->switchDraggerType();
		}

		// 选中node，delete键删除node
		if (event->key() == Qt::Key_Delete) {
			//m_lastSelectNode变为node的selection，需要清除选中node的dragger
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
		//清除dragger
		m_GUIEventHandler->lastNodeDraggerClear();
		//清除材质
		m_lastSelectNode->getOrCreateStateSet()->removeAttribute(osg::StateAttribute::MATERIAL);
	}

	if (saveName.size() != 0) {
		osgEarth::ModelLayer* ml = static_cast<osgEarth::ModelLayer*> (m_layers["object"]);
		osg::Node* node = ml->getNode();

		// 深拷贝
		osg::Node* nodeClone = static_cast<osg::Node*> (node->clone(osg::CopyOp::DEEP_COPY_ALL));

		// 获取变换矩阵的逆矩阵
		osg::MatrixTransform* matrixTranform = dynamic_cast<osg::MatrixTransform*> (nodeClone->asGroup()->getChild(0));
		osg::Matrix matrix = matrixTranform->getMatrix();
		osg::Matrix matrix_invert;
		matrix_invert.invert(matrix);

		// visitor坐标还原
		tmpvis nodeVisitor(matrix_invert);
		nodeClone->accept(nodeVisitor);

		osgDB::writeNodeFile(*nodeClone, saveName.toStdString(), new osgDB::Options("noRotation"));

		nodeClone = nullptr;
	}
}

void ModelEditor::saveAs() {
	QString saveName = QFileDialog::getSaveFileName(this, "另存为",".", "(*.obj)");
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
	//设置坐标系
	osgEarth::SpatialReference* wgs84 = osgEarth::SpatialReference::get("wgs84");
	osgEarth::GeoPoint position(wgs84, x, y, z);
	osgEarth::ModelLayer::Options opt;
	opt.osgOptionString() = "noRotation";
	//读取obj文件
	osg::ref_ptr<osgEarth::ModelLayer> modelLayer = new osgEarth::ModelLayer(opt);
	modelLayer->setURL(fileFullName.toLocal8Bit().data());
	modelLayer->setLocation(position);
	modelLayer->setName("arches");
	//加到地球节点
	earth_mnode->getMap()->addLayer(modelLayer);

	m_layers["object"] = modelLayer;
	m_fileNames["object"] = fileFullName;
}

// 初始化窗口的槽函数
void ModelEditor::initWindow() {
	// 获取osgQOpenGLWidget中的osgViewer对象
	m_viewer = m_osgqopenglWidget->getOsgViewer();

	// 设置osgViewer的相机操作器为地球操作器
	m_viewer->setCameraManipulator(m_em);

	// 设置osgViewer的场景数据为根节点
	m_viewer->setSceneData(m_root.get());

	//添加事件处理器
	m_viewer->addEventHandler(m_GUIEventHandler);

	//设置相机投影矩阵为透视投影，并防止窗口变扁
	m_viewer->getCamera()->setProjectionMatrixAsPerspective(
		30.0f, // 视野角度
		1.5f, // 宽高比
		1.0f,  // 近裁剪面距离
		10000.0f // 远裁剪面距离
	);
}

// MyEarth类的析构函数
ModelEditor::~ModelEditor() {
	// 如果m_osgqopenglWidget不为NULL，则删除它并设置为NULL
	if (m_osgqopenglWidget != NULL) {
		delete m_osgqopenglWidget;
		m_osgqopenglWidget = NULL;
	}
}
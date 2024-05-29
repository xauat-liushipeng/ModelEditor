#pragma once
#pragma execution_character_set("utf-8")
#include "utils.h"
#include "myguieventhandler.h"


class ModelEditor : public QMainWindow {
	Q_OBJECT

public:
	ModelEditor(QWidget* parent = Q_NULLPTR);
	~ModelEditor();

	// 初始化界面窗口内容
	void initWindowContent();

	// 加载影像
	void loadCameraImage(QString fileFullName);

	// 加载地形
	void loadTerrain(QString fileFullName);

	// 加载建筑白膜object
	void loadObj(QString fileFullName, double x = 110.055998715991, double y = 34.5245844139754, double z = 395.367827402428);

	// 键盘事件
	void keyPressEvent(QKeyEvent* event);

	// 保存：覆盖原文件
	void save(QString saveName);

	// 另存：保存到新文件
	void saveAs();

private slots:
	void initWindow();

private:
	// 存放地球上加载的layers
	QMap<QString, osg::Object*> m_layers;
	// 存放加载的模型文件名
	QMap<QString, QString> m_fileNames;
	// 场景
	osgViewer::Viewer* m_viewer;
	// osg窗体
	osgQOpenGLWidget* m_osgqopenglWidget;
	// 根节点
	osg::ref_ptr<osg::Group> m_root;
	// 地球节点
	osg::ref_ptr<osg::Node> m_earth;
	osgEarth::MapNode* earth_mnode;
	// 相机操作器
	osg::ref_ptr<osgEarth::Util::EarthManipulator> m_em;
	// gui event handler
	MyGUIEventHandler* m_GUIEventHandler;
	// 高亮的材质
	osg::ref_ptr<osg::Material> m_material;
	// 上一个选中的node
	osg::Node* m_lastSelectNode = nullptr;
};

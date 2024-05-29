#pragma once
#pragma execution_character_set("utf-8")
#include "utils.h"
#include "myguieventhandler.h"


class ModelEditor : public QMainWindow {
	Q_OBJECT

public:
	ModelEditor(QWidget* parent = Q_NULLPTR);
	~ModelEditor();

	// ��ʼ�����洰������
	void initWindowContent();

	// ����Ӱ��
	void loadCameraImage(QString fileFullName);

	// ���ص���
	void loadTerrain(QString fileFullName);

	// ���ؽ�����Ĥobject
	void loadObj(QString fileFullName, double x = 110.055998715991, double y = 34.5245844139754, double z = 395.367827402428);

	// �����¼�
	void keyPressEvent(QKeyEvent* event);

	// ���棺����ԭ�ļ�
	void save(QString saveName);

	// ��棺���浽���ļ�
	void saveAs();

private slots:
	void initWindow();

private:
	// ��ŵ����ϼ��ص�layers
	QMap<QString, osg::Object*> m_layers;
	// ��ż��ص�ģ���ļ���
	QMap<QString, QString> m_fileNames;
	// ����
	osgViewer::Viewer* m_viewer;
	// osg����
	osgQOpenGLWidget* m_osgqopenglWidget;
	// ���ڵ�
	osg::ref_ptr<osg::Group> m_root;
	// ����ڵ�
	osg::ref_ptr<osg::Node> m_earth;
	osgEarth::MapNode* earth_mnode;
	// ���������
	osg::ref_ptr<osgEarth::Util::EarthManipulator> m_em;
	// gui event handler
	MyGUIEventHandler* m_GUIEventHandler;
	// �����Ĳ���
	osg::ref_ptr<osg::Material> m_material;
	// ��һ��ѡ�е�node
	osg::Node* m_lastSelectNode = nullptr;
};

#pragma once
#include "utils.h"

// ����GUI�¼��������
class MyGUIEventHandler : public QObject, public osgGA::GUIEventHandler {
	Q_OBJECT

public:
	MyGUIEventHandler(osgEarth::Util::EarthManipulator* camera);
	~MyGUIEventHandler();

	//�ⲿ��ȡѡ�еĽڵ�
	osg::Node* getLastSelectedNode();

	//�ⲿͨ��Tab�����л�dragger����
	void switchDraggerType();

	//����ϸ�ѡ��node��dragger
	void lastNodeDraggerClear();

protected:
	//�¼�����
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);
	
	//����ѡ����handle�У�push��releaseͬһλ��ʱ����
	bool pick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	//Ϊ��ǰѡ�е�node����dragger
	void createCurNodeDragger();

signals:
	void selected();

private:
	float mX;
	float mY;

	osgManipulator::PointerInfo	m_pointer;
	osgManipulator::Dragger* m_activeDragger;

	//dragger����
	int m_draggerType = 0;
	//��ǰ���ѡ�е�node
	osg::Node* m_curNode = nullptr;
	//��ǰ���ѡ��node��dragger���´ε������λ��ʱ�Ƴ�
	osg::ref_ptr<osgManipulator::Dragger> m_lastNode_Dragger = nullptr;
	//��ǰ���ѡ��node��selection���´ε������λ��ʱ�Ƴ�
	osg::ref_ptr < osgManipulator::Selection> m_lastNode_Selection = nullptr;
};
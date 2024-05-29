#pragma once
#include "utils.h"

// 创建GUI事件处理程序
class MyGUIEventHandler : public QObject, public osgGA::GUIEventHandler {
	Q_OBJECT

public:
	MyGUIEventHandler(osgEarth::Util::EarthManipulator* camera);
	~MyGUIEventHandler();

	//外部获取选中的节点
	osg::Node* getLastSelectedNode();

	//外部通过Tab按键切换dragger类型
	void switchDraggerType();

	//清除上个选中node的dragger
	void lastNodeDraggerClear();

protected:
	//事件处理
	virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*);
	
	//鼠标点选：在handle中，push和release同一位置时触发
	bool pick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa);

	//为当前选中的node生成dragger
	void createCurNodeDragger();

signals:
	void selected();

private:
	float mX;
	float mY;

	osgManipulator::PointerInfo	m_pointer;
	osgManipulator::Dragger* m_activeDragger;

	//dragger类型
	int m_draggerType = 0;
	//当前点击选中的node
	osg::Node* m_curNode = nullptr;
	//当前点击选中node的dragger，下次点击任意位置时移除
	osg::ref_ptr<osgManipulator::Dragger> m_lastNode_Dragger = nullptr;
	//当前点击选中node的selection，下次点击任意位置时移除
	osg::ref_ptr < osgManipulator::Selection> m_lastNode_Selection = nullptr;
};
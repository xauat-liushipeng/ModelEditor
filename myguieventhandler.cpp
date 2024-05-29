#include "myguieventhandler.h"

MyGUIEventHandler::MyGUIEventHandler(osgEarth::Util::EarthManipulator* camera)
	: mX(0.0f)
	, mY(0.0f)
	, m_activeDragger(nullptr) {}

bool MyGUIEventHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object*, osg::NodeVisitor*) {
	osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);
	
	if (!view) {
		return false;
	}

	switch (ea.getEventType()) {
	case osgGA::GUIEventAdapter::PUSH:
	{
		mX = ea.getX();
		mY = ea.getY();
		m_pointer.reset();
		// ����һ�����ڼ�⽻����߶Ρ�
		osgUtil::LineSegmentIntersector::Intersections hits;
		if (view->computeIntersections(ea.getX(), ea.getY(), hits)) {
			m_pointer.setCamera(view->getCamera());
			m_pointer.setMousePosition(ea.getX(), ea.getY());
			for (const auto& hit : hits) {
				m_pointer.addIntersection(hit.nodePath, hit.getLocalIntersectPoint());
			}

			auto hitList = m_pointer._hitList.front().first;
			for (const auto& node : hitList) {
				osgManipulator::Dragger* pDragger = dynamic_cast<osgManipulator::Dragger*>(node);
				if (pDragger) {
					pDragger->handle(m_pointer, ea, aa);
					m_activeDragger = pDragger;
					break;
				}
				if (m_lastNode_Dragger) {
					m_lastNode_Dragger->handle(m_pointer, ea, aa);
					m_activeDragger = m_lastNode_Dragger;
				}
			}
		}
	}
		break;

	case osgGA::GUIEventAdapter::DRAG:
		if (m_activeDragger) {
			m_pointer._hitIter = m_pointer._hitList.begin();
			m_pointer.setCamera(view->getCamera());
			m_pointer.setMousePosition(ea.getX(), ea.getY());
			m_activeDragger->handle(m_pointer, ea, aa);
			return true;
		}
		break;
		
	case osgGA::GUIEventAdapter::RELEASE:
		m_activeDragger = nullptr;
		m_pointer.reset();

		if (ea.getX() == mX && ea.getY() == mY) {
			if (pick(ea, aa)) {
				createCurNodeDragger();
				return true;
			}
		}
		break;
			
	default:
		break;
	}

	return false;
}

bool MyGUIEventHandler::pick(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa) {
	if (m_curNode) {
		m_curNode = nullptr;
	}

	osgViewer::View* view = dynamic_cast<osgViewer::View*>(&aa);

	osgUtil::LineSegmentIntersector::Intersections hits;
	if (view->computeIntersections(ea.getX(), ea.getY(), hits)) {
		auto intersection = *hits.begin();
		osg::NodePath& nodePath = intersection.nodePath;

		if (nodePath.empty()) {
			return false;
		}

		for (const auto& node : nodePath) {
			if (QString::fromStdString(node->getName()).startsWith("Group")) {
				//ѡ��node����ֵ�������ź�
				m_curNode = node;
				emit this->selected();
				return true;
			}
		}
		emit this->selected();
	}

	//ûѡ��node������dragger���ͣ�����ϸ�node��dragger
	m_draggerType = 0;
	this->lastNodeDraggerClear();

	return false;
}

void MyGUIEventHandler::createCurNodeDragger() {
	if (m_curNode) {
		this->lastNodeDraggerClear();

		// ��ȡѡ�нڵ�ĸ��ڵ�
		osg::Group* parent = m_curNode->getParent(0);
		
		// matrix location
		osg::Matrixd matrix;
		matrix.makeTranslate(m_curNode->getBound().center());
		// matrix scale
		float scale = m_curNode->getBound().radius() * 1.5;
		matrix.preMultScale(osg::Vec3d(scale, scale, scale));
		
		// selection
		m_lastNode_Selection = new osgManipulator::Selection;
		m_lastNode_Selection->addChild(m_curNode);

		// dragger
		switch (m_draggerType)
		{
		case 0:
		default:
			//��άƽ��
			m_lastNode_Dragger = new osgManipulator::TranslateAxisDragger;
			break;
		case 1:
			//��ʽ����
			m_lastNode_Dragger = new osgManipulator::TabBoxDragger;
			break;
		case 2:
			//��ά��ת
			m_lastNode_Dragger = new osgManipulator::TrackballDragger;
			break;
		}
		m_lastNode_Dragger->setupDefaultGeometry();
		m_lastNode_Dragger->setMatrix(matrix);
		
		// ���ڵ�modelLayer���dragger��selection���Ƴ���ǰ�ӽڵ�
		parent->addChild(m_lastNode_Dragger.get());
		parent->addChild(m_lastNode_Selection.get());
		parent->removeChild(m_curNode);

		// �ص�
		m_lastNode_Dragger->addTransformUpdating(m_lastNode_Selection);

		// manager
		osg::ref_ptr<osgManipulator::CommandManager> m_commandManager = new osgManipulator::CommandManager;
		m_commandManager->connect(*m_lastNode_Dragger.get(), *m_lastNode_Selection.get());
	}
}

void MyGUIEventHandler::lastNodeDraggerClear() {
	if (m_lastNode_Selection) {
		osg::Group* parent = m_lastNode_Selection->getParent(0);

		//1.����dragger��dragger�Ӹ��ڵ�modelLayer�Ƴ���dragger���
		//
		if (parent->containsNode(m_lastNode_Dragger))
		{
			parent->removeChild(m_lastNode_Dragger);
			m_lastNode_Dragger = nullptr;
		}

		//2.����selection��selection��matrix�ӵ����ӽڵ�lastNode��
		//
		//�õ��任�����node
		osg::Matrix selectionMatrix = m_lastNode_Selection->getMatrix();
		osg::Node* lastNode = m_lastNode_Selection->getChild(0);
		//node����任
		tmpvis nodevisitor(selectionMatrix);
		lastNode->accept(nodevisitor);

		//������ϴ�ѡ��ͬһ���ڵ�,curNode��ΪlastNode
		if (m_curNode && m_lastNode_Selection == m_curNode->getParent(0)) {
			m_curNode = nullptr;
			m_curNode = lastNode;
		}

		//3.selection�Ӹ��ڵ�modelLayer�Ƴ������ӽڵ�lastNode�ӵ��丸�ڵ�modelLayer
		//
		//selection���ڵ�modelLayer���lastNode
		m_lastNode_Selection->getParent(0)->addChild(lastNode);
		//selection�Ƴ��ӽڵ�lastNode
		m_lastNode_Selection->removeChild(lastNode);
		//modellayer�Ƴ�selection
		m_lastNode_Selection->getParent(0)->removeChild(m_lastNode_Selection);

		//4.�ÿ�
		lastNode = nullptr;
		m_lastNode_Selection = nullptr;
	}
}

osg::Node* MyGUIEventHandler::getLastSelectedNode() {
	return m_curNode;
}

void MyGUIEventHandler::switchDraggerType() {
	// ��ק������0, 1, 2��������ѭ���л�
	m_draggerType = (m_draggerType + 1) % 3;
	// �����µ��������´�����ק��
	createCurNodeDragger();
}

MyGUIEventHandler::~MyGUIEventHandler() {}
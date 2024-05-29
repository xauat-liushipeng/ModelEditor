#pragma once
#include <QtWidgets/QMainWindow>
#include <QMenuBar>
#include <QToolBar>
#include <QPushButton>
#include <QFileDialog>
#include <QMessageBox>
#include <QKeyEvent>
#include <QDebug>

#include <osgEarth/EarthManipulator>
#include <osgEarth/GDAL>
#include <osgEarth/OGRFeatureSource>
#include <osgEarth/FeatureImageLayer>
#include <osgEarth/FeatureNode>
#include <osgEarth/Registry>
#include <osgEarth/GeoTransform>
#include <osgEarth/ModelLayer>
#include <osgEarth/Viewpoint>
#include <osgEarth/MapNode>

#include <osg/NodeVisitor>
#include <osg/Array>
#include <osg/BufferObject>
#include <osg/MatrixTransform>
#include <osg/KdTree>

#include <osgViewer/Viewer>
#include <osgViewer/View>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>

#include <osgManipulator/TranslateAxisDragger>
#include <osgManipulator/TrackballDragger>
#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/Selection>
#include <osgManipulator/CommandManager>
#include <osgManipulator/Dragger>

#include <osgGA/GUIEventHandler>

#include <osgQOpenGL/osgQOpenGLWidget>

class tmpvis : public osg::NodeVisitor {
public:
	tmpvis(double x, double y, double z)
		:osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
		m_move_x = x;
		m_move_y = y;
		m_move_z = z;
	}

	tmpvis(osg::Matrix matrix)
		:osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
		m_matrix = matrix;
	}

public:
	void apply(osg::Geometry& geometry) {
		osg::Array* vertexArray = geometry.getVertexArray();
		if (vertexArray) {
			osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(vertexArray);
			if (vertices) {
				for (osg::Vec3Array::iterator iter = vertices->begin(); iter != vertices->end(); ++iter) {
					osg::Vec3& vertex = *iter;
					if (m_move_x != 0 || m_move_y != 0 || m_move_z != 0) {
						vertex[0] += m_move_x;
						vertex[1] += m_move_y;
						vertex[2] += m_move_z;
					} else {
						vertex = vertex * m_matrix;
					}
				}
			}
			osg::BufferObject* vertexBufferObject = vertices->getBufferObject();
			vertexBufferObject->reBuild();
			vertexBufferObject->dirty();
			geometry.dirtyBound();
		}
		traverse(geometry);
	}

private:
	double m_move_x = 0.0;
	double m_move_y = 0.0;
	double m_move_z = 0.0;

	osg::Matrix m_matrix;
};

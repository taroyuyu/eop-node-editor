#include "NodeGraphWidget.h"

#include "NodeGraphScene.h"

#include <model/NodeFactory.h>

#include <QBoxLayout>
#include <QGraphicsView>

NodeGraphWidget::NodeGraphWidget(QWidget* parent)
: QWidget(parent)
, m_controller(m_node_graph)
{
	m_scene = new NodeGraphScene(this, m_controller);
	QBoxLayout* layout = new QBoxLayout(QBoxLayout::Down);
	m_view = new QGraphicsView(m_scene);
	layout->addWidget(m_view);

	setLayout(layout);

	connect(&m_controller, SIGNAL(node_graph_changed()), this, SIGNAL(node_graph_changed()));
	connect(&m_controller, SIGNAL(message(const QString&)), this, SIGNAL(message(const QString&)));
}

NodeGraphWidget::~NodeGraphWidget()
{
	delete m_node_factory;
}

void NodeGraphWidget::give_node_factory(NodeFactory* factory)
{
	m_controller.set_node_factory(factory);
	m_node_factory = factory;
}

const QVector<NodeModel*>& NodeGraphWidget::nodes() const
{
	return m_node_graph.nodes();
}
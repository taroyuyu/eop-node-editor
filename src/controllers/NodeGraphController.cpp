#include "NodeGraphController.h"

#include "model/NodeModel.h"
#include "model/NodeGraph.h"
#include "model/NodeFactory.h"
#include "model/NodePortModel.h"

#include <QPoint>
#include <assert.h>

NodeGraphController::NodeGraphController(NodeGraph& node_graph)
: m_node_graph(node_graph)
{
}

void NodeGraphController::set_node_factory(NodeFactory* factory)
{
	assert(m_node_factory == nullptr);
	m_node_factory = factory;
}

NodeModel* NodeGraphController::add_node(const QPointF& position)
{
	assert(m_node_factory != nullptr);
	NodeModel* model = m_node_factory->create_node_model_and_set_type();
	if (model == nullptr)
	{
		emit message("No node type selected!", false);
		return nullptr;
	}

	if (!m_node_graph.is_add_allowed(model))
	{
		delete model;
		return nullptr;
	}
	model->set_controller(this);
	model->set_position(position);
	model->create_port_models();
	m_node_graph.give_node(model);
	m_persisted = false;
	emit node_added(model);
	return model;
}

void NodeGraphController::delete_node(NodeModel* node_model)
{
	m_node_graph.remove_node(node_model);
	notify_node_graph_changed();
}

void NodeGraphController::delete_connection(NodeConnection* connection)
{
	delete connection;
	notify_node_graph_changed();
}

void NodeGraphController::clear_graph()
{
	m_first_connection_port = nullptr;
	m_second_connection_port = nullptr;
}

void NodeGraphController::set_first_connection_port(NodePortModel* port)
{
	m_first_connection_port = port;
}

void NodeGraphController::set_second_connection_port(NodePortModel* port)
{
	m_second_connection_port = port;
}

NodeConnection* NodeGraphController::create_connection()
{
	if (m_first_connection_port == nullptr || m_second_connection_port == nullptr)
	{
		emit message("Connection port is nullptr, this is VERY bad!", false);
		return nullptr;
	}

	if ((m_first_connection_port->num_connections() > 0) && (!m_first_connection_port->supports_multiple_connections()))
	{
		assert(m_first_connection_port->num_connections() == 1);
		emit message("First connection port already has a connection and does not support multiple!", false);
		return nullptr;
	}

	if ((m_second_connection_port->num_connections() > 0) && (!m_second_connection_port->supports_multiple_connections()))
	{
		assert(m_second_connection_port->num_connections() == 1);
		emit message("Second connection port already has a connection and does not support multiple!", false);
		return nullptr;
	}

	if (m_first_connection_port == m_second_connection_port)
	{
		emit message("Connection ports are the same!", false);
		return nullptr;
	}

	if (m_first_connection_port->port_type() == m_second_connection_port->port_type())
	{
		emit message("Port types are the same!", false);
		return nullptr;
	}

	if (m_first_connection_port->node_model() == m_second_connection_port->node_model())
	{
		emit message("Connection to the same node is not allowed!", false);
		return nullptr;
	}

	if (m_first_connection_port->has_connection(m_second_connection_port))
	{
		emit message("Connection already exists!", false);
		return nullptr;
	}

	NodePortModel* input_port = nullptr;
	NodePortModel* output_port = nullptr;

	if (m_first_connection_port->port_type() == NodePortModel::INPUT)
	{
		input_port = m_first_connection_port;
		output_port = m_second_connection_port;
	}
	else if (m_second_connection_port->port_type() == NodePortModel::INPUT)
	{
		input_port = m_second_connection_port;
		output_port = m_first_connection_port;
	}

	if (!input_port->may_connect_to(*output_port) || !output_port->may_connect_to(*input_port))
	{
		emit message("Connection between these port types is not allowed!", false);
		return nullptr;
	}

	// If output goes to an input of node earlier in graph, we have a circular dependency
	if (m_node_graph.scan_left(output_port->node_model(), input_port->node_model()))
	{
		emit message("Circular dependency found, this is not allowed!", false);
		return nullptr;
	}

	m_first_connection_port = nullptr;
	m_second_connection_port = nullptr;

	NodeConnection* connection = new NodeConnection(input_port, output_port);
	emit message("Connection created!", true);
	emit connection_created(connection);
	notify_node_graph_changed();
	return connection;
}

void NodeGraphController::set_persisted()
{
	m_persisted = true;
}

bool NodeGraphController::is_persisted() const
{
	return m_persisted;
}

void NodeGraphController::start_load()
{
	m_loading = true;
}

void NodeGraphController::end_load()
{
	notify_node_graph_changed();
	m_loading = false;
}

void NodeGraphController::notify_node_graph_changed()
{
	if (!m_loading)
	{
		m_persisted = false;
		emit node_graph_changed();
	}
}
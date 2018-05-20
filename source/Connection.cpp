#include "Connection.h"
#include "Node.h"


//-----------------------------------------------------------------------------
// Constructors
//-----------------------------------------------------------------------------

Connection::Connection(Node* from, Node* to)
	: m_input(from)
	, m_output(to)
{
}

Connection::~Connection()
{
}


//-----------------------------------------------------------------------------
// Getters
//-----------------------------------------------------------------------------

Node* Connection::GetInput()
{
	return m_input;
}

Node* Connection::GetOutput()
{
	return m_output;
}

Connection* Connection::GetLeftConnection() const
{
	if (m_input->GetLeftNode() != nullptr &&
		m_output->GetLeftNode() != nullptr)
	{
		if (m_input->GetLeftNode()->HasInput(m_output->GetLeftNode()))
			return m_input->GetLeftNode()->GetInputConnection(m_output->GetLeftNode());
		else if (m_output->GetLeftNode()->HasInput(m_input->GetLeftNode()))
			return m_output->GetLeftNode()->GetInputConnection(m_input->GetLeftNode());
	}
	return nullptr;
}

Connection* Connection::GetRightConnection() const
{
	if (m_input->GetRightNode() != nullptr &&
		m_output->GetRightNode() != nullptr)
	{
		if (m_input->GetRightNode()->HasInput(m_output->GetRightNode()))
			return m_input->GetRightNode()->GetInputConnection(m_output->GetRightNode());
		else if (m_output->GetRightNode()->HasInput(m_input->GetRightNode()))
			return m_output->GetRightNode()->GetInputConnection(m_input->GetRightNode());
	}
	return nullptr;
}

bool Connection::IsLeftMostLane() const
{
	Connection* left = GetLeftConnection();
	return (left == nullptr || left->GetLeftConnection() == this);
}

bool Connection::IsRightMostLane() const
{
	return (GetRightConnection() == nullptr);
}

LaneDivider Connection::GetLaneDivider(LaneSide side) const
{
	if (side == LaneSide::LEFT)
	{
		Connection* left = GetLeftConnection();
		if (left != nullptr)
			return m_input->m_leftDivider;
		else
			return LaneDivider::SOLID;
	}
	else
	{
		Connection* right = GetRightConnection();
		if (right != nullptr)
			return right->GetInput()->m_leftDivider;
		else
			return LaneDivider::SOLID;
	}
}

//float Connection::GetDistance() const
//{
//	return (m_centerArc1.length + m_centerArc2.length);
//}

//Vector2f Connection::GetPoint(Meters distance, LaneSide side, Meters offset)
//{
//	distance = Math::Clamp(distance, 0.0f,
//		m_leftArc1.length + m_leftArc2.length);
//
//	Vector2f point;
//	Biarc centerArc;
//	if (distance < m_leftArc1.length)
//	{
//		centerArc = m_leftArc1;
//	}
//	else
//	{
//		centerArc = m_leftArc2;
//		distance -= m_leftArc1.length;
//	}
//	point = centerArc.GetPoint(distance);
//
//	//if (side == LaneSide::LEFT)
//	//{
//	//	return point;
//	//}
//	//else
//	{
//		Vector2f normal;
//		float offsetFromCenter = offset;
//		if (side == LaneSide::RIGHT)
//			offsetFromCenter = m_input->GetWidth() - offsetFromCenter;
//		else if (side == LaneSide::CENTER)
//			offsetFromCenter = (m_input->GetWidth() * 0.5f) - offsetFromCenter;
//		if (centerArc.radius == 0.0f)
//		{
//			normal = RightPerpendicular((centerArc.end -
//				centerArc.start) / centerArc.length);
//		}
//		else
//		{
//			normal = RightPerpendicular(centerArc.center - centerArc.start);
//			if (centerArc.end.Dot(normal) < centerArc.center.Dot(normal))
//				offsetFromCenter = -offsetFromCenter;
//			normal = (point - centerArc.center) / centerArc.radius;
//		}
//		point += normal * offsetFromCenter;
//		return point;
//	}
//}

#pragma once

#include <queue>
#include <vector>

template<class W>
struct HuffmanTreeNode
{
	HuffmanTreeNode(const W& weight = W())
	: _left(nullptr)
	, _right(nullptr)
	, _parent(nullptr)
	, _weight(weight)
	{ }

	HuffmanTreeNode<W>* _left;
	HuffmanTreeNode<W>* _right;
	HuffmanTreeNode<W>* _parent;
	W _weight;
};

template<class W>
class HuffmanTree
{
	typedef HuffmanTreeNode<W> Node;

	struct Compare
	{
		bool operator()(const Node* left, const Node* right)
		{
			return left->_weight > right->_weight;
		}
	};


public:
	HuffmanTree()
		:_root(nullptr)
	{ }

	~HuffmanTree()
	{
		DestroyTree(_root);
	}

	//问题？---怎么创建小堆
	Node* CreatHuffmanTree(const std::vector<W>& weights, const W& invalid)
	{
		//注意：优先级队列默认情况是按照小于的方式比较的，然后创建的是一个大堆
		//小堆：将比较方式修改一下
		std::priority_queue<Node*, std::vector<Node*>, Compare>q;

		for (auto e : weights)
		{
			//过滤掉非法的weights
			if (invalid == e)
				continue;
			q.push(new Node(e));
		}
		while (q.size() > 1)
		{
			Node* left = q.top();
			q.pop();

			Node* right = q.top();
			q.pop();

			Node* parent = new Node(left->_weight + right->_weight);
			parent->_left = left;
			parent->_right = right;

			parent->_left = left;
			parent->_right = parent;
			q.push(parent);
		}

		_root = q.top();
		return _root;
	}

	Node* GetRoot()
	{
		return _root;
	}
private:
	void DestroyTree(Node* & root)
	{
		if (root)
		{
			DestroyTree(root->_left);
			DestroyTree(root->_right);
			delete root;
			root = nullptr;
		}
	}
private:
	Node* _root;
};
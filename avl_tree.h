#pragma once

namespace FlyZero
{
	template <typename ElementType>
	class AVL_Tree
	{
	public:
		struct Node
		{
			Node *right{ nullptr };
			Node *left{ nullptr };
			ElementType e;
			Node(const ElementType &e) : e(e) { }
		};

	public:
		/*
		 * Destructor
		 */
		~AVL_Tree(void) {
			Destroy(head);
		}

		/*
		 * Descript	Insert an element, if new node is created, size will be updated.
		 */
		void Insert(const ElementType &e) {
			if (Insert(&head, e)) {
				++nElement;
			}
		}

		/*
		 * Descript	Get size of the tree.
		 */
		unsigned int Size(void) const {
			return nElement;
		}

		/*
		 * Descript	Destroy the tree.
		 */
		void Destroy(void) {
			Destroy(head);
			head = nullptr;
			nElement = 0;
		}

	private:
		/*
		 * Descript	Insert an element at pNode.
		 * Param	pNode is the node to insert into, e is the element.
		 * Return	if non new node is created, return false; otherwise return true.
		 */
		static bool Insert(Node **pNode, const ElementType &e) {
			if (*pNode) {
				if (e < (*pNode)->e) {
					Insert(&(*pNode)->left, e);
					// TODO: Balance
				} else if ((*pNode)->e < e) {
					Insert(&(*pNode)->right, e);
					// TODO: Balance
				} else {
					(*pNode)->e = e;
					return false;
				}
			} else {
				*pNode = new Node(e);
				return true;
			}
		}

		/*
		 * Descript	Destroy the tree.
		 * Param	node is the tree node to destroy
		 */
		static void Destroy(Node *node) {
			if (node) {
				Destroy(node->left);
				Destroy(node->right);
				Destroy(node);
			}
		}

	private:
		Node *head{ nullptr };
		unsigned int nElement{ 0 };
	};
}

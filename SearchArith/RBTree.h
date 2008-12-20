#ifndef _RED_BLACK_TREE_INC_
#define _RED_BLACK_TREE_INC_

//typedef DWORD	DB_POS;

typedef enum { BLACK, RED } NodeColor;

typedef enum {
    RBT_STATUS_OK,
    RBT_STATUS_MEM_EXHAUSTED,
    RBT_STATUS_DUPLICATE_KEY,
    RBT_STATUS_KEY_NOT_FOUND
} RbtStatus;

typedef struct RbtNode
{
	DB_POS		left, right;		// left, right child
	DB_POS		parent;				// parent
    NodeColor	color;				// node color (BLACK, RED)
    DB_POS		data;				// user data
} RbtNode, *PRbtNode;

#define SENTINEL 1

struct RbtHead
{
	DB_POS		posRoot;
	DB_POS		posSentinel;
	DWORD		m_dwFlag;
};

#define 	POS_NULL		0				// Î»ÖÃÎª¿Õ


template<class __Allocator, class __FunCmpare>
class CRBTree_t
{
public:
	class iterator_base
	{
	public:
		iterator_base()
		{
			posCurrent = POS_NULL;
			m_pAllocator = NULL;
		}

		inline PRbtNode ToAddr(DB_POS pos)
		{
			return (PRbtNode)(m_pAllocator->PosToAddr(pos));
		}

		inline DB_POS ToPos(PRbtNode node)
		{
			return (DB_POS)(m_pAllocator->AddrToPos(node));
		}

		DB_POS Data()
		{
			PRbtNode	p;

			p = ToAddr(posCurrent);
			return p->data;
		}

		LPVOID operator*()
		{
			PRbtNode	p;

			p = ToAddr(posCurrent);
			return ToAddr(p->data);
		}

		bool operator!=(iterator_base &it)
		{
			return (posCurrent != it.posCurrent);
		}
	public:
		__Allocator		*m_pAllocator;
		DB_POS			posCurrent;

	protected:
		friend class CRBTree_t<__Allocator, __FunCmpare>;

	};

	class ascend_iterator : public iterator_base
	{
	public:
		ascend_iterator()
		{
		}
		ascend_iterator& operator++()
		{
			RbtNode		*p;

			p = ToAddr(posCurrent);
			if (p->right != SENTINEL)
			{
				// go right 1, then left to the end
				for (p = ToAddr(p->right); p->left != SENTINEL; p = ToAddr(p->left));
			}
			else
			{
				// while you're the right child, chain up parent link
				DB_POS		posParent;
				posParent = p->parent;
				while (posParent != POS_NULL && ToPos(p) == ToAddr(posParent)->right)
				{
					p = ToAddr(posParent);
					posParent = p->parent;
				}

				// return the "inorder" node
				posCurrent = ToPos(p);
			}
			if (posCurrent == SENTINEL)
				posCurrent = POS_NULL;

			return *this;
		}
	};

	class descend_iterator : public iterator_base
	{
	public:
		descend_iterator& operator++()
		{

			return (*this);
		}
	};

	descend_iterator desc_begin()
	{
		descend_iterator	it;

		it.m_pAllocator = m_pAllocator;

		for (it.posCurrent = m_pHead()->posRoot; ToAddr(it.posCurrent)->right != SENTINEL; it.posCurrent = ToAddr(it.posCurrent)->right);

		if (it.posCurrent == SENTINEL)
			it.posCurrent = NULL;

		return it;
	}

	descend_iterator desc_end()
	{
		descend_iterator	it;

		it.posCurrent = POS_NULL;
		it.m_pAllocator = m_pAllocator;

		return it;
	}

	ascend_iterator asc_begin()
	{
		ascend_iterator	it;

		it.m_pAllocator = m_pAllocator;

		for (it.posCurrent = m_pHead()->posRoot; ToAddr(it.posCurrent)->left != SENTINEL; it.posCurrent = ToAddr(it.posCurrent)->left);

		if (it.posCurrent == SENTINEL)
			it.posCurrent = NULL;

		return it;
	}

	ascend_iterator asc_end()
	{
		ascend_iterator	it;

		it.posCurrent = POS_NULL;
		it.m_pAllocator = m_pAllocator;

		return it;
	}

	LPVOID asc_front()
	{
		PRbtNode	p;

		if (m_pHead()->m_posRoot != SENTINEL)
		{
			p = ToAddr(m_pHead()->m_posRoot);
			while (p->left != SENTINEL)
				p = ToAddr(p->left);

			return m_pAllocator->PosToAddr(p->data);
		}

		return NULL;
	}

	LPVOID asc_back()
	{
		PRbtNode	p;

		if (m_pHead()->m_posRoot != SENTINEL)
		{
			p = ToAddr(m_pHead()->m_posRoot);
			while (p->right != SENTINEL)
				p = ToAddr(p->right);

			return m_pAllocator->PosToAddr(p->data);
		}

		return NULL;
	}

public:
	inline PRbtNode ToAddr(DB_POS pos)
	{
		return (PRbtNode)(m_pAllocator->PosToAddr(pos));
	}

	inline DB_POS ToPos(PRbtNode node)
	{
		return (DB_POS)(m_pAllocator->AddrToPos(node));
	}

	void test()
	{
		Insert(100);
		remove(GetRootPos());
		int		i;
		vector<RbtNode*>	vNode;
		Search(&i);
		SearchAll(&i, vNode, GetRootPos());
		ascend_iterator	ita;
		descend_iterator itd;

		ita = asc_begin();
		for (ita = asc_begin(); ita != asc_end(); ++ita)
		{
			
		}

		itd = desc_begin();
		for (itd = desc_begin(); itd != desc_end(); ++itd)
		{
			
		}
	}

protected:
	void rotateLeft(RbtNode *x)
	{
		// rotate node x to left
		
		RbtNode *y = ToAddr(x->right);
		
		// establish x->right link
		x->right = y->left;
		if (y->left != SENTINEL) ToAddr(y->left)->parent = ToPos(x);
		
		// establish y->parent link
		if (ToPos(y) != SENTINEL) y->parent = x->parent;
		if (x->parent) {
			if (x == ToAddr(ToAddr(x->parent)->left))
				ToAddr(x->parent)->left = ToPos(y);
			else
				ToAddr(x->parent)->right = ToPos(y);
		} else {
			m_pHead()->posRoot = ToPos(y);
		}
		
		// link x and y
		y->left = ToPos(x);
		if (ToPos(x) != SENTINEL) x->parent = ToPos(y);
	}
	
	void rotateRight(RbtNode *x)
	{
		// rotate node x to right
		
		RbtNode *y = ToAddr(x->left);
		
		// establish x->left link
		x->left = y->right;
		if (y->right != SENTINEL) ToAddr(y->right)->parent = ToPos(x);
		
		// establish y->parent link
		if (ToPos(y) != SENTINEL) y->parent = x->parent;
		if (x->parent) {
			if (ToPos(x) == ToAddr(x->parent)->right)
				ToAddr(x->parent)->right = ToPos(y);
			else
				ToAddr(x->parent)->left = ToPos(y);
		} else {
			m_pHead()->posRoot = ToPos(y);
		}
		
		// link x and y
		y->right = ToPos(x);
		if (ToPos(x) != SENTINEL) x->parent = ToPos(y);
	}
	
	void insertFixup(RbtNode *x) {
		RbtNode	*x_parent;
		
		// maintain red-black tree balance after inserting node x
		
		x_parent = ToAddr(x->parent);
		// check red-black properties
		// while (x != m_pHead()->posRoot && x_parent->color == RED) {
		for (x_parent = ToAddr(x->parent);
			x != ToAddr(m_pHead()->posRoot) && x_parent->color == RED;
			x_parent = ToAddr(x->parent)) {
			// we have a violation
			RbtNode *x_parent_parent = ToAddr(x_parent->parent);
			if (x->parent == x_parent_parent->left) {
				RbtNode *y = ToAddr(x_parent_parent->right);
				if (y->color == RED) {
					
					// uncle is RED
					x_parent->color = BLACK;
					y->color = BLACK;
					x_parent_parent->color = RED;
					x = x_parent_parent;
				} else {
					// uncle is BLACK
					if (x == ToAddr(x_parent->right)) {
						// make x a left child
						x = x_parent;
						rotateLeft(x);
					}
					
					// recolor and rotate
					x_parent->color = BLACK;
					x_parent_parent->color = RED;
					rotateRight(x_parent_parent);
				}
			} else {
				
				// mirror image of above code
				RbtNode *y = ToAddr(x_parent_parent->left);
				if (y->color == RED) {
					
					// uncle is RED
					x_parent->color = BLACK;
					y->color = BLACK;
					x_parent_parent->color = RED;
					x = x_parent_parent;
				} else {
					
					// uncle is BLACK
					if (x == ToAddr(x_parent->left)) {
						x = x_parent;
						rotateRight(x);
					}
					x_parent->color = BLACK;
					x_parent_parent->color = RED;
					rotateLeft(x_parent_parent);
				}
			}
		}
		ToAddr(m_pHead()->posRoot)->color = BLACK;
	}
	
	RbtStatus Insert(DB_POS data)
	{
		RbtNode *current, *parent, *x;
		DB_POS	poscurrent;
		
		// allocate node for data and insert in tree
		
		// find future parent
		poscurrent = m_pHead()->posRoot;
		parent = 0;
		while (poscurrent != SENTINEL) {
			current = ToAddr(poscurrent);
			int rc = m_funCompare(m_pAllocator->PosToAddr(data), m_pAllocator->PosToAddr(current->data));
			if (rc == 0)
				return RBT_STATUS_DUPLICATE_KEY;
			parent = current;
			poscurrent = (rc < 0) ? current->left : current->right;
		}
		
		// setup new node
		if ((x = (PRbtNode)m_pAllocator->Alloc(sizeof(*x))) == 0)
			return RBT_STATUS_MEM_EXHAUSTED;
		x->parent = ToPos(parent);
		x->left = SENTINEL;
		x->right = SENTINEL;
		x->color = RED;
		x->data = data;
		
		// insert node in tree
		if(parent) {
			if(m_funCompare(m_pAllocator->PosToAddr(data), m_pAllocator->PosToAddr(parent->data)) < 0)
				parent->left = ToPos(x);
			else
				parent->right = ToPos(x);
		} else {
			m_pHead()->posRoot = ToPos(x);
		}
		
		insertFixup(x);
		
		return RBT_STATUS_OK;
	}
	
	void deleteFixup(RbtNode *x) {
		
		// maintain red-black tree balance after deleting node x
		
		while (x != ToAddr(m_pHead()->posRoot) && x->color == BLACK) {
			RbtNode	*x_parent = ToAddr(x->parent);

			if (x == ToAddr(x_parent->left)) {
				RbtNode *w = ToAddr(x_parent->right);
				if (w->color == RED) {
					w->color = BLACK;
					x_parent->color = RED;
					rotateLeft (x_parent);
					w = ToAddr(x_parent->right);
				}
				if (ToAddr(w->left)->color == BLACK && ToAddr(w->right)->color == BLACK) {
					w->color = RED;
					x = x_parent;
				} else {
					if (ToAddr(w->right)->color == BLACK) {
						ToAddr(w->left)->color = BLACK;
						w->color = RED;
						rotateRight (w);
						w = ToAddr(x_parent->right);
					}
					w->color = x_parent->color;
					ToAddr(x->parent)->color = BLACK;
					ToAddr(w->right)->color = BLACK;
					rotateLeft (x_parent);
					x = ToAddr(m_pHead()->posRoot);
				}
			} else {
				RbtNode *w = ToAddr(x_parent->left);
				if (w->color == RED) {
					w->color = BLACK;
					x_parent->color = RED;
					rotateRight (x_parent);
					w = ToAddr(x_parent->left);
				}
				if (ToAddr(w->right)->color == BLACK && ToAddr(w->left)->color == BLACK) {
					w->color = RED;
					x = x_parent;
				} else {
					if (ToAddr(w->left)->color == BLACK) {
						ToAddr(w->right)->color = BLACK;
						w->color = RED;
						rotateLeft(w);
						w = ToAddr(x_parent->left);
					}
					w->color = x_parent->color;
					x_parent->color = BLACK;
					ToAddr(w->left)->color = BLACK;
					rotateRight(x_parent);
					x = ToAddr(m_pHead()->posRoot);
				}
			}
		}
		x->color = BLACK;
	}

	void remove(DB_POS posData)
	{
		remove2(posData, m_pHead()->posRoot);
	}

	void remove2(DB_POS posData, DB_POS posStartNode)
	{
		RbtNode		*current;
		DB_POS		poscurrent;
		poscurrent = posStartNode;
		while(poscurrent != SENTINEL) {
			current = ToAddr(poscurrent);
			int rc = m_funCompare(m_pAllocator->PosToAddr(posData),
				m_pAllocator->PosToAddr(current->data));
			if (rc == 0)
			{
				if (posData == current->data)
				{
					Erase(current);
					return;
				}
				if (current->left != SENTINEL)
					remove2(posData, current->left);
				if (current->right != SENTINEL)
					remove2(posData, current->right);
				return;
			}
			poscurrent = (rc < 0) ? current->left : current->right;
		}
	}

	RbtStatus Erase(RbtNode *z)
	{
		RbtNode *x, *y;
		
		if (z->left == SENTINEL || z->right == SENTINEL) {
			// y has a SENTINEL node as a child
			y = z;
		} else {
			// find tree successor with a SENTINEL node as a child
			y = ToAddr(z->right);
			while (y->left != SENTINEL) y = ToAddr(y->left);
		}
		
		// x is y's only child
		if (y->left != SENTINEL)
			x = ToAddr(y->left);
		else
			x = ToAddr(y->right);
		
		// remove y from the parent chain
		x->parent = y->parent;
		if (y->parent)
		{
			if (ToPos(y) == ToAddr(y->parent)->left)
				ToAddr(y->parent)->left = ToPos(x);
			else
				ToAddr(y->parent)->right = ToPos(x);
		}
		else
			m_pHead()->posRoot = ToPos(x);

		if (y != z)
			z->data = y->data;
		
		
		if (y->color == BLACK)
			deleteFixup(x);
		
		m_pAllocator->Free(ToPos(y));
		
		return RBT_STATUS_OK;
	}

	RbtNode *Search(LPVOID lpItem)
	{
		RbtNode		*current;
		DB_POS		poscurrent;
		poscurrent = m_pHead()->posRoot;
		while(poscurrent != SENTINEL) {
			current = ToAddr(poscurrent);
			int rc = m_funCompare(lpItem, m_pAllocator->PosToAddr(current->data));
			if (rc == 0) return current;
			poscurrent = (rc < 0) ? current->left : current->right;
		}
		return NULL;
	}

	void SearchAll(LPVOID lpItem, vector<PRbtNode> &vNode, DB_POS posStartNode)
	{
		RbtNode		*current;
		DB_POS		poscurrent;
		poscurrent = posStartNode;
		while(poscurrent != SENTINEL) {
			current = ToAddr(poscurrent);
			int rc = m_funCompare(lpItem, m_pAllocator->PosToAddr(current->data));
			if (rc == 0)
			{
				vNode.push_back(current);
				if (current->left != SENTINEL)
					SearchAll(lpItem, vNode, current->left);
				if (current->right != SENTINEL)
					SearchAll(lpItem, vNode, current->right);
				return;
			}
			poscurrent = (rc < 0) ? current->left : current->right;
		}
	}

	template<class __FunCmp2>
	int avlSearchAll(__FunCmp2 &funCmp, vector<PAVLNode> &vNode, DB_POS posStartNode)
	{
		RbtNode		*current;
		DB_POS		poscurrent;
		poscurrent = posStartNode;
		while(poscurrent != SENTINEL) {
			current = ToAddr(poscurrent);
			int rc = funCmp(m_pAllocator->PosToAddr(current->data));
			if (rc == 0)
			{
				vNode.push_back(current);
				if (current->left != SENTINEL)
					SearchAll(funCmp, vNode, current->left);
				if (current->right != SENTINEL)
					SearchAll(funCmp, vNode, current->right);
			}
			poscurrent = (rc < 0) ? current->left : current->right;
		}
	}

	void Clear()
	{
		m_nHeadOffset = 0;
		m_pAllocator = NULL;
	}

	void InitSet(__Allocator *pAllocator, int nHeadOffset)
	{
		m_pAllocator = pAllocator;
		m_nHeadOffset = nHeadOffset;

		if (m_pHead()->posSentinel == POS_NULL)
		{
			m_pHead()->posSentinel = CreateNode(POS_NULL);
			m_pHead()->posRoot = m_pHead()->posSentinel;
		}
	}

	void SetCompareFun(__FunCmpare &funCompare, DWORD dwFlag)
	{
		m_funCompare = funCompare;
		m_pHead()->m_dwFlag = dwFlag;
	}

	DWORD GetFlag() { return m_pHead()->m_dwFlag; }

	DB_POS GetRootPos()
	{
		return m_pHead()->posRoot;
	}

protected:
	DB_POS CreateNode(DB_POS posData)
	{
		RbtNode		*x;

		x = (PRbtNode)m_pAllocator->Alloc(RbtNode);
		if (x == 0)
			return POS_NULL;

		x->parent = POS_NULL;
		x->left = SENTINEL;
		x->right = SENTINEL;
		x->color = BLACK;
		x->data = posData;

		return ToPos(x);
	}

	RbtHead			*m_pHead()
	{
		return (RbtHead*)(m_pAllocator->GetRemainDataPtr() + m_nHeadOffset);
	}
	int				m_nHeadOffset;

	__FunCmpare		m_funCompare;
	__Allocator		*m_pAllocator;

};

#endif // _RED_BLACK_TREE_INC_

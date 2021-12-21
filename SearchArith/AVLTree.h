// AVLTree.h: interface for the CAVLTree class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AVLTREE_H__E08476B5_E717_4258_BC0B_A34EF6149A22__INCLUDED_)
#define AFX_AVLTREE_H__E08476B5_E717_4258_BC0B_A34EF6149A22__INCLUDED_


typedef DWORD	DB_POS;

typedef struct AVLNode{
    DB_POS		data;                  /* 结点的数据位置 */
    int			bf;                   /* 结点的平衡因子 */
	DB_POS	llink, rlink;       /* 分别指向结点的左、右子女 */
}AVLNode, *PAVLNode;

#define 	POS_NULL		0				// 位置为空

#define		BFB_LEFT			1
#define		BFB_EQUAL			0
#define		BFB_RIGHT			(-1)

// 二分比较函数定义，大于则返回1,小于返回-1,等于返回0
typedef int (*FUNCompare)(LPVOID, LPVOID);

typedef BOOL (*FUNVisit)(LPVOID, DB_POS dwPhisicalPos, LPVOID lpUserData);

#define __VERIFY_BF		0
#if __VERIFY_BF == 1
#define VERIFYBF(p)		VerifyBf(p)
#else
#define VERIFYBF(p)		
#endif

template<class __Allocator>
class CAVLTree  
{
public:
	class ascend_iterator
	{
	public:
		ascend_iterator()
		{
			posCurrent = POS_NULL;
			m_pAllocator = NULL;
		}

		inline PAVLNode ToAddr(DB_POS pos)
		{
			//assert(pos != POS_NULL);
			return (PAVLNode)(m_pAllocator->PosToAddr(pos));
		}

		inline DB_POS ToPos(PAVLNode node)
		{
			return (DB_POS)(m_pAllocator->AddrToPos(node));
		}

		LPVOID operator*()
		{
			PAVLNode	p;

			p = ToAddr(posCurrent);
			return ToAddr(p->data);
		}
//
//		bool operator=(ascend_iterator &it)
//		{
//			return (posCurrent == it.posCurrent);
//		}

		bool operator!=(ascend_iterator &it)
		{
			return (posCurrent != it.posCurrent);
		}

		ascend_iterator& operator++()
		{
			PAVLNode			p;

			if (posCurrent == POS_NULL)
				return *this;

			p = ToAddr(posCurrent);
			//assert(p->llink == POS_NULL);

			if (p->rlink != POS_NULL)
			{
				vStack.push_back(p->rlink);
				p = ToAddr(p->rlink);
			}
			else
			{
				if (vStack.size() == 0)
				{
					posCurrent = POS_NULL;
					return *this;
				}

				posCurrent = vStack.back();
				vStack.pop_back();
				return *this;
			}

			while (1)
			{
				if (p->llink != POS_NULL)
				{
					vStack.push_back(p->llink);
					p = ToAddr(p->llink);
					continue;
				}

				posCurrent = vStack.back();
				vStack.pop_back();
				break;
			}

			return (*this);
		}
	protected:
		vector<DB_POS>	vStack;
		DB_POS			posCurrent;
		__Allocator		*m_pAllocator;

		friend class CAVLTree<__Allocator>;

	};

	class descend_iterator
	{
	public:
		descend_iterator()
		{
			posCurrent = POS_NULL;
			m_pAllocator = NULL;
		}

		inline PAVLNode ToAddr(DB_POS pos)
		{
			//assert(pos != POS_NULL);
			return (PAVLNode)(m_pAllocator->PosToAddr(pos));
		}

		inline DB_POS ToPos(PAVLNode node)
		{
			return (DB_POS)(m_pAllocator->AddrToPos(node));
		}

		LPVOID operator*()
		{
			PAVLNode	p;

			p = ToAddr(posCurrent);
			return ToAddr(p->data);
		}

		bool operator!=(descend_iterator &it)
		{
			return (posCurrent != it.posCurrent);
		}

		descend_iterator& operator++()
		{
			PAVLNode			p;

			if (posCurrent == POS_NULL)
				return *this;

			p = ToAddr(posCurrent);

			if (p->llink != POS_NULL)
			{
				vStack.push_back(p->llink);
				p = ToAddr(p->llink);
			}
			else
			{
				if (vStack.size() == 0)
				{
					posCurrent = POS_NULL;
					return *this;
				}

				posCurrent = vStack.back();
				vStack.pop_back();
				return *this;
			}

			while (1)
			{
				if (p->rlink != POS_NULL)
				{
					vStack.push_back(p->rlink);
					p = ToAddr(p->rlink);
					continue;
				}

				posCurrent = vStack.back();
				vStack.pop_back();
				break;
			}

			return (*this);
		}
	protected:
		vector<DB_POS>	vStack;
		DB_POS			posCurrent;
		__Allocator		*m_pAllocator;

		friend class CAVLTree<__Allocator>;

	};

	descend_iterator desc_begin()
	{
		descend_iterator	it;
		PAVLNode	p;

		it.m_pAllocator = m_pAllocator;

		if (m_posRoot != POS_NULL)
		{
			p = ToAddr(m_posRoot);
			it.vStack.push_back(m_posRoot);
			while (1)
			{
				if (p->rlink != POS_NULL)
				{
					it.vStack.push_back(p->rlink);
					p = ToAddr(p->rlink);
					continue;
				}
				break;
			}
			it.posCurrent = it.vStack.back();
			it.vStack.pop_back();
		}

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
		PAVLNode	p;

		it.m_pAllocator = m_pAllocator;

		if (m_posRoot != POS_NULL)
		{
			p = ToAddr(m_posRoot);
			it.vStack.push_back(m_posRoot);
			while (1)
			{
				if (p->llink != POS_NULL)
				{
					it.vStack.push_back(p->llink);
					p = ToAddr(p->llink);
					continue;
				}
				break;
			}
			it.posCurrent = it.vStack.back();
			it.vStack.pop_back();
		}

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
		PAVLNode	p;

		if (m_posRoot != POS_NULL)
		{
			p = ToAddr(m_posRoot);
			while (1)
			{
				if (p->llink != POS_NULL)
				{
					p = ToAddr(p->llink);
					continue;
				}
				break;
			}

			return m_pAllocator->PosToAddr(p->data);
		}

		return NULL;
	}

	LPVOID asc_back()
	{
		PAVLNode	p;

		if (m_posRoot != POS_NULL)
		{
			p = ToAddr(m_posRoot);
			while (1)
			{
				if (p->rlink != POS_NULL)
				{
					p = ToAddr(p->rlink);
					continue;
				}
				break;
			}
			return m_pAllocator->PosToAddr(p->data);
		}

		return NULL;
	}

public:
	CAVLTree()
	{
		m_posRoot = POS_NULL;
		m_funCompare = NULL;
		m_pAllocator = NULL;
	}
	virtual ~CAVLTree()
	{
	}

	void Clear()
	{
		m_posRoot = POS_NULL;
		m_funCompare = NULL;
		m_pAllocator = NULL;
	}

	void SetAllocator(__Allocator *pAllocator)
	{
		m_pAllocator = pAllocator;
	}

	void SetCompareFun(FUNCompare funCompare, DWORD dwFlag)
	{
		m_funCompare = funCompare;
		m_dwFlag = dwFlag;
	}

	DWORD GetFlag() { return m_dwFlag; }

	DB_POS GetRootPos()
	{
		return m_posRoot;
	}

	inline PAVLNode ToAddr(DB_POS pos)
	{
		//assert(pos != POS_NULL);
		return (PAVLNode)(m_pAllocator->PosToAddr(pos));
	}

	inline DB_POS ToPos(PAVLNode node)
	{
		return (DB_POS)(m_pAllocator->AddrToPos(node));
	}

	PAVLNode avlSearch(LPVOID lpItem)
	{
		DB_POS	pos;
		PAVLNode	p;
		int			nResult;

		pos = m_posRoot;
		while (pos != POS_NULL)
		{
			p = ToAddr(pos);

			nResult = m_funCompare(lpItem, ToAddr(p->data));
			/* 寻找插入位置及最小不平衡子树 */
			if (nResult < 0)
				pos = p->llink;			/* 进入*p的左子树 */ 
			else if (nResult > 0)
				pos = p->rlink;         /* 进入*p的右子树 */ 
			else
				return p;/* AVL树中已经有关键码为key的结点 */
		}
		return NULL;
	}

	void avlSearchAll(LPVOID lpItem, vector<PAVLNode> &vNode, DB_POS posStartNode)
	{
		DB_POS	pos;
		PAVLNode	p;
		int			nResult;

		pos = posStartNode;
		while (pos != POS_NULL)
		{
			p = ToAddr(pos);

			nResult = m_funCompare(lpItem, ToAddr(p->data));
			/* 寻找插入位置及最小不平衡子树 */
			if (nResult == 0)
			{
				vNode.push_back(p);
				avlSearchAll(lpItem, vNode, p->llink);
				avlSearchAll(lpItem, vNode, p->rlink);
				break;
			}
			else if (nResult < 0)
				pos = p->llink;			/* 进入*p的左子树 */ 
			else
				pos = p->rlink;         /* 进入*p的右子树 */ 
		}
	}

	int avlSearchAll(FUNCompare funCmp, LPVOID lpItem, vector<PAVLNode> &vNode, DB_POS posStartNode)
	{
		DB_POS	pos;
		PAVLNode	p;
		int			nResult;

		pos = posStartNode;
		while (pos != POS_NULL)
		{
			p = ToAddr(pos);

			nResult = funCmp(lpItem, ToAddr(p->data));
			/* 寻找插入位置及最小不平衡子树 */
			if (nResult == 0)
			{
				vNode.push_back(p);
				if (p->llink != POS_NULL)
					avlSearchAll(funCmp, lpItem, vNode, p->llink);
				if (p->rlink != POS_NULL)
					avlSearchAll(funCmp, lpItem, vNode, p->rlink);
				break;
			}
			else if (nResult < 0)
				pos = p->llink;			/* 进入*p的左子树 */ 
			else
				pos = p->rlink;         /* 进入*p的右子树 */ 
		}
		return TRUE;
	}

	DB_POS creatNode(DB_POS data)
	{
		DB_POS		pos;
		PAVLNode		node;

		node = (PAVLNode)m_pAllocator->Alloc(sizeof(AVLNode));
		if (!node)
			return POS_NULL;
		pos = ToPos(node);

		node->data = data; node->bf = BFB_EQUAL;
		node->llink = node->rlink = POS_NULL;

		return pos;
	}

	int avlInsert(DB_POS data)
	{    
		PAVLNode	node_a, node_b, parent_a, p, parent_p, node;
		DB_POS		nodePos, pos;
		LPVOID		lpNewItemData;
		int			newbf;

		lpNewItemData = ToAddr(data);

		if (m_posRoot == POS_NULL)
		{
			/* 原来AVL树为空 */
			m_posRoot = creatNode(data);
			if (m_posRoot != POS_NULL)
				return ERR_OK;
			else
				return ERR_NO_MEM;
		}

		pos = m_posRoot;
		node_a = ToAddr(m_posRoot);
		parent_a = NULL;
		parent_p = NULL;
		p = NULL;

		while (pos != POS_NULL)
		{
			/* 寻找插入位置及最小不平衡子树 */
			parent_p = p;
			p = ToAddr(pos);
			if (m_funCompare(lpNewItemData, ToAddr(p->data)) <= 0)
				pos = p->llink;		/* 进入*p的左子树 */ 
			else
				pos = p->rlink;		/* 进入*p的右子树 */ 
			
			if (p->bf != BFB_EQUAL)
			{
				/* 寻找最小不平衡子树*node_a */
				parent_a = parent_p;
				node_a = p;
			}
		}
		
		// p 是最后的节点；parent_p 是最后的节点的父节点

		nodePos = creatNode(data);
		if (nodePos == POS_NULL)
			return ERR_NO_MEM;

		node = ToAddr(nodePos);
		if (m_funCompare(lpNewItemData, ToAddr(p->data)) <= 0)
			p->llink = nodePos;			/* 使新结点成为*p的左子女 */        
		else
			p->rlink = nodePos;			/* 使新结点成为*p的右子女 */ 
		
		/* 已经将新结点结点插入到AVL树中，node_a指向最小不平衡子树。 */
		if (m_funCompare(lpNewItemData, ToAddr(node_a->data)) <= 0)
		{
			/* 新结点是插在*node_a的左子树中 */
			pos = node_a->llink;
			node_b = ToAddr(node_a->llink);
			newbf = BFB_LEFT;
		}
		else
		{
			/* 新结点是插在*node_a的右子树中 */
			pos = node_a->rlink;
			node_b = ToAddr(node_a->rlink);
			newbf = BFB_RIGHT;
		}
		
		/* 修改*node_b到新结点路径上的各结点的平衡因子，*node_b为*node_a的子女 */
		while ( (pos != POS_NULL) && (pos != nodePos) )
		{
			p = ToAddr(pos);
			if (m_funCompare(lpNewItemData, ToAddr(p->data)) <= 0)
			{
				/* *p的左子树高度加1 */
				p->bf = BFB_LEFT;
				pos = p->llink;
			}
			else
			{
				/* *p的右子树高度加1 */
				p->bf = BFB_RIGHT;
				pos = p->rlink;
			}
		}
		
		/* *node_a原来的平衡因子为0，插入结点后只可能为1或－1，没有破坏树平衡 */
		if (node_a->bf == BFB_EQUAL)
		{
			node_a->bf = newbf;
			return ERR_OK;
		}
		
		/* 新结点插在原来高度较小的子树中 */
		if (node_a->bf == -newbf)
		{
			node_a->bf = BFB_EQUAL;
			return ERR_OK;
		}
		
		/* 下面是新结点插在原来高度较大的子树中，破坏了平衡性，要对子树进行调整 */
		if (newbf == BFB_LEFT)
		{
			/* 新结点插在*node_a的左子树中*/
			assert(node_b->bf != BFB_EQUAL);
			if (node_b->bf == BFB_LEFT)
				node_b = lL(node_a, node_b); /* LL型调整*/
			else
				node_b = lR(node_a, node_b); /* LR型调整*/
		}
		else
		{
			/* 新结点插在*node_a的右子树中*/
			// if(node_b->bf == 1)
			assert(node_b->bf != BFB_EQUAL);
			if (node_b->bf == BFB_RIGHT)
				node_b = rR(node_a, node_b);  /* RR型调整 */
			else
				node_b=rL(node_a, node_b);   /* RL 型调整 */
		}
		
		if (parent_a == NULL)  /* node_a 原来指向树的根结点 */
			m_posRoot = ToPos(node_b);
		else if(parent_a->llink == ToPos(node_a))
			parent_a->llink = ToPos(node_b);
		else
			parent_a->rlink = ToPos(node_b);

		return ERR_OK;
	}

	DB_POS FindMin(PAVLNode p)
	{
		while (p->llink != POS_NULL)
			p = ToAddr(p->llink);

		return p->data;
	}

	DB_POS FindMax(PAVLNode p)
	{
		while (p->rlink != POS_NULL)
			p = ToAddr(p->rlink);

		return p->data;
	}

	int remove(DB_POS posData)
	{
		if (m_posRoot == POS_NULL)
			return ERR_NOT_FOUND;

		BOOL	bHeightDec = FALSE;

		return remove2(posData, ToAddr(m_posRoot), NULL, bHeightDec);
	}

	int removeTreeOnly()
	{
		if (m_posRoot != POS_NULL)
		{
			removeTreeOnly(m_posRoot);
			m_posRoot = POS_NULL;
		}

		return ERR_OK;
	}

	void VisitLeftAll(DB_POS posStart, FUNVisit funVisit, LPVOID lpUserData)
	{
		PAVLNode			p;

		p = ToAddr(posStart);

		if (p->llink != POS_NULL)
			VisitLeftAll(p->llink, funVisit, lpUserData);

		// call output
		funVisit(ToAddr(p->data), p->data, lpUserData);

		if (p->rlink != POS_NULL)
			VisitLeftAll(p->rlink, funVisit, lpUserData);
	}


	void VisitLeft(DB_POS posStart, uint32 nCount, FUNVisit funVisit, LPVOID lpUserData)
	{
		PAVLNode			p;
		vector<PAVLNode>	vStack;
		uint32				i;

		if (nCount <= 0 || posStart == POS_NULL)
			return;

		p = ToAddr(posStart);

		vStack.push_back(p);
		i = 0;
		while (vStack.size())
		{
			if (p->llink != POS_NULL)
			{
				p = ToAddr(p->llink);
				vStack.push_back(p);
				continue;
			}

			p = vStack.back();
			vStack.pop_back();

			while (1)
			{
				funVisit(ToAddr(p->data), p->data, lpUserData);
				i++;
				if (i >= nCount)
					return;

				if (p->rlink != POS_NULL)
				{
					p = ToAddr(p->rlink);
					vStack.push_back(p);
					break;
				}

				if (vStack.size() == 0)
					break;

				p = vStack.back();
				vStack.pop_back();
			}
		}
//
//		p = ToAddr(posStart);
//		if (p->llink != POS_NULL)
//			VisitLeft(p->llink);
//
//		// call output
//		nCount--;
//		funVisit()
//
//		if (p->rlink != POS_NULL)
//			VisitLeft(p->rlink);
	}

	void VisitRight(DB_POS posStart, uint32 nCount, FUNVisit funVisit, LPVOID lpUserData)
	{
		PAVLNode			p;
		vector<PAVLNode>	vStack;
		uint32				i;

		if (nCount <= 0 || posStart == POS_NULL)
			return;

		p = ToAddr(posStart);

		vStack.push_back(p);
		i = 0;
		while (vStack.size())
		{
			if (p->rlink != POS_NULL)
			{
				p = ToAddr(p->rlink);
				vStack.push_back(p);
				continue;
			}

			p = vStack.back();
			vStack.pop_back();

			while (1)
			{
				funVisit(ToAddr(p->data), p->data, lpUserData);
				i++;
				if (i >= nCount)
					return;

				if (p->llink != POS_NULL)
				{
					p = ToAddr(p->llink);
					vStack.push_back(p);
					break;
				}

				if (vStack.size() == 0)
					break;

				p = vStack.back();
				vStack.pop_back();
			}
		}
	}

	int GetHeight(PAVLNode p)
	{
		int	nH1 = 0, nH2 = 0;
		if (p->llink)
			nH1 = GetHeight(ToAddr(p->llink));

		if (p->rlink)
			nH2 = GetHeight(ToAddr(p->rlink));

		if (nH1 >= nH2)
			return nH1 + 1;
		else
			return nH2 + 1;
	}

	int VerifyBf(PAVLNode p)
	{
		int	nH1 = 0, nH2 = 0;
		if (p->llink)
			nH1 = VerifyBf(ToAddr(p->llink));

		if (p->rlink)
			nH2 = VerifyBf(ToAddr(p->rlink));

		assert(nH1 - nH2 >= -1 && nH1 - nH2 <=1);
		assert(nH1 - nH2 == p->bf);

		if (nH1 >= nH2)
			return nH1 + 1;
		else
			return nH2 + 1;
	}

protected:
	//        B               A
	//     A     B2       A1      B
	//   A1 A2          AL      A2  B2   
	// AL              
	//
	PAVLNode lL(PAVLNode b, PAVLNode a)
	{
		b->bf = BFB_EQUAL;
		b->llink = a->rlink;
		a->bf = BFB_EQUAL;
		a->rlink = ToPos(b);                    /* a指向调整后的子树的根结点 */

		VERIFYBF(a);
		VERIFYBF(b);

		return a;
	}

	//        B               A
	//     A     B2       A1      B
	//   A1 A2          A3      A2  B2   
	// A3    A4                  A4
	//
	PAVLNode lLforDel(PAVLNode b, PAVLNode a)
	{
		b->bf = BFB_LEFT;
		b->llink = a->rlink;
		a->bf = BFB_RIGHT;
		a->rlink = ToPos(b);                    /* a指向调整后的子树的根结点 */

		VERIFYBF(a);
		VERIFYBF(b);

		return a;
	}


	//        B               A
	//     B1    A         B    A2
	//         A1  A2    B1 A1    AR
	//               AR 
	//
	PAVLNode rR(PAVLNode b, PAVLNode a)
	{
		b->bf = BFB_EQUAL;
		b->rlink = a->llink;
		a->bf = BFB_EQUAL;
		a->llink = ToPos(b);

		VERIFYBF(b);
		VERIFYBF(a);

		return a;
	}

	//        B               A
	//     B1    A         B    A2
	//         A1  A2    B1 A1    A4
	//       A3      A4   A3   
	//
	PAVLNode rRforDel(PAVLNode b, PAVLNode a)
	{
		b->bf = BFB_RIGHT;
		b->rlink = a->llink;
		a->bf = BFB_LEFT;
		a->llink = ToPos(b);

		VERIFYBF(b);
		VERIFYBF(a);

		return a;
	}

	//                               
	//         C               B
	//     A      C2        A     C
	// A1    B            A1  B1 B2   C2
	//     B1 B2                   br
	//          br                       
	//
	PAVLNode lR(PAVLNode c, PAVLNode a)
	{
		PAVLNode b = ToAddr(a->rlink);
		
		a->rlink = b->llink;
		c->llink = b->rlink;
		b->llink = ToPos(a);
		b->rlink = ToPos(c);
		
		switch (b->bf)
		{
		case BFB_EQUAL:
			c->bf = BFB_EQUAL; a->bf = BFB_EQUAL;
			break;
		case BFB_LEFT:
			a->bf = BFB_EQUAL; c->bf = BFB_RIGHT; b->bf = BFB_EQUAL;
			break;
		case BFB_RIGHT:
			a->bf = BFB_LEFT; c->bf = BFB_EQUAL; b->bf = BFB_EQUAL;
			break;
		}

		VERIFYBF(a);
		VERIFYBF(b);
		VERIFYBF(c);
		
		return b;
	}

	//                               
	//         C               B
	//     A      C2        A     C
	// A1    B            A1  B1 B2   C2
	//     B1 B2                     
	//                                   
	//
	PAVLNode lRforDel(PAVLNode c, PAVLNode a)
	{
		PAVLNode b = ToAddr(a->rlink);
		
		a->rlink = b->llink;
		c->llink = b->rlink;
		b->llink = ToPos(a);
		b->rlink = ToPos(c);
		
		switch (b->bf)
		{
		case BFB_EQUAL:
			c->bf = BFB_EQUAL; a->bf = BFB_LEFT; b->bf = BFB_LEFT;
			break;
		case BFB_LEFT:
			a->bf = BFB_LEFT; c->bf = BFB_RIGHT; b->bf = BFB_EQUAL;
			break;
		case BFB_RIGHT:
			a->bf = BFB_LEFT; c->bf = BFB_EQUAL; b->bf = BFB_EQUAL;
			break;
		}

		VERIFYBF(a);
		VERIFYBF(b);
		VERIFYBF(c);
		
		return b;
	}


	//                               
	//         C               B
	//     C2      A        C     A
	//           B   A1   C2 B1 B2  A1   
	//         B1 B2              br
	//             br                    
	//
	PAVLNode rL(PAVLNode c, PAVLNode a)
	{
		PAVLNode b = ToAddr(a->llink);
		
		a->llink = b->rlink;
		c->rlink = b->llink;
		b->llink = ToPos(c);
		b->rlink = ToPos(a);
		
		switch (b->bf)
		{
		case BFB_EQUAL:
			c->bf = BFB_EQUAL; a->bf = BFB_EQUAL;
			break;
		case BFB_LEFT:
			a->bf = BFB_RIGHT; c->bf = BFB_EQUAL; b->bf = BFB_EQUAL;
			break;
		case BFB_RIGHT:
			a->bf = BFB_EQUAL; c->bf = BFB_LEFT; b->bf = BFB_EQUAL;
			break;
		}

		VERIFYBF(a);
		VERIFYBF(b);
		VERIFYBF(c);
		
		return b;
	}

	//                               
	//         C               B
	//     C2      A        C     A
	//           B   A1   C2 B1 B2  A1   
	//         B1 B2                
	//                                   
	//
	PAVLNode rLforDel(PAVLNode c, PAVLNode a)
	{
		PAVLNode b = ToAddr(a->llink);
		
		a->llink = b->rlink;
		c->rlink = b->llink;
		b->llink = ToPos(c);
		b->rlink = ToPos(a);
		
		switch (b->bf)
		{
		case BFB_EQUAL:
			c->bf = BFB_EQUAL; a->bf = BFB_RIGHT; b->bf = BFB_RIGHT;
			break;
		case BFB_LEFT:
			a->bf = BFB_RIGHT; c->bf = BFB_EQUAL; b->bf = BFB_EQUAL;
			break;
		case BFB_RIGHT:
			a->bf = BFB_EQUAL; c->bf = BFB_LEFT; b->bf = BFB_EQUAL;
			break;
		}

		VERIFYBF(a);
		VERIFYBF(b);
		VERIFYBF(c);
		
		return b;
	}

	// 左边的节点被删除
	void banlanceleft(PAVLNode p, PAVLNode parent_p, BOOL &bHeightDec)
	{
		//  调整节点 p 的平衡度
		if (bHeightDec)
		{
			if (p->bf == BFB_LEFT)
			{
				p->bf = BFB_EQUAL;
				VERIFYBF(p);
			}
			else if (p->bf == BFB_EQUAL)
			{
				p->bf = BFB_RIGHT;
				VERIFYBF(p);
				bHeightDec = FALSE;
			}
			else
			{
				PAVLNode		temp, node_b;

				node_b = ToAddr(p->rlink);
				if (node_b->bf == BFB_RIGHT)
					temp = rR(p, node_b);  /* RR型调整 */
				else if (node_b->bf == BFB_EQUAL)
				{
					temp = rRforDel(p, node_b);  /* RR型调整 */
					bHeightDec = FALSE;
				}
				else
					temp=rL(p, node_b);   /* RL 型调整 */
				if (parent_p)
				{
					if (parent_p->llink == ToPos(p))
						parent_p->llink = ToPos(temp);
					else
						parent_p->rlink = ToPos(temp);
				}
				else
					m_posRoot = ToPos(temp);

				VERIFYBF(temp);
			}
		}

		VERIFYBF(p);
	}

	// 右边的节点被删除
	void banlanceright(PAVLNode p, PAVLNode parent_p, BOOL &bHeightDec)
	{
		//  调整节点 p 的平衡度
		if (bHeightDec)
		{
			if (p->bf == BFB_RIGHT)
			{
				p->bf = BFB_EQUAL;
				VERIFYBF(p);
			}
			else if (p->bf == BFB_EQUAL)
			{
				p->bf = BFB_LEFT;
				VERIFYBF(p);
				bHeightDec = FALSE;
			}
			else
			{
				PAVLNode		temp, node_b;

				node_b = ToAddr(p->llink);
				if (node_b->bf == BFB_LEFT)
					temp = lL(p, node_b); /* LL型调整*/
				else if (node_b->bf == BFB_EQUAL)
				{
					temp = lLforDel(p, node_b); /* LL型调整*/
					bHeightDec = FALSE;
				}
				else
					temp = lR(p, node_b); /* LR型调整*/
				if (parent_p)
				{
					if (parent_p->llink == ToPos(p))
						parent_p->llink = ToPos(temp);
					else
						parent_p->rlink = ToPos(temp);
				}
				else
					m_posRoot = ToPos(temp);

				VERIFYBF(p);
			}
		}

		VERIFYBF(p);
	}

	int remove2(DB_POS posData, PAVLNode p, PAVLNode pParent, BOOL &bHeightDec)
	{
		int			nCmp;
		PAVLNode	child;

		/* 寻找插入位置及最小不平衡子树 */
		nCmp = m_funCompare(ToAddr(posData), ToAddr(p->data));
		if (nCmp < 0)
		{

			//
			// 到左边节点查找，删除
			//
			if (p->llink != POS_NULL)
			{
				child = ToAddr(p->llink);
				if (remove2(posData, child, p, bHeightDec) == ERR_OK)
				{
					banlanceleft(p, pParent, bHeightDec);
					return ERR_OK;
				}
			}
		}
		else if (nCmp > 0)
		{
			//
			// 到右边节点查找，删除
			//
			if (p->rlink != POS_NULL)
			{
				child = ToAddr(p->rlink);
				if (remove2(posData, child, p, bHeightDec) == ERR_OK)
				{
					banlanceright(p, pParent, bHeightDec);
					return ERR_OK;
				}
			}
		}
		else
		{
			// equal
			if (p->data == posData)
			{
				//
				// 找到了
				//

				if (p->llink == POS_NULL && p->rlink == POS_NULL)
				{
					// 如果为叶子节点，直接删除
					DB_POS	posP;

					posP = ToPos(p);

					if (pParent != NULL)
					{
						if (pParent->llink == posP)
							pParent->llink = POS_NULL;
						else
							pParent->rlink = POS_NULL;
					}
					else
					{
						// 为根节点，需删除
						m_posRoot = POS_NULL;
					}

					bHeightDec = TRUE;

					m_pAllocator->Free(posP);
					return ERR_OK;
				}

				//
				// 子节点，需要移动下面的子节点
				//
				if ((p->bf == BFB_LEFT || p->bf == BFB_EQUAL) && p->llink != POS_NULL)
				{
					DB_POS		posDataMax;
					PAVLNode	child;

					child = ToAddr(p->llink);
					posDataMax = FindMax(child);
					if (remove2(posDataMax, child, p, bHeightDec) != ERR_OK)
						assert(0);

					p->data = posDataMax;

					banlanceleft(p, pParent, bHeightDec);
					return ERR_OK;
				}
				else if (p->rlink != POS_NULL)
				{
					assert(p->bf == BFB_RIGHT || p->bf == BFB_EQUAL);
					// p->bf == BFB_RIGHT
					DB_POS		posDataMin;

					child = ToAddr(p->rlink);
					posDataMin = FindMin(child);
					if (remove2(posDataMin, child, p, bHeightDec) != ERR_OK)
						assert(0);

					p->data = posDataMin;

					banlanceright(p, pParent, bHeightDec);
					return ERR_OK;
				}
				assert(0);
				return ERR_OK;
			}
			else
			{
				//
				// 到左边节点查找，删除
				//
				if (p->llink != POS_NULL)
				{
					child = ToAddr(p->llink);
					if (remove2(posData, child, p, bHeightDec) == ERR_OK)
					{
						banlanceleft(p, pParent, bHeightDec);
						return ERR_OK;
					}
				}

				//
				// 到右边节点查找，删除
				//
				if (p->rlink != POS_NULL)
				{
					child = ToAddr(p->rlink);
					if (remove2(posData, child, p, bHeightDec) == ERR_OK)
					{
						banlanceright(p, pParent, bHeightDec);
						return ERR_OK;
					}
				}
			}
		}

		return ERR_FALSE;
	}

	void removeTreeOnly(DB_POS posStart)
	{
		assert(posStart != POS_NULL);

		PAVLNode			p;

		p = ToAddr(posStart);

		if (p->llink != POS_NULL)
			removeTreeOnly(p->llink);

		if (p->rlink != POS_NULL)
			removeTreeOnly(p->rlink);

		m_pAllocator->Free(posStart);
	}

//	int search(DB_POS datapos, PAVLNode &p, PAVLNode &pParent, PAVLNode &node_a, PAVLNode &parent_a)
//	{
//		PAVLNode	node_a_bk = NULL, parent_a_bk = NULL;
//		int			nCmp;
//
//		/* 寻找插入位置及最小不平衡子树 */
//		nCmp = m_funCompare(ToAddr(datapos), ToAddr(p->data));
//		if (nCmp < 0)
//			pos = p->llink;		/* 进入*p的左子树 */ 
//		else if (nCmp > 0)
//			pos = p->rlink;		/* 进入*p的右子树 */
//		else
//		{
//			// equal
//			if (p->data == datapos)
//			{
//				return ERR_OK;
//			}
//			else
//			{
//				if (p->bf != BFB_EQUAL)
//				{
//					/* 寻找最小不平衡子树*node_a */
//					node_a_bk = node_a;
//					parent_a_bk = parent_a;
//
//					parent_a = pParent;
//					node_a = p;
//				}
//
//				if (p->llink != POS_NULL && search(datapos, ToAddr(p->llink), p, node_a, parent_a) == ERR_OK)
//					return ERR_OK;
//				if (p->rlink != POS_NULL && search(datapos, ToAddr(p->rlink), p, node_a, parent_a) == ERR_OK)
//					return ERR_OK;
//
//				// 未找到，恢复之前的最小子树
//				if (node_a_bk != NULL)
//				{
//					node_a = node_a_bk;
//					parent_a = parent_a_bk;
//				}
//			}
//		}
//
//		return ERR_NOT_FOUND;
//	}
//
//
//	int remove(DB_POS datapos)
//	{
//		PAVLNode	node_a, parent_a, p, parent_p;
//		DB_POS	nodePos, pos;
//		int			nRet;
//		int d;
//
//		if (m_posRoot == POS_NULL)
//		{
//			/* 原来AVL树为空 */
//			return 0;
//		}
//		
//		p = node_a = ToAddr(m_posRoot);
//		parent_a = NULL;
//		parent_p = NULL;
//
//		nRet = search(datapos, p, parent_p, node_a, parent_a);
//		if (nRet != ERR_OK)
//			return ERR_NOT_FOUND;
//
//		// p : 要删除的节点
//		if (p->llink == POS_NULL && p->rlink == POS_NULL)
//		{
//			// 直接删除
//			pos = ToPos(p);
//			if (parent_p->llink == pos)
//			{
//				parent_p->llink = POS_NULL;
//				if (parent_p->bf >= 0)
//					parent_p->bf -= 1;
//				else
//				{
//					assert(parent_p->rlink != POS_NULL);
//					rR(parent_p, parent_p->rlink);
//				}
//			}
//			else
//			{
//				parent_p->rlink = POS_NULL;
//				if (parent_p->bf <= 0)
//					parent_p->bf += 1;
//				else
//				{
//					assert(parent_p->llink != POS_NULL);
//					lL(parent_p, parent_p->llink);
//				}
//			}
//			//
//			m_pAllocator->Free(pos);
//		}
//		else
//		{
//			if (p->bf == -1)
//			{
//				// find right
//				int		bfOld;
//				assert(p->rlink != POS_NULL);
//				PAVLNode	right = ToAddr(p->rlink);
//				bfOld = right->bf;
//				DB_POS datamin = FindMin(right);
//				p->data = datamin;
//				remove(datamin);
//				if (bfOld != 0 && right->bf == 0)
//				{
//					//
//					// 高度变小了
//					p->bf = 0;
//					banlance(node_a, p);
//				}
//			}
//			else
//			{
//				DB_POS datamax = FindMax();
//				p->data = datamax;
//				remove(datamax);
//
//				p->bf -= 1;
//			}
//		}
//
//		return TRUE;
//	}
//	int remove(PAVLNode node)
//	{
//		// node->rlink = ;
//
//		return TRUE;
//	}
//
//	DB_POS FindMinChild(PAVLNode p)
//	{
//		if (p->llink == POS_NULL)
//		{
//			if (p->rlink == POS_NULL)
//				return POS_NULL;
//			p = ToAddr(p->rlink);
//		}
//
//		while (p->llink != POS_NULL)
//			p = ToAddr(p->llink);
//
//		return p->data;
//	}

protected:
	DB_POS		m_posRoot;
	FUNCompare		m_funCompare;
	DWORD			m_dwFlag;
	__Allocator		*m_pAllocator;

};

#endif // !defined(AFX_AVLTREE_H__E08476B5_E717_4258_BC0B_A34EF6149A22__INCLUDED_)

#pragma once


struct VarInfo
{
	TypeId				typeId;
	int16				nAddress;			// -1 for unavailable.
	int					nRet;
	bool				bStatic;			// Static variable?
	bool				bMemberVar;
};

//
// CVarDeclStack 
//
class CVarDeclStack
{
public:

	struct VarProperty
	{
		TypeId					typeId;
		// Local Variable: address related to bp: bp + nAddr
		// Static Variable: Offset in static variable.
		uint16					nAddress;
		bool					bStatic;			// Static variable?
	};

	typedef map<string, VarProperty*>	MapVarProp;

	// block is the name space of variables, for example: { }
	struct StmtBlock
	{
		StmtBlock() { m_pClass = NULL; m_pMethod = NULL; }
		CClass					*m_pClass;	// Only method block has this.
		CMemberMethod			*m_pMethod;
		MapVarProp				mapVarProp;
	};

	typedef vector<StmtBlock *>		StmtBlockStack;

public:
	CVarDeclStack();
	virtual ~CVarDeclStack();

	// travel in block
	void EnterClass(CClass *pClass);
	void EnterMethod(CMemberMethod *pMethod);
	void EnterBlock();

	void LeaveBlock();

	void CleanStack();

	CClass *GetCurClass();
	CMemberMethod *GetCurMethod();

	// Add new symbol declaration 
	int AddVarDecl(LPCSTR szName, TypeId typeId, bool bStatic);

	int DeclareStaticMemberVar(CClass *pClass, LPCSTR szName, TypeId typeId);

	// "this" is the first parameter of method call
	void Add_this_super_Var();

	VarInfo GetVariable(LPCSTR szName);

	// For local variable, return offset
	// For member variable, return member variable index.
	int GetVariableAddress(LPCSTR szName);

	bool IsStaticVariable(LPCSTR szName);

	int GetCurrentBlockSize();

	void InitStaticVar(VecStaticVar &vStaticVar);

protected:
	size_t GetCurrentMethodLocalVarCount();

/*	SSM_STACK_ADDR GetSymbolMemLoc(LPCTSTR szName);
	SSM_STACK_ADDR GetCurFuncStackSize();
*/
protected:
	void FreeStmtBlock(StmtBlock *pblock);

protected:
	StmtBlockStack			m_stmtBlockStack;

protected:
	int DeclareStaticVar(LPVOID pMethodOrClass, LPCSTR szVarName, TypeId typeId, int nStmtDeepOfMethod);
	//
	// Static variable space allocate
	//
	struct AddressTypeId
	{
		TypeId		typeId;
		uint16		nAddress;
	};
	typedef map<string, AddressTypeId>		MapStaticVars;
	MapStaticVars			m_mapStaticVars;

};

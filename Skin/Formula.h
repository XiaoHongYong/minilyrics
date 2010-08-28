/********************************************************************
	Created  :	2002/01/04	21:39
	FileName :	Formula.h
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#if !defined(AFX_FORMULA_H__29B0CFC1_7922_11D5_9E04_02608CAD9330__INCLUDED_)
#define AFX_FORMULA_H__29B0CFC1_7922_11D5_9E04_02608CAD9330__INCLUDED_

//#define EW_OPND_ERROR		0
#define FW_OPND_LONG		1
//#define FW_OPND_DOUBLE		2

#define FW_OPND_VAR_LONG	5

#define FW_OPTR				10
#define FW_OPTR_ADD			11
#define FW_OPTR_SUB			12
#define FW_OPTR_MUL			13
#define FW_OPTR_DIV			14
#define FW_OPTR_LBRACKET	15
#define FW_OPTR_RBRACKET	16
#define FW_OPTR_BEGIN		17
#define FW_OPTR_END			18


typedef struct	FORMULA_WORD{
	int			nWordType;		// '单词类型'
	int			Value;			// 值, 根据word的类型而定，可以是
								// 数值，也可能是一个待定的字符“变量”
								// 5, w, h
}FORMULA_WORD, *PFORMULA_WORD;

// ‘变量’的定义
typedef struct FORMULA_VAR{
	int			nVarName;		// variable name
	int			Value;			// variable value
}FORMULA_VAR, *PFORMULA_VAR;

typedef vector<PFORMULA_WORD>		FormulaArray;

class CFormula 
{
public:
	CFormula();
	virtual ~CFormula();

public:
	BOOL CalCualteValue(FORMULA_VAR var[], int &nRetValue);
	BOOL CalCualteValue(int &nRetValue);
	void SetFormula(LPCTSTR szExp);
	void SetFormula(int nConstValue);

	void Append(LPCTSTR szFormula);
	void Increase(int nValue);
	void Decrease(int nValue) { Increase(-nValue); }

	void SimpleOptimize();

	CFormula& operator =(CFormula &fRight);

	BOOL GetVarValue(int nVar, FORMULA_VAR vars[], int &nRetValue);

	LPCTSTR GetFormula() { return m_strFormula.c_str(); }
	bool Empty() { return m_strFormula.empty(); }

protected:
	BOOL PhraseAnalyse(FormulaArray &vWords);

protected:
	tstring			m_strFormula;

};

#endif // !defined(AFX_FORMULA_H__29B0CFC1_7922_11D5_9E04_02608CAD9330__INCLUDED_)

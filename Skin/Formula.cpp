/********************************************************************
	Created  :	2001/7/17	12:03:00
	FileName :	Formula.cpp
	Author   :	xhy
	
	Purpose  :	
*********************************************************************/

#include "stdafx.h"
#include "Formula.h"


BOOL _IsCharBlank(TCHAR ch)
{
	return (ch == ' ' || ch == '	');
}

#define _IsOperator(type)	(type > FW_OPTR)
#define _IsOperand(type)	(type < FW_OPTR)

//                  1, 2, 3, 4, 5,  6, 7, 8
//                  +, -, * , /, (, ), #, #
int OpPriorF[]={-1, 2, 2, 4, 4, 0, 6, 0, 0};
int OpPriorG[]={-1, 1, 1, 3, 3, 7, 0, 0, 0};

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CFormula::CFormula()
{
}

CFormula::~CFormula()
{
}

void CFormula::SetFormula(LPCTSTR szForm)
{
	m_strFormula = szForm;

	SimpleOptimize();
}

BOOL CFormula::CalCualteValue(FORMULA_VAR var[], int &nRetValue)
{
	BOOL	bRet = FALSE;

	FormulaArray		vWords;

	FormulaArray		arrOptr, arrOpnd;

	if (!PhraseAnalyse(vWords))
		goto FAILED;


	int nSize, cur;
	nSize = vWords.size();
	if (nSize <= 2){
		// 表达式为空
		goto FAILED;
	}

	PFORMULA_WORD	pOptrTop;
	PFORMULA_WORD	pWordCur, pWordPrev;

	// init
	cur = 0;
	
	pWordCur = vWords[cur];	// read '#' optr
	pOptrTop = pWordCur;
	arrOptr.push_back(pOptrTop);
	cur ++;

	pWordPrev = pWordCur;
	pWordCur = vWords[cur];	// read word
	cur ++;

	while (1){
		// operand
		if (_IsOperand(pWordCur->nWordType)){
			if (_IsOperand(pWordPrev->nWordType)){ // 语法错误！
				goto FAILED;
			}
			// add to arrOpnd
			arrOpnd.push_back(pWordCur);

			// read next word
			pWordPrev = pWordCur;
			pWordCur = vWords[cur];	// read word
			cur ++;
		}
		// operator
		else if (_IsOperator(pWordCur->nWordType)){
			// 语法错误！
//			if ( /*' 2 ( '*/ (pWordCur->nWordType == FW_OPTR_LBRACKET && _IsOperand(pWordPrev->dwWordType)) ||	
//				/*' ( + '*/ (pWordCur->nWordType != FW_OPTR_LBRACKET && pWordPrev->nWordType == FW_OPTR_LBRACKET) ||
//				/*' # ) '*/ (pWordCur->nWordType == FW_OPTR_RBRACKET && pWordPrev->nWordType != FW_OPTR_RBRACKET)){
//				goto FAILED;
//			}
			// 判断操作符的优先级....
			if (OpPriorF[pOptrTop->nWordType - FW_OPTR] < OpPriorG[pWordCur->nWordType - FW_OPTR])
			{
				// 将 pWordCur 入栈
				arrOptr.push_back(pWordCur);
				pOptrTop = pWordCur;

				if (pWordCur->nWordType == FW_OPTR_END){// end tag?
					goto FAILED;
				}
				pWordPrev = pWordCur;
				pWordCur = vWords[cur];	// read word
				cur ++;
			}
			else if (OpPriorF[pOptrTop->nWordType - FW_OPTR] > OpPriorG[pWordCur->nWordType - FW_OPTR]){
				// calculate the value:
				int tmp;
				tmp = arrOpnd.size();
				if (tmp < 2)
				{
					// must be: -1 or +2 ....
					if (tmp == 0)
						goto FAILED;
					PFORMULA_WORD	pOpnd1;
					int				nVal1;

					// get the operand
					pOpnd1 = arrOpnd[0];
					if (pOpnd1->nWordType == FW_OPND_VAR_LONG)
					{
						if (!GetVarValue(pOpnd1->Value, var, nVal1))
							goto FAILED;
					}
					else
						nVal1 = pOpnd1->Value;

					switch (pOptrTop->nWordType){
					case FW_OPTR_ADD:
						pOpnd1->Value = nVal1;
						break;
					case FW_OPTR_SUB:
						pOpnd1->Value = -nVal1;
						break;
					default:
						// 算式错误
						goto FAILED;
					}
					pOpnd1->nWordType = FW_OPND_LONG;
					// remove old operand and add new result operand
					arrOpnd[0] = pOpnd1;

					// popup optrTop
					arrOptr.pop_back();
					pOptrTop = arrOptr[arrOptr.size() - 1];
				}
				else
				{
					PFORMULA_WORD	pOpnd1, pOpnd2;
					int				nVal1, nVal2;

					// get the two operand
					pOpnd1 = arrOpnd[tmp - 2];
					pOpnd2 = arrOpnd[tmp - 1];
					if (pOpnd1->nWordType == FW_OPND_VAR_LONG)
					{
						if (!GetVarValue(pOpnd1->Value, var, nVal1))
							goto FAILED;
					}
					else
						nVal1 = pOpnd1->Value;
					if (pOpnd2->nWordType == FW_OPND_VAR_LONG)
					{
						if (!GetVarValue(pOpnd2->Value, var, nVal2))
							goto FAILED;
					}
					else
						nVal2 = pOpnd2->Value;
					
					switch (pOptrTop->nWordType){
					case FW_OPTR_ADD:
						pOpnd1->Value = nVal1 + nVal2;
						break;
					case FW_OPTR_SUB:
						pOpnd1->Value = nVal1 - nVal2;
						break;
					case FW_OPTR_MUL:
						pOpnd1->Value = nVal1 * nVal2;
						break;
					case FW_OPTR_DIV:
						pOpnd1->Value = nVal1 / nVal2;
						break;
					default:
						// 算式错误
						goto FAILED;
					}
					pOpnd1->nWordType = FW_OPND_LONG;
					// remove old operand and add new result operand
					arrOpnd.erase(arrOpnd.begin() + tmp - 1);
					arrOpnd[tmp - 2] = pOpnd1;

					// popup optrTop
					arrOptr.pop_back();
					pOptrTop = arrOptr[arrOptr.size() - 1];
				}
			}
			else{
//				if (! ((pOptrTop->nWordType == FW_OPTR_LBRACKET && pWordCur->nWordType == FW_OPTR_RBRACKET) ||
//					(pOptrTop->nWordType == FW_OPTR_BEGIN && pWordCur->nWordType == FW_OPTR_END)) )
//					goto FAILED;
				switch (pOptrTop->nWordType){
				case FW_OPTR_LBRACKET:
					// popup optrTop
					arrOptr.pop_back();
					pOptrTop = arrOptr[arrOptr.size() - 1];

					// read next word
					if (pWordCur->nWordType == FW_OPTR_END){// end tag?
						goto FAILED;
					}
					pWordPrev = pWordCur;
					pWordCur = vWords[cur];	// read word
					cur ++;
					break;
				case FW_OPTR_BEGIN:
					int tmp;
					tmp = arrOpnd.size();
					if (tmp != 1){
						goto FAILED;
					}

					PFORMULA_WORD	pOpnd;
					pOpnd = arrOpnd[0];

					if (pOpnd->nWordType == FW_OPND_VAR_LONG)
					{
						if (!GetVarValue(pOpnd->Value, var, pOpnd->Value))
							goto FAILED;
					}
					
					nRetValue = pOpnd->Value;
					goto SUCEED;
				default:
					// 算式错误
					goto FAILED;
					break;
				}
			}
		}
		else
			goto FAILED;
	}
FAILED:
	bRet = FALSE;
	goto END;
SUCEED:
	bRet = TRUE;
END:
	// remove old data
	int	nCount;
	nCount = vWords.size();
	for (int i = 0; i < nCount; i ++)
	{
		delete vWords[i];
	}
	vWords.clear();

	arrOpnd.clear();
	arrOptr.clear();

	return bRet;
}

BOOL CFormula::PhraseAnalyse(FormulaArray &vWords)
{
	if (m_strFormula.empty())
		return FALSE;

	int cur;
	PFORMULA_WORD	pWord;
	LPCTSTR			m_szFormula;
	cur = 0;

	m_szFormula = m_strFormula.c_str();

	pWord = new FORMULA_WORD;
	pWord->nWordType = FW_OPTR_BEGIN;
	pWord->Value = 0;
	vWords.push_back(pWord);
	pWord = NULL;

S_BEGIN:
	if (m_szFormula[cur] == 0)
		goto S_END;

	if (IsDigit(m_szFormula[cur]))
	{
		// 作为整数处理
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPND_LONG;
		pWord->Value = _ttoi(&(m_szFormula[cur]));
		vWords.push_back(pWord);
		pWord = NULL;

		while (IsDigit(m_szFormula[cur]))
			cur++;
		goto S_BEGIN;
	}

	// 算符+
	switch (m_szFormula[cur])
	{
	case '+':
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPTR_ADD;
		goto S_OPTR;
	case '-':
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPTR_SUB;
		goto S_OPTR;
	case '*':
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPTR_MUL;
		goto S_OPTR;
	case '/':
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPTR_DIV;
		goto S_OPTR;
	case '(':
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPTR_LBRACKET;
		goto S_OPTR;
	case ')':
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPTR_RBRACKET;
		goto S_OPTR;
	}

	// 变量 variable
	if (isalpha(m_szFormula[cur]))
	{
		pWord = new FORMULA_WORD;
		pWord->nWordType = FW_OPND_VAR_LONG;
		pWord->Value = m_szFormula[cur];
		vWords.push_back(pWord);
		pWord = NULL;
		
		cur ++;
		goto S_BEGIN;
	}

	while (_IsCharBlank(m_szFormula[cur]))
		cur ++;

	goto S_BEGIN;

S_OPTR:
	pWord->Value = 0;
	vWords.push_back(pWord);
	pWord = NULL;
	
	cur ++;
	goto S_BEGIN;

S_END:
	// add end tag
	pWord = new FORMULA_WORD;
	pWord->nWordType = FW_OPTR_END;
	pWord->Value = 0;
	vWords.push_back(pWord);

	return TRUE;
}

BOOL CFormula::GetVarValue(int nVar, FORMULA_VAR vars[], int &nRetValue)
{
	for (int i = 0; vars[i].nVarName != 0; i ++)
	{
		if (nVar == vars[i].nVarName)
		{
			nRetValue = vars[i].Value;
			return TRUE;
		}
	}

	return FALSE;
}

CFormula& CFormula::operator =(CFormula &fRight)
{
	m_strFormula = fRight.m_strFormula;

	return *this;
}

LPCTSTR IgnoreNumb(LPCTSTR szStr)
{
	while (*szStr >= '0' && *szStr <= '9')
		szStr++;

	return szStr;
}

void CFormula::SimpleOptimize()
{
	if (m_strFormula.empty())
		return;

	LPCTSTR		szEnd, szBeg, szPos, szOtherEnd;

	szPos = szEnd = m_strFormula.c_str() + m_strFormula.size() - 1;
	szBeg = m_strFormula.c_str();

	while (szPos >= szBeg && (isdigit(*szPos) || *szPos == ' ' || *szPos == '+' || *szPos == '-' || *szPos == '	'))
		szPos--;

	if (*szPos == '/' || *szPos == '*')
	{
		szPos++;
		szPos = strignore(szPos, _T("\x20\t"));
		if (!isdigit(*szPos))
			return;
		szPos = IgnoreNumb(szPos);
	}
	else
		szPos++;

	int		nResult = 0;

	szOtherEnd = szPos;
	// szPos + , - .....
	if (szPos == szBeg)
	{
		// 换算为+....
		szPos = strignore(szPos, _T("\x20\t"));
		if (IsEmptyString(szPos))
			return;
		if (isdigit(*szPos))
		{
			nResult = _ttoi(szPos);
			szPos = IgnoreNumb(szPos);
		}
	}

	szPos = strignore(szPos, _T("\x20\t"));
	while (*szPos)
	{
		if (*szPos == '+')
		{
			szPos++;
			szPos = strignore(szPos, _T("\x20\t"));
			if (isdigit(*szPos))
				nResult += _ttoi(szPos);
			else
				return;
		}
		else if (*szPos == '-')
		{
			szPos++;
			szPos = strignore(szPos, _T("\x20\t"));
			if (isdigit(*szPos))
				nResult -= _ttoi(szPos);
			else
				return;
		}
		else
			ASSERT(0);

		szPos = IgnoreNumb(szPos);
		szPos = strignore(szPos, _T("\x20\t"));
	}

	if (szOtherEnd < szEnd)
	{
		TCHAR		szResult[128];
		m_strFormula.erase(m_strFormula.begin() + (int)(szOtherEnd - szBeg), m_strFormula.end());
		if (m_strFormula.size() == 0)
		{
			_itot_s(nResult, szResult, CountOf(szResult), 10);
			m_strFormula = szResult;
		}
		else
		{
			if (nResult != 0)
			{
				if (nResult > 0)
					_stprintf_s(szResult, CountOf(szResult), _T("+%d"), nResult);
				else
					_itot_s(nResult, szResult, CountOf(szResult), 10);
				m_strFormula += szResult;
			}
		}
	}
}

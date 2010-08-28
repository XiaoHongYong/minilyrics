// MLHandleMap.h: interface for the CMLHandleMap class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _ML_HANDLE_MAP_INC_
#define _ML_HANDLE_MAP_INC_

template <class handle_t, class object_t>
class CMLHandleMap
{
public:
	typedef	std::map<handle_t, object_t *>	MAP_HC;
	CMLHandleMap()
	{
	}
	~CMLHandleMap()
	{
		MAP_HC::iterator	it;

		m_mVisit.Acquire();

		for (it = m_map.begin(); it != m_map.end(); it++)
		{
			(*it).second->Detach();
			delete (*it).second;
		}

		m_map.clear();

		m_mVisit.Release();
	}
	object_t *fromhandle(handle_t handle)
	{
		MAP_HC::iterator	it;

		m_mVisit.Acquire();
		it = m_map.find(handle);
		if (it == m_map.end())
		{
			object_t	*pObj;
			pObj = new object_t();
			m_map[handle] = pObj;
			pObj->Attach(handle);
			m_mVisit.Release();
			return pObj;
		}
		else
		{
			m_mVisit.Release();
			return (*it).second;
		}
	}
	void remove(handle_t handle)
	{
		MAP_HC::iterator	it;

		m_mVisit.Acquire();
		it = m_map.find(handle);
		if (it != m_map.end())
		{
			(*it).second->Detach();
			delete (*it).second;
			m_map.erase(it);
		}
		m_mVisit.Release();
	}

protected:
	MAP_HC				m_map;
	Mutex				m_mVisit;

};

#endif // _ML_HANDLE_MAP_INC_

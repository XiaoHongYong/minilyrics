#pragma once

template<class _Ty>
class auto_free_list : public list<_Ty *>
{
private:
	// Do NOT define copy construct
	auto_free_list(auto_free_list<_Ty> &copy) { }

public:
	auto_free_list() { }

	typedef list<_Ty *>		Parent;
#ifdef _MAC
    typedef _List_iterator<_Ty>                        iterator;
#endif

	~auto_free_list()
	{
		clear();
	}

	iterator erase(iterator _Where)
	{
		if (_Where != Parent::end())
			delete (*_Where);

		return Parent::erase(_Where);
	}

	iterator erase(iterator _First, iterator _Last)
	{
		for (iterator it = _First; it != _Last; ++it)
			delete (*it);

		return Parent::erase(_First, _Last);
	}

	void clear()
	{
		for (iterator it = Parent::begin(); it != Parent::end(); ++it)
			delete (*it);

		Parent::clear();
	}

};
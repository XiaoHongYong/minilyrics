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

	~auto_free_list()
	{
		clear();
	}

	iterator erase(const_iterator _Where)
	{
		if (_Where != end())
			delete (*_Where);

		return Parent::erase(_Where);
	}

	iterator erase(const_iterator _First, const_iterator _Last)
	{
		for (const_iterator it = _First; it != _Last; ++it)
			delete (*it);

		return Parent::erase(_First, _Last);
	}

	void clear()
	{
		for (const_iterator it = begin(); it != end(); ++it)
			delete (*it);

		Parent::clear();
	}

};
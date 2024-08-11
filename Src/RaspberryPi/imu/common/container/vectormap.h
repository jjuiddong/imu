//------------------------------------------------------------------------
// Name:    VectorMap.h
// Author:  jjuiddong
// Date:    2/28/2013
// 
// Map �� Vector�� ��ģ �ڷᱸ����.
// Fast Access ���� Map�� �̿��ϰ�, ��ü�� Ž���� ���� Vector�� �̿��Ѵ�.
// 
// Type�� ����ũ�� �����Ϳ��� �Ѵ�.
//	- �����ͷ� �����ؾ� ����Ÿ�� �����ȴ�.
//------------------------------------------------------------------------
#pragma once

#include "vectorhelper.h"

namespace common
{

	template <class KeyType, class Type>
	class VectorMap
	{
	public:
		typedef std::map<KeyType, Type> MapType;
		typedef std::vector<Type> VectorType;
		typedef typename MapType::iterator iterator;
		typedef typename MapType::value_type value_type;
		typedef typename VectorType::iterator viterator;

		VectorMap() {}
		VectorMap(int reserved) : m_seq(reserved) {}

		bool insert(const value_type &vt)
		{
			// insert map
			iterator it = m_map.find(vt.first);
			if (m_map.end() != it)
				return false; // Already Exist
			m_map.insert( vt );

			// insert vector
			putvector(m_seq, m_map.size() - 1, vt.second);
			return true;
		}

		// erase and removevector
		bool remove(const KeyType &key)
		{
			iterator it = m_map.find(key);
			if (m_map.end() == it)
				return false; // not found

			removevector(m_seq, it->second);
			m_map.erase(it);
			return true;
		}

		// erase and popvector
		bool remove2(const KeyType &key)
		{
			iterator it = m_map.find(key);
			if (m_map.end() == it)
				return false; // not found

			popvector2(m_seq, it->second);
			m_map.erase(it);
			return true;
		}


		// Type �� ����ũ�� ���� ����� �� �ִ� �Լ���.
		bool removebytype(const Type &ty)
		{
			iterator it = m_map.begin();
			while (m_map.end() != it)
			{
				if (ty == it->second)
				{
					removevector(m_seq, it->second);
					m_map.erase(it);
					return true;
				}
			}
			return false; // not found
		}

		bool empty() const
		{
			return m_map.empty();
		}

		void clear()
		{
			m_map.clear();
			m_seq.clear();
		}

		size_t size() const
		{
			return m_map.size();
		}

		void reserve(unsigned int size)
		{
			m_seq.reserve(size);
		}

		iterator find(const KeyType &key) { return m_map.find(key); }
		iterator begin() { return m_map.begin(); }
		iterator end() { return m_map.end(); }

	public:
		MapType	m_map;
		VectorType m_seq;
	};

}

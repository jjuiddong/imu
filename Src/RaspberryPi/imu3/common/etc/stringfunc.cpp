
#include "../../stdafx.h"
#include "stringfunc.h"


void common::replaceAll(std::string& str, const std::string& from, const std::string& to) {
	if(from.empty())
		return;
	size_t start_pos = 0;
	while((start_pos = str.find(from, start_pos)) != std::string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

// "  skdfjskdjf  "
// "$$$skdfjskdjf$$$"
// "skdfjskdjf"
string& common::trim(string &str)
{
	// �տ������� �˻�
	for (int i=0; i < (int)str.length(); ++i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			common::removevector2(str, i);
			--i;
		}
		else
			break;
	}

	// �ڿ������� �˻�
	for (int i=(int)str.length()-1; i >= 0; --i)
	{
		if ((str[i] == '\n') || (str[i] == '\t') || (str[i] == '\r') || (str[i] == ' '))
		{
			common::removevector2(str, i);
		}
		else
			break;
	}

	return str;
}
string common::trim2(const string &str)
{
	string val = str;
	trim(val);
	return val;
}


// "  skdfjskdjf  "
// "$$$skdfjskdjf$$$"
// "skdfjskdjf"
void common::trimw(wstring &str)
{
	//str = str2wstr(trim(wstr2str(str)));
}
wstring common::trimw2(const wstring &str)
{
	wstring val = str;
	trimw(val);
	return val;
}


//------------------------------------------------------------------------
// �����ڵ带 ��Ƽ����Ʈ ���ڷ� ��ȯ
//------------------------------------------------------------------------
std::string common::wstr2str(const std::wstring &wstr)
{
	// const int slength = (int)wstr.length() + 1;
	// const int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, 0, 0, NULL, FALSE);
	// char* buf = new char[len];
	// ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), slength, buf, len, NULL, FALSE);
	// std::string r(buf);
	// delete[] buf;
	// return r;
	return "";
}


//------------------------------------------------------------------------
// ��Ƽ����Ʈ ���ڸ� �����ڵ�� ��ȯ
//------------------------------------------------------------------------
std::wstring common::str2wstr(const std::string &str)
{
	// int len;
	// int slength = (int)str.length() + 1;
	// len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, 0, 0);
	// wchar_t* buf = new wchar_t[len];
	// ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), slength, buf, len);
	// std::wstring r(buf);
	// delete[] buf;
	// return r;
	return L"";
}


string common::wstr2utf8(const wstring &wstr)
{
	// const int slength = (int)wstr.length() + 1;
	// const int len = ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), slength, 0, 0, NULL, FALSE);
	// char* buf = new char[len];
	// ::WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), slength, buf, len, NULL, FALSE);
	// std::string r(buf);
	// delete[] buf;
	// return r;
	return "";
}


//------------------------------------------------------------------------
// _variant_t Ÿ���� ��Ʈ������ ��ȯ��Ų��. ����Ÿ ��¿��� ���������.
// isStringDoubleQuoto: ��Ʈ�� Ÿ���� ���, ���ʿ� �ֵ���ǥ�� ���´�.
//------------------------------------------------------------------------
// std::string common::variant2str(const _variant_t &var
// 	, const bool isStringDoubleQuoto //= false
// )
// {
// 	std::stringstream ss;
// 	switch (var.vt)
// 	{
// 	// modify: 2020-11-17
// 	// VT_BOOL type variant was diffrent we think
// 	// variant_t value has true: -1, false: 0
// 	// so direct access variant_t::boolVal was error occurr! maybe
// 	//case VT_BOOL: ss << var.boolVal; break;
// 	case VT_BOOL: ss << ((bool)var)? "1" : "0"; break;
// 	case VT_I1: ss << (char)var; break;
// 	case VT_UI1: ss << (unsigned int)(unsigned char)var; break;
// 	case VT_I2: ss << var.iVal; break;
// 	case VT_UI2: ss << (unsigned short)var; break;
// 	case VT_I4: ss << var.lVal; break;
// 	case VT_UI4: ss << (unsigned int)var; break;
// 	case VT_I8: ss << (__int64)var; break;
// 	case VT_UI8: ss << (unsigned __int64)var; break;
// 	case VT_R4: ss << var.fltVal; break;
// 	case VT_R8: ss << var.dblVal; break;

// 	case VT_BSTR:
// 	{
// #ifdef _UNICODE
// 		wstring str = (LPCTSTR)(_bstr_t)var.bstrVal;
// 		if (isStringDoubleQuoto)
// 		{
// 			ss << "\"" << common::wstr2str(str) << "\"";
// 		}
// 		else
// 		{
// 			ss << common::wstr2str(str);
// 		}
// #else
// 		//string str = (LPCTSTR)(_bstr_t)var.bstrVal;
// 		const bstr_t str = var;
// 		if (isStringDoubleQuoto)
// 		{
// 			ss << "\"" << str << "\"";
// 		}
// 		else
// 		{
// 			ss << str;
// 		}
// #endif
// 	}
// 	break;

// 	case VT_DECIMAL:
// 		break;

// 	case VT_INT: ss << var.intVal; break;
// 	case VT_UINT: ss << var.uintVal; break;
// 	default:
// 		break;
// 	}

// 	return ss.str();
// }


//------------------------------------------------------------------------
// string�� varType ���·� ��ȯ�ؼ� �����Ѵ�.
// VT_BSTR Ÿ���� ��� ::SysAllocString()�� �����ϰ�,
// ::SysFreeString() ���� �����ؾ� �Ѵ�.
// https://manylee.tistory.com/entry/VariantCopy-VariantInit-VariantClear
// https://panpro.tistory.com/75
//
// variant_t ���� �ڵ����� �޸𸮸� �����Ѵ�.
// https://stackoverflow.com/questions/39155447/how-to-clean-up-variant-t
//
//------------------------------------------------------------------------
// _variant_t common::str2variant(const VARTYPE &vt, const std::string &value)
// {
// 	_variant_t var;
// 	var.vt = vt;
// 	switch (vt)
// 	{
// 	// modify: 2020-11-17
// 	// VT_BOOL type variant was diffrent we think
// 	// variant_t value has true: -1, false: 0
// 	// so direct access variant_t::boolVal was maybe error occurred!
// 	//case VT_BOOL: var.boolVal = (bool)atoi(value.c_str()); break;
// 	case VT_BOOL: var = (bool)atoi(value.c_str()); break;
// 	case VT_I2: var.iVal = (short)atoi(value.c_str()); break;
// 	case VT_I4: var.lVal = (long)atoi(value.c_str()); break;
// 	case VT_R4: var.fltVal = (float)atof(value.c_str()); break;
// 	case VT_R8: var.dblVal = atof(value.c_str()); break;

// 	case VT_BSTR:
// 	{
// #ifdef _UNICODE
// 		//var.bstrVal = (_bstr_t)common::str2wstr(value).c_str();
// 		var.bstrVal = ::SysAllocString(str2wstr(value).c_str());
// #else
// 		var.bstrVal = ::SysAllocString(str2wstr(value).c_str());
// #endif
// 	}
// 	break;

// 	case VT_DECIMAL:
// 	case VT_I1:
// 	case VT_UI1:
// 	case VT_UI2:
// 	case VT_UI4:
// 		break;

// 	case VT_INT: var.intVal = (int)atoi(value.c_str()); break;
// 	case VT_UINT: var.uintVal = strtoul(value.c_str(), NULL, 0); break;
// 	default:
// 		return false;
// 	}
// 	return var;
// }


// Deep Copy variant
// VT_BSTR ��Ʈ�� �޸� ����
// _variant_t common::copyvariant(const _variant_t &var)
// {
// 	//_variant_t v;
// 	//if (VT_BSTR == var.vt)
// 	//{
// 	//	v.vt = var.vt;
// 	//	v.bstrVal = ::SysAllocString(var.bstrVal);
// 	//}
// 	//else
// 	//{
// 	//	v = var;
// 	//}
// 	_variant_t v;
// 	::VariantCopy(&v, &var);
// 	return v;
// }


// VT_BSTR Ÿ���� ��� ::SysFreeString() ���� �����ؾ� �Ѵ�.
// https://manylee.tistory.com/entry/VariantCopy-VariantInit-VariantClear
// https://panpro.tistory.com/75
//
// modify
//	variant_t �Ҹ��ڿ��� �ڵ����� �޸𸮸� �����Ѵ�.
// https://stackoverflow.com/questions/39155447/how-to-clean-up-variant-t
// ���� �� �Լ��� ȣ���� �ʿ� ����.
// void common::clearvariant(INOUT _variant_t &var)
// {
// 	::VariantClear(&var);
// 	//if (VT_BSTR == var.vt)
// 	//{
// 	//	::SysFreeString(var.bstrVal);
// 	//	var.vt = VT_EMPTY;
// 	//}
// 	//else
// 	//{
// 	//	::VariantInit(&var);
// 	//}
// }


//------------------------------------------------------------------------
// ��Ʈ������
//------------------------------------------------------------------------
std::string common::format(const char* fmt, ...)
{
	char textString[ 256] = {'\0'};
	va_list args;
	va_start ( args, fmt );
	vsnprintf_s( textString, sizeof(textString), _TRUNCATE, fmt, args );
	va_end ( args );
	return textString;
}



//------------------------------------------------------------------------
// ��Ʈ������ wstring ��
//------------------------------------------------------------------------
std::wstring common::formatw(const char* fmt, ...)
{
	char textString[ 256] = {'\0'};
	va_list args;
	va_start ( args, fmt );
	vsnprintf_s( textString, sizeof(textString), _TRUNCATE, fmt, args );
	va_end ( args );
	return str2wstr(textString);
}


// �Ѿ�� ���� str �� �ҹ��ڷ� �ٲ㼭 �����Ѵ�.
string& common::lowerCase(string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}
string common::lowerCase2(const string &str)
{
	string val = str;
	std::transform(val.begin(), val.end(), val.begin(), tolower);
	return val;
}

// �Ѿ�� ���� str �� �빮�ڷ� �ٲ㼭 �����Ѵ�.
string& common::upperCase(string &str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	return str;
}
string common::upperCase2(const string &str)
{
	string val = str;
	std::transform(val.begin(), val.end(), val.begin(), toupper);
	return val;
}


wstring& common::lowerCasew(wstring &str)
{
	std::transform(str.begin(), str.end(), str.begin(), tolower);
	return str;
}

wstring& common::upperCasew(wstring &str)
{
	std::transform(str.begin(), str.end(), str.begin(), toupper);
	return str;
}


// ���� ���ڿ� str���� ������ delimeter�� �����ؼ� out �� �����ؼ� �����Ѵ�.
// delimeter�� ������� �ʴ´�.
void common::tokenizer(const string &str, const string &delimeter
	, const string &ignoreStr, OUT vector<string> &out)
{
	string tmp = str;
	int offset = 0;
	int first = 0;

	while (!tmp.empty())
	{
		const int pos = (int)tmp.find(delimeter, offset);
		if (string::npos == pos)
		{
			string tok = common::trim2(tmp.substr(first));
			if (!tok.empty())
				out.push_back(tok);
			break;
		}
		else
		{
			const string tok = tmp.substr(offset, pos-offset);
			offset += (int)tok.length() + (int)delimeter.length();
			if (tok != ignoreStr)
			{
				out.push_back(tmp.substr(first, pos-first));
				first = offset;
			}
		}
	}
}


// ���� ���ڿ� str���� ������ delimeter �� �����ؼ� out �� �����ؼ� �����Ѵ�.
// delimeter �� ������� �ʴ´�.
void common::wtokenizer(const wstring &str, const wstring &delimeter, const wstring &ignoreStr, OUT vector<wstring> &out)
{
	wstring tmp = str;
	int offset = 0;
	int first = 0;

	while (!tmp.empty())
	{
		const int pos = (int)tmp.find(delimeter, offset);
		if (wstring::npos == pos)
		{
			wstring tok = common::trimw2(tmp.substr(first));
			if (!tok.empty())
				out.push_back(tok);
			//out.push_back(tmp.substr(first));
			break;
		}
		else
		{
			const wstring tok = tmp.substr(offset, pos - offset);
			offset += (int)tok.length() + (int)delimeter.length();
			if (tok != ignoreStr)
			{
				out.push_back(tmp.substr(first, pos - first));
				first = offset;
			}
		}
	}
}


// ���� ���ڿ� str���� ������ delimeters �� �����ؼ� out �� �����ؼ� �����Ѵ�.
// �� �� ���� ������ delimeters �� �� ���ڸ� �������� �Ѵ�.
// delimeters �� ������� �ʴ´�.
//
// ex) tokenizer2( "aaa; bbbb cccc ddd; eee", ", ", out)
//		out-> aaa bbb ccc ddd eee
void common::tokenizer2(const string &str, const string &delimeters, OUT vector<string> &out)
{
	string tmp = str;
	int offset = 0;

	while (tmp[offset])
	{
		string tok;
		tok.reserve(32);
		while (tmp[offset])
		{
			if (strchr(delimeters.c_str(), tmp[offset]))
			{
				offset++;
				break;
			}
			tok += tmp[offset++];
		}

		common::trim(tok);
		if (!tok.empty())
			out.push_back(tok);
	}
}


// delimeter�� ���е� ��Ʈ���� �Ľ��ؼ� int������ ��ȯ�� �� �����Ѵ�.
// �����޸� ����, �޸� ���縦 �ּ�ȭ�ϱ� ���� ���� �������.
void common::tokenizer3(const char *data, const int size, const char delimeter, OUT vector<int> &out)
{
	char buff[32];
	const char *src = data;
	char *dst = buff;
	int remain = size;
	while (remain > 0)
	{
		if ((*src == '\0') 
			|| (*src == delimeter)
			|| ((int)(dst - buff) > (ARRAYSIZE(buff)-1))
			)
		{
			*dst++ = NULL;
			out.push_back(atoi(buff));
			dst = buff;
		}
		else
		{
			*dst++ = *src;
		}
		++src;
		--remain;
	}

	// ������ ��ū ����, ������ ������ �ִٸ� ~
	if ((int)(dst - buff) > 0)
	{
		// null �߰�
		if ((int)(dst - buff) < ARRAYSIZE(buff))
		{
			*dst++ = NULL;
			out.push_back(atoi(buff));
		}
	}
}


// string parse with delimeter (space, newline, tab, double quote, comma)
void common::tokenizer_space(const char *str, OUT vector<string> &out)
{
	int state = 0;
	string tok;
	const char *c = str;
	while (*c)
	{
		if (0 == state)
		{
			switch (*c)
			{
			case '\n':
			case ' ':
			case '\t':
			case '\r':
			case ',':
				if (!tok.empty())
				{
					out.push_back(tok);
					tok.clear();
				}
				break;

			case '\"':
				state = 1;
				break;

			default:
				tok += *c;
				break;
			}
		}
		else if (1 == state)
		{
			if (*c == '\"')
			{
				out.push_back(tok);
				tok.clear();
				state = 0;
			}
			else
			{
				tok += *c;
			}
		}
		else
		{
			assert(0);
		}

		++c;
	}

	if (!tok.empty())
		out.push_back(tok);
}

void common::tokenizer_space(const string &str, OUT vector<string> &out)
{
	tokenizer_space(str.c_str(), out);
}


// int�� �迭�� delimeter�� �����ڷ��ؼ� ��Ʈ������ ��ȯ�Ѵ�.
// ���� �޸𸮻����� �ּ�ȭ�� �Լ�.
// datas -> string 
int common::strcomposite(char *buff, const int maxSize, const char delimeter, const vector<int> &datas)
{
	char temp[32];
	char *dst = buff;
	int remain = maxSize;
	for (int i=0; i < (int)datas.size(); ++i)
	{
		auto &data = datas[i];
		sprintf(temp, "%d", data);
		const int len = strlen(temp);
		if (len < remain)
		{
			strncpy(dst, temp, len);
			dst += len;
			remain -= len;
		}

		if ((remain > 1) && (i != (int)datas.size()-1))
		{
			*dst++ = delimeter;
			--remain;
		}
	}

	if (remain >= 1)
	{
		*dst++ = NULL;
	}
	return maxSize - remain;
}


// explicit all matching character
// %f, %d
// Last Variable Argument Must NULL
//
// etc)
//  scanner("test string 1-1", "test string %d-%d", &v1, &v2, NULL);
//
int common::scanner(const wchar_t *buf, const wchar_t *fmt, ...)
{
	va_list listPointer;
	va_start(listPointer, fmt);

	const wchar_t *pfL = fmt;
	const wchar_t *pfR = fmt;
	const wchar_t *pb = buf;
	int scanCount = 0;

	while (*pfR && *pb)
	{
		if (*pfR == '%')
		{
			const int cmpLen = pfR - pfL;
			if (cmpLen > 0)
				if (wcsncmp(pb, pfL, cmpLen))
					break; // not matching fail Scan

			pb += cmpLen;
			const wchar_t f = *++pfR; // next
			pfL = ++pfR; // next formatter

			switch (f)
			{
			case 'd': // integer
			{
				wchar_t tmp[32];
				int idx = 0;
				ZeroMemory(tmp, sizeof(tmp));

				while (*pb && (31 > idx))
				{
					if (!iswdigit(*pb))
						break;
					tmp[idx++] = *pb++;
				}

				void *arg = va_arg(listPointer, void*);
				if (!arg)
					break; // no argument

				*(int*)arg = wcstol(tmp, 0, 10);
				++scanCount;
			}
			break;

			case 'f': // floating
			{
				wchar_t tmp[32];
				int idx = 0;
				ZeroMemory(tmp, sizeof(tmp));

				while (*pb && (31 > idx))
				{
					if ((*pb == '.') && (!iswdigit(*pb)))
						break;
					tmp[idx++] = *pb++;
				}

				void *arg = va_arg(listPointer, void*);
				if (!arg)
					break; // no argument

				*(float*)arg = (float)wcstof(tmp, 0);
				++scanCount;
			}
			break;

			default: assert(0); break;
			}
		}
		else
		{
			++pfR;
		}
	}

	va_end(listPointer);

	if (!*pfR && *pfL)
	{
		if (!wcscmp(pfL, pb))
			return scanCount; // success
	}

	if (!*pfR && !*pb)
		return scanCount; // success

	return -1; // fail
}

#ifndef __TEMPLATERESOURCE_DATATYPE_H__
#define __TEMPLATERESOURCE_DATATYPE_H__

#include <stdint.h>
#include <string>

struct ResData1{
	uint32_t dwId;
	int nReserved;
};

struct ResData2{
	int nId;
	int nReserved;
};


template<typename ResT>
class ResTraits{
public:
	typedef int KeyType;
	static const std::string Name(){
		return "invalid";
	}
};

#define DEF_RES_TRAITS(ResT,KeyT) \
template<>\
class ResTraits<ResT>{\
public:\
	typedef KeyT KeyType;\
	static const std::string Name(){\
		return  "./"#ResT".xml";\
	}\
};


template<typename T>
typename ResTraits<T>::KeyType getKey(const T&){
	return ResTraits<T>::KeyType();
}

#define DEF_GET_KEY(ResType,KeyFiled) \
template<>\
ResTraits<ResType>::KeyType getKey<ResType>(const ResType& res){\
	return static_cast<ResTraits<ResType>::KeyType>(res.KeyFiled);\
}

#define DEF_BOTH_TRAITS_KEY(ResType,KeyType,KeyFiled)\
DEF_RES_TRAITS(ResType, KeyType)\
DEF_GET_KEY(ResType, KeyFiled)

DEF_BOTH_TRAITS_KEY(ResData1, uint32_t, dwId)
DEF_BOTH_TRAITS_KEY(ResData2, int, nId)

#endif

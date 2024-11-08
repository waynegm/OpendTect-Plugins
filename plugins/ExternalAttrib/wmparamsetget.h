#pragma once

#ifndef mSetIntInterval

#define mSetIntInterval( str, newval ) \
{ \
	mDynamicCastGet(Attrib::IntGateParam*,param,desc.getParam(str)) \
	const Interval<int> oldval( param->getIntValue(0), param->getIntValue(1) ); \
	if ( chtr_.set(oldval,newval) ) param->setValue( newval ); \
}
#endif

#ifndef mIfGetIntInterval

#define mIfGetIntInterval( str, var, setfunc ) \
	Attrib::ValParam* valparam##var =\
	const_cast<Attrib::ValParam*>(desc.getValParam(str));\
	if ( valparam##var ) \
	{ \
	Interval<int> var; \
	var.start_ = valparam##var->getIntValue(0); \
	var.stop_ = valparam##var->getIntValue(1); \
	if ( mIsUdf(var.start_) || mIsUdf(var.stop_) )\
	{\
	mDynamicCastGet(Attrib::IntGateParam*,gateparam##var,valparam##var);\
	if ( gateparam##var ) \
		var = gateparam##var->getDefaultGateValue();\
		}\
		setfunc; \
}
#endif

#ifndef mGetIntIntervalFromDesc

#define mGetIntIntervalFromDesc( __desc, var, varstring ) \
{\
var.start_ = __desc->getValParam(varstring)->getIntValue(0); \
var.stop_ = __desc->getValParam(varstring)->getIntValue(1); \
if ( mIsUdf(var.start_) || mIsUdf(var.stop_) )\
{\
Attrib::ValParam* valparam##var = \
const_cast<Attrib::ValParam*>(__desc->getValParam(varstring));\
mDynamicCastGet(Attrib::IntGateParam*,gateparam##var,valparam##var);\
if ( gateparam##var ) \
	var = gateparam##var->getDefaultGateValue();\
	}\
}

#define mGetIntInterval( var, varstring ) \
mGetIntIntervalFromDesc( desc_, var, varstring )

#endif

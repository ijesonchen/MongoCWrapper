#include "mfcafx.h"

// no necessary including AccessBase.h

#ifdef __AFXSTR_H__
#include "StudentAccess.h"
#endif // __AFXSTR_H__

#include "mfcnew.h"


namespace MongoClib
{

bool CreateAllIndces(void)
{
	bool bRet = true;

#ifdef __AFXSTR_H__
	bRet &= StudentAccess().CreateIndexs();
#endif // __AFXSTR_H__

	return bRet;
}

} // MongoClib

//=============================================================================
/**
 *  @file    Based_Pointer_Test_Lib.cpp
 *
 *  This test confirms the function of the Based_Pointer_Repository
 *
 *  @author Steve Williams  <steve@telxio>
 */
//=============================================================================


#include "ace/ACE.h"
#include "ace/svc_export.h"
#include "ace/Based_Pointer_Repository.h"

#if defined (ACE_OPENVMS)
// with OPENVMS symbol names > 31 cause us trouble with dlsym()
extern "C" ACE_Svc_Export void *
get_based_pointer_repo_inst ()
#else
extern "C" ACE_Svc_Export void *
get_based_pointer_repository_instance ()
#endif
{
    void* baddr = ACE_BASED_POINTER_REPOSITORY::instance();
    return baddr;
}


/* -*- c++ -*- */
#ifndef JAWS_PIPELINE_HANDLER_T_H
#define JAWS_PIPELINE_HANDLER_T_H

#include "JAWS/Export.h"
#include "JAWS/Pipeline.h"

template <class TYPE>
class JAWS_Pipeline_Abstract_Handler : public JAWS_Pipeline_Task
  // = TITLE
  //   Methods that are common to pipeline components
{
public:
  JAWS_Pipeline_Abstract_Handler ();
  virtual ~JAWS_Pipeline_Abstract_Handler ();
  // ACE_Task hooks

  virtual int put (ACE_Message_Block *mb, ACE_Time_Value *tv = 0);
  // inherited from ACE_Task

  virtual int handle_put (TYPE *data, ACE_Time_Value *tv) = 0;
  // Callback hook for specialized data processing
};

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
#include "JAWS/Pipeline_Handler_T.cpp"
#endif

#endif /* !defined (JAWS_PIPELINE_HANDLER_T_H) */

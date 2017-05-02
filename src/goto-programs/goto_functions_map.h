/*******************************************************************\

Author: Nathan.Phillips@diffblue.com

\*******************************************************************/

/// \file
/// virtual_mapt wrappers around goto functions.

#ifndef CPROVER_GOTO_PROGRAMS_GOTO_FUNCTIONS_MAP_H
#define CPROVER_GOTO_PROGRAMS_GOTO_FUNCTIONS_MAP_H

#include "goto_function_template.h"
#include <util/map_wrapper.h>


template<typename bodyt>
// NOLINTNEXTLINE(readability/namespace)  - can't template typedefs
using goto_functions_map_baset=
  virtual_mapt<
    irep_idt,
    goto_function_templatet<bodyt>,
    goto_function_templatet<bodyt> &>;

template<typename bodyt>
class goto_functions_mapt
  : public map_wrappert<irep_idt, goto_function_templatet<bodyt>>
{
private:
  typename goto_functions_mapt::underlying_mapt map;

public:
  goto_functions_mapt()
    : map_wrappert<irep_idt, goto_function_templatet<bodyt>>(map)
  {
  }
};


#endif

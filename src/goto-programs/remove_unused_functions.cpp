/*******************************************************************\

Module: Unused function removal

Author: CM Wintersteiger

\*******************************************************************/

/// \file
/// Unused function removal

#include "remove_unused_functions.h"

#include <util/message.h>

void remove_unused_functions(
  goto_functionst &functions,
  message_handlert &message_handler)
{
  std::set<irep_idt> used_functions;
  std::list<irep_idt> unused_functions;
  find_used_functions(
    goto_functionst::entry_point(), functions, used_functions);

  for(irep_idt fn_name : functions.function_map.keys())
  {
    if(used_functions.count(fn_name)==0)
      unused_functions.push_back(fn_name);
  }

  messaget message(message_handler);

  if(!unused_functions.empty())
  {
    message.statistics()
      << "Dropping " << unused_functions.size() << " of " <<
      functions.function_map.size() << " functions (" <<
      used_functions.size() << " used)" << messaget::eom;
  }

  for(const auto &f : unused_functions)
    functions.function_map.erase(f);
}

void find_used_functions(
  const irep_idt &start,
  goto_functionst &functions,
  std::set<irep_idt> &seen)
{
  std::pair<std::set<irep_idt>::const_iterator, bool> res =
    seen.insert(start);

  if(!res.second)
    return;
  else
  {
    goto_functionst::function_mapt::const_iterator f_it =
      functions.function_map.find(start);

    if(f_it!=functions.function_map.end())
    {
      forall_goto_program_instructions(it, f_it->second.body)
      {
        if(it->type==FUNCTION_CALL)
        {
          const code_function_callt &call =
            to_code_function_call(to_code(it->code));

          // check that this is actually a simple call
          assert(call.function().id()==ID_symbol);

          find_used_functions(call.function().get(ID_identifier),
                              functions,
                              seen);
        }
      }
    }
  }
}

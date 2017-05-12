/*******************************************************************\

Module: Symbol Table + CFG

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

/// \file
/// Symbol Table + CFG

#ifndef CPROVER_GOTO_PROGRAMS_GOTO_MODEL_H
#define CPROVER_GOTO_PROGRAMS_GOTO_MODEL_H

#include <util/symbol_table.h>
#include <util/language_file.h>

#include "goto_functions.h"

// A model is a pair consisting of a symbol table
// and the CFGs for the functions.

class goto_modelt
{
public:
  language_filest language_files;
  symbol_tablet symbol_table;
  goto_functionst &goto_functions;

  void clear()
  {
    symbol_table.clear();
    goto_functions.clear();
  }

  void output(std::ostream &out)
  {
    namespacet ns(symbol_table);
    goto_functions.output(ns, out);
  }

  explicit goto_modelt(goto_functionst &goto_functions):
    goto_functions(goto_functions)
  {
  }

  // Can't share goto_functionst with another model
  goto_modelt(const goto_modelt &)=delete;
  goto_modelt &operator=(const goto_modelt &)=delete;

  // Move operations need to be explicitly enabled as they are deleted with the
  // copy operations
  // default for move operations isn't available on Windows yet, so define
  //  explicitly (see https://msdn.microsoft.com/en-us/library/hh567368.aspx
  //  under "Defaulted and Deleted Functions")

  goto_modelt(goto_modelt &&other):
    symbol_table(std::move(other.symbol_table)),
    goto_functions(other.goto_functions)
  {
  }

  goto_modelt &operator=(goto_modelt &&other)
  {
    symbol_table=std::move(other.symbol_table);
    goto_functions=std::move(other.goto_functions);
    return *this;
  }
};

class eager_goto_modelt:public goto_modelt
{
private:
  eager_goto_functionst concrete_goto_functions;

public:
  eager_goto_modelt():goto_modelt(concrete_goto_functions)
  {
  }
};

#include "goto_functions_map_lazy.h"

class lazy_goto_modelt:public goto_modelt
{
private:
  goto_functions_map_lazyt<goto_programt> function_map;
  goto_functionst goto_functions;

public:
  lazy_goto_modelt(
      const std::function<void(
          goto_modelt &goto_model,
          const irep_idt &function_name,
          goto_functionst::goto_functiont &function)>
        &post_process_function,
      const std::function<void(
          goto_modelt &goto_model,
          goto_functionst &goto_functions)>
        &post_process_functions,
      message_handlert &message_handler)
    : goto_modelt(goto_functions),
      function_map(
        *this,
        [this, post_process_function] (
          const irep_idt &function_name,
          goto_functionst::goto_functiont &function)
        { return post_process_function(*this, function_name, function); },
        [this, post_process_functions] (goto_functionst &goto_functions)
        { return post_process_functions(*this, goto_functions); },
        message_handler),
      goto_functions(function_map)
  {
  }
};

#endif // CPROVER_GOTO_PROGRAMS_GOTO_MODEL_H

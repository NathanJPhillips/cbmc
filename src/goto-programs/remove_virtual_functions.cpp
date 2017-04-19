/*******************************************************************\

Module: Remove Virtual Function (Method) Calls

Author: Daniel Kroening, kroening@kroening.com

\*******************************************************************/

#include <util/prefix.h>
#include <util/type_eq.h>

#include "class_hierarchy.h"
#include "class_identifier.h"
#include "remove_virtual_functions.h"

/*******************************************************************\

   Class: remove_virtual_functionst

 Purpose:

\*******************************************************************/

class remove_virtual_functionst
{
public:
  remove_virtual_functionst(
    const symbol_tablet &_symbol_table,
    const goto_functionst &goto_functions);

  void operator()(goto_functionst &goto_functions);

  bool remove_virtual_functions(goto_programt &goto_program);

protected:
  const namespacet ns;
  const symbol_tablet &symbol_table;

  class_hierarchyt class_hierarchy;

  void remove_virtual_function(
    goto_programt &goto_program,
    goto_programt::targett target);
};

/*******************************************************************\

Function: remove_virtual_functionst::remove_virtual_functionst

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

remove_virtual_functionst::remove_virtual_functionst(
  const symbol_tablet &_symbol_table,
  const goto_functionst &goto_functions):
  ns(_symbol_table),
  symbol_table(_symbol_table)
{
  class_hierarchy(symbol_table);
}

/*******************************************************************\

Function: remove_virtual_functionst::remove_virtual_function

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void remove_virtual_functionst::remove_virtual_function(
  goto_programt &goto_program,
  goto_programt::targett target)
{
  const code_function_callt &code=
    to_code_function_call(target->code);

  const auto &vcall_source_loc=target->source_location;

  const exprt &function=code.function();
  assert(function.id()==ID_virtual_function);
  assert(!code.arguments().empty());

  class_hierarchyt::functionst functions;
  class_hierarchy.get_virtual_callsite_targets(function, ns, functions);

  if(functions.empty())
  {
    target->make_skip();
    return; // give up
  }

  // only one option?
  if(functions.size()==1)
  {
    assert(target->is_function_call());
    if(functions.begin()->symbol_expr==symbol_exprt())
      target->make_skip();
    else
      to_code_function_call(target->code).function()=
        functions.begin()->symbol_expr;
    return;
  }

  // the final target is a skip
  goto_programt final_skip;

  goto_programt::targett t_final=final_skip.add_instruction();
  t_final->source_location=vcall_source_loc;

  t_final->make_skip();

  // build the calls and gotos

  goto_programt new_code_calls;
  goto_programt new_code_gotos;

  exprt this_expr=code.arguments()[0];
  // If necessary, cast to the last candidate function to
  // get the object's clsid. By the structure of get_virtual_callsite_targets,
  // this is the parent of all other classes under consideration.
  const auto &base_classid=functions.back().class_id;
  const auto &base_function_symbol=functions.back().symbol_expr;
  symbol_typet suggested_type(base_classid);
  exprt c_id2=get_class_identifier_field(this_expr, suggested_type, ns);

  std::map<irep_idt, goto_programt::targett> calls;
  // Note backwards iteration, to get the least-derived candidate first.
  for(auto it=functions.crbegin(), itend=functions.crend(); it!=itend; ++it)
  {
    const auto &fun=*it;
    auto insertit=calls.insert(
      {fun.symbol_expr.get_identifier(), goto_programt::targett()});

    // Only create one call sequence per possible target:
    if(insertit.second)
    {
      goto_programt::targett t1=new_code_calls.add_instruction();
      t1->source_location=vcall_source_loc;
      if(!fun.symbol_expr.get_identifier().empty())
      {
      // call function
        t1->make_function_call(code);
        auto &newcall=to_code_function_call(t1->code);
        newcall.function()=fun.symbol_expr;
        pointer_typet need_type(symbol_typet(fun.symbol_expr.get(ID_C_class)));
        if(!type_eq(newcall.arguments()[0].type(), need_type, ns))
          newcall.arguments()[0].make_typecast(need_type);
      }
      else
      {
        // No definition for this type; shouldn't be possible...
        t1->make_assertion(false_exprt());
      }
      insertit.first->second=t1;
      // goto final
      goto_programt::targett t3=new_code_calls.add_instruction();
      t3->source_location=vcall_source_loc;
      t3->make_goto(t_final, true_exprt());
    }

    // If this calls the base function we just fall through.
    // Otherwise branch to the right call:
    if(fun.symbol_expr!=base_function_symbol)
    {
      exprt c_id1=constant_exprt(fun.class_id, string_typet());
      goto_programt::targett t4=new_code_gotos.add_instruction();
      t4->source_location=vcall_source_loc;
      t4->make_goto(insertit.first->second, equal_exprt(c_id1, c_id2));
    }
  }

  goto_programt new_code;

  // patch them all together
  new_code.destructive_append(new_code_gotos);
  new_code.destructive_append(new_code_calls);
  new_code.destructive_append(final_skip);

  // set locations
  Forall_goto_program_instructions(it, new_code)
  {
    const irep_idt property_class=it->source_location.get_property_class();
    const irep_idt comment=it->source_location.get_comment();
    it->source_location=target->source_location;
    it->function=target->function;
    if(!property_class.empty())
      it->source_location.set_property_class(property_class);
    if(!comment.empty())
      it->source_location.set_comment(comment);
  }

  goto_programt::targett next_target=target;
  next_target++;

  goto_program.destructive_insert(next_target, new_code);

  // finally, kill original invocation
  target->make_skip();
}

/*******************************************************************\

Function: remove_virtual_functionst::remove_virtual_functions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

bool remove_virtual_functionst::remove_virtual_functions(
  goto_programt &goto_program)
{
  bool did_something=false;

  Forall_goto_program_instructions(target, goto_program)
    if(target->is_function_call())
    {
      const code_function_callt &code=
        to_code_function_call(target->code);

      if(code.function().id()==ID_virtual_function)
      {
        remove_virtual_function(goto_program, target);
        did_something=true;
      }
    }

  if(did_something)
  {
    goto_program.update();
  }

  return did_something;
}

/*******************************************************************\

Function: remove_virtual_functionst::operator()

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void remove_virtual_functionst::operator()(goto_functionst &functions)
{
  bool did_something=false;

  for(goto_functionst::function_mapt::iterator f_it=
      functions.function_map.begin();
      f_it!=functions.function_map.end();
      f_it++)
  {
    goto_programt &goto_program=f_it->second.body;

    if(remove_virtual_functions(goto_program))
      did_something=true;
  }

  if(did_something)
    functions.compute_location_numbers();
}

/*******************************************************************\

Function: remove_virtual_functions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void remove_virtual_functions(
  const symbol_tablet &symbol_table,
  goto_functionst &goto_functions)
{
  remove_virtual_functionst
    rvf(symbol_table, goto_functions);

  rvf(goto_functions);
}

/*******************************************************************\

Function: remove_virtual_functions

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void remove_virtual_functions(goto_modelt &goto_model)
{
  remove_virtual_functions(
    goto_model.symbol_table, goto_model.goto_functions);
}

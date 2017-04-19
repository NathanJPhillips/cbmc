/*******************************************************************\

Module: Class Hierarchy

Author: Daniel Kroening

Date: April 2016

\*******************************************************************/

#include <ostream>

#include <util/std_types.h>
#include <util/symbol_table.h>

#include "class_hierarchy.h"

/*******************************************************************\

Function: class_hierarchyt::operator()

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void class_hierarchyt::operator()(const symbol_tablet &symbol_table)
{
  forall_symbols(it, symbol_table.symbols)
  {
    if(it->second.is_type && it->second.type.id()==ID_struct)
    {
      const struct_typet &struct_type=
        to_struct_type(it->second.type);

      const irept::subt &bases=
        struct_type.find(ID_bases).get_sub();

      for(const auto &base : bases)
      {
        irep_idt parent=base.find(ID_type).get(ID_identifier);
        if(parent.empty())
          continue;

        class_map[parent].children.push_back(it->first);
        class_map[it->first].parents.push_back(parent);
      }
    }
  }
}

/*******************************************************************\

Function: class_hierarchyt::get_children_trans_rec

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void class_hierarchyt::get_children_trans_rec(
  const irep_idt &c,
  idst &dest) const
{
  class_mapt::const_iterator it=class_map.find(c);
  if(it==class_map.end())
    return;
  const entryt &entry=it->second;

  for(const auto &child : entry.children)
    dest.push_back(child);

  // recursive calls
  for(const auto &child : entry.children)
    get_children_trans_rec(child, dest);
}

/*******************************************************************\

Function: class_hierarchyt::get_parents_trans_rec

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void class_hierarchyt::get_parents_trans_rec(
  const irep_idt &c,
  idst &dest) const
{
  class_mapt::const_iterator it=class_map.find(c);
  if(it==class_map.end())
    return;
  const entryt &entry=it->second;

  for(const auto &child : entry.parents)
    dest.push_back(child);

  // recursive calls
  for(const auto &child : entry.parents)
    get_parents_trans_rec(child, dest);
}

/*******************************************************************\

Function: class_hierarchyt::get_child_functions_rec

  Inputs: `this_id`: class name
          `last_method_defn`: the most-derived parent of `this_id`
             to define the requested function
          `component_name`: name of the function searched for

 Outputs: `functions` is assigned a list of {class name, function symbol}
          pairs indicating that if `this` is of the given class, then the
          call will target the given function. Thus if A <: B <: C and A
          and C provide overrides of `f` (but B does not),
          get_child_functions_rec("C", C.f, "f") -> [{"C", C.f},
                                                     {"B", C.f},
                                                     {"A", A.f}]

 Purpose: Used by get_virtual_callsite_targets to track the most-derived parent
          that provides an override of a given function.

\*******************************************************************/

void class_hierarchyt::get_child_functions_rec(
  const irep_idt &this_id,
  const symbol_exprt &last_method_defn,
  const irep_idt &component_name,
  const namespacet &ns,
  functionst &functions,
  std::set<irep_idt> &visited) const
{
  auto findit=class_map.find(this_id);
  if(findit==class_map.end())
    return;

  for(const auto &child : findit->second.children)
  {
    if(!visited.insert(child).second)
      continue;
    exprt method=get_method(child, component_name, ns);
    functiont function(child);
    if(method.is_not_nil())
    {
      function.symbol_expr=to_symbol_expr(method);
      function.symbol_expr.set(ID_C_class, child);
    }
    else
    {
      function.symbol_expr=last_method_defn;
    }
    functions.push_back(function);

    get_child_functions_rec(
      child,
      function.symbol_expr,
      component_name,
      ns,
      functions,
      visited);
  }
}

/*******************************************************************\

Function: class_hierarchyt::get_virtual_callsite_targets

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void class_hierarchyt::get_virtual_callsite_targets(
  const exprt &function,
  const namespacet &ns,
  functionst &functions) const
{
  assert(function.id()==ID_virtual_function);
  const irep_idt class_id=function.get(ID_C_class);
  const irep_idt component_name=function.get(ID_component_name);
  assert(!class_id.empty());

  functiont root_function=get_virtual_call_definition(
    class_id,
    component_name,
    ns);

  // iterate over all children, transitively
  std::set<irep_idt> visited;
  get_child_functions_rec(
    class_id,
    root_function.symbol_expr,
    component_name,
    ns,
    functions,
    visited);

  if(root_function.symbol_expr!=symbol_exprt())
    functions.push_back(root_function);
}

/*******************************************************************\

Function: class_hierarchyt::get_virtual_call_definition

  Inputs:
    class_id: The class of the reference on which the function is called.
    component_name: The component name of the function called.
    ns: The current namespace used to find definitions.

 Outputs:
    A functiont for the function definiton found

 Purpose:
    For a call to a function using a reference to a class of object, find the
    definition on a parent class that is being used.
    Used by get_virtual_callsite_targets.

\*******************************************************************/

class_hierarchyt::functiont class_hierarchyt::get_virtual_call_definition(
  const irep_idt &class_id,
  const irep_idt &component_name,
  const namespacet &ns) const
{
  // Start from current class, go to parents until something
  // is found.
  irep_idt c=class_id;
  while(!c.empty())
  {
    // Look for method on class c
    exprt method=get_method(c, component_name, ns);
    if(method.is_not_nil())
    {
      // Found a definition
      functiont root_function;
      root_function.class_id=c;
      root_function.symbol_expr=to_symbol_expr(method);
      root_function.symbol_expr.set(ID_C_class, c);
      return root_function;
    }

    const idst &parents=class_map.at(c).parents;
    if(parents.empty())
      // No parents - definition not found
      break;
    c=parents.front();
  }

  // No definition here; this is an abstract function.
  functiont function;
  function.class_id=class_id;
  return function;
}

/*******************************************************************\

Function: class_hierarchyt::get_method

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

exprt class_hierarchyt::get_method(
  const irep_idt &class_id,
  const irep_idt &component_name,
  const namespacet &ns) const
{
  irep_idt id=id2string(class_id)+"."+
              id2string(component_name);

  const symbolt *symbol;
  if(ns.lookup(id, symbol))
    return nil_exprt();

  return symbol->symbol_expr();
}

/*******************************************************************\

Function: class_hierarchyt::output

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

void class_hierarchyt::output(std::ostream &out) const
{
  for(const auto &c : class_map)
  {
    for(const auto &pa : c.second.parents)
      out << "Parent of " << c.first << ": "
          << pa << '\n';

    for(const auto &ch : c.second.children)
      out << "Child of " << c.first << ": "
          << ch << '\n';
  }
}

/*******************************************************************\

Function: output_dot

  Inputs:

 Outputs:

 Purpose:

\*******************************************************************/

std::ostream &output_dot(std::ostream &ostr, const class_hierarchyt &hierarchy)
{
  ostr << "digraph call_graph {\n"
       << "  rankdir=BT;\n"
       << "  node [fontsize=12 shape=box];\n";
  for(const auto &c : hierarchy.class_map)
    for(const auto &ch : c.second.parents)
      ostr << "  \"" << c.first << "\" -> "
           << "\"" << ch << "\" "
           << " [arrowhead=\"vee\"];"
           << "\n";
  ostr << "}\n";
  return ostr;
}

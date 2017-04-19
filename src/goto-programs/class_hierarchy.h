/*******************************************************************\

Module: Class Hierarchy

Author: Daniel Kroening

Date: April 2016

\*******************************************************************/

#ifndef CPROVER_GOTO_PROGRAMS_CLASS_HIERARCHY_H
#define CPROVER_GOTO_PROGRAMS_CLASS_HIERARCHY_H

#include <iosfwd>
#include <set>
#include <map>

#include <util/namespace.h>
#include <util/std_expr.h>


class class_hierarchyt
{
public:
  typedef std::vector<irep_idt> idst;

  class entryt
  {
  public:
    idst parents, children;
  };

  typedef std::map<irep_idt, entryt> class_mapt;
  class_mapt class_map;

  void operator()(const symbol_tablet &);

  // transitively gets all children
  idst get_children_trans(const irep_idt &id) const
  {
    idst result;
    get_children_trans_rec(id, result);
    return result;
  }

  // transitively gets all parents
  idst get_parents_trans(const irep_idt &id) const
  {
    idst result;
    get_parents_trans_rec(id, result);
    return result;
  }

  class functiont
  {
  public:
    functiont() {}
    explicit functiont(const irep_idt &_class_id) :
      class_id(_class_id)
    {}

    symbol_exprt symbol_expr;
    irep_idt class_id;
  };

  typedef std::vector<functiont> functionst;

  void get_virtual_callsite_targets(
    const exprt &,
    const namespacet &,
    functionst &) const;

  functiont get_virtual_call_definition(
    const irep_idt &real_classid,
    const irep_idt &function_name,
    const namespacet &ns) const;

  void output(std::ostream &) const;

protected:
  void get_children_trans_rec(const irep_idt &, idst &) const;
  void get_parents_trans_rec(const irep_idt &, idst &) const;
  void get_child_functions_rec(
    const irep_idt &,
    const symbol_exprt &,
    const irep_idt &,
    const namespacet &,
    functionst &,
    std::set<irep_idt> &visited) const;
  exprt get_method(
    const irep_idt &class_id,
    const irep_idt &component_name,
    const namespacet &ns) const;
};

std::ostream &output_dot(std::ostream &ostr, const class_hierarchyt &hierarchy);

#endif // CPROVER_GOTO_PROGRAMS_CLASS_HIERARCHY_H

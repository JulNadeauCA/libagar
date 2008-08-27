#include <agar/core.h>
#include <agar/gui.h>

double
agar_unit2base (double n, const AG_Unit *unit)
{
  return AG_Unit2Base (n, unit);
}

double
agar_base2unit (double n, const AG_Unit *unit)
{
  return AG_Base2Unit (n, unit);
}

double
agar_unit2unit (double n, const AG_Unit *ufrom, const AG_Unit *uto)
{
  return AG_Unit2Unit (n, ufrom, uto);
}

const char *
agar_unitabbr (const AG_Unit *unit)
{
  return AG_UnitAbbr (unit);
}

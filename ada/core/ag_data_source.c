#include <agar/core.h>

void ag_write_uint8(AG_DataSource *ds, Uint8 v) { return AG_WriteUint8(ds, v); }
void ag_write_sint8(AG_DataSource *ds, Sint8 v) { return AG_WriteSint8(ds, v); }
void ag_write_uint16(AG_DataSource *ds, Uint16 v) { return AG_WriteUint16(ds, v); }
void ag_write_sint16(AG_DataSource *ds, Sint16 v) { return AG_WriteSint16(ds, v); }
void ag_write_uint32(AG_DataSource *ds, Uint32 v) { return AG_WriteUint32(ds, v); }
void ag_write_sint32(AG_DataSource *ds, Sint32 v) { return AG_WriteSint32(ds, v); }
void ag_write_uint64(AG_DataSource *ds, Uint64 v) { return AG_WriteUint64(ds, v); }
void ag_write_sint64(AG_DataSource *ds, Sint64 v) { return AG_WriteSint64(ds, v); }
void ag_write_uint8_at(AG_DataSource *ds, Uint8 v, off_t pos) { return AG_WriteUint8At(ds, v, pos); }
void ag_write_sint8_at(AG_DataSource *ds, Sint8 v, off_t pos) { return AG_WriteSint8At(ds, v, pos); }
void ag_write_uint16_at(AG_DataSource *ds, Uint16 v, off_t pos) { return AG_WriteUint16At(ds, v, pos); }
void ag_write_sint16_at(AG_DataSource *ds, Sint16 v, off_t pos) { return AG_WriteSint16At(ds, v, pos); }
void ag_write_uint32_at(AG_DataSource *ds, Uint32 v, off_t pos) { return AG_WriteUint32At(ds, v, pos); }
void ag_write_sint32_at(AG_DataSource *ds, Sint32 v, off_t pos) { return AG_WriteSint32At(ds, v, pos); }
void ag_write_uint64_at(AG_DataSource *ds, Uint64 v, off_t pos) { return AG_WriteUint64At(ds, v, pos); }
void ag_write_sint64_at(AG_DataSource *ds, Sint64 v, off_t pos) { return AG_WriteSint64At(ds, v, pos); }

Uint8  ag_read_uint8(AG_DataSource *ds)  { return AG_ReadUint8(ds); }
Sint8  ag_read_sint8(AG_DataSource *ds)  { return AG_ReadSint8(ds); }
Uint16 ag_read_uint16(AG_DataSource *ds) { return AG_ReadUint16(ds); }
Sint16 ag_read_sint16(AG_DataSource *ds) { return AG_ReadSint16(ds); }
Uint32 ag_read_uint32(AG_DataSource *ds) { return AG_ReadUint32(ds); }
Sint32 ag_read_sint32(AG_DataSource *ds) { return AG_ReadSint32(ds); }
Uint64 ag_read_uint64(AG_DataSource *ds) { return AG_ReadUint64(ds); }
Sint64 ag_read_sint64(AG_DataSource *ds) { return AG_ReadSint64(ds); }

void ag_write_float(AG_DataSource *ds, float v) { AG_WriteFloat(ds, v); }
void ag_write_double(AG_DataSource *ds, double v) { AG_WriteDouble(ds, v); }
void ag_write_long_double(AG_DataSource *ds, long double v) { AG_WriteLongDouble(ds, v); }
void ag_write_float_at(AG_DataSource *ds, float v, off_t pos) { AG_WriteFloatAt(ds, v, pos); }
void ag_write_double_at(AG_DataSource *ds, double v, off_t pos) { AG_WriteDoubleAt(ds, v, pos); }
void ag_write_long_double_at(AG_DataSource *ds, double v, off_t pos) { AG_WriteLongDoubleAt(ds, v, pos); }

float ag_read_float(AG_DataSource *ds) { return AG_ReadFloat(ds); }
double ag_read_double(AG_DataSource *ds) { return AG_ReadDouble(ds); }
long double ag_read_long_double(AG_DataSource *ds) { return AG_ReadLongDouble(ds); }


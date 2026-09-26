// Copyright (c) rAthena Dev Teams - Licensed under GNU GPL
// For more information, see LICENCE in the main folder

#include "common/showmsg.hpp"
#include "emu_time.hpp"

void ShowEmuInfo(const char *string, ...) {
	va_list ap;
	va_start(ap, string);
	_vShowMessage(MSG_EMU_INFO, string, ap);
	va_end(ap);
}

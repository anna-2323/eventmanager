#pragma once
#include "qrencode.h"
#include <stddef.h>
#include <stdio.h>

int generate_ticket_qr(const char* token, char* path, size_t path_size);
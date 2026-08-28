#pragma once

typedef struct {
	int id;
	char name[100];
	int capacity;
	double price;
	char color[8];
	char svg_path[256];
	int available;
} Sector;

typedef struct {
	char period[11];  // "YYYY-MM-DD"
	int count;
} StatGrowth;

#pragma once

#include "Material.h"

class MTLReader {
public:
	vector <Material*> readFile(string pathfile);
};
